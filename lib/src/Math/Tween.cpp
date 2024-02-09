#include <Math/Tween.h>

const float M_PI = 3.14159265358979323846f;

Tween::Tween(UDim2 start, UDim2 end, float duration, TweenType type)
    : Start(start), End(end), Duration(duration), Type(type), Time(0)
{
}

UDim2 Tween::Update(float time)
{
    Time += time;

    if (Time > Duration) {
        Time = Duration;

        return End;
    }

    float t = Time / Duration;
    switch (Type) {
        case TweenType::Linear:
            return Start.Lerp(End, t);
        case TweenType::Quadratic:
            return Start.Lerp(End, t * t);
        case TweenType::Cubic:
            return Start.Lerp(End, t * t * t);
        case TweenType::Quartic:
            return Start.Lerp(End, t * t * t * t);
        case TweenType::Quintic:
            return Start.Lerp(End, t * t * t * t * t);
        case TweenType::Sinusoidal:
            return Start.Lerp(End, 1 - cosf(t * M_PI / 2));
        case TweenType::Exponential:
            return Start.Lerp(End, powf(2, 10 * (t - 1)));
        case TweenType::Circular:
            return Start.Lerp(End, 1 - sqrtf(1 - t * t));
        case TweenType::Elastic:
            return Start.Lerp(End, 1 - sinf((t * M_PI * 2 - M_PI / 2) * 10) / 2);
        case TweenType::Back:
            return Start.Lerp(End, t * t * (2.70158f * t - 1.70158f));
        case TweenType::Bounce:
            if (t < 1 / 2.75) {
                return Start.Lerp(End, 7.5625f * t * t);
            } else if (t < 2 / 2.75f) {
                t -= 1.5f / 2.75f;
                return Start.Lerp(End, 7.5625f * t * t + 0.75f);
            } else if (t < 2.5 / 2.75f) {
                t -= 2.25f / 2.75f;
                return Start.Lerp(End, 7.5625f * t * t + 0.9375f);
            } else {
                t -= 2.625f / 2.75f;
                return Start.Lerp(End, 7.5625f * t * t + 0.984375f);
            }
        default:
            return Start.Lerp(End, t);
    }
}

void Tween::Reset()
{
    Time = 0;
}

bool Tween::IsFinished() const
{
    return Time >= Duration;
}