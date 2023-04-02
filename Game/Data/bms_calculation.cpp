#include "bms_calculation.hpp"
#include "bms_struct.hpp"
#include "Util/Util.hpp"
#include <algorithm>

double Calculation::GetTrackDuration(double bpm, double measure) {
	return ((60000.0 / bpm) * 4.0) * measure;
}

double Calculation::GetStopDuration(double bpm, double duration) {
	return ((60000.0 / bpm) * 4.0) * (duration / 192.0);
}

Calculation::Timing::Timing(std::vector<BMS::BMSEvent>& currentTrackEvent, std::unordered_map<std::string, double> vbpms, std::unordered_map<std::string, double> vstops) {
	bpms = {};
	stops = {};

	for (auto& event : currentTrackEvent) {
		if (event.Params.size() == 0) continue;

		switch (event.Channel) {
			case 2: { // Measure change
				try {
					double value = std::atoi(mergeVector(event.Params, 0).c_str());
					if (value > 0) {
						currentMeasure = value;
					}
				}
				catch (std::invalid_argument&) {
					::printf("[BMS] [ERROR] Failed to parse Measure, undefined behavior may occured!");
				}

				break;
			}

			case 3: {
				for (int i = 0; i < event.Params.size(); i++) {
					auto& msg = event.Params[i];
					if (msg == "00") continue;

					BPMInfo info = {};
					info.Value = std::atof(msg.c_str());
					info.Measure = event.Measure;
					info.Offset = 1.0 * static_cast<int>(i) / static_cast<int>(event.Params.size());

					bpms.push_back(info);
				}

				break;
			}

			case 8: {
				for (int i = 0; i < event.Params.size(); i++) {
					auto& msg = event.Params[i];
					if (msg == "00") continue;

					if (vbpms.find(msg) != vbpms.end()) {
						BPMInfo info = {};
						info.Value = vbpms[msg];
						info.Measure = event.Measure;
						info.Offset = 1.0 * static_cast<int>(i) / static_cast<int>(event.Params.size());

						bpms.push_back(info);
					}
				}

				break;
			}

			case 9: {
				for (int i = 0; i < event.Params.size(); i++) {
					auto& msg = event.Params[i];
					if (msg == "00") continue;

					if (vstops.find(msg) != vstops.end()) {
						STOPInfo info = {};
						info.Value = vstops[msg];
						info.Measure = event.Measure;
						info.Offset = 1.0 * static_cast<int>(i) / static_cast<int>(event.Params.size());

						stops.push_back(info);
					}
				}

				break;
			}
		}
	}

	std::sort(bpms.begin(), bpms.end(), [](const BPMInfo& a, const BPMInfo& b) { return a.Offset < b.Offset; });
	std::sort(stops.begin(), stops.end(), [](const STOPInfo& a, const STOPInfo& b) { return a.Offset < b.Offset; });
}

double Calculation::Timing::GetStartTimeFromOffset(double bpm, double offset, bool inc) {
	std::vector<BPMInfo> bpmChanges = {};
	for (auto& bpm : bpms) {
		if (inc ? bpm.Offset <= offset : bpm.Offset < offset) {
			bpmChanges.push_back(bpm);
		}
	}

	auto trackDuration = GetTrackDuration(bpm, currentMeasure);
	auto duration = trackDuration * offset;
	if (bpmChanges.size() == 0) {
		return duration + GetStopTimeFromOffset(bpm, offset, inc);
	}

	double curBPM = bpm;
	auto beforePos = trackDuration * bpmChanges[0].Offset;

	for (int i = 0; i < bpmChanges.size(); i++) {
		auto& bc = bpmChanges[i];

		curBPM = bc.Value;
		beforePos += GetTrackDuration(curBPM, currentMeasure) * (i + 1 < bpmChanges.size() ? bpmChanges[i + 1].Offset - bc.Offset : offset - bc.Offset);
	}

	auto db = beforePos + GetStopTimeFromOffset(bpm, offset, inc);
	return db;
}

double Calculation::Timing::GetStopTimeFromOffset(double bpm, double offset, bool inc) {
	if (stops.size() == 0) return 0;

	double duration = 0;

	for (auto& stop : stops) {
		if (inc ? stop.Offset <= offset : stop.Offset < offset) {
			double curBPM = bpm;

			for (auto& bpm : bpms) {
				if (bpm.Offset <= offset) {
					curBPM = bpm.Value;
				}
				else {
					break;
				}
			}

			duration += GetStopDuration(curBPM, stop.Value);
		}
		else {
			break;
		}
	}

	return duration;
}

std::vector<std::pair<double, double>> Calculation::Timing::GetTimings(double timePos, double BPM) {
	std::vector<std::pair<double, double>> timings = {};

	for (auto& bpm : bpms) {
		double time = GetStartTimeFromOffset(BPM, bpm.Offset, false);
		timings.push_back({ timePos + time, bpm.Value });
	}

	for (auto& stop : stops) {
		double stopTime = GetStartTimeFromOffset(BPM, stop.Offset, true);
		double time = GetStartTimeFromOffset(BPM, stop.Offset, false);

		double tBPM = BPM;
		for (auto& bpm : bpms) {
			if (bpm.Offset <= stop.Offset) {
				tBPM = bpm.Value;
			}
			else {
				break;
			}
		}

		timings.push_back({ timePos + time, 0 });
		timings.push_back({ timePos + time + stopTime, tBPM });
	}

	return timings;
}
