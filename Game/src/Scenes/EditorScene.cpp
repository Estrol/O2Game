#include "EditorScene.hpp"

#include "MsgBox.h"
#include "SceneManager.h"
#include "Configuration.h"
#include "Rendering/Window.h"
#include "Texture/MathUtils.h"
#include "Audio/AudioManager.h"
#include "Fonts/FontResources.h"

#include "Imgui/ImguiUtil.h"
#include "Imgui/imgui_internal.h"

#include "../GameScenes.h"
#include "../EnvironmentSetup.hpp"
#include "../Engine/GameAudioSampleCache.hpp"

constexpr int noteSize = 12;
const std::vector<ImColor> laneColor = {
	ImColor(0, 0, 255),
	ImColor(0, 0, 255),
	ImColor(225, 242, 189),
	ImColor(99, 147, 224),
	ImColor(225, 242, 189),
	ImColor(224, 214, 99),
	ImColor(225, 242, 189),
	ImColor(99, 147, 224),
	ImColor(225, 242, 189)
};

double FindBPMAt(std::vector<std::pair<double, double>>& bpms, double offset) {
	int min = 0, max = (int)bpms.size() - 1;

	if (max == 0) {
		return bpms[0].second;
	}

	while (min <= max) {
		int mid = (min + max) / 2;

		bool afterMid = mid < 0 || bpms[mid].first <= offset;
		bool beforeMid = mid + 1 >= bpms.size() || bpms[mid + 1].first > offset;

		if (afterMid && beforeMid) {
			return bpms[mid].second;
		}
		else if (afterMid) {
			max = mid - 1;
		}
		else {
			min = mid + 1;
		}
	}

	return bpms[0].second;
}

struct INote {
	double Position = -1;
	int SampleRefId = -1;
	double Time = -1;
	double EndTime = -1;
	bool IsBPM = false;
	double BPMValue = -1;
	bool IsLN = false;
	int Channel = -1;

	float Vol;
	float Pan;
};

struct LineInfo {
	double Time;
	bool Major;
	bool Minor;
};

EditorScene::EditorScene() {
	m_exit = false;
	m_ready = false;
	m_currentNotespeed = 0.25;
	m_currentTime = 0;
}

void EditorScene::Update(double delta) {
	if (m_exit) {
		m_exit = false;

		m_autoscroll = false;
		StopSample();

		SceneManager::DisplayFade(100, [] {
			SceneManager::ChangeScene(GameScene::MAINMENU);
		});
	}
}

void EditorScene::Render(double delta) {
	if (!m_ready) return;
	
	ImguiUtil::NewFrame();

	auto flags = ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_MenuBar;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(MathUtil::ScaleVec2(1280, 720));

	GameWindow* wnd = GameWindow::GetInstance();

	float originScale = (wnd->GetBufferWidth() + wnd->GetBufferHeight()) / 15.6f;
	float targetScale = (wnd->GetWidth() + wnd->GetHeight()) / 15.6f;
	float fontScale = (targetScale / originScale);

	bool previousState = m_autoscroll;
	bool open_timing_table = false;
	bool change_difficulty = false;
	int difficulty_index = -1;

	std::map<int, std::string> m_wavs, m_oggs;

	for (auto& sample : m_samples) {
		if (sample.RefValue >= 1000) {
			m_oggs[sample.RefValue] = "#OGG : " + std::to_string(sample.RefValue - 1000);
		}
		else {
			m_wavs[sample.RefValue] = "#WAV : " + std::to_string(sample.RefValue);
		}
	}

	if (ImGui::Begin("MainWindow", NULL, flags)) {
		int menu_bar_height = 0;

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open")) {
					MsgBox::Show("EditorError", "Error", "Not Implemented");
;				}

				if (ImGui::MenuItem("Save")) {
					MsgBox::Show("EditorError", "Error", "Not Implemented");
				}

				if (ImGui::MenuItem("Save as")) {
					MsgBox::Show("EditorError", "Error", "Not Implemented");
				}

				ImGui::Separator();
				if (ImGui::BeginMenu("Import from")) {
					if (ImGui::MenuItem("BMS")) {
						MsgBox::Show("EditorError", "Error", "Not Implemented");
					}

					if (ImGui::MenuItem("osu!mania")) {
						MsgBox::Show("EditorError", "Error", "Not Implemented");
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Export as")) {
					if (ImGui::MenuItem("BMS")) {
						MsgBox::Show("EditorError", "Error", "Not Implemented");
					}

					if (ImGui::MenuItem("osu!mania")) {
						MsgBox::Show("EditorError", "Error", "Not Implemented");
					}

					ImGui::EndMenu();
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) {
					m_exit = true;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Editor")) {
				if (ImGui::MenuItem("Timing Table")) {
					open_timing_table = true;
				}
				
				ImGui::Text("---- Test ----");

				if (ImGui::MenuItem("Auto Scroll")) {
					m_autoscroll = !m_autoscroll;
				}

				if (ImGui::MenuItem("Reset Time")) {
					m_currentTime = 0;

					StopSample();
				}

				ImGui::EndMenu();
			}

			menu_bar_height = (int)ImGui::GetFrameHeight();

			ImGui::EndMenuBar();
		}

		if (ImGui::BeginChild("ChartInfo", MathUtil::ScaleVec2(225, 335), true, 0)) {
			ImGui::Text("Grid Size");

			int m_last_var1 = m_measureGridSize;
			int m_last_var2 = m_measureGridSeparator;

			ImGui::InputInt("###InputMeasureSize", &m_measureGridSize, 1, 2);
			ImGui::InputInt("###InputMeasureSeparator", &m_measureGridSeparator, 1, 2);

			if (m_last_var1 != m_measureGridSize || m_last_var2 != m_measureGridSeparator) {
				m_lines.clear();
				m_majorLines.clear();

				m_measureGridSize = std::clamp(m_measureGridSize, 8, 192);
				m_measureGridSeparator = std::clamp(m_measureGridSeparator, 1, 64);
			}

			ImGui::NewLine();
			ImGui::Text("Lane Width");
			ImGui::InputFloat("###InputLaneWidthSize", &m_laneWidth, 1.0f, 2.5f, "%.2f");

			m_laneWidth = std::clamp(m_laneWidth, 10.0f, 100.0f);

			ImGui::NewLine();
			ImGui::Text("Scroll Speed");
			ImGui::InputFloat("###InputScrollSpeed", &m_currentNotespeed, 0.01f, 0.1f, "%0.2f");

			m_currentNotespeed = std::clamp(m_currentNotespeed, 0.01f, 10.0f);

			ImGui::NewLine();
			ImGui::Text("Difficulty");

			std::vector<std::string> diffName = { "EX", "NX", "HX" };
			std::vector<std::string> diffDesc = {
				"Easy Mode",
				"Normal Mode",
				"Hard Mode"
			};

			for (int i = 0; i < 3; i++) {
				if (m_currentDifficulty == i) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				if (ImGui::Button(diffName[i].c_str(), ImVec2(80, 0))) {
					change_difficulty = true;
					difficulty_index = i;
				}

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
					ImGui::SetTooltip("%s", diffDesc[i].c_str());
				}

				if (m_currentDifficulty == i) {
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

				if (i != 2) {
					ImGui::SameLine();
				}
			}

			ImGui::EndChild();
		}

		ImGui::SameLine();
		auto cursorPosX = ImGui::GetCursorPosX();
		ImGui::NewLine();

		if (ImGui::BeginChild("SampleInfo", MathUtil::ScaleVec2(225, 350), true, ImGuiWindowFlags_NoScrollWithMouse)) {
			if (ImGui::BeginTabBar("SampleTabBar")) {
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));

				if (ImGui::BeginTabItem("WAV")) {
					if (ImGui::BeginChild("WAV_ChildWindow")) {
						for (auto& [key, value] : m_wavs) {
							if (ImGui::Button((value + "###m_wavs_list_btn" + std::to_string(key)).c_str(), MathUtil::ScaleVec2(250, 0))) {
								if (!m_autoscroll) {
									PlaySample(key);
								}
							}
						}

						ImGui::EndChild();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("OGG")) {
					if (ImGui::BeginChild("OGG_ChildWindow")) {
						for (auto& [key, value] : m_oggs) {
							if (ImGui::Button((value + "###m_oggs_list_btn" + std::to_string(key)).c_str(), MathUtil::ScaleVec2(250, 0))) {
								if (!m_autoscroll) {
									PlaySample(key);
								}
							}
						}

						ImGui::EndChild();
					}
					
					ImGui::EndTabItem();
				}

				ImGui::PopStyleVar();
				ImGui::EndTabBar();
			}

			ImGui::EndChild();
		}

		auto drawList = ImGui::GetWindowDrawList();
		double row_column_width = m_laneWidth;
		const int max_rows = 36;

		std::vector<double> lanePos;
		std::vector<std::pair<ImVec2, ImVec2>> toDraw;

		double mid_x = 1260 * 0.4;
		for (int i = 0; i < max_rows + 1; i++) {
			lanePos.push_back(i * row_column_width);

			toDraw.push_back({ MathUtil::ScaleVec2(i * row_column_width, 0), MathUtil::ScaleVec2(i * row_column_width, 720) });
		}
		
		ImColor bg(24, 24, 24);
		ImVec2 size = MathUtil::ScaleVec2(lanePos.back(), 720);
		size.x += cursorPosX;

		drawList->AddRectFilled(ImVec2(cursorPosX, 0), size, bg);
		drawList->AddRect(ImVec2(cursorPosX, 0), size, ImColor(255, 255, 255));

		for (int i = 0; i < lanePos.size(); i++) {
			auto Pos1 = MathUtil::ScaleVec2(lanePos[i], 0) + ImVec2(cursorPosX, 0);
			auto Pos2 = MathUtil::ScaleVec2(lanePos[i], 720) + ImVec2(cursorPosX, 0);

			if (i < laneColor.size() && i + 1 < lanePos.size()) {
				auto Pos3 = MathUtil::ScaleVec2(lanePos[i+1], 0) + ImVec2(cursorPosX, 0);
				auto Pos4 = MathUtil::ScaleVec2(lanePos[i+1], 720) + ImVec2(cursorPosX, 0);

				ImColor lane_color = laneColor[i];
				if (ImGui::IsMouseHoveringRect(Pos1, Pos4) && !m_autoscroll) {
					lane_color.Value.x *= 0.55f;
					lane_color.Value.y *= 0.55f;
					lane_color.Value.z *= 0.55f;
				}
				else {
					lane_color.Value.x *= 0.25f;
					lane_color.Value.y *= 0.25f;
					lane_color.Value.z *= 0.25f;
				}

				// draw rectangle filled with that color
				drawList->AddRectFilled(Pos2, Pos3, lane_color);
			}

			drawList->AddLine(Pos1, Pos2, ImColor(255, 255, 255));
		}

		std::vector<double> m_time_stop_at;
		double audio_length = 0;

		{ // Draw Notes Here
			if (m_autoscroll) {
				m_currentTime += ((float)delta * 1000.0f);
			}

			{
				const double BeatPerSecs = 240000.0;
				const double BPM = m_bpms[0].second;

				double currentMeasure = 0;
				int in_currentMeasure = 0;
				int maxMeasure = 300;
				double currentTimer = 0;
				double currentMeasureLength = 1.0;

				double currentBPM = BPM;
				if (m_lines.empty()) {
					std::vector<INote> measureList;
					for (int i = 0; i < maxMeasure; i++) {
						for (float j = 0; j < static_cast<double>(m_measureGridSize); j++) {
							INote note = {};
							note.Channel = 4;
							note.Position = static_cast<double>(i + j / static_cast<double>(m_measureGridSize));

							measureList.push_back(note);
						}
					}

					for (auto& note : m_notes) {
						if (note.Channel == 0 || note.Channel == 1) {
							measureList.push_back(note);
						}
					}

					std::sort(measureList.begin(), measureList.end(), [](auto& noteA, auto& noteB) { return noteA.Position < noteB.Position; });

					int count = 0;
					for (auto& note : measureList) {
						double Measure = std::floor(note.Position);
						double Position = (note.Position - std::floor(note.Position)) * currentMeasureLength;
						 
						currentTimer += (BeatPerSecs * ((Measure + Position) - currentMeasure)) / currentBPM;
						currentMeasure = Measure + Position;
						in_currentMeasure = static_cast<int>(std::floor(currentMeasure));

						if (note.Channel == 0) {
							currentMeasureLength = note.BPMValue;
						}
						if (note.Channel == 1) {
							currentBPM = note.BPMValue;
						}
						else {
							LineInfo info = {};
							info.Time = currentTimer;
							info.Major = note.Position == static_cast<double>(in_currentMeasure);
							info.Minor = count % m_measureGridSeparator == 0;

							count++;
							m_lines.push_back(info);

							if (info.Major) {
								m_majorLines.push_back(info);
							}
						}
					}
				}
				
				currentBPM = BPM;
				currentTimer = 0;
				currentMeasure = 0;
				currentMeasureLength = 1.0;
				for (auto& note : m_notes) {
					double Measure = std::floor(note.Position);
					double Position = (note.Position - std::floor(note.Position)) * currentMeasureLength;

					currentTimer += (BeatPerSecs * ((Measure + Position) - currentMeasure)) / currentBPM;
					currentMeasure = Measure + Position;

					if (note.Channel == 0) {
						currentMeasureLength = note.BPMValue;
					}
					if (note.Channel == 1) {
						currentBPM = note.BPMValue;
					}

					note.Time = currentTimer;
				}

				audio_length = currentTimer + 1500;
			}

			// Draw measure line
			int TimeCurrentMeasure = 0;

			if (!m_autoscroll) {
				for (int i = 0; i < m_lines.size(); i++) {
					double time = m_lines[i].Time * 100.0;
					double nextTime = 0;
					if (i + 1 < m_lines.size()) {
						nextTime = m_lines[i + 1].Time * 100.0;
					}

					m_time_stop_at.push_back(time);

					// hitPosition + ((initialTrackPos - offset) * (upscroll ? noteSpeed : -noteSpeed) / 100);
					double yPos = (720 * 0.8) + ((time - (m_currentTime * 100)) * -m_currentNotespeed / 100);
					double yPos2 = (720 * 0.8) + ((nextTime - (m_currentTime * 100)) * -m_currentNotespeed / 100);

					ImVec2 hitPosVec1 = MathUtil::ScaleVec2(0, yPos) + ImVec2(cursorPosX, 0);
					ImVec2 hitPosVec2 = MathUtil::ScaleVec2(lanePos.back(), yPos) + ImVec2(cursorPosX, 0);

					// draw line
					if (m_lines[i].Minor) {
						drawList->AddLine(hitPosVec1, hitPosVec2, ImColor(138, 138, 138), 1.0f);
					}
					else {
						drawList->AddLine(hitPosVec1, hitPosVec2, ImColor(96, 96, 96), 1.0f);
					}
				}
			}

			for (int i = 0; i < m_majorLines.size(); i++) {
				if ((i + 1) < m_majorLines.size()) {
					double current = m_majorLines[i].Time * 100;
					double next = m_majorLines[i + 1].Time * 100;

					if (current < m_currentTime * 100 && next > m_currentTime * 100) {
						TimeCurrentMeasure = i;
					}

					double yPos = (720 * 0.8) + ((current - (m_currentTime * 100)) * -m_currentNotespeed / 100);
					double yPos2 = (720 * 0.8) + ((next - (m_currentTime * 100)) * -m_currentNotespeed / 100);

					ImVec2 middlePos = MathUtil::ScaleVec2(mid_x, (yPos + yPos2) / 2) + ImVec2(cursorPosX, 0);

					std::string text = "";
					if (i < 10) {
						text += "#00";
					}
					else if (i < 100) {
						text += "#0";
					}
					else {
						text += "#";
					}

					ImVec2 hitPosVec1 = MathUtil::ScaleVec2(0, yPos) + ImVec2(cursorPosX, 0);
					ImVec2 hitPosVec2 = MathUtil::ScaleVec2(lanePos.back(), yPos) + ImVec2(cursorPosX, 0);
					drawList->AddLine(hitPosVec1, hitPosVec2, ImColor(255, 255, 0), 1.0f);

					text += std::to_string(i);

					ImVec2 textSize = FontResources::GetButtonFont()->CalcTextSizeA(75 * fontScale, FLT_MAX, 0, text.c_str());
					middlePos.x -= textSize.x / 2.0f;
					middlePos.y -= textSize.y / 2.0f;

					drawList->AddText(FontResources::GetButtonFont(), 75 * fontScale, middlePos, ImColor(128, 128, 128), text.c_str());
				}
			}

			std::vector<INote> toDraw;
			{
				double HoldTime[7] = { -1, -1, -1, -1, -1, -1, -1 };

				for (INote note : m_notes) {
					if (note.IsLN) {
						if (HoldTime[note.Channel - 2] != -1) {
							note.EndTime = note.Time;
							note.Time = HoldTime[note.Channel - 2];
							HoldTime[note.Channel - 2] = -1;

							toDraw.push_back(note);
						}
						else {
							HoldTime[note.Channel - 2] = note.Time;
						}
					}
					else {
						toDraw.push_back(note);
					}
				}
			}

			// Draw notes
			{	
				for (auto& note : toDraw) {
					double time = note.Time * 100;
					double yPos = ((720 * 0.8) + ((time - (m_currentTime * 100)) * -m_currentNotespeed / 100));
					yPos -= noteSize;

					// check if difference between time and m_currentTime is less than 5 seconds
					if (m_currentTime >= note.Time && m_autoscroll && !note.IsBPM) {
						if (std::abs(note.Time - m_currentTime) < 25) {
							PlaySample(note.SampleRefId);
						}
					}

					if (note.Channel >= lanePos.size()) {
						continue;
					}

					ImColor color = note.Channel < laneColor.size() ? laneColor[note.Channel] : ImColor(255, 255, 255);

					if ((yPos < 0 || yPos > 720) && !note.IsLN) {
						continue;
					}

					ImVec2 head_pos = MathUtil::ScaleVec2(lanePos[note.Channel], yPos) + ImVec2(cursorPosX, 0);
					ImVec2 head_size = MathUtil::ScaleVec2(lanePos[note.Channel + 1], yPos + noteSize) + ImVec2(cursorPosX, 0);

					ImVec2 tail_pos, tail_size;

					auto drawList = ImGui::GetWindowDrawList();

					if (note.EndTime != -1) {
						double endtime = note.EndTime * 100;
						double ayPos = ((720 * 0.8) + ((endtime - (m_currentTime * 100)) * -m_currentNotespeed / 100));
						ayPos -= noteSize;

						tail_pos = MathUtil::ScaleVec2(lanePos[note.Channel], ayPos) + ImVec2(cursorPosX, 0);
						tail_size = MathUtil::ScaleVec2(lanePos[note.Channel + 1], ayPos + noteSize) + ImVec2(cursorPosX, 0);

						auto lnColor = color;
						// multiply it by 0.8
						lnColor.Value.x *= 0.8f;
						lnColor.Value.y *= 0.8f;
						lnColor.Value.z *= 0.8f;

						drawList->AddRectFilled(tail_pos, head_size, lnColor);
						drawList->AddRectFilled(tail_pos, tail_size, color);
					}

					drawList->AddRectFilled(head_pos, head_size, color);

					if (note.IsBPM) {
						drawList->AddText(NULL, 13 * fontScale, head_pos, ImColor(255, 255, 255), ("#" + std::to_string(note.BPMValue)).c_str());
					}
					else {
						drawList->AddText(NULL, 13 * fontScale, head_pos, ImColor(0, 0, 0), ("#" + std::to_string(note.SampleRefId)).c_str());
					}
				}
			}
		}

		ImVec2 hitPosVec1 = MathUtil::ScaleVec2(0, 720 * 0.8) + ImVec2(cursorPosX, 0);
		ImVec2 hitPosVec2 = MathUtil::ScaleVec2(lanePos.back(), 720 * 0.8) + ImVec2(cursorPosX, 0);

		drawList->AddLine(hitPosVec1, hitPosVec2, ImColor(255, 255, 255), 5.0f);

		/* Input */
		auto rc1 = MathUtil::ScaleVec2(1260, 0) + ImVec2(0, menu_bar_height + 2.5f);
		auto rc2 = MathUtil::ScaleVec2(1277, 715);

		auto window_sz = rc2 - rc1;

		ImGui::SetNextWindowPos(rc1);
		drawList->AddRectFilled(rc1, rc2, ImColor(24, 24, 24));

		if (ImGui::BeginChild("###ProgresssChildWindow", window_sz, true, ImGuiWindowFlags_NoScrollbar)) {
			ImGui::BeginDisabled(m_autoscroll);

			ImGui::SetCursorPos(MathUtil::ScaleVec2(3.5, 3.25));
			ImGuiSliderFlags flags = ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Vertical;

			float lastSize = FontResources::GetReallyBigFontForSlider()->FontSize;
			FontResources::GetReallyBigFontForSlider()->FontSize = window_sz.y - MathUtil::ScaleVec2(0, 12).y;

			ImGui::PushFont(FontResources::GetReallyBigFontForSlider());
			ImGui::SliderFloat("###ProgressSlider", &m_currentTime, 0.0f, (float)audio_length, "", flags);
			ImGui::PopFont();

			FontResources::GetReallyBigFontForSlider()->FontSize = lastSize;
			ImGui::EndDisabled();
			ImGui::EndChild();
		}

		// check imgui mouse scroll=
		if (ImGui::IsWindowFocused() && ImGui::GetIO().MouseWheel != 0 && !m_autoscroll) {
			m_currentTime = std::clamp(m_currentTime + ImGui::GetIO().MouseWheel * 100.0f, 0.0f, FLT_MAX);
		}

		std::sort(m_time_stop_at.begin(), m_time_stop_at.end());

		// check space bar then toggle autoScroll
		if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
			if (m_autoscroll) {
				auto it = std::lower_bound(m_time_stop_at.begin(), m_time_stop_at.end(), m_currentTime * 100);
				if (it != m_time_stop_at.end()) {
					m_currentTime = (float)((*it) / 100.0);
				}
			}

			m_autoscroll = !m_autoscroll;
		}

		if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && !m_autoscroll) {
			auto it = std::lower_bound(m_time_stop_at.begin(), m_time_stop_at.end(), m_currentTime * 100);
			if (it != m_time_stop_at.end()) {
				if ((int)(it - m_time_stop_at.begin()) < (int)m_time_stop_at.size()) {
					m_currentTime = (float)((*(it + 1)) / 100.0);
				}
			}
		}
		
		if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && !m_autoscroll) {
			auto it = std::lower_bound(m_time_stop_at.begin(), m_time_stop_at.end(), m_currentTime * 100);
			if (it != m_time_stop_at.end()) {
				if (it - m_time_stop_at.begin() > 0) {
					m_currentTime = (float)((*(it - 1)) / 100.0);
				}
			}
		}

		if (m_currentTime >= audio_length) {
			m_autoscroll = false;
		}

		if (previousState && !m_autoscroll) {
			StopSample();
		}

		ImGui::End();
	}

	if (open_timing_table) {
		ImGui::OpenPopup("Timing List###editor_timing_list");
	}

	ImGuiIO& io = ImGui::GetIO();

	bool b = true;
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Timing List###editor_timing_list", &b, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize)) {
		auto child_sz = MathUtil::ScaleVec2(420, 300);
		
		if (ImGui::BeginChild("###imgui_pop_up_child_window", child_sz, false)) {
			if (ImGui::BeginTable("###bpm_table_view", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
				ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("BPM", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Position", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableHeadersRow();

				for (int row = 0; row < m_bpms.size(); row++) {
					ImGui::TableNextRow();
					for (int i = 0; i < 3; i++) {
						ImGui::TableSetColumnIndex(i);

						switch (i) {
							case 0: {
								ImGui::Text("%d.", (row) + 1);
								break;
							}

							case 1: {
								ImGui::Text("%.2f", m_bpms[row].second);
								break;
							}

							case 2: {
								ImGui::Text("%.2f", 0.25);
								break;
							}
						}
					}
				}

				ImGui::EndTable();
			}

			ImGui::EndChild();
		}

		/*if (ImGui::Button("Close###close_pop_up_window", MathUtil::ScaleVec2(100, 25))) {
			ImGui::CloseCurrentPopup();
		}*/

		ImGui::EndPopup();
	}

	if (change_difficulty) {
		LoadDifficulty(difficulty_index);
	}
}

bool EditorScene::Attach() {
	m_exit = false;
	m_autoscroll = false;
	m_currentTime = 0;

	GameWindow* wnd = GameWindow::GetInstance();
	m_oldBufferSize.x = (float)wnd->GetBufferWidth();
	m_oldBufferSize.y = (float)wnd->GetBufferHeight();

	GameAudioSampleCache::Dispose();

	wnd->ResizeBuffer(1280, 720);

	{
		std::string songId = EnvironmentSetup::Get("Key");
		std::filesystem::path file;

		if (songId.size() > 0) {
			file = Configuration::Load("Music", "Folder");
			file /= "o2ma" + songId + ".ojn";
		}

		try {
			m_ojn.reset();
			m_ojn = std::make_unique<O2::OJN>();
			m_ojn->Load(file);

			int diffIndex = EnvironmentSetup::GetInt("Difficulty");
			LoadDifficulty(diffIndex);
		}
		catch (std::runtime_error& e) {
			MsgBox::Show("Failed", "Error", "Failed to load " + file.string() + ":\n" + e.what());
		}
	}

	m_measureGridSize = 16, m_measureGridSeparator = 4;

	m_ready = true;
	SceneManager::DisplayFade(0, [] {});
	return true;
}

bool EditorScene::Detach() {
	GameWindow::GetInstance()->ResizeBuffer((int)m_oldBufferSize.x, (int)m_oldBufferSize.y);

	m_notes.clear();
	m_samples.clear();

	for (auto& it : m_tracked_audio_sample) {
		delete it.second;
	}

	m_tracked_audio_sample.clear();
	m_lines.clear();
	m_majorLines.clear();

	m_ready = false;
	AudioManager::GetInstance()->RemoveAll();

	return true;
}

void EditorScene::PlaySample(int index) {
	if (index == -1) {
		return;
	}

	if (m_audio_sample.find(index) == m_audio_sample.end()) {
		return;
	}

	if (m_tracked_audio_sample[index]) {
		m_tracked_audio_sample[index]->Stop();
		delete m_tracked_audio_sample[index];
		m_tracked_audio_sample[index] = nullptr;
	}

	auto sample = m_audio_sample[index]->CreateChannel().release();
	sample->Play();

	m_tracked_audio_sample[index] = sample;
}

void EditorScene::StopSample() {
	for (auto& sample : m_tracked_audio_sample) {
		if (sample.second) {
			sample.second->Stop();
			delete sample.second;
			sample.second = nullptr;
		}
	}

	m_tracked_audio_sample.clear();
}

void EditorScene::LoadDifficulty(int idx) {
	m_notes.clear();
	m_samples.clear();
	m_notes.clear();
	m_bpms.clear();

	m_autoscroll = false;
	StopSample();

	m_audio_sample.clear();

	AudioManager::GetInstance()->RemoveAll();
	m_currentDifficulty = idx;

	auto& difficulty = m_ojn->Difficulties[idx];

	for (auto& note : difficulty.Notes) {
		INote n = {};
		n.Position = note.Position;
		n.SampleRefId = note.SampleRefId;
		n.Channel = note.Channel;
		n.IsLN = note.EndPosition >= 0;

		if (n.Position < 0) {
			continue;
		}

		m_notes.push_back(n);

		if (note.EndPosition >= 0) {
			INote n2 = {};
			n2.Position = note.EndPosition;
			n2.SampleRefId = note.SampleRefId;
			n2.Channel = note.Channel;
			n2.IsLN = true;
			n2.Vol = note.Volume;
			n2.Pan = note.Pan;

			m_notes.push_back(n2);
		}
	}

	for (auto& bpm : difficulty.Timings) {
		INote n = {};
		n.Position = bpm.Position;
		n.IsBPM = true;
		n.BPMValue = bpm.BPM;
		n.Channel = 1;

		assert(n.Position != -1);

		m_bpms.push_back({ bpm.Time, bpm.BPM });
		m_notes.push_back(n);
	}

	for (auto& length : difficulty.MeasureLenghts) {
		INote n = {};
		n.Position = length.Position;
		n.IsBPM = true;
		n.BPMValue = length.BPM;
		n.Channel = 0;

		m_notes.push_back(n);
	}

	for (auto& sample : difficulty.AutoSamples) {
		INote n = {};
		n.Position = sample.Position;
		n.SampleRefId = sample.SampleRefId;
		n.Channel = sample.Channel;

		assert(n.Position != -1);

		m_notes.push_back(n);
	}

	for (auto& file : difficulty.Samples) {
		m_samples.push_back(file);

		m_audio_sample[file.RefValue] = nullptr;

		bool result = AudioManager::GetInstance()->CreateSample(
			std::to_string(file.RefValue),
			file.AudioData.data(),
			file.AudioData.size(),
			&m_audio_sample[file.RefValue]);

		if (!result) {
			MsgBox::Show("AudioFailed", "Error", "Failed to load sample: " + std::to_string(file.RefValue));
			break;
		}
	}

	std::sort(m_notes.begin(), m_notes.end(), [](auto& n1, auto& n2) {
		return n1.Position < n2.Position;
	});
}
