#include "Exception/SDLException.h"
#include <SDL2/SDL.h>

SDLException::SDLException() {
    memset(m_msg, 0, sizeof(m_msg));

    SDL_GetErrorMsg(m_msg, sizeof(m_msg));
}

const char* SDLException::what() const throw() {
    return m_msg;
}
