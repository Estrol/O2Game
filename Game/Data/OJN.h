#pragma once
#include <string>
#include <vector>
#include <unordered_map>

union Event {
	float BPM;
	struct {
		uint16_t Value;
		uint8_t VolPan;
		uint8_t Type;
	};
};

struct Package {
	uint32_t Measure;
	uint16_t Channel;
	uint16_t EventCount;

	std::vector<Event> Events;
};

struct Sample {
	int Index;
	std::vector<char> Buffer;
};

struct O2Note {
	int StartTime;
	int EndTime;
	int LaneIndex;
	int SampleIndex;
};

struct O2BPM {
	int StartTime;
	double Value;
};

struct Header {
	int songid;
	char signature[4];
	float encode_version;
	int genre;
	float bpm;
	short level[4];
	int event_count[3];
	int note_count[3];
	int measure_count[3];
	int package_count[3];
	short old_encode_version;
	short old_songid;
	char old_genre[20];
	int bmp_size;
	int old_file_version;
	char title[64];
	char artist[32];
	char noter[32];
	char ojm_file[32];
	int cover_size;
	int time[3];
	int note_offset[3];
	int cover_offset;
};

class OJN {
public:
	OJN();
	~OJN();

	void Load(std::string& filePath);

	Header Header;
	bool IsValid();

private:
	bool m_valid = false;
};