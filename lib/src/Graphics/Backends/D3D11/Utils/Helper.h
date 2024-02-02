/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef D3DHELPER_H_
#define D3DHELPER_H_

#define ESTZEROMEMORY(data, len) \
    memset((void *)&data, 0, len)

#define CHECKERROR(hr, err_msg)                        \
    if (!SUCCEEDED(hr)) {                              \
        throw Exceptions::EstException("%s", err_msg); \
    }

#endif