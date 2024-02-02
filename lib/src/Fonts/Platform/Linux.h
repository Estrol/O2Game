/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __LINUXFONTS_H_
#define __LINUXFONTS_H_
#if __linux__
#include "Base.h"

namespace Fonts::Platform {
    namespace Linux {
        FontFileInfo FindFont(std::string file);
    }
} // namespace Fonts::Platform
#endif
#endif