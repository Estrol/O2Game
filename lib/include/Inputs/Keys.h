/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __KEYS_H_
#define __KEYS_H_

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_scancode.h>

namespace Inputs {
    enum class Keys {
        INVALID_KEY = 0,

        A = SDL_SCANCODE_A,
        B = SDL_SCANCODE_B,
        C = SDL_SCANCODE_C,
        D = SDL_SCANCODE_D,
        E = SDL_SCANCODE_E,
        F = SDL_SCANCODE_F,
        G = SDL_SCANCODE_G,
        H = SDL_SCANCODE_H,
        I = SDL_SCANCODE_I,
        J = SDL_SCANCODE_J,
        K = SDL_SCANCODE_K,
        L = SDL_SCANCODE_L,
        M = SDL_SCANCODE_M,
        N = SDL_SCANCODE_N,
        O = SDL_SCANCODE_O,
        P = SDL_SCANCODE_P,
        Q = SDL_SCANCODE_Q,
        R = SDL_SCANCODE_R,
        S = SDL_SCANCODE_S,
        T = SDL_SCANCODE_T,
        U = SDL_SCANCODE_U,
        V = SDL_SCANCODE_V,
        W = SDL_SCANCODE_W,
        X = SDL_SCANCODE_X,
        Y = SDL_SCANCODE_Y,
        Z = SDL_SCANCODE_Z,

        Zero = SDL_SCANCODE_0,
        One = SDL_SCANCODE_1,
        Two = SDL_SCANCODE_2,
        Three = SDL_SCANCODE_3,
        Four = SDL_SCANCODE_4,
        Five = SDL_SCANCODE_5,
        Six = SDL_SCANCODE_6,
        Seven = SDL_SCANCODE_7,
        Eight = SDL_SCANCODE_8,
        Nine = SDL_SCANCODE_9,

        Return = SDL_SCANCODE_RETURN,
        EscapeK = SDL_SCANCODE_ESCAPE,
        Backspace = SDL_SCANCODE_BACKSPACE,
        Tab = SDL_SCANCODE_TAB,
        Space = SDL_SCANCODE_SPACE,

        Minus = SDL_SCANCODE_MINUS,
        Equals = SDL_SCANCODE_EQUALS,
        LeftBracket = SDL_SCANCODE_LEFTBRACKET,
        RightBracket = SDL_SCANCODE_RIGHTBRACKET,
        BackSlash = SDL_SCANCODE_BACKSLASH,
        NonuShash = SDL_SCANCODE_NONUSHASH,
        SemiColon = SDL_SCANCODE_SEMICOLON,
        Apostrophe = SDL_SCANCODE_APOSTROPHE,
        Grave = SDL_SCANCODE_GRAVE,
        Comma = SDL_SCANCODE_COMMA,
        Period = SDL_SCANCODE_PERIOD,
        Slash = SDL_SCANCODE_SLASH,
        CapsLock = SDL_SCANCODE_CAPSLOCK,

        F1 = SDL_SCANCODE_F1,
        F2 = SDL_SCANCODE_F2,
        F3 = SDL_SCANCODE_F3,
        F4 = SDL_SCANCODE_F4,
        F5 = SDL_SCANCODE_F5,
        F6 = SDL_SCANCODE_F6,
        F7 = SDL_SCANCODE_F7,
        F8 = SDL_SCANCODE_F8,
        F9 = SDL_SCANCODE_F9,
        F10 = SDL_SCANCODE_F10,
        F11 = SDL_SCANCODE_F11,
        F12 = SDL_SCANCODE_F12,

        PrintScreen = SDL_SCANCODE_PRINTSCREEN,
        ScrollLock = SDL_SCANCODE_SCROLLLOCK,
        Pause = SDL_SCANCODE_PAUSE,
        Insert = SDL_SCANCODE_INSERT,
        Home = SDL_SCANCODE_HOME,
        PageUp = SDL_SCANCODE_PAGEUP,
        PageDown = SDL_SCANCODE_PAGEDOWN,
        End = SDL_SCANCODE_END,

        Right = SDL_SCANCODE_RIGHT,
        Left = SDL_SCANCODE_LEFT,
        Down = SDL_SCANCODE_DOWN,
        Up = SDL_SCANCODE_UP,

        NumLockClear = SDL_SCANCODE_NUMLOCKCLEAR,

        KeypadDivide = SDL_SCANCODE_KP_DIVIDE,
        KeypadMultiply = SDL_SCANCODE_KP_MULTIPLY,
        KeypadMinus = SDL_SCANCODE_KP_MINUS,
        KeypadPlus = SDL_SCANCODE_KP_PLUS,
        KeypadEnter = SDL_SCANCODE_KP_ENTER,
        KeypadPeriod = SDL_SCANCODE_KP_PERIOD,

        KeypadOne = SDL_SCANCODE_KP_1,
        KeyPadTwo = SDL_SCANCODE_KP_2,
        KeyPadThree = SDL_SCANCODE_KP_3,
        KeyPadFour = SDL_SCANCODE_KP_4,
        KeyPadFive = SDL_SCANCODE_KP_5,
        KeyPadSix = SDL_SCANCODE_KP_6,
        KeyPadSeven = SDL_SCANCODE_KP_7,
        KeyPadEight = SDL_SCANCODE_KP_8,
        KeyPadNine = SDL_SCANCODE_KP_9,
        KeyPadZero = SDL_SCANCODE_KP_0,
    };

    enum class Mouse {
        INVALID_MOUSE = 0,

        Left = SDL_BUTTON_LEFT,
        Middle = SDL_BUTTON_MIDDLE,
        Right = SDL_BUTTON_RIGHT,
        X1 = SDL_BUTTON_X1,
        X2 = SDL_BUTTON_X2,
    };

    enum class Type {
        KeyDown,
        KeyUp,
        MouseDown,
        MouseUp,
    };

    struct MouseState
    {
        double X;
        double Y;

        bool IsDown;
    };

    struct KeyboardState
    {
        Keys Key;
        bool IsDown;
    };

    struct State
    {
        Type Type;

        union {
            MouseState    Mouse;
            KeyboardState Keyboard;
        };
    };
} // namespace Inputs

#endif