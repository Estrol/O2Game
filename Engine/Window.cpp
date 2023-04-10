#include "Window.hpp"
#include "Renderer.hpp"
#include <SDL2/SDL_syswm.h>

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

void Window::SetWindowTitle(std::string& title) {
	m_mainTitle = title;

	if (m_subTitle.empty()) {
		SDL_SetWindowTitle(m_window, m_mainTitle.c_str());
	}
	else {
		SDL_SetWindowTitle(m_window, (m_mainTitle + " - " + m_subTitle).c_str());
	}
}

void Window::SetWindowSubTitle(std::string& subTitle) {
	m_subTitle = subTitle;

	if (m_subTitle.empty()) {
		SDL_SetWindowTitle(m_window, m_mainTitle.c_str());
	}
	else {
		SDL_SetWindowTitle(m_window, (m_mainTitle + " - " + m_subTitle).c_str());
	}
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
