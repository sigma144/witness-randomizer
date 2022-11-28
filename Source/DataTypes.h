#pragma once

struct RgbColor {
	RgbColor() : R(1.f), G(1.f), B(1.f) {}
	RgbColor(float R, float G, float B) : R(R), G(G), B(B) {}
	float R;
	float G;
	float B;
};
