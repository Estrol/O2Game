#include <SDL2/SDL.h>
#include <map>
#include <vector>

#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui.h"
#include "MsgBox.h"
#include "Texture/MathUtils.h"

struct MsgBoxSession
{
    std::string Id;
    std::string Title;
    std::string Content;
    MsgBoxType  Type;
};

namespace {
    std::map<std::string, int> m_results;
    std::vector<MsgBoxSession> m_msgbox;
} // namespace

int MsgBox::GetResult(std::string Id)
{
    int result = m_results[Id];
    m_results[Id] = -1;

    return result;
}

void MsgBox::Draw()
{
    if (m_msgbox.size() > 0) {
        ImguiUtil::NewFrame();

        MsgBoxSession &session = m_msgbox.back();
        std::string    Id = session.Title + "###MsgBox";

        bool close = false;
        ImGui::OpenPopup(Id.c_str());

        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(Id.c_str(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", session.Content.c_str());

            switch (session.Type) {
                case MsgBoxType::YESNOCANCEL:
                {
                    if (ImGui::Button("Yes", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Enter)) {
                        m_results[session.Id] = 1;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("No", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
                        m_results[session.Id] = 2;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                        m_results[session.Id] = 3;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    break;
                }

                case MsgBoxType::YESNO:
                {
                    if (ImGui::Button("Yes", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Enter)) {
                        m_results[session.Id] = 1;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("No", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
                        m_results[session.Id] = 2;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }

                case MsgBoxType::OKCANCEL:
                {
                    if (ImGui::Button("Ok", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Enter)) {
                        m_results[session.Id] = 4;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                        m_results[session.Id] = 3;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }

                default:
                {
                    if (ImGui::Button("Ok", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                        m_results[session.Id] = 4;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }
            }

            ImGui::EndPopup();
        }

        if (close) {
            m_msgbox.pop_back();
        }
    }
}

bool MsgBox::Any()
{
    return m_msgbox.size() > 0;
}

void MsgBox::Show(std::string Id, std::string Title, std::string fmt)
{
    return Show(Id, Title, fmt, MsgBoxType::OK);
}

void MsgBox::Show(std::string Id, std::string Title, std::string fmt, MsgBoxType type)
{
    for (auto &it : m_msgbox) {
        if (it.Id == Id) {
            return;
        }
    }

    m_results[Id] = -1;
    m_msgbox.push_back({ Id, Title, fmt, type });
}

int MsgBox::ShowOut(std::string title, std::string fmt)
{
    return ShowOut(title, fmt, MsgBoxType::OK, MsgBoxFlags::BTN_NOTHING);
}

int MsgBox::ShowOut(std::string title, std::string fmt, MsgBoxType type, MsgBoxFlags flags)
{
    SDL_MessageBoxData data = {};
    data.title = title.c_str();
    data.message = fmt.c_str();
    data.flags = 0;

    switch (flags) {
        case MsgBoxFlags::BTN_INFO:
        {
            data.flags |= SDL_MESSAGEBOX_INFORMATION;
            break;
        }

        case MsgBoxFlags::BTN_WARNING:
        {
            data.flags |= SDL_MESSAGEBOX_WARNING;
            break;
        }

        case MsgBoxFlags::BTN_ERROR:
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

    std::vector<SDL_MessageBoxButtonData> buttons;
    switch (type) {
        case MsgBoxType::YESNOCANCEL:
        {
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" });
            buttons.push_back({ 0, 2, "No" });
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 3, "Cancel" });
            break;
        }

        case MsgBoxType::YESNO:
        {
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Yes" });
            buttons.push_back({ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "No" });
            break;
        }

        case MsgBoxType::OKCANCEL:
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

    data.numbuttons = (int)buttons.size();
    data.buttons = buttons.data();

    int buttonid;
    SDL_ShowMessageBox(&data, &buttonid);

    return buttonid;
}
