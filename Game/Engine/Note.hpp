#pragma once
#include "../../Engine/Keys.h"
#include "../Resources/GameResources.hpp"
#include "NoteResult.hpp"

enum class NoteType : uint8_t;
class GameTrack;
class RhythmEngine;
class DrawableNote;
class DrawableTile;
class AudioSampleChannel;

enum class NoteState {
	NORMAL_NOTE,

	HOLD_PRE,
	HOLD_MISSED_ACTIVE,
	HOLD_ON_HOLDING,
	HOLD_PASSED,
	
	DO_REMOVE
};

struct NoteInfoDesc {
	DrawableNote* Image;
	NoteImageType ImageType;
	NoteImageType ImageBodyType;

	int KeysoundIndex;
	int Volume;
	int Pan;
	
	double StartTime;
	double EndTime;
	double StartBPM;
	double EndBPM;

	int Lane;
	NoteType Type;

	double InitialTrackPosition;
	double EndTrackPosition;
};

class Note {
public:
	Note(RhythmEngine* engine, GameTrack* track);
	~Note();

	void Load(NoteInfoDesc* desc);

	void Update(double delta);
	void Render(double delta);

	double GetInitialTrackPosition() const;
	double GetStartTime() const;
	double GetBPMTime() const;
	double GetHitTime() const;

	int GetKeysoundId() const;
	int GetKeyVolume() const;
	int GetKeyPan() const;
	NoteType GetType() const;

	std::tuple<bool, NoteResult> CheckHit();
	std::tuple<bool, NoteResult> CheckRelease();
	void OnHit(NoteResult result);
	void OnRelease(NoteResult result);

	void SetXPosition(int x);
	void SetDrawable(bool drawable);
	
	bool IsHoldEffectDrawable();
	bool IsDrawable();
	bool IsRemoveable();

	bool IsHeadHit();
	bool IsTailHit();

	void Release();

private:
	bool m_drawAble;
	bool m_removeAble;

	RhythmEngine* m_engine;
	GameTrack* m_track;

	DrawableNote* m_head;
	DrawableTile* m_body;
	DrawableNote* m_tail;

	NoteImageType m_imageType;
	NoteImageType m_imageBodyType;

	int m_laneOffset;
	double m_startTime;
	double m_endTime;
	double m_startBPM;
	double m_endBPM;

	int m_keysoundIndex;
	int m_lane;

	int m_keyVolume;
	int m_keyPan;

	NoteType m_type;
	NoteState m_state;
	NoteResult m_hitResult;

	double m_initialTrackPosition;
	double m_endTrackPosition;

	bool m_shouldDrawHoldEffect;
	bool m_didHitHead;
	bool m_didHitTail;
	bool m_ignore = true;

	double m_hitTime;
	double m_hitPos;
	double m_relPos;

	double m_lastScoreTime;
};