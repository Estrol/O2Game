#pragma once

class UDim
{
public:
	UDim();
	UDim(double scale, double offset);

	double Scale;
	double Offset;

	UDim Lerp(UDim destination, float alpha);

	UDim operator+ (UDim const& udim);
	UDim operator- (UDim const& udim);
	bool operator== (UDim const& udim);
	bool operator!= (UDim const& udim);
};