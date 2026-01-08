// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#pragma once

#include <array>
#include <vector>

enum SymbolId {
	Arrow1E  = 0x00,
	Arrow1SE = 0x01,
	Arrow1S  = 0x02,
	Arrow1SW = 0x03,
	Arrow1W  = 0x04,
	Arrow1NW = 0x05,
	Arrow1N  = 0x06,
	Arrow1NE = 0x07,
	Arrow2E  = 0x08,
	Arrow2SE = 0x09,
	Arrow2S  = 0x0A,
	Arrow2SW = 0x0B,
	Arrow2W  = 0x0C,
	Arrow2NW = 0x0D,
	Arrow2N  = 0x0E,
	Arrow2NE = 0x0F,
	Arrow3E  = 0x10,
	Arrow3SE = 0x11,
	Arrow3S  = 0x12,
	Arrow3SW = 0x13,
	Arrow3W  = 0x14,
	Arrow3NW = 0x15,
	Arrow3N  = 0x16,
	Arrow3NE = 0x17,

	NUM_SYMBOLS, // Must be last
};

constexpr int GetWitnessDecorationId(SymbolId symbolId) { return ((int)symbolId << 16) + 0x80700; }

class SymbolData {
public:
	static std::vector<float> GenerateData();

private:
	using Point = std::array<double, 2>;
	using Shape = std::vector<Point>;

	static Shape RotateClockwise(const Shape& shape, int degrees);
	static std::array<Shape, SymbolId::NUM_SYMBOLS> GetAllShapes();
};