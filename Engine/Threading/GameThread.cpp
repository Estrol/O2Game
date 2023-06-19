#include "GameThread.hpp"

GameThread::GameThread() {
	m_run = false;
	m_background = false;
}

GameThread::~GameThread() {
	if (m_run) {
		Stop();
	}
}

void GameThread::Run(std::function<void()> callback, bool background) {
	m_main_cb = callback;
	m_run = true;
	m_background = background;

	if (background) {
		m_thread = std::thread([&] {
			while (m_run) {
				m_main_cb();

				if (m_queue_cb.size()) {
					std::function<void()> queue_cb = m_queue_cb.back();
					m_queue_cb.pop_back();

					queue_cb();
				}
			}
		});
	}
}

void GameThread::Update() {
	if (m_background) {
		return;
	}

	m_main_cb();
	
	if (m_queue_cb.size()) {
		std::function<void()> queue_cb = m_queue_cb.back();
		m_queue_cb.pop_back();

		queue_cb();
	}
}

void GameThread::QueueAction(std::function<void()> callback) {
	m_queue_cb.push_back(callback);
}

void GameThread::Stop() {
	if (m_background) {
		m_run = false;
		m_thread.join();
	}
}
