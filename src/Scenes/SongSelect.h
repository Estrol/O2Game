/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include <Screens/Base.h>
#include <UI/Image.h>

class SongSelect : public Screens::Base
{
public:
    SongSelect();
    ~SongSelect();

    void Update(double delta) override;
    void Draw(double delta) override;

    bool Attach() override;
    bool Detach() override;

private:
    std::shared_ptr<UI::Image> m_background;
    std::shared_ptr<UI::Image> m_musicList;
};