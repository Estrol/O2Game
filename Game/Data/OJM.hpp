#pragma once
#include <string>
#include <vector>

struct O2Sample {
	uint32_t RefValue;
	std::vector<uint8_t> AudioData;
};

class OJM {
public:
	~OJM();

	void Load(std::string fileName);
	bool IsValid();

	std::vector<O2Sample> Samples;
private:
	void LoadM30Data(std::fstream& fs);
	void LoadOJMData(std::fstream& fs, bool encrypted);
	
	
	bool m_valid;
};