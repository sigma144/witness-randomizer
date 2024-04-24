#pragma once

#include <map>
#include <stdint.h>
#include <tuple>
#include <vector>

struct Point {
	int first;
	int second;
	Point() { first = 0; second = 0; };
	Point(int x, int y) { if (pillarWidth) first = (x + pillarWidth) % pillarWidth; else first = x; second = y; }
	Point operator+(const Point& p) { return { first + p.first, second + p.second }; }
	Point operator*(int d) { return { first * d, second * d }; }
	Point operator/(int d) { return { first / d, second / d }; }
	bool operator==(const Point& p) const { return first == p.first && second == p.second; };
	bool operator!=(const Point& p) const { return first != p.first || second != p.second; };
	friend bool operator<(const Point& p1, const Point& p2) { if (p1.first == p2.first) return p1.second < p2.second; return p1.first < p2.first; };
	static int pillarWidth;
};

class Decoration
{
public:
	enum Shape : int {
		Exit =		0x600001,
		Start =		0x600002,
		Stone =		0x100,
		Star =		0x200,
		Poly =		0x400,
		Eraser =	0x500,
		Triangle =	0x600,
		Triangle1 = 0x10600,
		Triangle2 = 0x20600,
		Triangle3 = 0x30600,
		Triangle4 = 0x40600,
		Arrow =		0x700,
		Arrow1 = 0x1700,
		Arrow2 = 0x2700,
		Arrow3 = 0x3700,
		Can_Rotate = 0x1000,
		Negative = 0x2000,
		Gap = 0x100000,
		Gap_Row = 0x300000,
		Gap_Column = 0x500000,
		Dot = 0x20,
		Dot_Row = 0x240020,
		Dot_Column = 0x440020,
		Dot_Intersection = 0x600020,
		Empty = 0xA00,
	};
	enum Color : int {
		None = 0,
		Black = 0x1,
		White = 0x2,
		Red =	0x3, //Doesn't work sadly
		Purple = 0x4,
		Green = 0x5,
		Cyan = 0x6,
		Magenta = 0x7,
		Yellow = 0x8,
		Blue = 0x9,
		Orange = 0xA,
		X = 0xF,
	};
};

enum IntersectionFlags : int {
	ROW = 0x200000,
	COLUMN = 0x400000,
	INTERSECTION = 0x600000,
	ENDPOINT = 0x1,
	STARTPOINT = 0x2,
	OPEN = 0x3, //Puzzle loader flag - not to be written out
	PATH = 0x4, //Generator use only
	NO_POINT = 0x8, //Points that nothing connects to
	GAP = 0x100000,
	DOT = 0x20,
	DOT_IS_BLUE = 0x100,
	DOT_IS_ORANGE = 0x200,
	DOT_IS_INVISIBLE = 0x1000,
	DOT_SMALL = 0x2000,
	DOT_MEDIUM = 0x4000,
	DOT_LARGE = 0x8000,
};

class Endpoint {
public:
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
		_x = x;
		_y = y;
		_dir = dir;
		_flags = flags;
	}

	int GetX() {return _x;}
	void SetX(int x) {_x = x;}
	int GetY() {return _y;}
	void SetY(int y) {_y = y;}
	Direction GetDir() {return _dir;}
	int GetFlags() { return _flags; }
	void SetDir(Direction dir) {_dir = dir;}

private:
	int _x, _y, _flags;
	Direction _dir;
};

struct Color {
	float r;
	float g;
	float b;
	float a;
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
	Panel(int id);

	void Read();
	void Read(int id) { this->id = id; Read(); }
	void Write();
	void Write(int id) { this->id = id; Write(); }

	void SetSymbol(int x, int y, Decoration::Shape symbol, Decoration::Color color);
	void SetShape(int x, int y, int shape, bool rotate, bool negative, Decoration::Color color);
	void ClearSymbol(int x, int y);
	void SetGridSymbol(int x, int y, Decoration::Shape symbol, Decoration::Color color);
	void ClearGridSymbol(int x, int y);
	void Resize(int width, int height);

	Color GetBackgroundColor();

	static void StartArrowWatchdogs(const std::map<int, int>& shuffleMappings = {});

	enum Style {
		SYMMETRICAL = 0x2, //Not on the town symmetry puzzles? IDK why.
		NO_BLINK = 0x4,
		HAS_DOTS = 0x8,
		IS_2COLOR = 0x10,
		HAS_STARS = 0x40,
		HAS_TRIANGLES = 0x80,
		HAS_STONES = 0x100,
		HAS_ERASERS = 0x1000,
		HAS_SHAPERS = 0x2000,
		IS_PIVOTABLE = 0x8000,
	};

	enum Symmetry { //NOTE - Not all of these are valid symmetries for certain puzzles
		None, Horizontal, Vertical, Rotational,
		RotateLeft, RotateRight, FlipXY, FlipNegXY, ParallelH, ParallelV, ParallelHFlip, ParallelVFlip,
		PillarParallel, PillarHorizontal, PillarVertical, PillarRotational
	};
	Symmetry symmetry;

	float pathWidth;
	enum ColorMode { Default, Reset, Alternate, WriteColors, Treehouse, TreehouseAlternate };
	ColorMode colorMode;
	bool decorationsOnly;
	bool enableFlash;

private:

	void ReadAllData();
	void ReadIntersections();
	void WriteIntersections();
	void ReadDecorations();
	void WriteDecorations();

	Point get_sym_point(int x, int y, Symmetry symmetry)
	{
		switch (symmetry) {
		case None: return Point(x, y);
		case Symmetry::Horizontal: return Point(x, _height - 1 - y);
		case Symmetry::Vertical: return Point(_width - 1 - x, y);
		case Symmetry::Rotational: return Point(_width - 1 - x, _height - 1 - y);
		case Symmetry::RotateLeft: return Point(y, _width - 1 - x);
		case Symmetry::RotateRight: return Point(_height - 1 - y, x);
		case Symmetry::FlipXY: return Point(y, x);
		case Symmetry::FlipNegXY: return Point(_height - 1 - y, _width - 1 - x);
		case Symmetry::ParallelH: return Point(x, y == _height / 2 ? _height / 2 : (y + (_height + 1) / 2) % (_height + 1));
		case Symmetry::ParallelV: return Point(x == _width / 2 ? _width / 2 : (x + (_width + 1) / 2) % (_width + 1), y);
		case Symmetry::ParallelHFlip: return Point(_width - 1 - x, y == _height / 2 ? _height / 2 : (y + (_height + 1) / 2) % (_height + 1));
		case Symmetry::ParallelVFlip: return Point(x == _width / 2 ? _width / 2 : (x + (_width + 1) / 2) % (_width + 1), _height - 1 - y);
		case Symmetry::PillarParallel: return Point(x + _width / 2, y);
		case Symmetry::PillarHorizontal: return Point(x + _width / 2, _height - 1 - y);
		case Symmetry::PillarVertical: return Point( _width / 2 - x, y);
		case Symmetry::PillarRotational: return Point(_width / 2 - x, _height - 1 - y);
		}
		return Point(x, y);
	}

	Point get_sym_point(int x, int y) { return get_sym_point(x, y, symmetry); }
	Point get_sym_point(Point p) { return get_sym_point(p.first, p.second, symmetry); }
	Point get_sym_point(Point p, Symmetry symmetry) { return get_sym_point(p.first, p.second, symmetry); }
	Endpoint::Direction get_sym_dir(Endpoint::Direction direction, Symmetry symmetry) {
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
	int get_num_grid_points() { return ((_width + 1) / 2) * ((_height + 1) / 2); }
	int get_num_grid_blocks() { return (_width / 2) * (_height / 2);  }
	int get_parity() { return (get_num_grid_points() + 1) % 2; }
	Color get_color_rgb(int color) {
		if (colorMode == ColorMode::Treehouse) {
			switch (color) {
			case 1: return { 0, 0, 0, 1 }; //Black
			case 2: return { 1, 1, 1, 1 }; //White
			case 3: return { 1, 0, 0, 1 }; //Red (Not used)
			case 4: return { 1, 0, 1, 1 }; //Magenta
			case 5: return { 1, 0.5, 0, 1 }; //Orange
			case 6: return { 0, 1, 0, 1 }; //Green
			default: return { 0, 0, 0, 0 };
			}
		}
		else if (colorMode == ColorMode::TreehouseAlternate) {
			switch (color) {
			case 1: return { 0, 0, 0, 1 }; //Black
			case 2: return { 0, 0, 1, 1 }; //White->Blue
			case 3: return { 1, 0, 0, 1 }; //Red (Not used)
			case 4: return { 1, 0, 1, 1 }; //Magenta
			case 5: return { 1, 0.5, 0, 1 }; //Orange
			case 6: return { 1, 1, 1, 1 }; //Green->White
			default: return { 0, 0, 0, 0 };
			}
		}
		switch (color) {
		case Decoration::Color::Black: return { 0, 0, 0, 1 };
		case Decoration::Color::White: return { 1, 1, 1, 1 };
		case Decoration::Color::Red: return { 1, 0, 0, 1 };
		case Decoration::Color::Green: return { 0, 1, 0, 1 };
		case Decoration::Color::Blue: return { 0, 0, 1, 1 };
		case Decoration::Color::Cyan: return { 0, 1, 1, 1 };
		case Decoration::Color::Magenta: return { 1, 0, 1, 1 };
		case Decoration::Color::Yellow: return { 1, 1, 0, 1 };
		case Decoration::Color::Orange: return { 1, 0.5, 0, 1 };
		case Decoration::Color::Purple: return { 0.5, 0, 1, 1 };
		case Decoration::Color::X: {
			Color xColor = GetBackgroundColor();
			xColor.a = 1;
			return xColor;
		}
		default: return { 0, 0, 0, 0 };
		}
	}

	std::pair<int, int> loc_to_xy(int location) {
		int height2 = (_height - 1) / 2;
		int width2 = (_width + 1) / 2;

		int x = 2 * (location % width2);
		int y = 2 * (height2 - location / width2);
		return {x, y};
	}

	int xy_to_loc(int x, int y) {
		int height2 = (_height - 1) / 2;
		int width2 = (_width + 1) / 2;

		int rowsFromBottom = height2 - y/2;
		return rowsFromBottom * width2 + x/2;
	}

	std::pair<int, int> dloc_to_xy(int location) {
		int height2 = (_height - 3) / 2;
		int width2 = _width / 2;

		int x = 2 * (location % width2) + 1;
		int y = 2 * (height2 - location / width2) + 1;
		return {x, y};
	}

	int xy_to_dloc(int x, int y) {
		int height2 = (_height - 3) / 2;
		int width2 = _width / 2;

		int rowsFromBottom = height2 - (y - 1)/2;
		return rowsFromBottom * width2 + (x - 1)/2;
	}

	int locate_segment(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b) {
		for (int i = 0; i < connections_a.size(); i++) {
			std::pair<int,int> coord1 = loc_to_xy(connections_a[i]);
			std::pair<int,int> coord2 = loc_to_xy(connections_b[i]);
			int x1 = coord1.first, y1 = coord1.second, x2 = coord2.first, y2 = coord2.second;
			if (Point::pillarWidth) {
				if ((x1 == (x - 1 + Point::pillarWidth) % Point::pillarWidth && x2 == (x + 1) % Point::pillarWidth && y1 == y && y2 == y) ||
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

	bool break_segment(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b, std::vector<float>& intersections, std::vector<int>& intersectionFlags) {
		int i = locate_segment(x, y, connections_a, connections_b);
		if (i == -1) {
			return false;
		}
		int other_connection = connections_b[i];
		connections_b[i] = static_cast<int>(intersectionFlags.size());
		connections_a.push_back(static_cast<int>(intersectionFlags.size()));
		connections_b.push_back(other_connection);
		intersections.push_back(static_cast<float>(minx + x * unitWidth));
		intersections.push_back(static_cast<float>(miny + (_height - 1 - y) * unitHeight));
		intersectionFlags.push_back(_grid[x][y]);
		return true;
	}

	bool break_segment_gap(int x, int y, std::vector<int>& connections_a, std::vector<int>& connections_b, std::vector<float>& intersections, std::vector<int>& intersectionFlags) {
		int i = locate_segment(x, y, connections_a, connections_b);
		if (i == -1) {
			return false;
		}
		int other_connection = connections_b[i];
		connections_b[i] = static_cast<int>(intersectionFlags.size() + 1);
		connections_a.push_back(other_connection);
		connections_b.push_back(static_cast<int>(intersectionFlags.size()));
		if (!(_grid[x][y] & IntersectionFlags::GAP)) {
			_grid[x][y] |= (x % 2 == 0 ? IntersectionFlags::COLUMN : IntersectionFlags::ROW);
			connections_a.push_back(static_cast<int>(intersectionFlags.size()));
			connections_b.push_back(static_cast<int>(intersectionFlags.size() + 1));
		}
		double xOffset = _grid[x][y] & IntersectionFlags::ROW ? 0.5 : 0;
		double yOffset = _grid[x][y] & IntersectionFlags::COLUMN ? 0.5 : 0;
		intersections.push_back(static_cast<float>(minx + (x + xOffset) * unitWidth));
		intersections.push_back(static_cast<float>(miny + (_height - 1 - y - yOffset) * unitHeight));
		intersections.push_back(static_cast<float>(minx + (x - xOffset) * unitWidth));
		intersections.push_back(static_cast<float>(miny + (_height - 1 - y + yOffset) * unitHeight));
		intersectionFlags.push_back(_grid[x][y]);
		intersectionFlags.push_back(_grid[x][y]);
		return true;
	}

	void render_arrow(int x, int y, int ticks, int dir, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& polygons) {
		std::vector<float> positions = { 0.1f, 0.45f, 0.1f, 0.55f, 0.85f, 0.45f, 0.85f, 0.55f,
			0.9f, 0.5f, 0.75f, 0.5f, 0.45f, 0.2f, 0.6f, 0.2f, 0.45f, 0.8f, 0.6f, 0.8f,
			0.7f, 0.5f, 0.55f, 0.5f, 0.25f, 0.2f, 0.4f, 0.2f, 0.25f, 0.8f, 0.4f, 0.8f,
			0.5f, 0.5f, 0.35f, 0.5f, 0.05f, 0.2f, 0.2f, 0.2f, 0.05f, 0.8f, 0.2f, 0.8f, };
		std::vector<int> polys = { 0, 1, 2, 0, 1, 2, 3, 0,
			4, 5, 7, 0, 5, 6, 7, 0, 4, 5, 9, 0, 5, 8, 9, 0,
			10, 11, 13, 0, 11, 12, 13, 0, 10, 11, 15, 0, 11, 14, 15, 0,
			16, 17, 19, 0, 17, 18, 19, 0, 16, 17, 21, 0, 17, 20, 21, 0,
		};
		std::vector<int> angles = { -90, 90, 0, 180, -45, 45, 135, -135 };
		float angle = angles[dir] * 3.141592653589793238f / 180;
		for (int i = 0; i < positions.size(); i += 2) {
			//Translate to center
			positions[i] -= 0.5f;
			positions[i + 1] -= 0.5f;
			if (ticks == 3 && dir > 3) positions[i] += 0.1f;
			//Scale
			positions[i] *= unitHeight * 1.5f;
			positions[i + 1] *= unitHeight * 1.5f;
			//Rotate
			float newx = positions[i] * cos(angle) - positions[i + 1] * sin(angle);
			float newy = positions[i] * sin(angle) + positions[i + 1] * cos(angle);
			positions[i] = newx; positions[i + 1] = newy;
			//Translate to correct position
			positions[i] += intersections[xy_to_loc(x, y) * 2] + unitWidth;
			positions[i + 1] += intersections[xy_to_loc(x, y) * 2 + 1] - unitWidth;
		}
		int posIndex = 0, polyIndex = 0;
		if (ticks == 1) {
			posIndex = 20; polyIndex = 24;
		}
		if (ticks == 2) {
			posIndex = 32; polyIndex = 40;
		}
		if (ticks == 3) {
			posIndex = 44; polyIndex = 56;
		}
		int baseIndex = static_cast<int>(intersectionFlags.size());
		for (int i = 0; i < posIndex; i++) {
			intersections.push_back(positions[i]);
			if (i % 2 == 0) intersectionFlags.push_back(IntersectionFlags::NO_POINT);
		}
		for (int i = 0; i < polyIndex; i++) {
			polygons.push_back(polys[i] + baseIndex);
		}
	}

	int _width, _height;

	std::vector<std::vector<int>> _grid;
	std::vector<Point> _startpoints;
	std::vector<Endpoint> _endpoints;
	float minx, miny, maxx, maxy, unitWidth, unitHeight;
	int _style;
	bool _resized;
	int id;

	static std::vector<Panel> generatedPanels;
	static std::vector<std::tuple<int, int>> arrowPuzzles;

	friend class PanelExtractionTests;
	friend class Generate;
	friend class PuzzleList;
	friend class Special;
	friend class MultiGenerate;
	friend class ArrowWatchdog;
	friend class APRandomizer;
	friend class TextureLoader;
};