#include "Vector2.hpp"

Vector2::Vector2() {
	X = 0;
	Y = 0;
}

Vector2::Vector2(double x, double y) {
	X = x;
	Y = y;
}

double Vector2::Cross(Vector2 other) {
	return this->X * other.Y - other.X * this->Y;
}

double Vector2::Dot(Vector2 v) {
	return this->X * v.X + this->Y * v.Y;
}

Vector2 Vector2::Lerp(Vector2 destination, float alpha) {
	return (*this) * (1.0 - alpha) + (destination * alpha);
}

Vector2 Vector2::operator+(Vector2 const& vector) {
	return { this->X + vector.X, this->Y + vector.Y };
}

Vector2 Vector2::operator-(Vector2 const& vector) {
	return { this->X - vector.X, this->Y - vector.Y };
}

Vector2 Vector2::operator*(Vector2 const& vector) {
	return { this->X * vector.X, this->Y * vector.Y };
}

Vector2 Vector2::operator/(Vector2 const& vector) {
	return { this->X / vector.X, this->Y / vector.Y };
}

Vector2 Vector2::operator*(double const& number) {
	return { this->X * number, this->Y * number };
}

Vector2 Vector2::operator/(double const& number) {
	return { this->X / number, this->Y / number };
}

bool Vector2::operator==(Vector2 const& vector) {
	return this->X == vector.X && this->Y == vector.Y;
}

bool Vector2::operator!=(Vector2 const& vector) {
	return this->X != vector.X || this->Y != vector.Y;
}