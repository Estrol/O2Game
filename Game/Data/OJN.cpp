#include "OJN.h"
#include <fstream>

OJN::OJN() {
	Header = {};
}

OJN::~OJN() {
}

void OJN::Load(std::string& file) {
	char signature[] = {'o', 'j', 'n', '\0'};

	std::fstream fs(file, std::ios::binary | std::ios::in);
	fs.read((char*)&Header, sizeof(Header));

	if (memcmp(Header.signature, signature, 4) != 0) {
		::printf("Invalid OJN file: %s\n", file.c_str());
		return;
	}
}
