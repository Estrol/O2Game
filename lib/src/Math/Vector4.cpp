/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Math/Vector4.h>

Vector4::Vector4()
{
    X = Y = Z = W = 0;
}

Vector4::Vector4(double x, double y, double z, double w)
{
    X = x;
    Y = y;
    Z = z;
    W = w;
}

Vector4::Vector4(Vector2 xy, Vector2 zw)
{
    X = xy.X;
    Y = xy.Y;
    Z = zw.X;
    W = zw.Y;
}

Vector4 Vector4::Cross(Vector4 other)
{
    return Vector4(
        Y * other.Z - Z * other.Y,
        Z * other.X - X * other.Z,
        X * other.Y - Y * other.X,
        0);
}

double Vector4::Dot(Vector4 other)
{
    return X * other.X + Y * other.Y + Z * other.Z + W * other.W;
}

Vector4 Vector4::Lerp(Vector4 v, float alpha)
{
    return Vector4(
        X + alpha * (v.X - X),
        Y + alpha * (v.Y - Y),
        Z + alpha * (v.Z - Z),
        W + alpha * (v.W - W));
}

Vector4 Vector4::operator+(Vector4 const &vector)
{
    return Vector4(X + vector.X, Y + vector.Y, Z + vector.Z, W + vector.W);
}

Vector4 Vector4::operator-(Vector4 const &vector)
{
    return Vector4(X - vector.X, Y - vector.Y, Z - vector.Z, W - vector.W);
}

Vector4 Vector4::operator*(Vector4 const &vector)
{
    return Vector4(X * vector.X, Y * vector.Y, Z * vector.Z, W * vector.W);
}

Vector4 Vector4::operator/(Vector4 const &vector)
{
    return Vector4(X / vector.X, Y / vector.Y, Z / vector.Z, W / vector.W);
}

Vector4 Vector4::operator*(double const &number)
{
    return Vector4(X * number, Y * number, Z * number, W * number);
}

Vector4 Vector4::operator/(double const &number)
{
    return Vector4(X / number, Y / number, Z / number, W / number);
}

Vector4 Vector4::operator*(float const &number)
{
    return Vector4(X * number, Y * number, Z * number, W * number);
}

Vector4 Vector4::operator/(float const &number)
{
    return Vector4(X / number, Y / number, Z / number, W / number);
}

bool Vector4::operator==(Vector4 const &vector)
{
    return X == vector.X && Y == vector.Y && Z == vector.Z && W == vector.W;
}

bool Vector4::operator!=(Vector4 const &vector)
{
    return !(*this == vector);
}