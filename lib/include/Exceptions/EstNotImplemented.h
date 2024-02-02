/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __ESTNOTIMPLEMENTEDEXCEPTION_H_
#define __ESTNOTIMPLEMENTEDEXCEPTION_H_

#include "EstException.h"
#include <API.h>
#include <exception>
#include <string>

namespace Exceptions {
    class EstNotImplemented : public EstException
    {
    public:
        EstNotImplemented();
        EstNotImplemented(const char *message, ...);
        virtual ~EstNotImplemented() throw();
        virtual const char *what() const throw();

    private:
        std::string m_Message;
    };
} // namespace Exceptions

#endif