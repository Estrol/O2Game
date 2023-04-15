#include "OJN.h"
#include <fstream>
#include <filesystem>
#include "Util/Util.hpp"
#include "bms_calculation.hpp"
#include <assert.h>

struct O2TimingComparer {
	bool operator() (const O2Timing& x, const O2Timing& y) const {
		return x.MesStart < y.MesStart;
	}
};

using namespace O2;

void ParseNoteData(OJN* ojn, std::map<int, std::vector<Package>>& pkg);
std::stringstream LoadOJNFile(std::string path);

OJN::OJN() {
	Header = {};
}

OJN::~OJN() {
	
}

void OJN::Load(std::string& file) {
	char signature[] = {'o', 'j', 'n', '\0'};

	CurrrentDir = std::filesystem::path(file).parent_path().string();

	std::fstream fs(file, std::ios::binary | std::ios::in);
	fs.read((char*)&Header, sizeof(Header));

	if (memcmp(Header.signature, signature, 4) != 0) {
		::printf("Invalid OJN file: %s\n", file.c_str());
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

			for (int i = 0; i < pkg.EventCount; i++) {
				Event ev = {};
				fs.read((char*)&ev, sizeof(ev));

				pkg.Events.push_back(ev);
			}

			if (pkg.EventCount > 0) {
				difficulty[i].push_back(pkg);
			}
		}
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

void ParseNoteData(OJN* ojn, std::map<int, std::vector<Package>>& pkg) {
	struct RawDiff {
		std::vector<NoteEvent> Notes;
		std::vector<BPMChange> BPMs;
	};

	std::map<int, RawDiff> diffs;

	for (auto& [diff, packages] : pkg) {
		NoteEvent* hold[7] = {};
		std::vector<BPMChange> bpmChanges = {};
		std::vector<NoteEvent> notes = {};

		BPMChange initialBPM = {};
		initialBPM.Measure = 0;
		initialBPM.BPM = ojn->Header.bpm;
		bpmChanges.push_back(initialBPM);

		for (auto& package : packages) {
			for (int i = 0; i < package.EventCount; i++) {
				auto& ev = package.Events[i];

				if (package.Channel == 1 || package.Channel == 0) {
					if (ev.BPM == 0) continue;

					BPMChange t = {};
					t.BPM = ev.BPM;
					t.Measure = package.Measure + (static_cast<float>(i) / package.EventCount);

					bpmChanges.push_back(t);
				}
				else {
					if (ev.Value == 0) continue;

					NoteEvent note = {};
					note.Value = ev.Value - 1;
					note.Measure = package.Measure + (static_cast<float>(i) / package.EventCount);

					note.Vol = ((ev.VolPan >> 4) & 0x0F) / 16.0f;
					note.Pan = (ev.VolPan & 0x0F);

					if (note.Pan == 0) {
						note.Pan = 8;
					}

					note.Pan = (note.Pan - 8) / 8;
					note.Type = ev.Type;

					if (ev.Type % 8 > 3) {
						note.Value += 1000;
					}

					note.Type %= 4;
					note.Channel = package.Channel;

					if (note.Type == 2 && package.Channel < 9 && hold[package.Channel - 2] == nullptr) {
						notes.push_back(note);
						hold[package.Channel - 2] = &notes.back();
						continue;
					}

					else if (note.Type == 3 && package.Channel < 9 && hold[package.Channel - 2] != nullptr) {
						auto prev = hold[package.Channel - 2];
						prev->Value = note.Value;
						prev->Pan = note.Pan;
						prev->Vol = note.Vol;
						prev->MeasureEnd = note.Measure;
						prev->Type = 3;

						hold[package.Channel - 2] = nullptr;
						continue;
					}

					else if (note.Type == 0) {
						note.MeasureEnd = -1;
					}

					notes.push_back(note);
				}
			}
		}

		diffs[diff] = { notes, bpmChanges };
	}

	OJM ojm = {};
	std::string path = (std::filesystem::path(ojn->CurrrentDir) / ojn->Header.ojm_file).string();
	ojm.Load(path);

	std::map<int, std::vector<O2Timing>> perDifftiming;

	// Generate timings
	for (auto& [diff, data] : diffs) {
		BPMChange* prev = nullptr;
		double msTime = 0;

		std::vector<O2Timing> timings = {};

		for (auto& t : data.BPMs) {
			O2Timing timing = {};

			if (!prev) {
				prev = &t;

				timing.MesStart = t.Measure;
				timing.MsMarking = 0;
				timing.MsPerMark = 240.0 / t.BPM * 1000.0;
				timings.push_back(timing);
			}
			else {
				double msPerMe = 240.0 / prev->BPM * 1000.0;
				double dt = t.Measure - prev->Measure;

				msTime += dt * msPerMe;
				prev = &t;

				float prevMS = timings.back().MsMarking;
				if (msTime - prevMS < 1) {
					timings.back().MsMarking--;
				}

				timing.MesStart = t.Measure;
				timing.MsMarking = msTime;
				timing.MsPerMark = 240.0 / t.BPM * 1000.0;
				timings.push_back(timing);
			}
		}

		perDifftiming[diff] = timings;
	}

	// Generate Notes
	for (auto& [diff, data] : diffs) {
		auto& timings = perDifftiming[diff];
		OJNDifficulty difficulty = {};

		for (auto& note : data.Notes) {
			O2Timing* prev = &timings[0];
			O2Timing* next = nullptr;

			O2Timing toFind = {};
			toFind.MesStart = note.Measure;

			auto it = std::lower_bound(timings.begin(), timings.end(), toFind, O2TimingComparer());
			if (it == timings.begin()) {
				prev = &(*it);
			}
			else {
				prev = &(*(it - 1));
			}

			if (note.Type == 3) {
				toFind.MesStart = note.MeasureEnd;

				it = std::lower_bound(timings.begin(), timings.end(), toFind, O2TimingComparer());
				if (it == timings.begin()) {
					next = &(*it);
				}
				else {
					next = &(*(it - 1));
				}
			}

			O2Note n = {};
			n.LaneIndex = note.Channel;

			double sdt = note.Measure - prev->MesStart;
			double startOffset = prev->MsPerMark * sdt;
			double hitStart = prev->MsMarking + startOffset;

			n.IsLN = false;
			n.StartTime = hitStart;
			if (note.Type == 3) {
				double edt = note.MeasureEnd - next->MesStart;
				double endOffset = next->MsPerMark * edt;
				double hitEnd = next->MsMarking + endOffset;

				n.IsLN = true;
				n.EndTime = hitEnd;
			}

			n.SampleRefId = note.Value;
			if (note.Channel > 8) {
				difficulty.AutoSamples.push_back(n);
			}
			else {
				n.LaneIndex -= 2;

				difficulty.Notes.push_back(n);
			}
		}

		difficulty.Timings = timings;
		difficulty.Samples = ojm.Samples;
		ojn->Difficulties[diff] = std::move(difficulty);
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
