#pragma once
#include <vector>

struct Quaternion {
	float x;
	float y;
    float z;
	float w;

    // Basic operations
    double Length() const;
    Quaternion Normalize();

    // Complex operations
    Quaternion Mul(const Quaternion& other) const;
    Quaternion Rotate90();
    void RotateVector(std::vector<float>& vector);
};
