#pragma once
#include <iostream>

#define SDL_EXCEPTION_MAX_CHARS 500

class SDLException : std::exception {
public:
	SDLException();
	const char* what() const throw();

private:
	char m_msg[SDL_EXCEPTION_MAX_CHARS];
};