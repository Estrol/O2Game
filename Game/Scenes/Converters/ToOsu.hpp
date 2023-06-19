#pragma once
#include <filesystem>
#include "../../Data/ojn.h"

namespace Converters {
	static bool SaveTo(O2::OJN* ojn, const char* path);
}