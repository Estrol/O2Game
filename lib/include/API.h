/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

// clang-format off
#ifndef __API_H_
    #if defined(EST_EXPORT)
        #if defined(_WIN32)
            #define EST_API __declspec(dllexport)
        #elif defined(__GNUC__)
            #define EST_API __attribute__((visibility("default")))
        #endif
    #else
        #if defined(_WIN32)
            #define EST_API __declspec(dllimport)
        #elif defined(__GNUC__)
            #define EST_API
        #endif
    #endif
#endif
// clang-format on