#include "OJN.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "Util/Util.hpp"
#include <assert.h>
#include <string.h>
#include <cmath>

using namespace O2;

OJN::OJN() {
	Header = {};
}

OJN::~OJN() {
	if (IsValid()) {
		for (int i = 0; i < 3; i++) {
			auto& diff = Difficulties[i];
			diff.Samples.clear();
		}
	}
}

void OJN::Load(std::filesystem::path& file) {
	char signature[] = {'o', 'j', 'n', '\0'};

	CurrrentDir = file.parent_path().string();

	auto fs = LoadOJNFile(file);

	fs.read((char*)&Header, sizeof(Header));

	if (memcmp(Header.signature, signature, 4) != 0) {
		::printf("Invalid OJN file: %s\n", file.string().c_str());
		::printf("Dumping 1-3 byte: %c%c%c\n", Header.signature[0], Header.signature[1], Header.signature[2]);
		
		throw std::runtime_error("Invalid OJN Header at file: " + file.string());
	}
	
	KeyCount = 7;
	if (Header.encode_version == 5.0) {
		fs.read((char*)&KeyCount, sizeof(int));
	}

	std::map<int, std::vector<Package>> difficulty;
	for (int i = 0; i < 3; i++) {
		int startOffset = Header.data_offset[i];
		int endOffset = Header.data_offset[i + 1];

		int size = endOffset - startOffset;
		fs.seekg(startOffset, std::ios::beg);

		difficulty[i] = {};

		if (size > 0) {
			for (int j = 0; j < Header.package_count[i]; j++) {
				if (fs.tellg() > endOffset) {
					throw std::runtime_error("Block data size overflow! at file: " + file.string());
				}

				Package pkg = {};
				fs.read((char*)&pkg.Measure, 4);
				fs.read((char*)&pkg.Channel, 2);
				fs.read((char*)&pkg.EventCount, 2);

				if (pkg.EventCount > 192) {
					throw std::runtime_error("Event count at measure: " + std::to_string(pkg.Measure) + " exceed the limit! (limit: 192)");
				}

				for (int i = 0; i < pkg.EventCount; i++) {
					Event ev = {};
					if (pkg.Channel == 0 || pkg.Channel == 1) {
						fs.read((char*)&ev.BPM, sizeof(float));
					}
					else {
						fs.read((char*)&ev.Value, sizeof(short));
						fs.read((char*)&ev.VolPan, sizeof(char));
						fs.read((char*)&ev.Type, sizeof(char));
					}

					pkg.Events.push_back(ev);
				}

				if (pkg.EventCount > 0) {
					difficulty[i].push_back(pkg);
				}
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
					ev.CellSize = package.EventCount;

					events[i].push_back(ev);
				}
				else {
					if (event.Value == 0) {
						continue;
					}

					NoteEvent ev = {};
					ev.Measure = package.Measure;
					ev.Channel = package.Channel;
					ev.CellSize = package.EventCount;
					ev.Position = position;
					ev.Value = (float)event.Value - 1.0f;

					if (event.Type % 8 > 3 || event.Type == 4) {
						ev.Value += 1000.0f;
					}

					// nvm, we need parse it :troll:

					float volume = ((event.VolPan >> 4) & 0x0F) / 16.0f;
					if (volume == 0.0f) {
						volume = 1.0f;
					}

					float pan = (float)(event.VolPan & 0x0F);
					if (pan == 0.0f) {
						pan = 8.0f;	
					}

					pan -= 8.0f;
					pan /= 8.0f;

					ev.Volume = volume;
					ev.Pan = pan;

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
	const int START_TIME = 1500;

	for (int i = 0; i < 3; i++) {
		std::vector<O2Note> notes;
		std::vector<O2Note> autoSamples;
		std::vector<O2Timing> bpmChanges;
		std::vector<O2Timing> measureLengthChanges;
		std::vector<double> measureList;

		double currentBPM = Header.bpm;
		double measureFraction = 1;
		double measurePosition = 0;
		double timer = START_TIME;

		bpmChanges.push_back({ Header.bpm, timer });
		measureList.push_back(0);
		
		double holdNotes[7] = {};
		float holdNotesPos[7] = { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 };

		int currentMeasure = 0;

		// sort based on measure + position
		std::vector<NoteEvent> sortedEvents = events[i];
		std::sort(sortedEvents.begin(), sortedEvents.end(), [](NoteEvent& ev1, NoteEvent& ev2) {
			return (ev1.Measure + ev1.Position) < (ev2.Measure + ev2.Position);
		});

		for (auto& event : sortedEvents) {
			while (event.Measure > currentMeasure) {
				timer += (BEATS_PER_MSEC * (measureFraction - measurePosition)) / currentBPM;
				measureList.push_back(timer);

				currentMeasure++;
				measurePosition = 0;
				measureFraction = 1;
			}

			double position = event.Position * measureFraction;
			timer += (BEATS_PER_MSEC * (position - measurePosition)) / currentBPM;
			measurePosition = position;

			if (event.Channel == 0) {
				measureLengthChanges.push_back({ event.Value, timer, (float)(event.Measure + event.Position) });
				measureFraction = event.Value;
			}
			else if (event.Channel == 1) {
				bpmChanges.push_back({ event.Value, timer, (float)(event.Measure + event.Position) });
				currentBPM = event.Value;
			}
			else if (event.Channel < 9) {
				int laneIndex = event.Channel - 2;

				switch (event.Type) {
					case NoteEventType::HoldStart: {
						holdNotes[laneIndex] = timer;
						holdNotesPos[laneIndex] = (float)(event.Measure + event.Position);
						break;
					}

					case NoteEventType::HoldEnd: {
						if (holdNotesPos[laneIndex] != -1) {
							O2Note note = {};
							note.StartTime = holdNotes[laneIndex];
							note.EndTime = timer;
							note.IsLN = true;
							note.SampleRefId = static_cast<int>(event.Value);
							note.LaneIndex = laneIndex;
							note.Volume = event.Volume;
							note.Pan = event.Pan;
							note.Channel = event.Channel;
							note.Position = holdNotesPos[laneIndex];
							note.EndPosition = (float)(event.Measure + event.Position);
							holdNotesPos[laneIndex] = -1;
							holdNotes[laneIndex] = -1;

							assert(note.Position != -1);
							assert(note.EndPosition != -1);

							notes.push_back(note);
						}
						break;
					}

					default: {
						O2Note note = {};
						note.StartTime = timer;
						note.IsLN = false;
						note.SampleRefId = static_cast<int>(event.Value);
						note.LaneIndex = laneIndex; 
						note.Volume = event.Volume;
						note.Pan = event.Pan;
						note.Channel = event.Channel;
						note.Position = (float)(event.Measure + event.Position);

						assert(note.Position != -1);

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
				sample.Volume = event.Volume;
				sample.Pan = event.Pan;
				sample.Channel = event.Channel;
				sample.Position = (float)(event.Measure + event.Position);

				assert(sample.Position != -1);

				autoSamples.push_back(sample);
			}
		}
		
		OJNDifficulty diff = {};
		diff.AutoSamples = autoSamples;
		diff.Notes = notes;
		diff.Timings = bpmChanges;
		diff.Measures = measureList;
		diff.Samples = ojm.Samples;
		diff.MeasureLenghts = measureLengthChanges;
		diff.AudioLength = timer + 500;
		diff.Valid = !notes.empty();

		Difficulties[i] = std::move(diff);
	}
}

std::stringstream OJN::LoadOJNFile(std::filesystem::path path) {
	std::fstream fs(path, std::ios::in | std::ios::binary);
	if (!fs.is_open()) {
		throw std::runtime_error("Failed to open: " + path.string());
	}

	fs.seekg(0, std::ios::end);
	size_t sz = fs.tellg();
	fs.seekg(0, std::ios::beg);

	char* input = new char[sz];
	fs.read(input, sz);

	char newSign[3] = { 'n', 'e', 'w' };
	char checkSign[3];

	fs.seekg(0, std::ios::beg);
	fs.read(checkSign, 3);

	if (memcmp(newSign, checkSign, 3) == 0) {
		fs.seekg(3, std::ios::beg);
		uint8_t blockSz = 0, mainKey = 0, midKey = 0, initialKey = 0;
		fs.read((char*)&blockSz, 1);
		fs.read((char*)&mainKey, 1);
		fs.read((char*)&midKey, 1);
		fs.read((char*)&initialKey, 1);

		uint8_t* key = new uint8_t[blockSz];
		memset(key, mainKey, blockSz);
		key[0] = initialKey;
		key[(int)std::floor(blockSz / 2.0f)] = midKey;

		size_t outputLen = sz - fs.tellg();
		char* output = new char[outputLen];
		for (int i = 0; i < outputLen; i += blockSz) {
			for (int j = 0; j < blockSz; j++) {
				int offset = i + j;
				if (offset >= outputLen) {
					goto DONE;
				}

				output[offset] = (char)(input[sz - (offset + 1)] ^ key[j]);
			}
		}

		DONE:
		fs.close();

		std::stringstream ss;
		ss.write(output, outputLen);

		delete[] input;
		delete[] output;

		return ss;
	}
	else {
		fs.close();

		std::stringstream ss;
		ss.write(input, sz);

		delete[] input;

		return ss;
	}
}
