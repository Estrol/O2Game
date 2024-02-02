/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Graphics/NativeWindow.h>
#include <UI/Dialog.h>

using namespace UI;

Dialog::Dialog()
{
}

Dialog::~Dialog()
{
}

void Dialog::Show(std::string Id, std::string title, std::string message, MsgBox::Type type, MsgBox::Flags flags)
{
    DialogSession session;
    session.Id = Id;
    session.Title = title;
    session.Message = message;
    session.Type = type;
    session.flags = flags;

    m_msgBoxes[Id] = session;
}

MsgBox::Result Dialog::GetResult(std::string Id)
{
    MsgBox::Result result = m_msgBoxResults[Id];
    m_msgBoxResults.erase(Id);

    return result;
}

void Dialog::Draw(Rect clip)
{
    if (m_msgBoxes.size() == 0)
        return;

    if (!m_text) {
        m_text = std::make_shared<Text>();
    }

    if (!m_background) {
        m_background = std::make_shared<Rectangle>();
    }

    auto &first = m_msgBoxes.begin()->second;

    std::shared_ptr<Graphics::Texture2D> icon = nullptr;
    if (m_icons.find(first.flags) != m_icons.end())
        icon = m_icons[first.flags];

    auto windowBufferSize = Graphics::NativeWindow::Get()->GetBufferSize();

    /**
     * The dialog position is like this
     * [-------------------[]X]
     * [                      ]
     * [      [  RECT ]       ]
     * [                      ]
     * [----------------------]
     */

    UDim2 rectSize;
}
