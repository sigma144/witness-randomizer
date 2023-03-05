#pragma once

struct RgbColor {
	RgbColor() : R(1.f), G(1.f), B(1.f), A(1.f) {}
	RgbColor(float R, float G, float B) : R(R), G(G), B(B), A(1.f) {}
	RgbColor(float R, float G, float B, float A) : R(R), G(G), B(B), A(A) {}

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
};
