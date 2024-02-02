/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __VECTOR3_H_
#define __VECTOR3_H_

class Vector3
{
public:
    Vector3();
    Vector3(double x, double y, double z);

    Vector3 Cross(Vector3 other);
    double  Dot(Vector3 v);
    Vector3 Lerp(Vector3 v, float alpha);

    Vector3 operator+(Vector3 const &vector);
    Vector3 operator-(Vector3 const &vector);
    Vector3 operator*(Vector3 const &vector);
    Vector3 operator/(Vector3 const &vector);
    Vector3 operator*(double const &number);
    Vector3 operator/(double const &number);
    bool    operator==(Vector3 const &vector);
    bool    operator!=(Vector3 const &vector);

    double X;
    double Y;
    double Z;
};

#endif