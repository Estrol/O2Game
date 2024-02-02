/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

#include <Screens/Base.h>
#include <UI/Image.h>
#include <UI/Rectangle.h>
#include <UI/Text.h>

class Loading : public Screens::Base
{
public:
    Loading();
    ~Loading();

    void Update(double delta) override;
    void Draw(double delta) override;

    bool Attach() override;
    bool Detach() override;

private:
    // can't load chart lmao
    bool fucked = false;
    bool is_shown = false;
    bool is_ready = false;
    bool is_audio_loaded = false;
    bool dont_dispose = false;

    int currentProgress = 0;
    int maxProgress = 0;

    double                         m_counter;
    std::shared_ptr<UI::Image>     m_LoadingBG;
    std::shared_ptr<UI::Rectangle> m_LoadingBar;
    std::shared_ptr<UI::Text>      m_Text;
};