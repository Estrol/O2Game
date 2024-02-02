/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __VECTOR2_H_
#define __VECTOR2_H_

class Vector2
{
public:
    Vector2();
    Vector2(double x, double y);

    Vector2 Cross(Vector2 other);
    double  Dot(Vector2 v);
    Vector2 Lerp(Vector2 v, float alpha);

    Vector2 operator+(Vector2 const &vector);
    Vector2 operator-(Vector2 const &vector);
    Vector2 operator*(Vector2 const &vector);
    Vector2 operator/(Vector2 const &vector);
    Vector2 operator*(double const &number);
    Vector2 operator/(double const &number);
    bool    operator==(Vector2 const &vector);
    bool    operator!=(Vector2 const &vector);

    double X;
    double Y;
};

#endif