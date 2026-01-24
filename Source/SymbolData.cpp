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

SymbolData::Shape SymbolData::Scale(const Shape& shape, double scale) {
	Shape scaled;
	scaled.resize(shape.size());
	for (int i = 0; i < shape.size(); i++) {
		scaled[i][0] = shape[i][0] * scale;
		scaled[i][1] = shape[i][1] * scale;
	}
	return scaled;
}

SymbolData::Shape SymbolData::Translate(const Shape& shape, double dx, double dy) {
	Shape translated;
	translated.resize(shape.size());
	for (int i = 0; i < shape.size(); i++) {
		translated[i][0] = shape[i][0] + dx;
		translated[i][1] = shape[i][1] + dy;
	}
	return translated;
}

SymbolData::Shape SymbolData::FlipX(const Shape& shape) {
	Shape flipped;
	flipped.resize(shape.size());
	for (int i = 0; i < shape.size(); i++) {
		flipped[i][0] = shape[i][0] * -1;
		flipped[i][1] = shape[i][1];
	}
	return flipped;
}

std::vector<SymbolData::Shape> SymbolData::RotateClockwise(const std::vector<SymbolData::Shape>& shapes, int degrees)  {
	std::vector<Shape> rotated;
	for (Shape shape : shapes) {
		rotated.push_back(RotateClockwise(shape, degrees));
	}
	return rotated;
}

std::vector<SymbolData::Shape> SymbolData::Scale(const std::vector<SymbolData::Shape>& shapes, double scale) {
	std::vector<Shape> scaled;
	for (Shape shape : shapes) {
		scaled.push_back(Scale(shape, scale));
	}
	return scaled;
}

std::vector<SymbolData::Shape> SymbolData::Translate(const std::vector<SymbolData::Shape>& shapes, double dx, double dy) {
	std::vector<Shape> translated;
	for (Shape shape : shapes) {
		translated.push_back(Translate(shape, dx, dy));
	}
	return translated;
}

std::vector<SymbolData::Shape> SymbolData::FlipX(const std::vector<SymbolData::Shape>& shapes) {
	std::vector<Shape> flipped;
	for (Shape shape : shapes) {
		flipped.push_back(FlipX(shape));
	}
	return flipped;
}

std::vector<SymbolData::Shape> SymbolData::DrawCounter(const Shape& shape, int count) {
	// Offset used by the triangle in game. This corresponds to 3/4 the distance between the center of the triangles of a 2 triangles clue
	double offset = 0.3199218765;
	std::vector<SymbolData::Shape> countered = {};
	switch(count) {
		case 1:
			countered.push_back(shape);
			break;
		case 2:
			countered.push_back(Translate(shape, offset * -1, 0));
			countered.push_back(Translate(shape, offset, 0));
			break;
		case 3:
			countered.push_back(shape);
			countered.push_back(Translate(shape, offset * -2, 0));
			countered.push_back(Translate(shape, offset * 2, 0));
			break;
		case 4:
			countered.push_back(Translate(shape, offset * -1, offset * -1));
			countered.push_back(Translate(shape, offset, offset * -1));
			countered.push_back(Translate(shape, offset * -1, offset));
			countered.push_back(Translate(shape, offset, offset));
			break;
		case 5:
			countered.push_back(Translate(shape, 0, offset * -1));
			countered.push_back(Translate(shape, offset * -2, offset * -1));
			countered.push_back(Translate(shape, offset * 2, offset * -1));
			countered.push_back(Translate(shape, offset * -1, offset));
			countered.push_back(Translate(shape, offset, offset));
			break;
		case 6:
			countered.push_back(Translate(shape, 0, offset * -1));
			countered.push_back(Translate(shape, offset * -2, offset * -1));
			countered.push_back(Translate(shape, offset * 2, offset * -1));
			countered.push_back(Translate(shape, 0, offset));
			countered.push_back(Translate(shape, offset * -2, offset));
			countered.push_back(Translate(shape, offset * 2, offset));
			break;
		default:
			break;
	}
	return countered;
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
			assert(size <= VERTICES_PER_SYMBOL + (2 * j)); // Shape was bigger than the allowed size

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

int SymbolData::GetValFromSymbolID(int symbolID) {
	return ((int)symbolID << 16) + 0x80700;
}

SymbolID SymbolData::GetSymbolIDFromVal(int val) {
	return static_cast<SymbolID>(((val & ~0xFF) - 0x80700) >> 16);
}

Symbol SymbolData::GetSymbolFromVal(int val) {
	int symbolID = GetSymbolIDFromVal(val);
	for (int i = 1; i < sizeof(SYMBOL_TYPES) / sizeof(SymbolID); i++) {
		if (symbolID < SYMBOL_TYPES[i]) {
			int color = val & 0xF;
			int type = i - 1;
			int variant = symbolID - SYMBOL_TYPES[type];
			return static_cast<Symbol>(0x700 | color | (type << 12) | (variant << 20));
		}
	}
	return Empty;
}

int SymbolData::GetValFromSymbol(int val) {
	int color = val & 0xF;
	int type = (val & 0xFF000) >> 12;
	int variant = (val & 0xFF00000) >> 20;
	SymbolID symbolID = static_cast<SymbolID>(SYMBOL_TYPES[type] + variant);
	return GetValFromSymbolID(symbolID) | color;
}

std::array<std::vector<SymbolData::Shape>, SymbolID::NUM_SYMBOLS> SymbolData::GetAllShapes() {

	std::array<std::vector<Shape>, NUM_SYMBOLS> data;
	data[BigSquare] = { Shape{ {-1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}, {1.0, -1.0} } };

	AddArrows(data);
	AddAntiTriangles(data);
	AddCaves(data);

	return data;
}

void SymbolData::AddArrows(std::array<std::vector<Shape>, SymbolID::NUM_SYMBOLS>& data) {
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

	data[ARROW1E] = {RotateClockwise(arrow1, 0)};
	data[ARROW1SE] = {RotateClockwise(arrow1, 45)};
	data[ARROW1S] = {RotateClockwise(arrow1, 90)};
	data[ARROW1SW] = {RotateClockwise(arrow1, 135)};
	data[ARROW1W] = {RotateClockwise(arrow1, 180)};
	data[ARROW1NW] = {RotateClockwise(arrow1, 225)};
	data[ARROW1N] = {RotateClockwise(arrow1, 270)};
	data[ARROW1NE] = {RotateClockwise(arrow1, 315)};

	data[ARROW2E] = {RotateClockwise(arrow2, 0)};
	data[ARROW2SE] = {Translate(RotateClockwise(arrow2, 45), translate / 2, translate / 2)};
	data[ARROW2S] = {RotateClockwise(arrow2, 90)};
	data[ARROW2SW] = {Translate(RotateClockwise(arrow2, 135), -translate / 2, translate / 2)};
	data[ARROW2W] = {RotateClockwise(arrow2, 180)};
	data[ARROW2NW] = {Translate(RotateClockwise(arrow2, 225), -translate / 2, -translate / 2)};
	data[ARROW2N] = {RotateClockwise(arrow2, 270)};
	data[ARROW2NE] = {Translate(RotateClockwise(arrow2, 315), translate / 2, -translate / 2)};

	
	data[ARROW3E] = {RotateClockwise(arrow3, 0)};
	data[ARROW3SE] = {Translate(RotateClockwise(arrow3, 45), translate, translate)};
	data[ARROW3S] = {RotateClockwise(arrow3, 90)};
	data[ARROW3SW] = {Translate(RotateClockwise(arrow3, 135), -translate, translate)};
	data[ARROW3W] = {RotateClockwise(arrow3, 180)};
	data[ARROW3NW] = {Translate(RotateClockwise(arrow3, 225), -translate, -translate)};
	data[ARROW3N] = {RotateClockwise(arrow3, 270)};
	data[ARROW3NE] = {Translate(RotateClockwise(arrow3, 315), translate, -translate)};
}

void SymbolData::AddAntiTriangles(std::array<std::vector<Shape>, SymbolID::NUM_SYMBOLS>& data) {
	Shape triangle = { { 0.25,  0.2}, {-0.25,  0.2}, {0, -0.2} };
	data[ANTITRIANGLE1] = DrawCounter(triangle, 1);
	data[ANTITRIANGLE2] = DrawCounter(triangle, 2);
	data[ANTITRIANGLE3] = DrawCounter(triangle, 3);
	data[ANTITRIANGLE4] = DrawCounter(triangle, 4);
}

void SymbolData::AddCaves(std::array<std::vector<Shape>, SymbolID::NUM_SYMBOLS>& data) {
	Shape pip = {
		{-0.175, 0.0},
		{0.0, 0.175},
		{0.175, 0.0},
		{0.0, -0.175},
	};

	Shape viewer = {
		{0.2, 0.8},
		{0.0, 0.9},
		{-0.2, 0.8},
	};

	std::vector<Shape> fourViewers = Combine(viewer, RotateClockwise(viewer, 90), RotateClockwise(viewer, 180), RotateClockwise(viewer, 270));
	data[CAVE1] = Combine(DrawCounter(pip, 1), fourViewers);
	data[CAVE2] = Combine(DrawCounter(pip, 2), fourViewers);
	data[CAVE3] = Combine(DrawCounter(pip, 3), fourViewers);
	data[CAVE4] = Combine(DrawCounter(pip, 4), fourViewers);
	data[CAVE5] = Combine(DrawCounter(pip, 5), fourViewers);
	data[CAVE6] = Combine(DrawCounter(pip, 6), fourViewers);
	data[CAVE7] = Combine(Translate(DrawCounter(pip, 5), 0, 0.3199218765), Translate(DrawCounter(pip, 2), 0, -2 * 0.3199218765), fourViewers);
	data[CAVE8] = Combine(Translate(DrawCounter(pip, 5), 0, -0.3199218765), Translate(DrawCounter(pip, 3), 0, 2 * 0.3199218765), fourViewers);
	data[CAVE9] = Combine(Translate(DrawCounter(pip, 6), 0, -0.3199218765), Translate(DrawCounter(pip, 3), 0, 2 * 0.3199218765), fourViewers);
}