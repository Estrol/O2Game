/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __MSG_BOX_EX_H
#define __MSG_BOX_EX_H

#include <MsgBox.h>

namespace MsgBox {
    void Draw(double delta);

    void   InShow(const std::string Id, const std::string title, const std::string message, Type type = Type::None, Flags flags = Flags::None);
    Result GetResult(const std::string Id);
} // namespace MsgBox

#endif