/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __ACCMULATEDTIMEWATCH_H_
#define __ACCMULATEDTIMEWATCH_H_

class AccurateTimeWatch
{
public:
    AccurateTimeWatch(float fixedStep = 1.0f / 60.0f);
    ~AccurateTimeWatch();

    void   Update(double delta);
    bool   Tick();
    double GetFixedStep() const { return m_FixedStep; }

private:
    double m_AccumulatedTime;
    double m_FixedStep;
};

#endif