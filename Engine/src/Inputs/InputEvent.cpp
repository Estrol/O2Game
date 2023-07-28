#include "Inputs/InputEvent.h"

bool InputEvent::IsActive() {
	return m_activated;
}

void InputEvent::Disconnect() {
	if (m_activated) {
		m_callback(m_inputIndex);
		m_activated = false;
	}
}

void InputEvent::CreateCallback(m_event_callback callback, int index) {
	m_callback = callback;
	m_inputIndex = index;
	m_activated = true;
}
