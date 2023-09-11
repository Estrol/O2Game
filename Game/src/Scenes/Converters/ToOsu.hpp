#pragma once
#include <filesystem>
#include "../../Data/OJN.h"

namespace Converters {
	static bool SaveTo(O2::OJN* ojn, const char* path);
}