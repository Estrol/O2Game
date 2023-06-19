#include "../Engine/Game.hpp"
#include "../Engine/Scene.hpp"
#include "../Engine/Imgui/imgui.h"
#include "../Engine/Imgui/imguiUtil.hpp"
#include "../Engine/MathUtils.hpp"
#include "../Engine/AudioManager.hpp"
#include "../Engine/AudioSample.hpp"

#include "../Game/Data/OJM.hpp";
#include "../Game/Data/OJM.cpp";

#include "../Game/Data/ojn.h"
#include "../Game/Data/ojn.cpp"

bool isWithinRange(int point, int minRange, int maxRange) {
	return (point >= minRange && point <= maxRange);
}

bool isCollision(int top, int bottom, int min, int max) {
	// Check if the top value of the rectangle is within the range
	if (top >= min && top <= max) {
		return true;  // Collision detected
	}

	// Check if the bottom value of the rectangle is within the range
	if (bottom >= min && bottom <= max) {
		return true;  // Collision detected
	}

	// Check if the range is completely inside the rectangle
	if (top <= min && bottom >= max) {
		return true;  // Collision detected
	}

	// No collision detected
	return false;
}

struct Note {
	double Time;
	double EndTime;
	int Sample;
	int Lane;

	float BPM;
	bool isBPM = false;
};

class Main : public Scene {
public:
	double noteSpeed = 0.5;

	std::vector<Note> m_notes;
	std::vector<double> m_measure_lines;
	std::unordered_map<int, AudioSample*> m_samples;

	std::unordered_map<int, AudioSampleChannel*> m_tracked_sample;

	std::vector<double> m_timing_marker;

	double maxTime = 0;
	std::filesystem::path path = "";

	Main() : Scene() {
		OJN ojn;
		path = "E:\\Games\\O2JamINT\\Musi1\\o2ma898.ojn";

		ojn.Load(path);

		maxTime = ojn.Difficulties[2].AudioLength;

		for (auto& note : ojn.Difficulties[2].Notes) {
			Note n;
			n.Time = note.StartTime;
			n.Lane = note.LaneIndex + 2;
			n.EndTime = -1;
			if (note.EndTime != 0) {
				n.EndTime = note.EndTime;
			}

			n.Sample = note.SampleRefId;
			m_notes.push_back(n);
		}

		for (auto& note : ojn.Difficulties[2].Measures) {
			m_measure_lines.push_back(note);
		}

		for (auto& note : ojn.Difficulties[2].AutoSamples) {
			Note n;
			n.Time = note.StartTime;
			n.Lane = 9;
			n.EndTime = -1;
			n.Sample = note.SampleRefId;

			m_notes.push_back(n);
		}

		for (auto& sample : ojn.Difficulties[2].Samples) {
			m_samples[sample.RefValue] = nullptr;

			AudioManager::GetInstance()->CreateSample(
				std::to_string(sample.RefValue),
				sample.AudioData.data(),
				sample.AudioData.size(),
				&m_samples[sample.RefValue]);
		}

		for (auto& bpm : ojn.Difficulties[2].Timings) {
			Note n;
			n.Time = bpm.Time;
			n.Lane = 1;
			n.EndTime = -1;
			n.Sample = -1;

			n.BPM = bpm.BPM;
			n.isBPM = true;

			m_notes.push_back(n);
		}
	}

	~Main() override {

	}

	void PlaySample(int index) {
		if (index == -1) {
			return;
		}

		if (m_samples.find(index) == m_samples.end()) {
			return;
		}

		std::cout << "Playing sample: " << index << std::endl;

		if (m_tracked_sample[index]) {
			m_tracked_sample[index]->Stop();
			delete m_tracked_sample[index];
			m_tracked_sample[index] = nullptr;
		}

		auto sample = m_samples[index]->CreateChannel().release();
		sample->Play();

		m_tracked_sample[index] = sample;
	}

	void StopSample() {
		for (auto& sample : m_tracked_sample) {
			if (sample.second) {
				sample.second->Stop();
				delete sample.second;
				sample.second = nullptr;
			}
		}

		m_tracked_sample.clear();
	}

	void Render(double delta) override {
		ImguiUtil::NewFrame();

		auto flags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_MenuBar;

		ImGui::SetNextWindowSize(ImVec2(1920, 1080));
		ImGui::SetNextWindowPos(ImVec2(0, 0));

		bool previous = autoScroll;
		bool bPlay = false;

		if (ImGui::Begin("FullscreenWindow",
			nullptr,
			flags
		)) {
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("New")) {

					}

					if (ImGui::MenuItem("Open File")) {

					}

					if (ImGui::MenuItem("Close File")) {

					}

					if (ImGui::MenuItem("Exit")) {
						SceneManager::GetInstance()->StopGame();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Test")) {
					if (ImGui::MenuItem("Play in U2OC")) {
						bPlay = true;
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Editor")) {
					if (ImGui::MenuItem("Auto scroll")) {
						autoScroll = !autoScroll;
					}

					if (ImGui::MenuItem("Reset to beginning")) {
						if (Time != 0) {
							StopSample();
						}

						Time = 0;
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			// get current window draw list
			auto drawList = ImGui::GetWindowDrawList();

			float row_column_width = 65.0f;
			int max_rows = 7 + 2 + 1;
			
			std::vector<double> lanePos;

			std::vector<std::pair<ImVec2, ImVec2>> toDraw;

			double mid_x = 0;
			for (int i = 0; i < max_rows + 1; i++) {
				lanePos.push_back(i * row_column_width);

				toDraw.push_back({ MathUtil::ScaleVec2(i * row_column_width, 0), MathUtil::ScaleVec2(i * row_column_width, 720) });

				// check if mid
				if (i == max_rows / 2) {
					// find next row then assign mix_x
					mid_x = (i + 0.5) * row_column_width;
				}
			}

			// draw rectangle at (0, 0) with size (100, 100) and color red
			ImColor bg(76, 84, 88);
			ImVec2 size = MathUtil::ScaleVec2(lanePos.back(), 720);

			drawList->AddRectFilled(ImVec2(0, 0), size, bg);
			drawList->AddRect(ImVec2(0, 0), size, ImColor(255, 255, 255));

			// draw lanes
			for (int i = 0; i < lanePos.size(); i++) {
				drawList->AddLine(MathUtil::ScaleVec2(lanePos[i], 0), MathUtil::ScaleVec2(lanePos[i], 720), ImColor(255, 255, 255));
			}

			ImVec2 hitPosVec1 = MathUtil::ScaleVec2(0, 720 * 0.8);
			ImVec2 hitPosVec2 = MathUtil::ScaleVec2(lanePos.back(), 720 * 0.8);

			const double BeatPerSecs = 240000.0;
			const double BPM = 240.0;

			int currentMeasure = 0;
			int maxMeasure = 300;
			double currentTimer = 0;

			std::vector<double> measure_time = {};
			if (m_measure_lines.size() == 0) {
				measure_time.push_back(currentTimer);

				for (int i = 0; i < maxMeasure; i++) {
					currentTimer += BeatPerSecs / BPM;

					measure_time.push_back(currentTimer);
				}
			}
			else {
				measure_time = m_measure_lines;
			}

			Time += (autoScroll ? delta * 1000 : 0);
			if (Time > maxTime) {
				autoScroll = false;
			}

			int TimeCurrentMeasure = 0;

			for (int i = 0; i < measure_time.size(); i++) {
				double time = measure_time[i] * 100.0;
				double nextTime = 0;
				if (i + 1 < measure_time.size()) {
					nextTime = measure_time[i + 1] * 100.0;
				}

				// hitPosition + ((initialTrackPos - offset) * (upscroll ? noteSpeed : -noteSpeed) / 100);
				double yPos = (720 * 0.8) + ((time - (Time * 100)) * -noteSpeed / 100);
				double yPos2 = (720 * 0.8) + ((nextTime - (Time * 100)) * -noteSpeed / 100);
				
				ImVec2 hitPosVec1 = MathUtil::ScaleVec2(0, yPos);
				ImVec2 hitPosVec2 = MathUtil::ScaleVec2(lanePos.back(), yPos);

				// draw line
				drawList->AddLine(hitPosVec1, hitPosVec2, ImColor(255, 255, 255), 1.0f);

				// find middle pos of yPos and yPos 2 then draw number with scale 25
				if (nextTime != 0) {
					// check if Time is between time and nextTime
					if (Time * 100 >= time && Time * 100 <= nextTime) {
						TimeCurrentMeasure = i;
					}

					ImVec2 middlePos = MathUtil::ScaleVec2(mid_x, (yPos + yPos2) / 2);
					drawList->AddText(NULL, 32, middlePos, ImColor(255, 255, 255), std::to_string(i).c_str());
				}
			}

			int noteSize = 10;

			/*
				0-1: Blue
				2: white
				3: blue-light
				4: white
				5: yellow
				6: white
				7: blue-light
				8: white
				9: yellow-dark
			*/
			std::vector<ImColor> laneColor = {
				ImColor(0, 0, 255),
				ImColor(0, 0, 255),
				ImColor(225, 242, 189),
				ImColor(99, 147, 224),
				ImColor(225, 242, 189),
				ImColor(224, 214, 99),
				ImColor(225, 242, 189),
				ImColor(99, 147, 224),
				ImColor(225, 242, 189),
				ImColor(255, 255, 0)
			};

			for (auto& note : m_notes) {
				double time = note.Time * 100;
				double yPos = ((720 * 0.8) + ((time - (Time * 100)) * -noteSpeed / 100));
				yPos -= noteSize;

				ImColor color = laneColor[note.Lane];

				ImVec2 pos = MathUtil::ScaleVec2(lanePos[note.Lane], yPos);
				ImVec2 size = MathUtil::ScaleVec2(lanePos[note.Lane + 1], yPos + noteSize);

				ImVec2 pos1 = MathUtil::ScaleVec2(lanePos[note.Lane], yPos);
				ImVec2 size1 = MathUtil::ScaleVec2(lanePos[note.Lane + 1], yPos + noteSize);

				if (note.EndTime != -1) {
					double Endtime = note.EndTime * 100;

					double ayPos = ((720 * 0.8) + ((Endtime - (Time * 100)) * -noteSpeed / 100));
					ayPos -= noteSize;

					pos1 = MathUtil::ScaleVec2(lanePos[note.Lane], ayPos);
					size1 = MathUtil::ScaleVec2(lanePos[note.Lane + 1], ayPos + noteSize);

					auto lnColor = color;
					// multiply it by 0.8
					lnColor.Value.x *= 0.8;
					lnColor.Value.y *= 0.8;
					lnColor.Value.z *= 0.8;
					

					drawList->AddRectFilled(pos1, size, lnColor);

					drawList->AddRectFilled(pos1, size1, color);
				}

				drawList->AddRectFilled(pos, size, color);

				ImVec2 pos5 = MathUtil::ScaleVec2(lanePos[note.Lane], yPos);
				if (note.isBPM) {
					// draw text: #BPM %.2%
					drawList->AddText(NULL, 16, pos5, ImColor(255, 255, 255), ("#BPM" + std::to_string(note.BPM)).c_str());
				}
				else {
					if (note.Sample != -1) {
						drawList->AddText(NULL, 16, pos5, ImColor(0, 0, 0), ("#" + std::to_string(note.Sample)).c_str());
					}
				}

				// calculate difference between Time and note.Time within 1-2 seconds
				double diff = std::abs(Time - note.Time);
				if (diff < 25 && autoScroll && note.Time <= Time) {
					PlaySample(note.Sample);
				}
			}

			std::vector<std::string> lane_tex = {
				"SIGNATURE", "BPM", "LANE_1", "LANE_2", "LANE_3", "LANE_4", "LANE_5", "LANE_6", "LANE_7", "AUTOSOUND"
			};

			for (int i = 0; i < lane_tex.size(); i++) {
				// calc text size
				ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(16, FLT_MAX, 0, lane_tex[i].c_str());
				float initialPos = lanePos[i] + (lanePos[i + 1] - lanePos[i]) / 2;
				ImVec2 textPos = MathUtil::ScaleVec2(initialPos - textSize.x / 2, 15);

				drawList->AddText(NULL, 16, textPos, ImColor(255, 255, 255), lane_tex[i].c_str());
			}

			// draw line
			drawList->AddLine(hitPosVec1, hitPosVec2, ImColor(255, 255, 255), 5.0f);

			auto x_pos_d = lanePos.back();
			ImGui::SetCursorPosX(MathUtil::ScaleVec2(x_pos_d + 10, 0).x);
			if (ImGui::BeginChild("###a", MathUtil::ScaleVec2(400, 200), true, 0)) {
				ImGui::Text("Time: %.2f", Time);
				ImGui::Text("Note Speed: %.2f", noteSpeed);
				ImGui::Text("Measure: %d", TimeCurrentMeasure);

				if (ImGui::Button("Speed -")) {
					noteSpeed -= 0.01;
				}

				if (ImGui::Button("Speed +")) {
					noteSpeed += 0.01;
				}

				ImGui::EndChild();
			}

			// check imgui mouse scroll
			if (ImGui::GetIO().MouseWheel != 0 && !autoScroll) {
				Time = std::clamp(Time + ImGui::GetIO().MouseWheel * 100, 0.0, DBL_MAX);
			}

			// check space bar then toggle autoScroll
			if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
				autoScroll = !autoScroll;
			}

			if (previous && !autoScroll) {
				StopSample();
			}

			ImGui::End();
		}

		if (bPlay) {
			// How to start process using win32?
			// https://stackoverflow.com/questions/15435994/how-to-start-process-using-win32
			
			STARTUPINFOA info = {};
			PROCESS_INFORMATION processInfo = {};

			std::string cmd = "cmd.exe /c start " + (std::filesystem::current_path() / "Game.exe").string() + " " + path.string() + " -a";
			char* writable = new char[cmd.size() + 1];
			ZeroMemory(writable, cmd.size() + 1);

			std::copy(cmd.begin(), cmd.end(), writable);

			if (!CreateProcessA(NULL, writable, NULL, NULL, FALSE, 0, NULL, NULL, &info, &processInfo)) {
				std::cout << "Error: " << GetLastError() << std::endl;
			}

			delete[] writable;
		}
	}

	void Update(double delta) override {
		
	}

	bool Attach() override {
		return true;
	}

	bool Detach() override {
		return true;
	}

	double Time = 0;
	bool autoScroll = false;
};

class Editor : public Game {
public:
	Editor() : Game() {
		
	}

	~Editor() {

	}

	bool Init() override {
		SetBufferSize(1280, 720);
		SetWindowSize(1920, 1080);
		SetRenderMode(RendererMode::DIRECTX11);

		bool result = Game::Init();
		if (result) {
			m_window->SetScaleOutput(true);

			SceneManager::AddScene(0, new Main);
			SceneManager::ChangeScene(0);
		}

		return result;
	}

	void Render(double delta) override {
		SceneManager::GetInstance()->Render(delta);
	}

	void Update(double delta) override {
		SceneManager::GetInstance()->Update(delta);
	}
};

int main() {
	Editor e;
	if (e.Init()) {
		e.Run(165);
	}
}