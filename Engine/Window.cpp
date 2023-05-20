#include "Window.hpp"
#include "Renderer.hpp"
#include <SDL2/SDL_syswm.h>
#include "Imgui/imgui_impl_sdl2.h"
#include <codecvt>

Window::Window() {
	m_window = nullptr;

	m_width = 0;
	m_height = 0;
	m_bufferWidth = 0;
	m_bufferHeight = 0;

	m_mainTitle = "";
	m_subTitle = "";
}

Window::~Window() {
	if (m_window != nullptr) {
		Destroy();
	}
}

Window* Window::s_instance = nullptr;

bool Window::Create(RendererMode mode, std::string title, int width, int height, int bufferWidth, int bufferHeight) {
	if (m_window != nullptr) {
		return false;
	}

	m_width = width;
	m_height = height;
	m_bufferWidth = bufferWidth;
	m_bufferHeight = bufferHeight;
	m_mainTitle = title;

	uint32_t flags = SDL_WINDOW_SHOWN;
	m_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

	if (m_window == nullptr) {
		MessageBoxA(NULL, SDL_GetError(), "EstEngine Error", MB_ICONERROR);

		return false;
	}
	
	return true;
}

bool Window::Destroy() {
	if (m_window == nullptr) {
		return true;
	}

	SDL_DestroyWindow(m_window);
	m_window = nullptr;

	return true;
}

SDL_Window* Window::GetWindow() const {
	return m_window;
}

HWND Window::GetHandle() const {
	SDL_SysWMinfo wmInfo{};
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_window, &wmInfo);
	return wmInfo.info.win.window;
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

void Window::SetWindowTitle(std::string& title) {
	m_mainTitle = title;

	std::u8string u8title(m_mainTitle.begin(), m_mainTitle.end());
	std::u8string u8sub(m_subTitle.begin(), m_subTitle.end());
	std::u8string combined = u8title + (u8sub.empty() ? u8sub : u8" - " + u8sub);

#if _MSC_VER
	std::wstring _title = UTF8_to_wchar(combined.c_str());
	SetWindowTextW(GetHandle(), (LPCWSTR)_title.c_str());
#else
	if (m_subTitle.empty()) {
		SDL_SetWindowTitle(m_window, (const char*)u8title.c_str());
	}
	else {
		SDL_SetWindowTitle(m_window, (const char*)(u8title + u8" - " + u8sub).c_str());
	}
#endif
}

void Window::SetWindowSubTitle(std::string& subTitle) {
	m_subTitle = subTitle;

	std::u8string u8title(m_mainTitle.begin(), m_mainTitle.end());
	std::u8string u8sub(m_subTitle.begin(), m_subTitle.end());

	std::u8string combined = u8title + (u8sub.empty() ? u8sub : u8" - " + u8sub);

#if _MSC_VER
	std::wstring _title = UTF8_to_wchar(combined.c_str());
	SetWindowTextW(GetHandle(), (LPCWSTR)_title.c_str());
#else
	if (m_subTitle.empty()) {
		SDL_SetWindowTitle(m_window, (const char*)u8title.c_str());
	}
	else {
		SDL_SetWindowTitle(m_window, (const char*)(u8title + u8" - " + u8sub).c_str());
	}
#endif
}

int Window::GetWidth() const {
	return m_width;
}

int Window::GetHeight() const {
	return m_height;
}

int Window::GetBufferWidth() const {
	return m_bufferWidth;
}

int Window::GetBufferHeight() const {
	return m_bufferHeight;
}

float Window::GetWidthScale() {
	return static_cast<float>(m_width) / static_cast<float>(m_bufferWidth);
}

float Window::GetHeightScale() {
	return static_cast<float>(m_height) / static_cast<float>(m_bufferHeight);
}

Window* Window::GetInstance() {
	if (s_instance == nullptr) {
		s_instance = new Window();
	}

	return s_instance;
}

void Window::Release() {
	if (s_instance != nullptr) {
		delete s_instance;
		s_instance = nullptr;
	}
}
