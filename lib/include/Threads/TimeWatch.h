#ifndef __TIMEWATCH_H_
#define __TIMEWATCH_H_

class TimeWatch
{
public:
    TimeWatch();
    ~TimeWatch();

    double Tick();
    void SetTickRate(double tickRate);

private:
    double m_LastTick;
    double m_CurTick;
    double m_TickRate;
};

#endif