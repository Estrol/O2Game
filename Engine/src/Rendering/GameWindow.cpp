#include "Rendering/Window.h"
#include "Rendering/Renderer.h"
#include <SDL2/SDL_syswm.h>
#include "../Data/Imgui/imgui_impl_sdl2.h"
#include <codecvt>
#include "MsgBox.h"

GameWindow::GameWindow() {
	m_window = nullptr;
	m_resizeRenderer = false;

	m_width = 0;
	m_height = 0;
	m_bufferWidth = 0;
	m_bufferHeight = 0;

	m_mainTitle = "";
	m_subTitle = "";
}

GameWindow::~GameWindow() {
	if (m_window != nullptr) {
		Destroy();
	}
}

GameWindow* GameWindow::s_instance = nullptr;

bool GameWindow::Create(RendererMode mode, std::string title, int width, int height, int bufferWidth, int bufferHeight) {
	if (m_window != nullptr) {
		return false;
	}

	m_width = width;
	m_height = height;
	m_bufferWidth = bufferWidth;
	m_bufferHeight = bufferHeight;
	m_mainTitle = title;

	uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
	if (mode == RendererMode::OPENGL) {
		flags |= SDL_WINDOW_OPENGL;
	}
	if (mode == RendererMode::VULKAN) {
		flags |= SDL_WINDOW_VULKAN;
	}

	// check if width and height are same in current display
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	bool is_fullscreen = false;
	if (width == dm.w && height == dm.h) {
		is_fullscreen = true;
	}

	m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

	if (m_window == nullptr) {
		MsgBox::ShowOut(SDL_GetError(), "EstEngine Error");

		return false;
	}
	
	if (is_fullscreen) {
		SDL_SetWindowBordered(m_window, SDL_FALSE);
	}

	return true;
}

bool GameWindow::Destroy() {
	if (m_window == nullptr) {
		return true;
	}

	SDL_DestroyWindow(m_window);
	m_window = nullptr;

	return true;
}

void GameWindow::ResizeWindow(int width, int height) {
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);

	bool is_fullscreen = false;
	if (width == dm.w && height == dm.h) {
		is_fullscreen = true;
	}

	m_width = width;
	m_height = height;

	SDL_SetWindowSize(m_window, width, height);
	SDL_SetWindowFullscreen(m_window, is_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	SDL_SetWindowBordered(m_window, is_fullscreen ? SDL_FALSE : SDL_TRUE);
	
	if (!is_fullscreen) {
		SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}

	m_resizeRenderer = true;
}

void GameWindow::ResizeBuffer(int width, int height) {
	m_bufferWidth = width;
	m_bufferHeight = height;

	m_resizeRenderer = true;
}

bool GameWindow::ShouldResizeRenderer() {
	return m_resizeRenderer;
}

void GameWindow::HandleResizeRenderer() {
	m_resizeRenderer = false;
}

SDL_Window* GameWindow::GetWindow() const {
	return m_window;
}

std::wstring UTF8_to_wchar(const char8_t* in) {
	std::wstring out;
	unsigned int codepoint;
	while (*in != 0)
	{
		unsigned char ch = static_cast<unsigned char>(*in);
		if (ch <= 0x7f)
			codepoint = ch;
		else if (ch <= 0xbf)
			codepoint = (codepoint << 6) | (ch & 0x3f);
		else if (ch <= 0xdf)
			codepoint = ch & 0x1f;
		else if (ch <= 0xef)
			codepoint = ch & 0x0f;
		else
			codepoint = ch & 0x07;
		++in;
		if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
		{
			if (sizeof(wchar_t) > 2)
				out.append(1, static_cast<wchar_t>(codepoint));
			else if (codepoint > 0xffff)
			{
				codepoint -= 0x10000;
				out.append(1, static_cast<wchar_t>(0xd800 + (codepoint >> 10)));
				out.append(1, static_cast<wchar_t>(0xdc00 + (codepoint & 0x03ff)));
			}
			else if (codepoint < 0xd800 || codepoint >= 0xe000)
				out.append(1, static_cast<wchar_t>(codepoint));
		}
	}
	return out;
}

void GameWindow::SetWindowTitle(std::string& title) {
	m_mainTitle = title;

	SDL_SetWindowTitle(m_window, title.c_str());
}

void GameWindow::SetWindowSubTitle(std::string& subTitle) {
	m_subTitle = subTitle;
}

int GameWindow::GetWidth() const {
	return m_width;
}

int GameWindow::GetHeight() const {
	return m_height;
}

int GameWindow::GetBufferWidth() const {
	return m_bufferWidth;
}

int GameWindow::GetBufferHeight() const {
	return m_bufferHeight;
}

float GameWindow::GetWidthScale() {
	return static_cast<float>(m_width) / static_cast<float>(m_bufferWidth);
}

float GameWindow::GetHeightScale() {
	return static_cast<float>(m_height) / static_cast<float>(m_bufferHeight);
}

void GameWindow::SetScaleOutput(bool value) {
	m_scaleOutput = value;
}

bool GameWindow::IsScaleOutput() {
	return m_scaleOutput;
}

GameWindow* GameWindow::GetInstance() {
	if (s_instance == nullptr) {
		s_instance = new GameWindow();
	}

	return s_instance;
}

void GameWindow::Release() {
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
	}
}
