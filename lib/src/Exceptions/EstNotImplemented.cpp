/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Exceptions/EstNotImplemented.h>
#include <cstdarg>

using namespace Exceptions;

EstNotImplemented::EstNotImplemented()
{
    m_Message = "Missing implementation";
}

EstNotImplemented::EstNotImplemented(const char *message, ...) : EstException(message)
{
    char buffer[1024];

    va_list args;
    va_start(args, message);
    vsprintf(buffer, message, args);
    va_end(args);

    m_Message = std::string(buffer) + " Not implemented";
}

EstNotImplemented::~EstNotImplemented() throw()
{
}

const char *EstNotImplemented::what() const throw()
{
    return m_Message.c_str();
}