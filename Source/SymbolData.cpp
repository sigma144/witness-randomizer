// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "SymbolData.h"
#include <algorithm>
#include <cmath>

#undef min
#undef max
#include "earcut.hpp"

// Do all of the math with doubles to avoid precision loss. We convert to floats at the very end.
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

std::vector<float> SymbolData::GenerateData() {
	// To make the assembly simpler, we assume all symbols have a fixed size.
	// Excess polygons will just be empty triangles (all coordinates set to 0)
	// I picked 256 as a reasonable power of 2 which also provides a reasonable shape size.
	constexpr int FLOATS_PER_SYMBOL = 256;
	constexpr int POINTS_PER_SYMBOL = 128;
	constexpr int TRIANGLES_PER_SYMBOL = 42;
	constexpr int VERTICES_PER_SYMBOL = 44;

	std::array<Shape, NUM_SYMBOLS> allShapes = GetAllShapes();

	std::vector<float> data(allShapes.size() * FLOATS_PER_SYMBOL, 0.0f);
	for (int i = 0; i < NUM_SYMBOLS; i++) {
		assert(allShapes[i].size() <= VERTICES_PER_SYMBOL); // Shape was bigger than the allowed size

		// Use the earcut library to divide the polygon into triangles.
		// This returns an ordered list of points in the shape which comprise the triangles.
		std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(std::vector<Shape>{ allShapes[i] });
		assert(indices.size() <= POINTS_PER_SYMBOL); // earcut somehow resulted in too many points

		// Insert the triangulated polygon into the data array, and cast down to floats for the game
		for (int j = 0; j < indices.size(); j++) {
			data[i * FLOATS_PER_SYMBOL + j * 2]     = (float)allShapes[i][indices[j]][0]; // x coordinate
			data[i * FLOATS_PER_SYMBOL + j * 2 + 1] = (float)allShapes[i][indices[j]][1]; // y coordinate
		}
	}

	return data;
}

std::array<SymbolData::Shape, NUM_SYMBOLS> SymbolData::GetAllShapes() {
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

	std::array<Shape, NUM_SYMBOLS> data;
	data[Arrow1E ] = RotateClockwise(arrow1, 0);
	data[Arrow1SE] = RotateClockwise(arrow1, 45);
	data[Arrow1S ] = RotateClockwise(arrow1, 90);
	data[Arrow1SW] = RotateClockwise(arrow1, 135);
	data[Arrow1W ] = RotateClockwise(arrow1, 180);
	data[Arrow1NW] = RotateClockwise(arrow1, 225);
	data[Arrow1N ] = RotateClockwise(arrow1, 270);
	data[Arrow1NE] = RotateClockwise(arrow1, 315);

	data[Arrow2E ] = RotateClockwise(arrow2, 0);
	data[Arrow2SE] = RotateClockwise(arrow2, 45);
	data[Arrow2S ] = RotateClockwise(arrow2, 90);
	data[Arrow2SW] = RotateClockwise(arrow2, 135);
	data[Arrow2W ] = RotateClockwise(arrow2, 180);
	data[Arrow2NW] = RotateClockwise(arrow2, 225);
	data[Arrow2N ] = RotateClockwise(arrow2, 270);
	data[Arrow2NE] = RotateClockwise(arrow2, 315);

	data[Arrow3E ] = RotateClockwise(arrow3, 0);
	data[Arrow3SE] = RotateClockwise(arrow3, 45);
	data[Arrow3S ] = RotateClockwise(arrow3, 90);
	data[Arrow3SW] = RotateClockwise(arrow3, 135);
	data[Arrow3W ] = RotateClockwise(arrow3, 180);
	data[Arrow3NW] = RotateClockwise(arrow3, 225);
	data[Arrow3N ] = RotateClockwise(arrow3, 270);
	data[Arrow3NE] = RotateClockwise(arrow3, 315);

	return data;
}