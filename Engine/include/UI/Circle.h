#pragma once

#include "UIBase.h"
#include <Rendering/WindowsTypes.h>

class Circle : public UIBase {
public:
    Circle();
    virtual ~Circle();

    float Radius;
    int Counts;

protected:
    void OnDraw() override;
};