#pragma once
#include <string>
#include "Scene.h"
#include "Imgui/imgui.h"

class Overlay : public Scene {
public:
    Overlay() = default;
    virtual ~Overlay() = default;

    ImVec2 GetPositon() const;
    ImVec2 GetSize() const;
    std::string GetName() const;
    bool IsClosed();

protected:
    std::string m_name;

    std::string m_title;
    ImVec2 m_position;
    ImVec2 m_size;

    bool m_exit;
};