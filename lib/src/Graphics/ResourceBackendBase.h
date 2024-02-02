/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __RESOURCEBACKENDBASE_H_
#define __RESOURCEBACKENDBASE_H_

namespace Graphics {
    namespace Backends {
        class ResourceBackend
        {
        public:
            virtual ~ResourceBackend() = default;
        };
    } // namespace Backends
} // namespace Graphics

#endif