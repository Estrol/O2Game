/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include "MsgBoxEx.h"

#include <Graphics/NativeWindow.h>
#include <Graphics/Renderer.h>
#include <Imgui/imgui.h>

#include <map>
#include <vector>

struct MsgBoxSession
{
    std::string   Id;
    std::string   Title;
    std::string   Content;
    MsgBox::Type  Type;
    MsgBox::Flags Flags;
};

namespace {
    std::map<std::string, MsgBox::Result> m_results;
    std::vector<MsgBoxSession>            m_msgbox;
} // namespace

namespace MathUtil {
    static ImVec2 ScaleVec2(ImVec2 vec)
    {
        Rect bufferSz = Graphics::NativeWindow::Get()->GetBufferSize();
        Rect windowSz = Graphics::NativeWindow::Get()->GetWindowSize();

        return vec;
    }
} // namespace MathUtil

void MsgBox::Draw(double delta)
{
    if (m_msgbox.size() > 0) {
        Graphics::Renderer::Get()->ImGui_NewFrame();

        MsgBoxSession &session = m_msgbox.back();
        std::string    Id = session.Title + "###MsgBox";

        bool close = false;
        ImGui::OpenPopup(Id.c_str());

        ImGuiIO &io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(Id.c_str(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", session.Content.c_str());

            switch (session.Type) {
                case MsgBox::Type::YesNoCancel:
                {
                    if (ImGui::Button("Yes", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Enter)) {
                        m_results[session.Id] = MsgBox::Result::Yes;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("No", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
                        m_results[session.Id] = MsgBox::Result::No;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                        m_results[session.Id] = MsgBox::Result::Cancel;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    break;
                }

                case MsgBox::Type::YesNo:
                {
                    if (ImGui::Button("Yes", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Enter)) {
                        m_results[session.Id] = MsgBox::Result::Yes;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("No", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
                        m_results[session.Id] = MsgBox::Result::No;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }

                case MsgBox::Type::OkCancel:
                {
                    if (ImGui::Button("Ok", MathUtil::ScaleVec2(ImVec2(40, 0))) || ImGui::IsKeyDown(ImGuiKey_Enter)) {
                        m_results[session.Id] = MsgBox::Result::Ok;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                        m_results[session.Id] = MsgBox::Result::Cancel;

                        close = true;
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }

                default:
                {
                    if (ImGui::Button("Ok", MathUtil::ScaleVec2(ImVec2(40, 0)))) {
                        m_results[session.Id] = MsgBox::Result::Ok;

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

        Graphics::Renderer::Get()->ImGui_EndFrame();
    }
}

void MsgBox::InShow(const std::string Id, const std::string title, const std::string message, MsgBox::Type type, MsgBox::Flags flags)
{
    MsgBoxSession session = {};
    session.Id = Id;
    session.Title = title;
    session.Content = message;
    session.Type = type;
    session.Flags = flags;

    m_msgbox.push_back(session);
}

MsgBox::Result MsgBox::GetResult(const std::string Id)
{
    return m_results[Id];
}