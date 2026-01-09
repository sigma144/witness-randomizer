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

SymbolData::Shape SymbolData::Scale(const Shape& shape, double scale)
{
	Shape scaled;
	scaled.resize(shape.size());
	for (int i = 0; i < shape.size(); i++) {
		scaled[i][0] = shape[i][0] * scale;
		scaled[i][1] = shape[i][1] * scale;
	}
	return scaled;
}

SymbolData::Shape SymbolData::Translate(const Shape& shape, double dx, double dy)
{
	Shape translated;
	translated.resize(shape.size());
	for (int i = 0; i < shape.size(); i++) {
		translated[i][0] = shape[i][0] + dx;
		translated[i][1] = shape[i][1] + dy;
	}
	return translated;
}

std::vector<float> SymbolData::GenerateData() {
	// To make the assembly simpler, we assume all symbols have a fixed size.
	// Excess polygons will just be empty triangles (all coordinates set to 0)
	// I picked 256 as a reasonable power of 2 which also provides a reasonable shape size.
	constexpr int FLOATS_PER_SYMBOL = 256;
	constexpr int POINTS_PER_SYMBOL = 128;
	constexpr int TRIANGLES_PER_SYMBOL = 42;
	constexpr int VERTICES_PER_SYMBOL = 44;

	std::array<std::vector<Shape>, NUM_SYMBOLS> allShapes = GetAllShapes();

	std::vector<float> data(allShapes.size() * FLOATS_PER_SYMBOL, 0.0f);
	for (int i = 0; i < NUM_SYMBOLS; i++) {
		size_t size = 0;
		size_t indiceSize = 0;
		int dataIndex = 0;
		for (int j = 0; j < allShapes[i].size(); j++) {
			size += allShapes[i][j].size();
			assert(size <= VERTICES_PER_SYMBOL); // Shape was bigger than the allowed size

			// Use the earcut library to divide the polygon into triangles.
			// This returns an ordered list of points in the shape which comprise the triangles.
			std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(std::vector<Shape>{ allShapes[i][j] });
			indiceSize += indices.size();
			assert(indiceSize <= POINTS_PER_SYMBOL); // earcut somehow resulted in too many points

			// Insert the triangulated polygon into the data array, and cast down to floats for the game
			for (int k = 0; k < indices.size(); k++) {
				data[i * FLOATS_PER_SYMBOL + (dataIndex) * 2]     = (float)allShapes[i][j][indices[k]][0]; // x coordinate
				data[i * FLOATS_PER_SYMBOL + (dataIndex) * 2 + 1] = (float)allShapes[i][j][indices[k]][1]; // y coordinate
				dataIndex++;
			}
		}
	}
	return data;
}

std::array<std::vector<SymbolData::Shape>, SymbolId::NUM_SYMBOLS> SymbolData::GetAllShapes() {

	std::array<std::vector<Shape>, NUM_SYMBOLS> data;
	data[BigSquare] = { Shape{ {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0} } };

	AddArrows(data);

	return data;
}

void SymbolData::AddArrows(std::array<std::vector<Shape>, SymbolId::NUM_SYMBOLS>& data)
{
	// Sigma's arrows
	Shape arrow1 = {
		{ 0.4,  0.1},
		{-0.8,  0.1},
		{-0.8, -0.1},
		{ 0.4, -0.1},
		{-0.1, -0.6},
		{ 0.2, -0.6},
		{ 0.8,  0.0},
		{ 0.2,  0.6},
		{-0.1,  0.6},
	};

	Shape arrow2 = {
		{-0.8, -0.1},
		{ 0.0, -0.1},
		{-0.5, -0.6},
		{-0.2, -0.6},
		{ 0.3, -0.1},
		{ 0.4, -0.1},
		{-0.1, -0.6},
		{ 0.2, -0.6},
		{ 0.8,  0.0},
		{ 0.2,  0.6},
		{-0.1,  0.6},
		{ 0.4,  0.1},
		{ 0.3,  0.1},
		{-0.2,  0.6},
		{-0.5,  0.6},
		{ 0.0,  0.1},
		{-0.8,  0.1},
	};

	Shape arrow3 = {
		{-0.8, -0.1},
		{-0.4, -0.1},
		{-0.9, -0.6},
		{-0.6, -0.6},
		{-0.1, -0.1},
		{ 0.0, -0.1},
		{-0.5, -0.6},
		{-0.2, -0.6},
		{ 0.3, -0.1},
		{ 0.4, -0.1},
		{-0.1, -0.6},
		{ 0.2, -0.6},
		{ 0.8,  0.0},
		{ 0.2,  0.6},
		{-0.1,  0.6},
		{ 0.4,  0.1},
		{ 0.3,  0.1},
		{-0.2,  0.6},
		{-0.5,  0.6},
		{ 0.0,  0.1},
		{-0.1,  0.1},
		{-0.6,  0.6},
		{-0.9,  0.6},
		{-0.4,  0.1},
		{-0.8,  0.1},
	};

	double scale = 0.8;
	double translate = 0.15;

	arrow1 = Scale(arrow1, scale);
	arrow2 = Scale(arrow2, scale);
	arrow3 = Scale(arrow3, scale);

	data[Arrow1E] = {RotateClockwise(arrow1, 0)};
	data[Arrow1SE] = {RotateClockwise(arrow1, 45)};
	data[Arrow1S] = {RotateClockwise(arrow1, 90)};
	data[Arrow1SW] = {RotateClockwise(arrow1, 135)};
	data[Arrow1W] = {RotateClockwise(arrow1, 180)};
	data[Arrow1NW] = {RotateClockwise(arrow1, 225)};
	data[Arrow1N] = {RotateClockwise(arrow1, 270)};
	data[Arrow1NE] = {RotateClockwise(arrow1, 315)};

	data[Arrow2E] = {RotateClockwise(arrow2, 0)};
	data[Arrow2SE] = {Translate(RotateClockwise(arrow2, 45), translate / 2, translate / 2)};
	data[Arrow2S] = {RotateClockwise(arrow2, 90)};
	data[Arrow2SW] = {Translate(RotateClockwise(arrow2, 135), -translate / 2, translate / 2)};
	data[Arrow2W] = {RotateClockwise(arrow2, 180)};
	data[Arrow2NW] = {Translate(RotateClockwise(arrow2, 225), -translate / 2, -translate / 2)};
	data[Arrow2N] = {RotateClockwise(arrow2, 270)};
	data[Arrow2NE] = {Translate(RotateClockwise(arrow2, 315), translate / 2, -translate / 2)};

	
	data[Arrow3E] = {RotateClockwise(arrow3, 0)};
	data[Arrow3SE] = {Translate(RotateClockwise(arrow3, 45), translate, translate)};
	data[Arrow3S] = {RotateClockwise(arrow3, 90)};
	data[Arrow3SW] = {Translate(RotateClockwise(arrow3, 135), -translate, translate)};
	data[Arrow3W] = {RotateClockwise(arrow3, 180)};
	data[Arrow3NW] = {Translate(RotateClockwise(arrow3, 225), -translate, -translate)};
	data[Arrow3N] = {RotateClockwise(arrow3, 270)};
	data[Arrow3NE] = {Translate(RotateClockwise(arrow3, 315), translate, -translate)};
}

