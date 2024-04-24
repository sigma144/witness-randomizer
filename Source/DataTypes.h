#pragma once

#include <vector>

struct RgbColor {
	RgbColor() : R(1.f), G(1.f), B(1.f), A(1.f) {}
	RgbColor(float R, float G, float B) : R(R), G(G), B(B), A(1.f) {}
	RgbColor(float R, float G, float B, float A) : R(R), G(G), B(B), A(A) {}
	RgbColor(const RgbColor& color, float A) : R(color.R), G(color.G), B(color.B), A(A) {}

	float R;
	float G;
	float B;
	float A;

	int argb() const {
		return ((int)(A * 255.5) << 24) |
			((int)(R * 255.5) << 16) |
			((int)(G * 255.5) << 8) |
			((int)(B * 255.5));
	}

	static RgbColor lerpHSV(RgbColor a, RgbColor b, float t);
};

struct Vector2 {
	Vector2() : X(0), Y(0) {}
	Vector2(float X, float Y) : X(X), Y(Y) {}

	float X;
	float Y;

	friend bool operator==(const Vector2& lhs, const Vector2& rhs) {
		return lhs.X == rhs.X && lhs.Y == rhs.Y;
	}

	friend bool operator!=(const Vector2& lhs, const Vector2& rhs) {
		return !(lhs == rhs);
	}

	friend Vector2 operator+(const Vector2& lhs, const Vector2& rhs) {
		return Vector2(lhs.X + rhs.X, lhs.Y + rhs.Y);
	}

	friend Vector2 operator-(const Vector2& lhs, const Vector2& rhs) {
		return Vector2(lhs.X - rhs.X, lhs.Y - rhs.Y);
	}

	friend Vector2 operator*(const Vector2& lhs, const float& rhs) {
		return Vector2(lhs.X * rhs, lhs.Y * rhs);
	}

	friend Vector2 operator/(const Vector2& lhs, const float& rhs) {
		return Vector2(lhs.X / rhs, lhs.Y / rhs);
	}

	float length() const;
	Vector2 normalized() const;
};

struct Vector3 {
	Vector3() : X(0), Y(0), Z(0) {}
	Vector3(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
	Vector3(std::vector<float> xyz) : X(xyz[0]), Y(xyz[1]), Z(xyz[2]) {}

	float X;
	float Y;
	float Z;

	friend bool operator==(const Vector3& lhs, const Vector3& rhs) {
		return lhs.X == rhs.X && lhs.Y == rhs.Y && lhs.Z == rhs.Z;
	}

	friend bool operator!=(const Vector3& lhs, const Vector3& rhs) {
		return !(lhs == rhs);
	}

	friend Vector3 operator+(const Vector3& lhs, const Vector3& rhs) {
		return Vector3(lhs.X + rhs.X, lhs.Y + rhs.Y, lhs.Z + rhs.Z);
	}

	friend Vector3 operator-(const Vector3& lhs, const Vector3& rhs) {
		return Vector3(lhs.X - rhs.X, lhs.Y - rhs.Y, lhs.Z - rhs.Z);
	}

	friend Vector3 operator*(const Vector3& lhs, const float& rhs) {
		return Vector3(lhs.X * rhs, lhs.Y * rhs, lhs.Z * rhs);
	}

	friend float operator*(const Vector3& lhs, const Vector3& rhs) {
		return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
	}

	friend Vector3 operator/(const Vector3& lhs, const float& rhs) {
		return Vector3(lhs.X / rhs, lhs.Y / rhs, lhs.Z / rhs);
	}

	float length() const;
	Vector3 normalized() const;
};