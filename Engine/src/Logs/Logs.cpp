#include <Logs.h>
#include <cstdarg>
#include <stdio.h>

#include "Console.h"

void Logs::Puts(const char* fmt, ...) {
    char buffer[256] = {};

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    ::puts(buffer);
    Console::Send(buffer);
}