/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#ifndef __UDIM_H_
#define __UDIM_H_

class UDim
{
public:
    UDim();
    UDim(double scale, double offset);

    double Scale;
    double Offset;

    UDim Lerp(UDim destination, float alpha);

    UDim operator+(UDim const &udim);
    UDim operator-(UDim const &udim);
    bool operator==(UDim const &udim);
    bool operator!=(UDim const &udim);
    bool operator<=(UDim const &udim);
    bool operator>=(UDim const &udim);
    bool operator<(UDim const &udim);
    bool operator>(UDim const &udim);
};

#endif