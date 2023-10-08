#pragma once

#include "UIBase.h"
#include <Rendering/WindowsTypes.h>

class Quad : public UIBase {
public:
    Quad();
    virtual ~Quad();

protected:
    void OnDraw() override;
};