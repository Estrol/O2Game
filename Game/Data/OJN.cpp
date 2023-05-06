#include "OJN.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Util/Util.hpp"
#include <assert.h>

using namespace O2;

//std::stringstream LoadOJNFile(std::string path);

OJN::OJN() {
	Header = {};
}

OJN::~OJN() {
	
}

void OJN::Load(std::filesystem::path& file) {
	char signature[] = {'o', 'j', 'n', '\0'};

	CurrrentDir = file.parent_path().string();

	std::fstream fs(file, std::ios::binary | std::ios::in);
	if (!fs.is_open()) {
		::printf("Failed to open: %s\n", file.string().c_str());
		return;
	}

	fs.read((char*)&Header, sizeof(Header));

	if (memcmp(Header.signature, signature, 4) != 0) {
		::printf("Invalid OJN file: %s\n", file.string().c_str());
		::printf("Dumping 1-3 byte: %c%c%c\n", Header.signature[0], Header.signature[1], Header.signature[2]);
		return;
	}

	std::map<int, std::vector<Package>> difficulty;
	for (int i = 0; i < 3; i++) {
		int startOffset = Header.data_offset[i];
		int endOffset = Header.data_offset[i + 1];

		int size = endOffset - startOffset;
		fs.seekg(startOffset, std::ios::beg);

		difficulty[i] = {};

		// loop until endOffset
		while (fs.tellg() < endOffset) {
			Package pkg = {};
			fs.read((char*)&pkg.Measure, 4);
			fs.read((char*)&pkg.Channel, 2);
			fs.read((char*)&pkg.EventCount, 2);

			if (pkg.EventCount == 0) {
				__debugbreak();
			}

			for (int i = 0; i < pkg.EventCount; i++) {
				Event ev = {};
				fs.read((char*)&ev, sizeof(ev));

				pkg.Events.push_back(ev);
			}

			if (pkg.EventCount > 0) {
				difficulty[i].push_back(pkg);
			}
		}

		// sort by measure
		std::sort(difficulty[i].begin(), difficulty[i].end(), [](const Package& x, const Package& y) {
			return x.Measure < y.Measure;
		});
	}

	fs.seekg(Header.data_offset[3], std::ios::beg);
	if (Header.cover_size > 0) {
		BackgroundImage.resize(Header.cover_size);
		fs.read((char*)BackgroundImage.data(), Header.cover_size);
	}

	if (Header.bmp_size > 0) {
		ThumbnailImage.resize(Header.bmp_size);
		fs.read((char*)ThumbnailImage.data(), Header.bmp_size);
	}

	fs.close();

	ParseNoteData(this, difficulty);

	m_valid = true;
}

bool OJN::IsValid() {
	return m_valid;
}

void OJN::ParseNoteData(OJN* ojn, std::map<int, std::vector<Package>>& pkg) {
	std::map<int, std::vector<NoteEvent>> events;
	for (int i = 0; i < 3; i++) {
		auto& packages = pkg[i];

		for (auto& package : packages) {
			for (int f = 0; f < package.EventCount; f++) {
				auto& event = package.Events[f];
				double position = static_cast<float>(f) / static_cast<float>(package.EventCount);

				if (package.Channel == 1 || package.Channel == 0) {
					if (event.BPM == 0) {
						continue;
					}

					NoteEvent ev = {};
					ev.Measure = package.Measure;
					ev.Channel = package.Channel;
					ev.Position = position;
					ev.Value = event.BPM;

					events[i].push_back(ev);
				}
				else {
					if (event.Value == 0) {
						continue;
					}

					NoteEvent ev = {};
					ev.Measure = package.Measure;
					ev.Channel = package.Channel;
					ev.Position = position;
					ev.Value = event.Value - 1;

					if (event.Type % 8 > 3) {
						ev.Value += 1000;
					}

					// do we need parse the VolPan here?
					// nowdays OJN/OJM do not use VolPan like BMS

					int type = event.Type % 4;
					switch (type) {
						case 2: {
							ev.Type = NoteEventType::HoldStart;
							break;
						}

						case 3: {
							ev.Type = NoteEventType::HoldEnd;
							break;
						}

						default: {
							ev.Type = NoteEventType::Note;
							break;
						}
					}

					events[i].push_back(ev);
				}
			}
		}
	}

	OJM ojm = {};
	auto path = CurrrentDir / Header.ojm_file;
	ojm.Load(path);

	if (!ojm.IsValid()) {
		std::cout << "[OJM] Failed to load: " << path.string() << std::endl;
	}

	// default: 240 BPM
	const int BEATS_PER_MSEC = 4 * 60 * 1000;

	for (int i = 0; i < 3; i++) {
		std::vector<O2Note> notes;
		std::vector<O2Note> autoSamples;
		std::vector<O2Timing> bpmChanges;
		std::vector<double> measureList;

		bpmChanges.push_back({ Header.bpm, 0 });
		measureList.push_back(0);

		double currentBPM = Header.bpm;
		double measureFraction = 1;
		double measurePosition = 0;
		double timer = 0;
		
		double holdNotes[7] = {};
		int currentMeasure = 0;

		// sort based on measure + position
		std::vector<NoteEvent> sortedEvents = events[i];
		std::sort(sortedEvents.begin(), sortedEvents.end(), [=](NoteEvent& ev1, NoteEvent& ev2) {
			return (ev1.Measure + ev1.Position) < (ev2.Measure + ev2.Position);
		});

		for (auto& event : sortedEvents) {
			while (event.Measure > currentMeasure) {
				timer += (BEATS_PER_MSEC * (measureFraction - measurePosition)) / currentBPM;
				measureList.push_back(timer);

				currentMeasure++;
				measurePosition = 0;
			}

			timer += (BEATS_PER_MSEC * (event.Position - measurePosition)) / currentBPM;
			measurePosition = event.Position;

			if (event.Channel == 1) {
				bpmChanges.push_back({ event.Value, timer });
				currentBPM = event.Value;
			}
			else if (event.Channel < 9) {
				int laneIndex = event.Channel - 2;

				switch (event.Type) {
					case NoteEventType::HoldStart: {
						holdNotes[laneIndex] = timer;
						break;
					}

					case NoteEventType::HoldEnd: {
						O2Note note = {};
						note.StartTime = holdNotes[laneIndex];
						note.EndTime = timer;
						note.IsLN = true;
						note.SampleRefId = static_cast<int>(event.Value);
						note.LaneIndex = laneIndex;

						notes.push_back(note);
						break;
					}

					default: {
						O2Note note = {};
						note.StartTime = timer;
						note.IsLN = false;
						note.SampleRefId = static_cast<int>(event.Value);
						note.LaneIndex = laneIndex;

						notes.push_back(note);
						break;
					}
				}
			}
			else {
				O2Note sample = {};
				sample.StartTime = timer;
				sample.LaneIndex = -1;
				sample.SampleRefId = static_cast<int>(event.Value);

				autoSamples.push_back(sample);
			}
		}
		
		OJNDifficulty diff = {};
		diff.AutoSamples = autoSamples;
		diff.Notes = notes;
		diff.Timings = bpmChanges;
		diff.Measures = measureList;
		diff.Samples = ojm.Samples;
		diff.AudioLength = timer + 500;

		Difficulties[i] = std::move(diff);
	}
}

//std::stringstream LoadOJNFile(std::string path) {
//	std::fstream fs(path, std::ios::in | std::ios::binary);
//
//	fs.seekg(0, std::ios::end);
//	size_t sz = fs.tellg();
//	fs.seekg(0, std::ios::beg);
//
//	char* input = new char[sz];
//	fs.read(input, sz);
//
//	char newSign[3] = { 'n', 'e', 'w' };
//	char checkSign[3];
//
//	fs.seekg(0, std::ios::beg);
//	fs.read(checkSign, 3);
//
//	if (memcmp(newSign, checkSign, 3) == 0) {
//		fs.seekg(3, std::ios::beg);
//		uint8_t blockSz = 0, mainKey = 0, midKey = 0, initialKey = 0;
//		fs.read((char*)&blockSz, 1);
//		fs.read((char*)&mainKey, 1);
//		fs.read((char*)&midKey, 1);
//		fs.read((char*)&initialKey, 1);
//
//		uint8_t* key = new uint8_t[blockSz];
//		memset(key, mainKey, blockSz);
//		key[0] = initialKey;
//		key[(int)std::floor(blockSz / 2.0f)] = midKey;
//
//		size_t outputLen = sz - fs.tellg();
//		char* output = new char[outputLen];
//		for (int i = 0; i < outputLen; i += blockSz) {
//			for (int j = 0; j < blockSz; j++) {
//				int offset = i + j;
//				if (offset >= outputLen) {
//					// TODO: return
//				}
//
//				output[offset] = (char)(input[sz - (offset + 1)] ^ key[j]);
//			}
//		}
//	}
//	else {
//		
//	}
//}
