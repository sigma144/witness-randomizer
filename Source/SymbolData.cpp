// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "SymbolData.h"
#include <algorithm>
#include <cmath>

// Do all of the math with doubles to avoid precision loss. We can convert to floats when we're at the very end.
constexpr double PI_OVER_180 = 0.017453292519943295;
SymbolData::Shape SymbolData::RotateClockwise(const Shape& shape, int degrees) {
	double sine   = std::sin(degrees * PI_OVER_180);
	double cosine = std::cos(degrees * PI_OVER_180);

	Shape rotated;
	rotated.resize(shape.size());
	for (int i = 0; i < shape.size(); i++) {
		rotated[i][0] = shape[i][0] * cosine - shape[i][1] * sine;
		rotated[i][1] = shape[i][0] * sine   + shape[i][1] * cosine;
	}

	return rotated;
}

// To make the assembly simpler, we assume all symbols have a fixed size.
// Excess polygons will just be empty triangles (all coordinates set to 0)
constexpr int VERTICES_PER_SYMBOL = 44;
constexpr int TRIANGLES_PER_SYMBOL = 42;
constexpr int POINTS_PER_SYMBOL = 128;
constexpr int FLOATS_PER_SYMBOL = 256;

std::vector<float> SymbolData::GenerateData() {
	std::vector<Shape> allShapes = GetAllShapes();

	std::vector<float> data(allShapes.size() * FLOATS_PER_SYMBOL, 0.0f);
	for (int i = 0; i < allShapes.size(); i++) {
		assert(allShapes[i].size() <= VERTICES_PER_SYMBOL);

		// Use the earcut library to divide the polygon into triangles.
		// This returns an ordered list of points in the shape which comprise the triangles.
		std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(std::vector<Shape>{ allShapes[i] });
		assert(indices.size() <= POINTS_PER_SYMBOL);

		// Insert the triangulated polygon into the data array
		for (int j = 0; j < indices.size(); j++) {
			data[i * FLOATS_PER_SYMBOL + j * 2]     = (float)allShapes[i][indices[j]][0]; // x coordinate
			data[i * FLOATS_PER_SYMBOL + j * 2 + 1] = (float)allShapes[i][indices[j]][1]; // y coordinate
		}
	}

	return data;
}

std::vector<SymbolData::Shape> SymbolData::GetAllShapes() {
	// Sigma's arrows
	Shape arrow1 = {
		{ 0.04,  0.01},
		{-0.08,  0.01},
		{-0.08, -0.01},
		{ 0.04, -0.01},
		{-0.01, -0.06},
		{ 0.02, -0.06},
		{ 0.08,  0.00},
		{ 0.02,  0.06},
		{-0.01,  0.06},
	};

	Shape arrow2 = {
		{-0.08, -0.01},
		{ 0.00, -0.01},
		{-0.05, -0.06},
		{-0.02, -0.06},
		{ 0.03, -0.01},
		{ 0.04, -0.01},
		{-0.01, -0.06},
		{ 0.02, -0.06},
		{ 0.08,  0.00},
		{ 0.02,  0.06},
		{-0.01,  0.06},
		{ 0.04,  0.01},
		{ 0.03,  0.01},
		{-0.02,  0.06},
		{-0.05,  0.06},
		{ 0.00,  0.01},
		{-0.08,  0.01},
	};

	Shape arrow3 = {
		{-0.08, -0.01},
		{-0.04, -0.01},
		{-0.09, -0.06},
		{-0.06, -0.06},
		{-0.01, -0.01},
		{ 0.00, -0.01},
		{-0.05, -0.06},
		{-0.02, -0.06},
		{ 0.03, -0.01},
		{ 0.04, -0.01},
		{-0.01, -0.06},
		{ 0.02, -0.06},
		{ 0.08,  0.00},
		{ 0.02,  0.06},
		{-0.01,  0.06},
		{ 0.04,  0.01},
		{ 0.03,  0.01},
		{-0.02,  0.06},
		{-0.05,  0.06},
		{ 0.00,  0.01},
		{-0.01,  0.01},
		{-0.06,  0.06},
		{-0.09,  0.06},
		{-0.04,  0.01},
		{-0.08,  0.01},
	};

	return {
		RotateClockwise(arrow1, 0),
		RotateClockwise(arrow1, 45),
		RotateClockwise(arrow1, 90),
		RotateClockwise(arrow1, 135),
		RotateClockwise(arrow1, 180),
		RotateClockwise(arrow1, 225),
		RotateClockwise(arrow1, 270),
		RotateClockwise(arrow1, 315),

		RotateClockwise(arrow2, 0),
		RotateClockwise(arrow2, 45),
		RotateClockwise(arrow2, 90),
		RotateClockwise(arrow2, 135),
		RotateClockwise(arrow2, 180),
		RotateClockwise(arrow2, 225),
		RotateClockwise(arrow2, 270),
		RotateClockwise(arrow2, 315),

		RotateClockwise(arrow3, 0),
		RotateClockwise(arrow3, 45),
		RotateClockwise(arrow3, 90),
		RotateClockwise(arrow3, 135),
		RotateClockwise(arrow3, 180),
		RotateClockwise(arrow3, 225),
		RotateClockwise(arrow3, 270),
		RotateClockwise(arrow3, 315),
	};
}