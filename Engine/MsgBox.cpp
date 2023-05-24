#include "MsgBox.hpp"
#include <vector>
#include <map>

#include "./Imgui/ImguiUtil.hpp"
#include "./Imgui/imgui.h"

struct MsgBoxSession {
	std::string Id;
	std::string Title;
	std::string Content;
	MsgBoxType Type;
};

namespace {
	std::map<std::string, int> m_results;
	std::vector<MsgBoxSession> m_msgbox;
}

int MsgBox::GetResult(std::string Id) {
	int result = m_results[Id];
	m_results[Id] = -1;

	return result;
}

void MsgBox::Draw() {
	ImguiUtil::NewFrame();
	
	if (m_msgbox.size() > 0) {
		MsgBoxSession& session = m_msgbox.back();
		std::string Id = session.Title + "###MsgBox";

		bool close = false;
		ImGui::OpenPopup(Id.c_str());

		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal(Id.c_str(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text(session.Content.c_str());

			switch (session.Type) {
				case MsgBoxType::YESNOCANCEL: {
					if (ImGui::Button("Yes") || ImGui::IsKeyDown(ImGuiKey_Enter)) {
						m_results[session.Id] = 1;

						close = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("No") || ImGui::IsKeyDown(ImGuiKey_Escape)) {
						m_results[session.Id] = 2;

						close = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel")) {
						m_results[session.Id] = 3;

						close = true;
						ImGui::CloseCurrentPopup();
					}

					break;
				}

				case MsgBoxType::YESNO: {
					if (ImGui::Button("Yes") || ImGui::IsKeyDown(ImGuiKey_Enter)) {
						m_results[session.Id] = 1;

						close = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("No") || ImGui::IsKeyDown(ImGuiKey_Escape)) {
						m_results[session.Id] = 2;

						close = true;
						ImGui::CloseCurrentPopup();
					}
					break;
				}

				case MsgBoxType::OKCANCEL: {
					if (ImGui::Button("Ok") || ImGui::IsKeyDown(ImGuiKey_Enter)) {
						m_results[session.Id] = 4;

						close = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel")) {
						m_results[session.Id] = 3;

						close = true;
						ImGui::CloseCurrentPopup();
					}
					break;
				}

				default: {
					if (ImGui::Button("Ok")) {
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

void MsgBox::Show(std::string Id, std::string Title, std::string fmt) {
	return Show(Id, Title, fmt, MsgBoxType::OK);
}

void MsgBox::Show(std::string Id, std::string Title, std::string fmt, MsgBoxType type) {
	for (auto& it : m_msgbox) {
		if (it.Id == Id) {
			return;
		}
	}

	m_results[Id] = -1;
	m_msgbox.push_back({ Id, Title, fmt, type });
}