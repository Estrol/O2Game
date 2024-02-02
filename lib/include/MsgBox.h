/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __MSGBOX_H_
#define __MSGBOX_H_

#include <string>

namespace MsgBox {
    enum class Type {
        None,
        Ok,
        OkCancel,
        YesNo,
        YesNoCancel
    };

    enum class Flags {
        None,
        Info,
        Warning,
        Error
    };

    enum class Result {
        None,
        Ok,
        Cancel,
        Yes,
        No
    };

    Result Show(const std::string title, const std::string message, Type type = Type::None, Flags flags = Flags::None);
} // namespace MsgBox

#endif