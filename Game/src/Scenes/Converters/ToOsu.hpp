#pragma once
#include "../../Data/OJN.h"
#include <filesystem>

namespace Converters {
    static bool SaveTo(O2::OJN *ojn, const char *path);
}