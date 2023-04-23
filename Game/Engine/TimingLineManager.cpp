#include "TimingLineManager.hpp"
#include "RhythmEngine.hpp"

TimingLineManager::TimingLineManager(RhythmEngine* engine) {
	m_engine = engine;
	m_timingLines = {};
	m_timingInfos = {};

	int mapLength = engine->GetAudioLength();
	auto bpms = engine->GetBPMs();

	double recycle = (300000.0 / 4.0) / engine->GetNotespeed();

	for (int i = 0; i < bpms.size(); i++) {
		double target = (i + 1) < bpms.size() ? bpms[i + 1].StartTime - 1 : mapLength;
		float signature = bpms[i].TimeSignature;
		float maxBPM = 9999;

		double msPerBeat = 60000.0f / (std::min)(std::abs(bpms[i].Value), maxBPM);
		double increment = signature * msPerBeat;

		if (increment <= 0) continue;

		double songPos = bpms[i].StartTime;
		while (songPos < target) {
			double offset = engine->GetPositionFromOffset(songPos);
			
			if (!(m_engine->GetTrackPosition() - offset > recycle 
				&& songPos < m_engine->GetGameAudioPosition())) {
				
				TimingLineDesc desc = {};
				desc.Engine = engine;
				desc.StartTime = songPos;
				desc.Offset = offset;
				desc.ImagePos = 3;
				desc.ImageSize = 193;

				m_timingInfos.push(desc);
			}

			songPos += increment;
		}
	}
}

TimingLineManager::TimingLineManager(RhythmEngine* engine, std::vector<double> list) {
	m_engine = engine;
	m_timingLines = {};
	m_timingInfos = {};

	for (int i = 0; i < list.size(); i++) {
		double offset = m_engine->GetPositionFromOffset(list[i]);

		TimingLineDesc desc = {};
		desc.Engine = engine;
		desc.StartTime = list[i];
		desc.Offset = offset;
		desc.ImagePos = 3;
		desc.ImageSize = 193;
		
		m_timingInfos.push(desc);
	}
}

TimingLineManager::~TimingLineManager() {
	for (int i = 0; i < m_timingInfos.size(); i++) {
		m_timingInfos.pop();
	}

	for (int i = 0; i < m_timingLines.size(); i++) {
		delete m_timingLines.front();
		m_timingLines.pop();
	}
}

void TimingLineManager::Init() {
	double recycle = (300000.0 / 4.0) / m_engine->GetNotespeed();

	while (m_timingInfos.size() > 0) {
		auto& info = m_timingInfos.front();
		
		if (m_engine->GetTrackPosition() - info.Offset > recycle) {
			m_timingInfos.pop();
		}
		else if (m_engine->GetTrackPosition() - info.Offset < recycle) {
			TimingLine* t = new TimingLine();
			t->Load(&info);
			m_timingLines.push(t);
			m_timingInfos.pop();
		}
		else {
			break;
		}
	}
}

void TimingLineManager::Update(double delta) {
	double recycle = (300000.0 / 4) / m_engine->GetNotespeed();

	if (m_timingLines.size() > 0) {
		for (auto it = m_timingLines.begin(); it != m_timingLines.end(); it++) {
			(*it)->Update(delta);
		}
	}

	while (m_timingLines.size() > 0
		&& m_timingLines.front()->GetTrackPosition() > recycle
		&& m_timingLines.front()->GetStartTime() < m_engine->GetGameAudioPosition()) {
		TimingLine* t = m_timingLines.front();
		m_timingLines.pop();

		if (m_timingInfos.size() > 0) {
			TimingLineDesc desc = m_timingInfos.front();
			m_timingInfos.pop();

			t->Load(&desc);
			m_timingLines.push(t);
		}
		else {
			delete t;
		}
	}

	while (m_timingInfos.size() > 0 
		&& m_engine->GetTrackPosition() - m_timingInfos.front().Offset > m_engine->GetPrebufferTiming()) {
		TimingLine* t = new TimingLine();
		t->Load(&m_timingInfos.front());
		m_timingLines.push(t);
		m_timingInfos.pop();
	}
}

void TimingLineManager::Render(double delta) {
	if (m_timingLines.size() > 0) {
		for (auto it = m_timingLines.begin(); it != m_timingLines.end(); it++) {
			(*it)->Render(delta);
		}
	}
}