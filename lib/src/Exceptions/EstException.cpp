/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Exceptions/EstException.h>
#include <cstdarg>
using namespace Exceptions;

Exceptions::EstException::EstException()
{
    m_Message = "Unknown exception";
}

EstException::EstException(const char *message, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, message);
    vsprintf(buffer, message, args);
    va_end(args);

    m_Message = buffer;
}

EstException::~EstException() throw()
{
}

const char *EstException::what() const throw()
{
    return m_Message.c_str();
}
