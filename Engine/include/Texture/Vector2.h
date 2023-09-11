#pragma once

class Vector2
{
public:
	Vector2();
	Vector2(double x, double y);

	double Cross(Vector2 other);
	double Dot(Vector2 v);
	Vector2 Lerp(Vector2 v, float alpha);

	Vector2 operator+ (Vector2 const& vector);
	Vector2 operator- (Vector2 const& vector);
	Vector2 operator* (Vector2 const& vector);
	Vector2 operator/ (Vector2 const& vector);
	Vector2 operator* (double const& number);
	Vector2 operator/ (double const& number);
	bool operator== (Vector2 const& vector);
	bool operator!= (Vector2 const& vector);

	double X;
	double Y;
};