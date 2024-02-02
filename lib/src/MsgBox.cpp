/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <MsgBox.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <vector>

MsgBox::Result MsgBox::Show(const std::string title, const std::string message, MsgBox::Type type, MsgBox::Flags flags)
{
    SDL_MessageBoxData data = {};
    data.title = title.c_str();
    data.message = message.c_str();
    data.flags = 0;

    std::vector<SDL_MessageBoxButtonData> buttons;
    switch (type) {
        case Type::YesNoCancel:
        {
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" });
            buttons.push_back({ 0, 2, "No" });
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 3, "Cancel" });
            break;
        }

        case Type::YesNo:
        {
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" });
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "No" });
            break;
        }

        case Type::OkCancel:
        {
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 4, "Ok" });
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 3, "Cancel" });
            break;
        }

        default:
        {
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 4, "Ok" });
            break;
        }
    }

    switch (flags) {
        case Flags::Info:
        {
            data.flags |= SDL_MESSAGEBOX_INFORMATION;
            break;
        }

        case Flags::Warning:
        {
            data.flags |= SDL_MESSAGEBOX_WARNING;
            break;
        }

        case Flags::Error:
        {
            data.flags |= SDL_MESSAGEBOX_ERROR;
            break;
        }

        default:
        {
            data.flags = 0;
            break;
        }
    }

    data.numbuttons = (int)buttons.size();
    data.buttons = buttons.data();

    int buttonid;
    SDL_ShowMessageBox(&data, &buttonid);

    Result result;
    switch (buttonid) {
        case 1:
        {
            result = Result::Yes;
            break;
        }

        case 2:
        {
            result = Result::No;
            break;
        }

        case 3:
        {
            result = Result::Cancel;
            break;
        }

        default:
        {
            result = Result::Ok;
            break;
        }
    }

    return result;
}