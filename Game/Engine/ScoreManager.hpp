#pragma once
#include "NoteResult.hpp"
#include <functional>

constexpr int kMaxJamGauge = 100;
constexpr int kMaxLife = 100;

struct NoteHitInfo {
	NoteResult Result;
	double TimeToEnd;
	bool IsRelease;
	bool Ignore;
	int Type;
};

class ScoreManager {
public:
	ScoreManager();
	~ScoreManager();

	void OnHit(NoteHitInfo info);
	void OnLongNoteHold(HoldResult result);
	void ListenHit(std::function<void(NoteHitInfo)>);
	void ListenJam(std::function<void(int)>);
	void ListenLongNote(std::function<void()>);

	int GetPills() const;
	int GetLife() const;
	int GetJamGauge() const;
	std::tuple<int, int, int, int, int, int, int, int, int, int, int> GetScore() const;
private:
	void AddLife(int sz);

	int m_score;
	int m_cool;
	int m_good;
	int m_bad;
	int m_miss;
	int m_jamCombo;
	int m_maxJamCombo;
	int m_combo;
	int m_maxCombo;

	int m_numOfPills;
	int m_coolCombo;

	int m_jamGauge;
	int m_life;

	int m_lnCombo;
	int m_lnMaxCombo;

	std::function<void(NoteHitInfo)> m_callback;
	std::function<void()> m_lncallback;
	std::function<void(int)> m_jamCallback;
};