#pragma once

#include <set>
#include <map>
#include <stdint.h>
#include <tuple>
#include <functional>
#include "SymbolData.h"
#include "Enums.h"

struct Point {
	int x, y;
	Point() { x = 0; y = 0; };
	Point(int x, int y) { this->x = x; this->y = y; }
	Point operator+(const Point& p) { return { x + p.x, y + p.y }; }
	Point operator*(int d) { return { x * d, y * d }; }
	Point operator/(int d) { return { x / d, y / d }; }
	bool operator==(const Point& p) const { return x == p.x && y == p.y; };
	bool operator!=(const Point& p) const { return x != p.x || y != p.y; };
	friend bool operator<(const Point& p1, const Point& p2) { if (p1.x == p2.x) return p1.y < p2.y; return p1.x < p2.x; };
};

struct Endpoint {
	enum Direction {
		NONE = 0,
		LEFT = 1,
		RIGHT = 2,
		UP = 4,
		DOWN = 8,
		UP_LEFT = 5,
		UP_RIGHT = 6,
		DOWN_LEFT = 9,
		DOWN_RIGHT = 10
	};
	Endpoint(int x, int y, Direction dir, int flags) {
		this->x = x; this->y = y; this->dir = dir; this->flags = flags;
	}
	int x, y, flags;
	Direction dir;
};

struct Color {
	float r, g, b, a;
	friend bool operator <(const Color& lhs, const Color& rhs) {return lhs.r * 8 + lhs.g * 4 + lhs.b * 2 + lhs.a > rhs.r * 8 + rhs.g * 4 + rhs.b * 2 + rhs.a;}
};

struct SolutionPoint {
	int pointA, pointB, pointC, pointD;
	float f1x, f1y, f2x, f2y, f3x, f3y, f4x, f4y;
	int endnum;
};

class Panel
{
public:
	Panel();
	Panel(PanelID id);

	void read();
	void read(PanelID id) { this->id = id; read(); }
	void write();
	void write(PanelID id) { this->id = id; write(); }
	int get(int x, int y);
	void set(int x, int y, int val);
	int get(Point p) { return get(p.x, p.y); }
	void set(Point p, int val) { set(p.x, p.y, val); }
	std::vector<std::vector<int>> getGrid() { return grid; }
	void setGrid(std::vector<std::vector<int>> grid) { this->grid = grid; }
	int getFlag(int x, int y, int flag) { return get(x, y) & flag; };
	void setFlag(int x, int y, int flag) { set(x, y, get(x, y) | flag); };
	void clearFlag(int x, int y, int flag) { set(x, y, get(x, y) & ~flag); };
	int getSymbolColor(int x, int y) { return grid[x][y] & 0xF; }
	int getSymbolShape(int x, int y) { return grid[x][y] & 0xF00; }
	void setSymbol(int x, int y, Symbol symbol, SymbolColor color);
	void setShape(int x, int y, int shape, bool rotate, bool negative, SymbolColor color);
	void setGridSymbol(int x, int y, Symbol symbol, SymbolColor color); //TODO: Rename this?
	void resize(int width, int height);
	int getNumGridPoints() { return (isCylinder ? width / 2 : (width + 1) / 2) * ((height + 1) / 2); }
	int getNumGridBlocks() { return (width / 2) * (height / 2); }
	int getParity() { return (getNumGridPoints() + 1) % 2; }

	std::set<Point> getRegion(Point pos);
	std::vector<int> getSymbolsInRegion(Point pos);
	Point getSymPoint(int x, int y) { return getSymPoint(x, y, symmetry); }
	Point getSymPoint(int x, int y, Symmetry symmetry);
	int countColor(const std::set<Point>& region, int color);
	int countSides(Point pos);
	int countCrossings(Point pos, Point dir);

	Point indexToPoint(int index);
	int pointToIndex(int x, int y);
	Point decorationIndexToPoint(int index);
	int pointToDecorationIndex(int x, int y);

	static std::vector<Point> DIRECTIONS, DIRECTIONS8, DIRECTIONS_2, DIRECTIONS8_2;

	PanelID id;
	int width, height; //Dimensions of internal grid, including both edge and block spaces
	std::vector<Point> startpoints;
	std::vector<Endpoint> endpoints;
	Symmetry symmetry;
	int style;
	float lineThickness;
	bool isCylinder, disableFlash, decorationsOnly;
	enum ColorMode { Default, Reset, Alternate, WriteColors };
	ColorMode colorMode;

private:

	void readIntersections();
	void writeIntersections();
	void readDecorations();
	void writeDecorations();

	Endpoint::Direction getSymDir(Endpoint::Direction direction, Symmetry symmetry);
	Color getColorRGB(SymbolColor color);
	int locateSegment(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b);
	bool breakSegment(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b, std::vector<float>& intersections, std::vector<int>& intersectionFlags);
	bool breakSegmentGap(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b, std::vector<float>& intersections, std::vector<int>& intersectionFlags);

	std::vector<std::vector<int>> grid;
	float minx, miny, maxx, maxy, unitWidth, unitHeight;
	bool resized;

	friend class Special;
};