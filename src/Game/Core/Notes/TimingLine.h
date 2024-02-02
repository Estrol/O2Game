/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once

class RhythmEngine;
class Sprite;
class Texture2D;
class Image;

struct TimingLineDesc
{
    double StartTime;
    double Offset;
    int    ImagePos;
    int    ImageSize;

    RhythmEngine *Engine;
};

class TimingLine
{
public:
    TimingLine();
    ~TimingLine();

    void   Load(TimingLineDesc desc);
    void   Update(double delta);
    void   Draw(double delta);
    double GetOffset() const;
    double GetStartTime() const;
    double GetTrackPosition() const;

private:
    double m_startTime, m_offset, m_currentTrackPosition;
    int    m_imagePos, m_imageSize;

    Sprite       *m_line;
    RhythmEngine *m_engine;
    Image        *m_image;
};