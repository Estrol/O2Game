/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __ESTEXCEPTION_H_
#define __ESTEXCEPTION_H_

#include <API.h>
#include <exception>
#include <string>

namespace Exceptions {
    class EstException : public std::exception
    {
    public:
        EstException();
        EstException(const char *message, ...);
        virtual ~EstException() throw();
        virtual const char *what() const throw();

    private:
        std::string m_Message;
    };
} // namespace Exceptions

#endif