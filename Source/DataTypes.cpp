#include "DataTypes.h"

#include <cmath>

float Vector2::length() const {
	return sqrt(X*X + Y*Y);
}

Vector2 Vector2::normalized() const {
	return *this / length();
}

float Vector3::length() const {
	return sqrt(X * X + Y * Y + Z * Z);
}

Vector3 Vector3::normalized() const {
	return *this / length();
}