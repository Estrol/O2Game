#include "AutoReplay.hpp"

constexpr int kReleaseDelay = 20;

NoteInfo* GetNextHitObject(std::vector<NoteInfo>& hitObject, int index) {
	int lane = hitObject[index].LaneIndex;

	for (int i = index + 1; i < hitObject.size(); i++) {
		if (hitObject[i].LaneIndex == lane) {
			return &hitObject[i];
		}
	}

	return nullptr;
}

double CalculateReleaseTime(NoteInfo* currentHitObject, NoteInfo* nextHitObject) {
	if (currentHitObject->Type == NoteType::HOLD) {
		return currentHitObject->EndTime;
	}

	double Time = currentHitObject->Type == NoteType::HOLD ? currentHitObject->EndTime 
		: currentHitObject->StartTime; 

	bool canDelayFully = nextHitObject == nullptr ||
		nextHitObject->StartTime > Time + kReleaseDelay;

	return Time + (canDelayFully ? kReleaseDelay : (nextHitObject->StartTime - Time) * 0.9 );
}

std::vector<ReplayHitInfo> AutoReplay::CreateReplay(Chart* chart) {
	std::vector<ReplayHitInfo> result;

	for (int i = 0; i < chart->m_notes.size(); i++) {
		auto& currentHitObject = chart->m_notes[i];
		auto nextHitObject = GetNextHitObject(chart->m_notes, i);

		double HitTime = currentHitObject.StartTime;
		double ReleaseTime = CalculateReleaseTime(&currentHitObject, nextHitObject);

		result.push_back({ HitTime, (int)currentHitObject.LaneIndex, ReplayHitType::KEY_DOWN });
		result.push_back({ ReleaseTime, (int)currentHitObject.LaneIndex, ReplayHitType::KEY_UP });
	}

	return result;
}
