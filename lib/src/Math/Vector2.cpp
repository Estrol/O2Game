/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#include <Math/Vector2.h>

Vector2::Vector2()
{
    X = 0;
    Y = 0;
}

Vector2::Vector2(double x, double y)
{
    X = x;
    Y = y;
}

Vector2 Vector2::Cross(Vector2 other)
{
    return Vector2(
        Y * other.X - X * other.Y,
        X * other.Y - Y * other.X);
}

double Vector2::Dot(Vector2 v)
{
    return this->X * v.X + this->Y * v.Y;
}

Vector2 Vector2::Lerp(Vector2 v, float alpha)
{
    return Vector2(
        this->X + alpha * (v.X - this->X),
        this->Y + alpha * (v.Y - this->Y));
}

Vector2 Vector2::operator+(Vector2 const &vector)
{
    return { this->X + vector.X, this->Y + vector.Y };
}

Vector2 Vector2::operator-(Vector2 const &vector)
{
    return { this->X - vector.X, this->Y - vector.Y };
}

Vector2 Vector2::operator*(Vector2 const &vector)
{
    return { this->X * vector.X, this->Y * vector.Y };
}

Vector2 Vector2::operator/(Vector2 const &vector)
{
    return { this->X / vector.X, this->Y / vector.Y };
}

Vector2 Vector2::operator*(double const &number)
{
    return { this->X * number, this->Y * number };
}

Vector2 Vector2::operator/(double const &number)
{
    return { this->X / number, this->Y / number };
}

bool Vector2::operator==(Vector2 const &vector)
{
    return this->X == vector.X && this->Y == vector.Y;
}

bool Vector2::operator!=(Vector2 const &vector)
{
    return this->X != vector.X || this->Y != vector.Y;
}