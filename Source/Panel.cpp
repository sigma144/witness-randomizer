// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Panel.h"
#include "Special.h"
#include "Randomizer.h"
#include "Watchdog.h"
#include <sstream>
#include <fstream>

template <class T>
int find(const std::vector<T> &data, T search, size_t startIndex = 0) {
	for (size_t i=startIndex ; i<data.size(); i++) {
		if (data[i] == search) return static_cast<int>(i);
	}
	return -1;
}

std::vector<Point> Panel::DIRECTIONS = { Point(0, 1), Point(0, -1), Point(1, 0), Point(-1, 0) };
std::vector<Point> Panel::DIRECTIONS8 = { Point(1, 0), Point(1, -1), Point(0, -1), Point(-1, -1), Point(-1, 0), Point(-1, 1), Point(0, 1), Point(1, 1) };
std::vector<Point> Panel::DIRECTIONS_2 = { Point(0, 2), Point(0, -2), Point(2, 0), Point(-2, 0) };
std::vector<Point> Panel::DIRECTIONS8_2 = { Point(2, 0), Point(2, -2), Point(0, -2), Point(-2, -2), Point(-2, 0), Point(-2, 2), Point(0, 2), Point(2, 2) };

Panel::Panel() { }

Panel::Panel(PanelID id) {
	read(id);
}

void Panel::read() {
	memory = Memory::get();
	lineThickness = 1;
	fixBackground = decorationsOnly = false;
	colorMode = Default;
	style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	isCylinder = memory->ReadPanelData<int>(id, IS_CYLINDER);
	width = 2 * memory->ReadPanelData<int>(id, GRID_SIZE_X) - 1;
	if (isCylinder) width++;
	height = 2 * memory->ReadPanelData<int>(id, GRID_SIZE_Y) - 1;
	if (width <= 0 || height <= 0 || width > 30 || height > 30)
		width = height = 0;
	grid.resize(width);
	for (auto& row : grid) row.resize(height);
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			grid[x][y] = 0;
		}
	}
	startpoints.clear();
	endpoints.clear();
	path.clear();
	readIntersections();
	readDecorations();
}

void Panel::write() {
	memory->WritePanelData<int>(id, GRID_SIZE_X, { (width + 1) / 2 });
	memory->WritePanelData<int>(id, GRID_SIZE_Y, { (height + 1) / 2 });
	if (fixBackground && memory->ReadPanelData<int>(id, NUM_COLORED_REGIONS) > 0) {
		//Make two triangles that cover the whole panel
		std::vector<int> newRegions = { 0, pointToIndex(width - 1, 0), pointToIndex(0, 0), 0, pointToIndex(width - 1, height - 1), pointToIndex(width - 1, 0), 0, 0 };
		memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(newRegions.size()) / 4 });
		memory->WriteArray(id, COLORED_REGIONS, newRegions);
	}
	if (!decorationsOnly)
		writeIntersections();
	else { //Write dots over existing intersection flags
		std::vector<int> iflags = memory->ReadArray<int>(id, DOT_FLAGS, memory->ReadPanelData<int>(id, NUM_DOTS));
		for (int x = 0; x < width; x += 2) {
			for (int y = 0; y < height; y += 2) {
				if (getFlag(x, y, Dot)) {
					iflags[x / 2 + (y / 2) * (width / 2 + 1)] = get(x, y);
					style |= Style::HAS_DOTS;
				}
			}
		}
		memory->WriteArray<int>(id, DOT_FLAGS, iflags);
	}
	writeDecorations();
	if (disableFlash) style |= NO_BLINK;
	else style &= ~NO_BLINK;
	memory->WritePanelData<int>(id, STYLE_FLAGS, style);
	memory->WritePanelData<uintptr_t>(id, SEQUENCE, 0);
	memory->WritePanelData<int>(id, SEQUENCE_LEN, 0);
	float patternWidth = memory->ReadPanelData<float>(id, PATTERN_SCALE);
	float preComputation = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);
	if (lineThickness != 1) memory->WritePanelData<float>(id, PATTERN_SCALE, lineThickness * patternWidth / preComputation);
	memory->WritePanelData<int>(id, NEEDS_REDRAW, true);
}

int Panel::get(int x, int y) {
	if (y < 0 || y >= height) return OFF_GRID;
	if (x < 0) return isCylinder && x > -width ? grid[x + width][y] : OFF_GRID;
	if (x >= width) return isCylinder && x < width * 2 - 1 ? grid[x - width][y] : OFF_GRID;
	return grid[x][y];
}

void Panel::set(int x, int y, int val) {
	if (y < 0 || y >= height) return;
	if (isCylinder && x < 0) x += width;
	if (isCylinder && x >= width) x -= width;
	if (x < 0 || x >= width) return;
	grid[x][y] = val;
	if (val == PATH)
		path[{x, y}] = static_cast<int>(path.size());
}

void Panel::setSymbol(int x, int y, Symbol symbol, SymbolColor color) {
	int gridx = x * 2 + (symbol == Dot_Column || symbol == Gap_Column ? 0 : 1);
	int gridy = y * 2 + (symbol == Dot_Row || symbol == Gap_Row ? 0 : 1);
	if (symbol & DOT) {
		if (color == Blue || color == Cyan)
			color = static_cast<SymbolColor>(DOT_IS_BLUE);
		else if (color == Orange || color == Yellow)
			color = static_cast<SymbolColor>(DOT_IS_ORANGE);
		else color = NoColor;
		if (symmetry) {
			Point sp = getSymPoint(gridx, gridy);
			set(sp.x, sp.y, symbol & ~Dot);
		}
	}
	if (symbol == Gap_Row || symbol == Gap_Column) color = NoColor;
	set(gridx, gridy, symbol | color);
}

void Panel::setShape(int x, int y, int shape, bool rotate, bool negative, SymbolColor color) {
	if (!shape) return;
	int symbol = Poly;
	while (!(shape & 0xf)) shape >>= 4;
	while (!(shape & 0x1111)) shape >>= 1;
	shape <<= 16;
	if (rotate) shape |= Rotate;
	else shape &= ~Rotate;
	if (negative) shape |= Negative;
	else shape &= ~Negative;
	set(x * 2 + 1, y * 2 + 1, symbol | shape | color);
}

void Panel::setGridSymbol(int x, int y, Symbol symbol, SymbolColor color) {
	if (symbol == Start) startpoints.push_back({ x, y });
	if (symbol == Exit) {
		Endpoint::Direction dir;
		if (y == 0) dir = Endpoint::Direction::UP;
		else if (y == height - 1) dir = Endpoint::Direction::DOWN;
		else if (x == 0) dir = Endpoint::Direction::LEFT;
		else dir = Endpoint::Direction::RIGHT;
		if (id == TUT_VAULT || id == TUT_2EXIT) { //TODO: Get rid of this hardcoding
			if (x == 0) dir = Endpoint::Direction::LEFT;
			else dir = Endpoint::Direction::RIGHT;
		}
		if (symmetry == Symmetry::ParallelH || symmetry == Symmetry::ParallelHFlip) {
			if (x == 0) dir = Endpoint::Direction::LEFT;
			if (x == width - 1) dir = Endpoint::Direction::RIGHT;
		}
		endpoints.emplace_back(Endpoint(x, y, dir, ENDPOINT | 
			(dir == Endpoint::Direction::UP || dir == Endpoint::Direction::DOWN ? COLUMN : ROW)));
	}
	else set(x, y, symbol | color);
}

void Panel::resize(int width, int height) {
	//TODO: Do default start and end locations based on symmetry
	for (Point &s : startpoints) {
		if (s.x == this->width - 1) s.x = width - 1;
		if (s.y == this->height - 1) s.y = height - 1;
	}
	for (Endpoint &e : endpoints) {
		if (e.x == this->width - 1) e.x = width - 1;
		if (e.y == this->height - 1) e.y = height - 1;
	}
	if (this->width != this->height || width != height) {
		float maxDim = max(maxx - minx, maxy - miny);
		float unitSize = maxDim / max(width - 1, height - 1);
		minx = 0.5f - unitSize * (width - 1) / 2;
		maxx = 0.5f + unitSize * (width - 1) / 2;
		miny = 0.5f - unitSize * (height - 1) / 2;
		maxy = 0.5f + unitSize * (height - 1) / 2;
	}
	this->width = width;
	this->height = height;
	grid.resize(width);
	for (auto& row : grid) row.resize(height);
	fixBackground = true;
}

std::set<Point> Panel::getRegion(Point pos) {
	std::set<Point> region;
	std::vector<Point> check;
	check.push_back(pos);
	region.insert(pos);
	while (check.size() > 0) {
		Point p = check[check.size() - 1];
		check.pop_back();
		for (Point dir : DIRECTIONS) {
			Point p1 = p + dir;
			if (get(p1.x, p1.y) == PATH || get(p1.x, p1.y) == OPEN) continue;
			Point p2 = p + dir * 2;
			if (get(p2.x, p2.y) < 0 || (get(p2.x, p2.y) & Empty) == Empty) continue;
			if (region.insert(p2).second) {
				check.push_back(p2);
			}
		}
	}
	return region;
}

std::vector<int> Panel::getSymbolsInRegion(Point pos) {
	std::vector<int> symbols;
	for (Point p : getRegion(pos)) {
		if (get(p.x, p.y)) symbols.push_back(get(p.x, p.y));
	}
	return symbols;
}

Point Panel::getSymPoint(int x, int y, Symmetry symmetry) {
	switch (symmetry) {
	case NoSymmetry: return Point(x, y);
	case Symmetry::Horizontal: return Point(x, height - 1 - y);
	case Symmetry::Vertical: return Point(width - 1 - x, y);
	case Symmetry::Rotational: return Point(width - 1 - x, height - 1 - y);
	case Symmetry::RotateLeft: return Point(y, width - 1 - x);
	case Symmetry::RotateRight: return Point(height - 1 - y, x);
	case Symmetry::FlipXY: return Point(y, x);
	case Symmetry::FlipNegXY: return Point(height - 1 - y, width - 1 - x);
	case Symmetry::ParallelH: return Point(x, y == height / 2 ? height / 2 : (y + (height + 1) / 2) % (height + 1));
	case Symmetry::ParallelV: return Point(x == width / 2 ? width / 2 : (x + (width + 1) / 2) % (width + 1), y);
	case Symmetry::ParallelHFlip: return Point(width - 1 - x, y == height / 2 ? height / 2 : (y + (height + 1) / 2) % (height + 1));
	case Symmetry::ParallelVFlip: return Point(x == width / 2 ? width / 2 : (x + (width + 1) / 2) % (width + 1), height - 1 - y);
	case Symmetry::PillarParallel: return Point((x + width / 2) % width, y);
	case Symmetry::PillarHorizontal: return Point((x + width / 2) % width, height - 1 - y);
	case Symmetry::PillarVertical: return Point((width / 2 + width - x) % width, y);
	case Symmetry::PillarRotational: return Point((width / 2 + width - x) % width, height - 1 - y);
	}
	return Point(x, y);
}

//Returns the grid symetric solution point as defined by the panel's REFLECTION_DATA
int Panel::getSymSolutionPoint(int index) {
	if (style & SYMMETRICAL) {
		int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
		std::vector<int> symmetryData = memory->ReadArray<int>(id, REFLECTION_DATA, numIntersections);
		return symmetryData[index];
	}
	else {
		return index;
	}
}

#define LOG_DEBUG(fmt, ...) LogDebug(__FILE__, __LINE__, fmt, __VA_ARGS__)
bool Panel::checkCustomSymbols(bool flash) {
	//memory->LogDebug("Checking symbols");
	bool success = true;
	for (int x = 1; x < width; x++) {
		for (int y = 1; y < height; y++) {
			int symbol = get(x, y);
			if (getType(symbol) != Custom) continue; //Skip non-custom symbols
			if (!checkSymbol({ x, y })) {
				if (!flash) return false;
				//memory->LogDebug("Symbol at %d, %d NOT valid", x, y);
				memory->WriteToArray(id, DECORATION_FLAGS, pointToDecorationIndex(x, y), 1);
				success = false;
			}
			else if (flash) {
				//memory->LogDebug("Symbol at %d, %d IS valid", x, y);
				memory->WriteToArray(id, DECORATION_FLAGS, pointToDecorationIndex(x, y), 0);
			}
		}
	}
	return success;
}

bool Panel::checkSymbol(Point pos, int symbol) {
	if (preCalcResult.count(pos)) return preCalcResult[pos];
	Symbol type = getType(get(pos));
	if (type == Stone) {
		if (!checkStone(pos, symbol)) return false;
	}
	else if (type == Triangle) {
		if (!checkTriangle(pos, symbol)) return false;
	}
	else if (type == Star) {
		if (!checkStar(pos, symbol)) return false;
	}
	else if (type == Poly) {
		if (!checkShape(pos, symbol)) return false;
	}
	else if (type == Custom) {
		symbol = SymbolData::GetSymbolFromVal(symbol);
		type = static_cast<Symbol>(symbol & 0xFF00);
		if (type == Arrow) {
			if (!checkArrow(pos, symbol)) return false;
		}
		else if (type == AntiTriangle) {
			if (!checkAntiTriangle(pos, symbol)) return false;
		}
		else if (type == Cave) {
			if (!checkCave(pos, symbol)) return false;
		}
		else if (type == Minesweeper0) {
			if (!checkMinesweeper(pos, symbol)) return false;
		}
		else if (type == Flower) {
			if (!checkFlower(pos, symbol)) return false;
		}
	}
	return true;
}

bool Panel::checkStone(Point pos, int symbol) {
	std::set<Point> region = getRegion(pos);
	for (Point p : region) {
		int sym = get(p);
		if (getType(sym) == Stone && getColor(sym) != getColor(symbol)) return false;
	}
	return true;
}

bool Panel::checkStar(Point pos, int symbol) {
	std::set<Point> region = getRegion(pos);
	return countColor(region, getColor(symbol)) == 2;
}

bool Panel::checkShape(Point pos, int symbol) {
	std::set<Point> region = getRegion(pos);
	std::vector<int> shapes;
	std::set<Point> shapePos;
	int totalArea = 0;
	for (Point p : region) {
		int sym = get(p);
		if (getType(sym) == Poly) {
			shapes.emplace_back(sym);
			shapePos.insert(p);
			while (sym) {
				totalArea += (sym & 1);
				sym >>= 1;
			}
		}
	}
	bool result;
	if (totalArea == 0) result = true;
	else result = (totalArea == region.size()); //TODO: Write an actual shape checker
	for (Point p : shapePos) {
		preCalcResult[p] = result;
	}
	return result;
}

bool Panel::checkTriangle(Point pos, int symbol) {
	return countSides(pos) == (symbol >> 16);
}

bool Panel::checkArrow(Point pos, int symbol) {
	int targetCount = (symbol >> 23) + 1;
	Point dir = DIRECTIONS8_2[(symbol >> 20) & 0x7];
	return countCrossings(pos, dir) == targetCount;
}

bool Panel::checkAntiTriangle(Point pos, int symbol) {
	int targetCount = (symbol >> 20) + 1;
	return countTurns(pos) == targetCount;
}

bool Panel::checkCave(Point pos, int symbol) {
	int targetCount = (symbol >> 20) + 1;
	int count = 1;
	for (Point dir : DIRECTIONS) {
		Point temp = pos;
		while (get(temp.x + dir.x + dir.x, temp.y + dir.y + dir.y) != OFF_GRID && (get(temp.x + dir.x + dir.x, temp.y + dir.y + dir.y) & Empty) != Empty && get(temp.x + dir.x, temp.y + dir.y) != PATH) {
			count++;
			temp = temp + dir + dir;
		}
	}
	return count == targetCount;
}

bool Panel::checkMinesweeper(Point pos, int symbol) { //TODO: Handle empty spaces (these should probably just return OFF_GRID)
	int targetCount = (symbol >> 20);
	int count = 0;
	std::set<Point> region = getRegion(pos);
	for (Point dir : Panel::DIRECTIONS8_2) {
		if (get(pos + dir) != OFF_GRID && !region.count(pos + dir))
			count++;
	}
	return count == targetCount;
}

bool Panel::checkFlower(Point pos, int symbol) {
	std::set<Point> region = getRegion(pos);
	std::set<Point> col, row;
	for (Point p : region) {
		if (p.x == pos.x)
			row.insert(p);
		if (p.y == pos.y)
			col.insert(p);
	}
	int color = getColor(symbol);
	int colorCount = countColor(region, color);
	return (countColor(col, color) == colorCount) != (countColor(row, color) == colorCount);
}

//Count the occurrence of the given symbol color in the given region
int Panel::countColor(const std::set<Point>& region, int color) {
	int count = 0;
	for (Point p : region) {
		int sym = get(p);
		if (sym && (sym & 0xf) == color)
			count++;
	}
	return count;
}

//Count how many sides are touched by the line (for the triangles)
int Panel::countSides(Point pos) {
	int count = 0;
	for (Point dir : DIRECTIONS) {
		Point p = pos + dir;
		if (get(p) == PATH) {
			count++;
		}
	}
	return count;
}

//Count the number of times the given vector is passed through (for the arrows)
int Panel::countCrossings(Point pos, Point dir) {
	pos = pos + dir / 2;
	int count = 0;
	while (get(pos) != OFF_GRID) {
		if (get(pos) == PATH) count++;
		pos = pos + dir;
	}
	return count;
}

int Panel::countTurns(Point pos) //TODO: Use path order to get multiple exits working correctly
{
	int count = 0;
	for (Point dir : { Point(1, 1), Point(1, -1), Point(-1, -1), Point(-1, 1) }) {
		Point p = pos + dir;
		bool pathH = get(p + Point(-1, 0)) == PATH || get(p + Point(1, 0)) == PATH;
		bool pathV = get(p + Point(0, -1)) == PATH || get(p + Point(0, 1)) == PATH;
		for (Endpoint& e : endpoints) {
			if (e.x == p.x && e.y == p.y && get(e.x, e.y) == PATH) {
				switch (e.dir) {
				case Endpoint::LEFT:
				case Endpoint::RIGHT: pathH = true; break;
				case Endpoint::UP:
				case Endpoint::DOWN: pathV = true; break;
				default: pathH = pathV = true;
				}
			}
		}
		if (pathH && pathV)
			count++;
	}
	//Mid-segment exits
	for (Endpoint& e : endpoints) {
		if (e.x == pos.x && abs(e.y - pos.y) == 1 && get(e.x, e.y) == PATH)
			count++;
		if (e.y == pos.y && abs(e.x - pos.x) == 1 && get(e.x, e.y) == PATH)
			count++;
	}
	return count;
}

void Panel::readDecorations() {
	int numDecorations = memory->ReadPanelData<int>(id, NUM_DECORATIONS);
	std::vector<int> decorations = memory->ReadArray<int>(id, DECORATIONS, numDecorations);
	for (int i=0; i<numDecorations; i++) {
		Point p = decorationIndexToPoint(i);
		set(p, decorations[i]);
		if ((decorations[i] & Empty) == Empty)
			fixBackground = true;
	}
	preCalcResult.clear();
}

void Panel::writeDecorations() {
	std::vector<int> decorations;
	std::vector<Color> decorationColors;
	bool any = false;
	style &= ~0x3fc0; //Remove all element flags
	for (int y = height - 2; y > 0; y -= 2) {
		for (int x = 1; x < width; x += 2) {
			int val = get(x, y);
			decorations.push_back(val);
			decorationColors.push_back(getColorRGB(getColor(val)));
			if (val) any = true;
			int type = getType(val);
			if (type == Stone) style |= HAS_STONES;
			else if (type == Star) style |= HAS_STARS;
			else if (type == Poly) style |= HAS_SHAPERS;
			else if (type == Eraser) style |= HAS_ERASERS;
			else if (type == Triangle) style |= HAS_TRIANGLES;
			else if (type != 0) style |= HAS_CUSTOM;
		}
	}
	if (!any) {
		memory->WritePanelData<int>(id, NUM_DECORATIONS, 0);
	}
	else {
		memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
		if (colorMode == ColorMode::WriteColors || memory->ReadPanelData<long>(id, DECORATION_COLORS))
			memory->WriteArray<Color>(id, DECORATION_COLORS, decorationColors);
		else if (colorMode == ColorMode::Reset)
			memory->WritePanelData<int>(id, PUSH_SYMBOL_COLORS, 0);
		else if (colorMode == ColorMode::Alternate)
			memory->WritePanelData<int>(id, PUSH_SYMBOL_COLORS, 1);
	}
	if (any || memory->ReadPanelData<int>(id, DECORATIONS)) {
		memory->WriteArray<int>(id, DECORATIONS, decorations);
		for (int i = 0; i < decorations.size(); i++) decorations[i] = 0;
		memory->WriteArray<int>(id, DECORATION_FLAGS, decorations);
	}
}

void Panel::readIntersections() {
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<float> intersections = memory->ReadArray<float>(id, DOT_POSITIONS, numIntersections * 2);
	int numGridPoints = this->getNumGridPoints();
	if (numGridPoints >= 2) {
		minx = intersections[0]; miny = intersections[1];
		maxx = intersections[numGridPoints * 2 - 2]; maxy = intersections[numGridPoints * 2 - 1];
	}
	else {
		minx = miny = 0.1f; maxx = maxy = 0.9f;
		startpoints = { { 0, height - 1 } };
		endpoints = { Endpoint(width - 1, 0, Endpoint::Direction::RIGHT, ENDPOINT | ROW) };
		return;
	}
	if (minx > maxx) std::swap(minx, maxx);
	if (miny > maxy) std::swap(miny, maxy);
	unitWidth = (maxx - minx) / (width - 1);
	if (isCylinder) unitWidth = 1.0f / width;
	unitHeight = (maxy - miny) / (height - 1);
	//TODO: Recognize other types of symmmetry?
	std::vector<int> symmetryData = memory->ReadPanelData<int>(id, REFLECTION_DATA) ?
		memory->ReadArray<int>(id, REFLECTION_DATA, numIntersections) : std::vector<int>();
	if (symmetryData.size() == 0) symmetry = NoSymmetry;
	else if (symmetryData[0] == numGridPoints - 1) symmetry = Rotational;
	else if (symmetryData[0] == width / 2 && intersections[1] == intersections[3]) symmetry = Vertical;
	else symmetry = Horizontal;
	//Read intersection flags
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	for (int i = 0; i < numGridPoints; i++) {
		int x = static_cast<int>(std::round((intersections[i * 2] - minx) / unitWidth));
		int y = height - 1 - static_cast<int>(std::round((intersections[i * 2 + 1] - miny) / unitHeight));
		set(x, y, intersectionFlags[i]);
		if (intersectionFlags[i] & STARTPOINT) {
			startpoints.push_back({x, y});
		}
	}
	//Clear all line segments, them fill in using connections
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (x % 2 != y % 2)
				set(x, y, OPEN);
		}
	}
	int numConnections = memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	std::vector<int> connections_a = memory->ReadArray<int>(id, DOT_CONNECTION_A, numConnections);
	std::vector<int> connections_b = memory->ReadArray<int>(id, DOT_CONNECTION_B, numConnections);
	for (int i = 0; i < connections_a.size(); i++) {
		if (connections_a[i] >= numGridPoints || connections_b[i] >= numGridPoints) continue;
		int x = static_cast<int>(std::round((intersections[connections_a[i] * 2] - minx) / unitWidth));
		int y = height - 1 - static_cast<int>(std::round((intersections[connections_a[i] * 2 + 1] - miny) / unitHeight));
		int x2 = static_cast<int>(std::round((intersections[connections_b[i] * 2] - minx) / unitWidth));
		int y2 = height - 1 - static_cast<int>(std::round((intersections[connections_b[i] * 2 + 1] - miny) / unitHeight));
		set((x + x2) / 2, (y + y2) / 2, 0);
	}
	//Iterate the remaining intersections (endpoints, dots, gaps)
	for (int i = numGridPoints; i < numIntersections; i++) {
		float xd = (intersections[i * 2] - minx) / unitWidth;
		float yd = (intersections[i * 2 + 1] - miny) / unitHeight;
		int x = std::clamp((int)std::round(xd), 0, width - 1);
		int y = height - 1 - std::clamp((int)std::round(yd), 0, height - 1);
		if (intersectionFlags[i] & GAP) {
			float xd2 = (intersections[i * 2 + 2] - minx) / unitWidth;
			float yd2 = (intersections[i * 2 + 3] - miny) / unitHeight;
			x = std::clamp((int)std::round((xd + xd2) / 2), 0, width - 1);
			y = height - 1 - std::clamp((int)std::round((yd + yd2) / 2), 0, height - 1);
			bool fake = false;
			for (int j = 0; j < numConnections; j++) {
				if (connections_a[j] == i && connections_b[j] == i + 1 ||
					connections_a[j] == i + 1 && connections_b[j] == i) {
					//Fake symmetry wall
					fake = true;
					set(x, y, 0);
					i++;
					break;
				}
			}
			if (fake) continue;
			i++;
		}
		if (intersectionFlags[i] & ENDPOINT) {
			for (int j = 0; j < numConnections; j++) {
				int location = -1;
				if (connections_a[j] == i) location = connections_b[j];
				if (connections_b[j] == i) location = connections_a[j];
				if (location != -1) {
					Endpoint::Direction dir = Endpoint::Direction::NONE;
					if (intersections[2 * i] < intersections[2 * location]) { // Our (i) x coordinate is less than the target's (location)
						dir = (Endpoint::Direction)(dir | Endpoint::Direction::LEFT);
					}
					if (intersections[2 * i] > intersections[2 * location]) {
						dir = (Endpoint::Direction)(dir | Endpoint::Direction::RIGHT);
					}
					if (intersections[2 * i + 1] < intersections[2 * location + 1]) { // y coordinate is 0 (bottom) 1 (top), so this check is reversed.
						dir = (Endpoint::Direction)(dir | Endpoint::Direction::DOWN);
					}
					if (intersections[2 * i + 1] > intersections[2 * location + 1]) {
						dir = (Endpoint::Direction)(dir | Endpoint::Direction::UP);
					}
					xd = (intersections[location * 2] - minx) / unitWidth;
					yd = (intersections[location * 2 + 1] - miny) / unitHeight;
					x = std::clamp((int)std::round(xd), 0, width - 1);
					y = height - 1 - std::clamp((int)std::round(yd), 0, height - 1);
					endpoints.emplace_back(Endpoint(x, y, dir, intersectionFlags[i]));
					break;
				}
			}
		}
		else {
			set(x, y, intersectionFlags[i]);
		}
	}	
}

void Panel::writeIntersections() {
	std::vector<float> intersections;
	std::vector<int> intersectionFlags;
	std::vector<int> connections_a;
	std::vector<int> connections_b;
	std::vector<int> symmetryData;
	std::vector<int> polygons;
	unitWidth = (maxx - minx) / (width - 1);
	if (isCylinder) unitWidth = 1.0f / width;
	unitHeight = (maxy - miny) / (height - 1);
	for (Point p : startpoints) {
		setFlag(p.x, p.y, STARTPOINT);
	}
	style &= ~HAS_DOTS;
	//Iterate intersections
	for (int y = height - 1; y >= 0; y -= 2) {
		for (int x = 0; x < width; x += 2) {
			intersections.push_back(static_cast<float>(minx + x * unitWidth));
			intersections.push_back(static_cast<float>(miny + (height - 1 - y) * unitHeight));
			if (get(x, y) == PATH) intersectionFlags.push_back(INTERSECTION);
			else if (getFlag(x, y, NO_POINT)) intersectionFlags.push_back(get(x, y));
			else intersectionFlags.push_back(get(x, y) | INTERSECTION);
			if (getFlag(x, y, DOT)) {
				style |= HAS_DOTS;
				if (getFlag(x, y, DOT_IS_BLUE) || getFlag(x, y, DOT_IS_ORANGE))
					style |= IS_2COLOR;
			}
			if (y > 0 && get(x, y - 1) != OPEN) {
				connections_a.push_back(pointToIndex(x, y - 2));
				connections_b.push_back(pointToIndex(x, y));
			}
			if (x > 0 && get(x - 1, y) != OPEN) {
				connections_a.push_back(pointToIndex(x - 2, y));
				connections_b.push_back(pointToIndex(x, y));
			}
			if (symmetry) {
				symmetryData.push_back(pointToIndex(getSymPoint(x, y).x, getSymPoint(x, y).y));
			}
		}
		if (isCylinder) {
			connections_a.push_back(pointToIndex(width - 2, y));
			connections_b.push_back(pointToIndex(0, y));
		}
	}
	if (symmetry) {
		//Rearrange exits to be in symmetric pairs
		for (int i = 0; i < endpoints.size(); i += 2) {
			Point sp = getSymPoint(endpoints[i].x, endpoints[i].y);
			for (int j = i + 1; j < endpoints.size(); j++) {
				if (endpoints[j].x == sp.x && endpoints[j].y == sp.y) {
					std::swap(endpoints[i + 1], endpoints[j]);
					break;
				}
			}
		}
	}
	double endDist = isCylinder ? 0.03 : 0.05;
	for (int i = 0; i < endpoints.size(); i++) {
		Endpoint endpoint = endpoints[i];
		int x = endpoint.x; int y = endpoint.y;
		if (x % 2 || y % 2) {
			breakSegment(x, y, connections_a, connections_b, intersections, intersectionFlags);
			connections_a.push_back(static_cast<int>(intersectionFlags.size()) - 1); // Target to connect to
		}
		else {
			connections_a.push_back(pointToIndex(endpoint.x, endpoint.y)); // Target to connect to
		}
		connections_b.push_back(static_cast<int>(intersectionFlags.size()));  // This endpoint
		double xPos = minx + endpoint.x * unitWidth;
		double yPos = miny + (height - 1 - endpoint.y) * unitHeight;
		if (endpoint.dir & Endpoint::Direction::LEFT) xPos -= endDist;
		if (endpoint.dir & Endpoint::Direction::RIGHT) xPos += endDist;
		if (endpoint.dir & Endpoint::Direction::UP) yPos += endDist;
		if (endpoint.dir & Endpoint::Direction::DOWN) yPos -= endDist;
		intersections.push_back(static_cast<float>(xPos));
		intersections.push_back(static_cast<float>(yPos));
		intersectionFlags.push_back(endpoint.flags);
		if (symmetry) {
			Point sp = getSymPoint(endpoint.x, endpoint.y);
			for (int j = 0; j < endpoints.size(); j++) {
				if (endpoints[j].x == sp.x && endpoints[j].y == sp.y) {
					symmetryData.push_back(getNumGridPoints() + j);
					break;
				}
				if (j == endpoints.size() - 1) symmetryData.push_back(getNumGridPoints() + i); //No exit matches up with it symmetrically
			}
		}
	}
	//Iterate edges, checking for mid-segment symbols
	for (int y = height - 1; y >= 0; y--) {
		for (int x = 0; x < width; x++) {
			if (x % 2 == y % 2) continue;
			if (get(x, y) == 0 || get(x, y) == OPEN || get(x, y) == PATH) continue;
			if (getFlag(x, y, DOT)) {
				style |= HAS_DOTS;
				if (getFlag(x, y, DOT_IS_BLUE) || getFlag(x, y, DOT_IS_ORANGE))
					style |= IS_2COLOR;
			}
			if (locateSegment(x, y, connections_a, connections_b) == -1)
				continue;
			if (getFlag(x, y, GAP)) {
				if (!breakSegmentGap(x, y, connections_a, connections_b, intersections, intersectionFlags))
					continue;
				if (symmetry) {
					auto[sx, sy] = getSymPoint(x, y);
					breakSegmentGap(sx, sy, connections_a, connections_b, intersections, intersectionFlags);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 1);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 2);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 3);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 4);
					if (x % 2 == 0 && getSymDir(Endpoint::Direction::UP, symmetry) == Endpoint::Direction::UP ||
						y % 2 == 0 && getSymDir(Endpoint::Direction::LEFT, symmetry) == Endpoint::Direction::LEFT || symmetry == Symmetry::FlipXY) {
						std::swap(symmetryData[symmetryData.size() - 1], symmetryData[symmetryData.size() - 2]);
						std::swap(symmetryData[symmetryData.size() - 3], symmetryData[symmetryData.size() - 4]);
					}
				}
			}
			else {
				if (get(x, y) == COLUMN || get(x, y) == ROW)
					continue;
				if (!breakSegment(x, y, connections_a, connections_b, intersections, intersectionFlags))
					continue;
				if (symmetry) {
					auto[sx, sy] = getSymPoint(x, y);
					if (!breakSegment(sx, sy, connections_a, connections_b, intersections, intersectionFlags))
						continue;
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 1);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 2);
				}
			}
		}
	}
	//Symmetry Data
	if (symmetry) {
		style |= Style::SYMMETRICAL;
		memory->WriteArray<int>(id, REFLECTION_DATA, symmetryData);
	}
	else {
		style &= ~Style::SYMMETRICAL;
		memory->WritePanelData<long long>(id, REFLECTION_DATA, 0);
	}
	memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) });
	memory->WriteArray<float>(id, DOT_POSITIONS, intersections);
	memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags);
	memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connections_a.size()) });
	memory->WriteArray<int>(id, DOT_CONNECTION_A, connections_a);
	memory->WriteArray<int>(id, DOT_CONNECTION_B, connections_b);
}

Endpoint::Direction Panel::getSymDir(Endpoint::Direction direction, Symmetry symmetry) {
	int dirIndex = -1;
	if (direction == Endpoint::Direction::LEFT) dirIndex = 0;
	if (direction == Endpoint::Direction::RIGHT) dirIndex = 1;
	if (direction == Endpoint::Direction::UP) dirIndex = 2;
	if (direction == Endpoint::Direction::DOWN) dirIndex = 3;
	std::vector<Endpoint::Direction> mapping;
	switch (symmetry) {
		case Symmetry::Horizontal: mapping = { Endpoint::Direction::LEFT, Endpoint::Direction::RIGHT, Endpoint::Direction::DOWN, Endpoint::Direction::UP }; break;
		case Symmetry::Vertical: mapping = { Endpoint::Direction::RIGHT, Endpoint::Direction::LEFT, Endpoint::Direction::UP, Endpoint::Direction::DOWN }; break;
		case Symmetry::Rotational: mapping = { Endpoint::Direction::RIGHT, Endpoint::Direction::LEFT, Endpoint::Direction::DOWN, Endpoint::Direction::UP }; break;
		case Symmetry::RotateLeft: mapping = { Endpoint::Direction::DOWN, Endpoint::Direction::UP, Endpoint::Direction::LEFT, Endpoint::Direction::RIGHT }; break;
		case Symmetry::RotateRight: mapping = { Endpoint::Direction::UP, Endpoint::Direction::DOWN, Endpoint::Direction::RIGHT, Endpoint::Direction::LEFT }; break;
		case Symmetry::FlipXY: mapping = { Endpoint::Direction::UP, Endpoint::Direction::DOWN, Endpoint::Direction::LEFT, Endpoint::Direction::RIGHT }; break;
		case Symmetry::FlipNegXY: mapping = { Endpoint::Direction::DOWN, Endpoint::Direction::UP, Endpoint::Direction::RIGHT, Endpoint::Direction::LEFT }; break;
		case Symmetry::ParallelH: mapping = { Endpoint::Direction::LEFT, Endpoint::Direction::RIGHT, Endpoint::Direction::UP, Endpoint::Direction::DOWN }; break;
		case Symmetry::ParallelV: mapping = { Endpoint::Direction::LEFT, Endpoint::Direction::RIGHT, Endpoint::Direction::UP, Endpoint::Direction::DOWN }; break;
		case Symmetry::ParallelHFlip: mapping = { Endpoint::Direction::RIGHT, Endpoint::Direction::LEFT, Endpoint::Direction::UP, Endpoint::Direction::DOWN }; break;
		case Symmetry::ParallelVFlip: mapping = { Endpoint::Direction::LEFT, Endpoint::Direction::RIGHT, Endpoint::Direction::DOWN, Endpoint::Direction::UP }; break;
		default: mapping = { Endpoint::Direction::LEFT, Endpoint::Direction::RIGHT, Endpoint::Direction::UP, Endpoint::Direction::DOWN }; break;
	}
	return mapping[dirIndex];
}

Color Panel::getColorRGB(SymbolColor color) {
	switch (color) {
		case Black: return { 0, 0, 0, 1 };
		case White: return { 1, 1, 1, 1 };
		case Red: return { 1, 0, 0, 1 };
		case Green: return { 0, 1, 0, 1 };
		case Blue: return { 0, 0, 1, 1 };
		case Cyan: return { 0, 1, 1, 1 };
		case Magenta: return { 1, 0, 1, 1 };
		case Yellow: return { 1, 1, 0, 1 };
		case Orange: return { 1, 0.5, 0, 1 };
		case Purple: return { 0.6f, 0, 1, 1 };
		case Invisible: { //Copy background color TODO: Broken on double mode
			Color xColor = memory->ReadPanelData<Color>(id, BACKGROUND_REGION_COLOR);
			xColor.a = 1;
			return xColor;
		}
	}
	return { 0, 0, 0, 0 };
}

Point Panel::indexToPoint(int index) {
	int height2 = (height - 1) / 2;
	int width2 = (width + 1) / 2;
	int x = 2 * (index % width2);
	int y = 2 * (height2 - index / width2);
	return { x, y };
}

int Panel::pointToIndex(int x, int y) {
	int height2 = (height - 1) / 2;
	int width2 = (width + 1) / 2;
	int rowsFromBottom = height2 - y / 2;
	return rowsFromBottom * width2 + x / 2;
}

Point Panel::decorationIndexToPoint(int location) {
	int height2 = (height - 3) / 2;
	int width2 = width / 2;
	int x = 2 * (location % width2) + 1;
	int y = 2 * (height2 - location / width2) + 1;
	return { x, y };
}

int Panel::pointToDecorationIndex(int x, int y) {
	int height2 = (height - 3) / 2;
	int width2 = width / 2;
	int rowsFromBottom = height2 - (y - 1) / 2;
	return rowsFromBottom * width2 + (x - 1) / 2;
}

int Panel::locateSegment(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b) {
	for (int i = 0; i < connections_a.size(); i++) {
		Point coord1 = indexToPoint(connections_a[i]);
		Point coord2 = indexToPoint(connections_b[i]);
		int x1 = coord1.x, y1 = coord1.y, x2 = coord2.x, y2 = coord2.y;
		if (isCylinder) {
			if ((x1 == (x - 1 + width) % width && x2 == (x + 1) % width && y1 == y && y2 == y) ||
				(y1 == y - 1 && y2 == y + 1 && x1 == x && x2 == x)) {
				return i;
			}
		}
		else if ((x1 == x - 1 && x2 == x + 1 && y1 == y && y2 == y) ||
			(y1 == y - 1 && y2 == y + 1 && x1 == x && x2 == x)) {
			return i;
		}
	}
	return -1;
}

bool Panel::breakSegment(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b, std::vector<float>& intersections, std::vector<int>& intersectionFlags) {
	int i = locateSegment(x, y, connections_a, connections_b);
	if (i == -1) return false;
	int other_connection = connections_b[i];
	connections_b[i] = static_cast<int>(intersectionFlags.size());
	connections_a.push_back(static_cast<int>(intersectionFlags.size()));
	connections_b.push_back(other_connection);
	intersections.push_back(static_cast<float>(minx + x * unitWidth));
	intersections.push_back(static_cast<float>(miny + (height - 1 - y) * unitHeight));
	intersectionFlags.push_back(get(x, y) == PATH ? INTERSECTION : get(x, y));
	return true;
}

bool Panel::breakSegmentGap(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b, std::vector<float>& intersections, std::vector<int>& intersectionFlags) {
	int i = locateSegment(x, y, connections_a, connections_b);
	if (i == -1) return false;
	int other_connection = connections_b[i];
	connections_b[i] = static_cast<int>(intersectionFlags.size() + 1);
	connections_a.push_back(other_connection);
	connections_b.push_back(static_cast<int>(intersectionFlags.size()));
	if (!getFlag(x, y, GAP)) {
		setFlag(x, y, x % 2 == 0 ? COLUMN : ROW);
		connections_a.push_back(static_cast<int>(intersectionFlags.size()));
		connections_b.push_back(static_cast<int>(intersectionFlags.size() + 1));
	}
	double xOffset = getFlag(x, y, ROW) ? 0.5 : 0;
	double yOffset = getFlag(x, y, COLUMN) ? 0.5 : 0;
	intersections.push_back(static_cast<float>(minx + (x + xOffset) * unitWidth));
	intersections.push_back(static_cast<float>(miny + (height - 1 - y - yOffset) * unitHeight));
	intersections.push_back(static_cast<float>(minx + (x - xOffset) * unitWidth));
	intersections.push_back(static_cast<float>(miny + (height - 1 - y + yOffset) * unitHeight));
	intersectionFlags.push_back(get(x, y) == PATH ? INTERSECTION : get(x, y));
	intersectionFlags.push_back(get(x, y) == PATH ? INTERSECTION : get(x, y));
	return true;
}
