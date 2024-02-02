/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __WIN32FONTS_H_
#define __WIN32FONTS_H_
#if _WIN32
#include "Base.h"

namespace Fonts::Platform {
    namespace Win32 {
        FontFileInfo FindFont(std::string file);
    }
} // namespace Fonts::Platform

#endif
#endif