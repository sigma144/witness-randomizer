// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#pragma once

#include <array>
#include <vector>
#include "Enums.h"

//NOTE: When SymbolData is updated, the game must be closed and re-opened for changes to take effect.

enum SymbolID : int { //This list, SYMBOL_TYPES, and Symbol enums must have the same ordering
	ARROW1E  = 0x00,
	ARROW1SE = 0x01,
	ARROW1S  = 0x02,
	ARROW1SW = 0x03,
	ARROW1W  = 0x04,
	ARROW1NW = 0x05,
	ARROW1N  = 0x06,
	ARROW1NE = 0x07,
	ARROW2E  = 0x08,
	ARROW2SE = 0x09,
	ARROW2S  = 0x0A,
	ARROW2SW = 0x0B,
	ARROW2W  = 0x0C,
	ARROW2NW = 0x0D,
	ARROW2N  = 0x0E,
	ARROW2NE = 0x0F,
	ARROW3E  = 0x10,
	ARROW3SE = 0x11,
	ARROW3S  = 0x12,
	ARROW3SW = 0x13,
	ARROW3W  = 0x14,
	ARROW3NW = 0x15,
	ARROW3N  = 0x16,
	ARROW3NE = 0x17,

	ANTITRIANGLE1 = 0x18,
	ANTITRIANGLE2 = 0x19,
	ANTITRIANGLE3 = 0x1A,
	ANTITRIANGLE4 = 0x1B,

	BigSquare = 0x1C,

	NUM_SYMBOLS, // Must be last
};

inline constexpr SymbolID SYMBOL_TYPES[] = { ARROW1E, ANTITRIANGLE1, BigSquare };

class SymbolData {
public:
	static std::vector<float> GenerateData();
	static int GetValFromSymbolID(int symbolID);
	static SymbolID GetSymbolIDFromVal(int val);
	static Symbol GetSymbolFromVal(int val);
	static int GetValFromSymbol(int val);

private:
	using Point = std::array<double, 2>;
	using Shape = std::vector<Point>;

	static Shape RotateClockwise(const Shape& shape, int degrees);
	static Shape Scale(const Shape& shape, double scale);
	static Shape Translate(const Shape& shape, double dx, double dy);
	static Shape FlipX(const Shape& shape);
	static std::vector<Shape> RotateClockwise(const std::vector<Shape>& shapes, int degrees);
	static std::vector<Shape> Scale(const std::vector<Shape>& shapes, double scale);
	static std::vector<Shape> Translate(const std::vector<Shape>& shapes, double dx, double dy);
	static std::vector<Shape> FlipX(const std::vector<Shape>& shapes);
	static std::vector<Shape> DrawCounter(const Shape& shape, int count);
	static std::vector<Shape> CombineHelper(std::vector<Shape> combined) { return combined; };
	template<typename... Params>
	static std::vector<Shape> CombineHelper(std::vector<Shape> combined, Shape shape, Params... params) {
		combined.push_back(shape);
		return CombineHelper(combined, params...);
	};
	template<typename... Params>
	static std::vector<Shape> CombineHelper(std::vector<Shape> combined, std::vector<Shape> shapes, Params... params) {
		for (Shape shape : shapes) {
			combined.push_back(shape);
		}
		return CombineHelper(combined, params...);
	};
	// Combines any number of shape and vector of shapes into a singular vector of shape
	template<typename... Params>
	static std::vector<Shape> Combine(Params... params) { 
		std::vector<Shape> combined;
		return CombineHelper(combined, params...);
	};

	static std::array<std::vector<Shape>, SymbolID::NUM_SYMBOLS> GetAllShapes();
	static void AddArrows(std::array<std::vector<Shape>, SymbolID::NUM_SYMBOLS>& data);
	static void AddAntiTriangles(std::array<std::vector<Shape>, SymbolID::NUM_SYMBOLS>& data);
};