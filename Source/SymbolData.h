// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#pragma once

#undef min
#undef max
#include "earcut.hpp"

#include <array>
#include <vector>

class SymbolData {
public:
	static std::vector<float> GenerateData();

private:
	using Point = std::array<double, 2>;
	using Shape = std::vector<Point>;

	static Shape RotateClockwise(const Shape& shape, int degrees);
	static std::vector<Shape> GetAllShapes();
};