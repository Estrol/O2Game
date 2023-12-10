#include "Texture/UDim2.h"

UDim2::UDim2()
{
    X = {};
    Y = {};
}

UDim2::UDim2(UDim x, UDim y)
{
    X = x;
    Y = y;
}

UDim2::UDim2(double xScale, double xOffset, double yScale, double yOffset)
{
    X = UDim(xScale, xOffset);
    Y = UDim(yScale, yOffset);
}

UDim2 UDim2::fromScale(double x, double y)
{
    return UDim2({ x, 0 }, { y, 0 });
}

UDim2 UDim2::fromOffset(double x, double y)
{
    return UDim2({ 0, x }, { 0, y });
}

UDim2 UDim2::Lerp(UDim2 destionation, float alpha)
{
    return { this->X.Lerp(destionation.X, alpha), this->Y.Lerp(destionation.Y, alpha) };
}

UDim2 UDim2::operator+(UDim2 const &udim2)
{
    return UDim2(this->X + udim2.X, this->Y + udim2.Y);
}

UDim2 UDim2::operator-(UDim2 const &udim2)
{
    return UDim2(this->X - udim2.X, this->Y - udim2.Y);
}

bool UDim2::operator==(UDim2 const &udim2)
{
    return (this->X == udim2.X) && (this->Y == udim2.Y);
}

bool UDim2::operator<=(UDim2 const &udim2)
{
    return (this->X <= udim2.X) && (this->Y <= udim2.Y);
}

bool UDim2::operator>=(UDim2 const &udim2)
{
    return (this->X >= udim2.X) && (this->Y >= udim2.Y);
}
