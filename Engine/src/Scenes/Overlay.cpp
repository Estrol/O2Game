#include "Overlay.h"

ImVec2 Overlay::GetPositon() const {
    return m_position;
}

ImVec2 Overlay::GetSize() const {
    return m_size;
}

std::string Overlay::GetName() const {
    return m_name;
}

bool Overlay::IsClosed() {
    return m_exit;
}
