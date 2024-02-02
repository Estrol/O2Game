/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Math/Vector3.h>

Vector3::Vector3()
{
    X = 0;
    Y = 0;
    Z = 0;
}

Vector3::Vector3(double x, double y, double z)
{
    X = x;
    Y = y;
    Z = z;
}

Vector3 Vector3::Cross(Vector3 other)
{
    return Vector3(
        Y * other.Z - Z * other.Y,
        Z * other.X - X * other.Z,
        X * other.Y - Y * other.X);
}

Vector3 Vector3::Lerp(Vector3 v, float alpha)
{
    return Vector3(
        X + alpha * (v.X - X),
        Y + alpha * (v.Y - Y),
        Z + alpha * (v.Z - Z));
}

double Vector3::Dot(Vector3 v)
{
    return X * v.X + Y * v.Y + Z * v.Z;
}

Vector3 Vector3::operator+(Vector3 const &vector)
{
    return Vector3(X + vector.X, Y + vector.Y, Z + vector.Z);
}

Vector3 Vector3::operator-(Vector3 const &vector)
{
    return Vector3(X - vector.X, Y - vector.Y, Z - vector.Z);
}

Vector3 Vector3::operator*(Vector3 const &vector)
{
    return Vector3(X * vector.X, Y * vector.Y, Z * vector.Z);
}

Vector3 Vector3::operator/(Vector3 const &vector)
{
    return Vector3(X / vector.X, Y / vector.Y, Z / vector.Z);
}

Vector3 Vector3::operator*(double const &number)
{
    return Vector3(X * number, Y * number, Z * number);
}

Vector3 Vector3::operator/(double const &number)
{
    return Vector3(X / number, Y / number, Z / number);
}

bool Vector3::operator==(Vector3 const &vector)
{
    return (X == vector.X) && (Y == vector.Y) && (Z == vector.Z);
}

bool Vector3::operator!=(Vector3 const &vector)
{
    return !(*this == vector);
}