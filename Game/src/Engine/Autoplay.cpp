#include "Autoplay.h"
#include "./Timing/TimingBase.h"

constexpr int kReleaseDelay = 25;

const double kBaseBPM = 240.0;
const double kMaxTicks = 192.0;
const double kNoteCoolHitRatio = 6.0;

NoteInfo *GetNextHitObject(std::vector<NoteInfo> &hitObject, int index)
{
    int lane = hitObject[index].LaneIndex;

    for (int i = index + 1; i < hitObject.size(); i++) {
        if (hitObject[i].LaneIndex == lane) {
            return &hitObject[i];
        }
    }

    return nullptr;
}

double CalculateReleaseTime(NoteInfo *currentHitObject, NoteInfo *nextHitObject)
{
    if (currentHitObject->Type == NoteType::HOLD) {
        return currentHitObject->EndTime;
    }

    double Time = currentHitObject->Type == NoteType::HOLD ? currentHitObject->EndTime
                                                           : currentHitObject->StartTime;

    bool canDelayFully = nextHitObject == nullptr ||
                         nextHitObject->StartTime > Time + kReleaseDelay;

    return Time + (canDelayFully ? kReleaseDelay : (nextHitObject->StartTime - Time) * 0.9);
}

std::vector<Autoplay::ReplayHitInfo> Autoplay::CreateReplay(Chart *chart)
{
    std::vector<Autoplay::ReplayHitInfo> result;

    TimingBase timingBase(chart->m_bpms, chart->m_svs, chart->InitialSvMultiplier);

    for (int i = 0; i < chart->m_notes.size(); i++) {
        auto &currentHitObject = chart->m_notes[i];
        auto  nextHitObject = GetNextHitObject(chart->m_notes, i);

        double HitTime = currentHitObject.StartTime;
        double ReleaseTime = CalculateReleaseTime(&currentHitObject, nextHitObject);

        double bpmAtHit = timingBase.GetBPMAt(HitTime);
        double bpmAtRelease = timingBase.GetBPMAt(ReleaseTime);

        int tries = 0;

        while (true) {
            double beat = kBaseBPM / kMaxTicks / bpmAtHit * 1000.0;
            double cool = beat * kNoteCoolHitRatio;
            double late_cool = -cool;

            double _HIT = currentHitObject.StartTime - HitTime;

            if (_HIT >= late_cool && _HIT <= cool) {
                break;
            }

            if (_HIT > cool) {
                HitTime++;
            } else {
                HitTime--;
            }

            if (++tries >= 500) {
                break;
            }
        }

        tries = 0;

        while (true) {
            double beat = kBaseBPM / kMaxTicks / bpmAtRelease * 1000.0;
            double cool = beat * kNoteCoolHitRatio;
            double late_cool = -cool;

            double _HIT = (currentHitObject.Type == NoteType::HOLD ? currentHitObject.EndTime : currentHitObject.StartTime) - ReleaseTime;

            if (_HIT >= late_cool && _HIT <= cool) {
                break;
            }

            if (_HIT > cool) {
                ReleaseTime++;
            } else {
                ReleaseTime--;
            }

            if (++tries >= 500) {
                break;
            }
        }

        result.push_back({ HitTime, (int)currentHitObject.LaneIndex, ReplayHitType::KEY_DOWN });
        result.push_back({ ReleaseTime, (int)currentHitObject.LaneIndex, ReplayHitType::KEY_UP });
    }

    return result;
}