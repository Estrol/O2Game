/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef MSGBOX_H
#define MSGBOX_H

#include "Rectangle.h"
#include "Text.h"
#include "UIBase.h"
#include <Graphics/GraphicsTexture2D.h>
#include <MsgBox.h>
#include <string>
#include <unordered_map>

namespace UI {
    struct DialogSession
    {
        std::string   Id;
        std::string   Title;
        std::string   Message;
        MsgBox::Type  Type;
        MsgBox::Flags flags;
    };

    class Dialog
    {
    public:
        Dialog();
        ~Dialog();

        void           Show(std::string Id, std::string title, std::string message, MsgBox::Type type, MsgBox::Flags flags);
        MsgBox::Result GetResult(std::string Id);

        void Draw(Rect clip);

    private:
        std::unordered_map<std::string, DialogSession>  m_msgBoxes;
        std::unordered_map<std::string, MsgBox::Result> m_msgBoxResults;

        std::unordered_map<MsgBox::Flags, std::shared_ptr<Graphics::Texture2D>> m_icons;

        std::shared_ptr<Rectangle> m_background;
        std::shared_ptr<Text>      m_text;
    };
} // namespace UI

#endif // MSGBOX_H