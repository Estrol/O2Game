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

Calculation::Timing::Timing(
	std::vector<BMS::BMSEvent>& currentTrackEvent, 
	std::unordered_map<std::string, double> vbpms, 
	std::unordered_map<std::string, double> vstops, 
	bool IsOJN) {

	bpms = {};
	stops = {};

	for (auto& event : currentTrackEvent) {
		if (event.Params.size() == 0) continue;

		switch (event.Channel) {
			case 2: { // Measure change
				try {
					auto msg = mergeVector(event.Params, 0);
					double value = std::atof(msg.c_str());
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

					int value;
					try {
						value = std::stoi(msg, nullptr, 16);
					}
					catch (std::invalid_argument&) {
						::printf("[BMS] [ERROR] Failed to parse BPM, undefined behavior may occured!");
						continue;
					}

					BPMInfo info = {};
					info.Value = value;
					info.Measure = event.Measure;
					info.Offset = 1.0 * static_cast<float>(i) / static_cast<float>(event.Params.size());

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
						info.Offset = 1.0 * static_cast<float>(i) / static_cast<float>(event.Params.size());

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
						info.Offset = 1.0 * static_cast<float>(i) / static_cast<float>(event.Params.size());

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
	if (!bpmChanges.size()) {
		return duration + GetStopTimeFromOffset(bpm, offset, inc);
	}

	double curBPM = bpm;
	auto beforePos = trackDuration * bpmChanges[0].Offset;

	auto it = bpmChanges.begin();
	while (it != bpmChanges.end()) {
		curBPM = it->Value;

		if (it + 1 == bpmChanges.end()) {
			beforePos += GetTrackDuration(curBPM, currentMeasure) * (offset - it->Offset);
			break;
		}

		auto next = it + 1;
		beforePos += GetTrackDuration(curBPM, currentMeasure) * (next->Offset - it->Offset);
		it++;
	}

	return beforePos + GetStopTimeFromOffset(bpm, offset, inc);
}

double Calculation::Timing::GetStopTimeFromOffset(double bpm, double offset, bool inc) {
	if (stops.size() == 0) return 0;

	double duration = 0;
	auto it = stops.begin();

	while (it != stops.end() && (inc ? it->Offset <= offset : it->Offset < offset)) {
		double currentBPM = bpm;

		auto bpmIt = bpms.begin();
		while (bpmIt != bpms.end() && bpmIt->Offset <= it->Offset) {
			currentBPM = bpmIt->Value;
			bpmIt++;
		}

		duration += GetStopDuration(currentBPM, it->Value);
		it++;
	}

	return duration;
}


std::vector<Calculation::Info> Calculation::Timing::GetTimings(double timePos, double BPM) {
	std::vector<Info> timings = {};

	if (bpms.size() == 0 && stops.size() == 0) {
		return timings;
	}

	for (auto& bpm : bpms) {
		double time = GetStartTimeFromOffset(BPM, bpm.Offset, false);

		Info t = {};
		t.StartTime = timePos + time;
		t.Value = bpm.Value;
		t.CurrentMeasure = currentMeasure;

		timings.push_back(t);
	}

	for (auto& stop : stops) {
		double time = GetStartTimeFromOffset(BPM, stop.Offset, false);
		double stopTime = GetStopTimeFromOffset(BPM, stop.Offset);

		double tBPM = BPM;
		for (auto& bpm : bpms) {
			if (bpm.Offset <= stop.Offset) {
				tBPM = bpm.Value;
			}
			else {
				break;
			}
		}

		Info t = {};
		t.StartTime = timePos + time;
		t.Value = 0;
		t.CurrentMeasure = currentMeasure;

		timings.push_back(t);

		Info t2 = {};
		t2.StartTime = timePos + time + stopTime;
		t2.Value = tBPM;
		t2.CurrentMeasure = currentMeasure;

		timings.push_back(t2);
	}

	return timings;
}
