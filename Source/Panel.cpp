// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Panel.h"
#include "Special.h"
#include "Memory.h"
#include "Randomizer.h"
#include "Watchdog.h"
#include <sstream>
#include <fstream>

int Point::pillarWidth = 0;
std::vector<Panel> Panel::generatedPanels;
std::vector<std::tuple<int, int>> Panel::arrowPuzzles;

template <class T>
int find(const std::vector<T> &data, T search, size_t startIndex = 0) {
	for (size_t i=startIndex ; i<data.size(); i++) {
		if (data[i] == search) return static_cast<int>(i);
	}
	return -1;
}

Panel::Panel(int id) {
	Read(id);
}

void Panel::Read() {
	Memory* memory = Memory::get();
	_width = 2 * memory->ReadPanelData<int>(id, GRID_SIZE_X) - 1;
	if (memory->ReadPanelData<int>(id, IS_CYLINDER)) {
		_width++;
		Point::pillarWidth = _width;
	}
	else Point::pillarWidth = 0;
	_height = 2 * memory->ReadPanelData<int>(id, GRID_SIZE_Y) - 1;
	if (_width <= 0 || _height <= 0 || _width > 30 || _height > 30) {
		int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
		_width = _height = static_cast<int>(std::round(sqrt(numIntersections))) * 2 - 1;
	}
	_grid.resize(_width);
	for (auto& row : _grid) row.resize(_height);
	for (int x = 0; x < _width; x++) {
		for (int y = 0; y < _height; y++) {
			_grid[x][y] = 0;
		}
	}
	_startpoints.clear();
	_endpoints.clear();

	_style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	ReadAllData();
	ReadIntersections();
	ReadDecorations();
	pathWidth = 1;
	_resized = false;
	colorMode = ColorMode::Default;
	decorationsOnly = false;
	enableFlash = false;
}

void Panel::Write() {
	Memory* memory = Memory::get();
	memory->WritePanelData<int>(id, GRID_SIZE_X, { (_width + 1) / 2 });
	memory->WritePanelData<int>(id, GRID_SIZE_Y, { (_height + 1) / 2 });
	if (_resized && memory->ReadPanelData<int>(id, NUM_COLORED_REGIONS) > 0) {
		//Make two triangles that cover the whole panel
		std::vector<int> newRegions = { 0, xy_to_loc(_width - 1, 0), xy_to_loc(0, 0), 0, xy_to_loc(_width - 1, _height - 1), xy_to_loc(_width - 1, 0), 0, 0 };
		memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(newRegions.size()) / 4 });
		memory->WriteArray(id, COLORED_REGIONS, newRegions);
	}

	if (!decorationsOnly) WriteIntersections();
	else {
		std::vector<int> iflags = memory->ReadArray<int>(id, DOT_FLAGS, memory->ReadPanelData<int>(id, NUM_DOTS));
		for (int x = 0; x < _width; x += 2) {
			for (int y = 0; y < _height; y += 2) {
				if (_grid[x][y] & Decoration::Dot) {
					iflags[x / 2 + (y / 2) * (_width / 2 + 1)] = _grid[x][y];
					_style |= Style::HAS_DOTS;
				}
			}
		}
		memory->WriteArray<int>(id, DOT_FLAGS, iflags);
	}
	WriteDecorations();
	if (enableFlash) _style &= ~NO_BLINK;
	memory->WritePanelData<int>(id, STYLE_FLAGS, { _style });
	if (pathWidth != 1) memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { pathWidth });
	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
	generatedPanels.push_back(*this);
}

void Panel::SetSymbol(int x, int y, Decoration::Shape symbol, Decoration::Color color)
{
	int gridx = x * 2 + (symbol & IntersectionFlags::COLUMN ? 0 : 1);
	int gridy = y * 2 + (symbol & IntersectionFlags::ROW ? 0 : 1);
	if (symbol & IntersectionFlags::DOT) {
		if (color == Decoration::Color::Blue || color == Decoration::Color::Cyan)
			color = static_cast<Decoration::Color>(IntersectionFlags::DOT_IS_BLUE);
		else if (color == Decoration::Color::Orange || color == Decoration::Color::Yellow)
			color = static_cast<Decoration::Color>(IntersectionFlags::DOT_IS_ORANGE);
		else color = Decoration::Color::None;
		if (symmetry) {
			Point sp = get_sym_point(gridx, gridy);
			SetGridSymbol(sp.first, sp.second, static_cast<Decoration::Shape>(symbol & ~Decoration::Dot), Decoration::Color::None);
		}
	}
	else if (symbol & IntersectionFlags::ROW || symbol & IntersectionFlags::COLUMN)
		color = Decoration::Color::None;
	SetGridSymbol(gridx, gridy, symbol, color);
}

void Panel::SetShape(int x, int y, int shape, bool rotate, bool negative, Decoration::Color color)
{
	if (!shape) return;
	int symbol = Decoration::Shape::Poly;
	while (!(shape & 0xf)) shape >>= 4;
	while (!(shape & 0x1111)) shape >>= 1;
	shape <<= 16;
	if (rotate) shape |= Decoration::Shape::Can_Rotate;
	else shape &= ~Decoration::Shape::Can_Rotate;
	if (negative) shape |= Decoration::Shape::Negative;
	else shape &= ~Decoration::Shape::Negative;
	_grid[x * 2 + 1][y * 2 + 1] = symbol | shape | color;
}

void Panel::ClearSymbol(int x, int y)
{
	ClearGridSymbol(x * 2 + 1, y * 2 + 1);
}

void Panel::SetGridSymbol(int x, int y, Decoration::Shape symbol, Decoration::Color color)
{
	if (symbol == Decoration::Start) _startpoints.push_back({ x, y });
	if (symbol == Decoration::Exit) {
		Endpoint::Direction dir;
		if (y == 0) dir = Endpoint::Direction::UP;
		else if (y == _height - 1) dir = Endpoint::Direction::DOWN;
		else if (x == 0) dir = Endpoint::Direction::LEFT;
		else dir = Endpoint::Direction::RIGHT;
		if (id == 0x033D4 || id == 0x0A3B5) {
			if (x == 0) dir = Endpoint::Direction::LEFT;
			else dir = Endpoint::Direction::RIGHT;
		}
		if (symmetry == Symmetry::ParallelH || symmetry == Symmetry::ParallelHFlip) {
			if (x == 0) dir = Endpoint::Direction::LEFT;
			if (x == _width - 1) dir = Endpoint::Direction::RIGHT;
		}
		_endpoints.emplace_back(Endpoint(x, y, dir, IntersectionFlags::ENDPOINT | 
			(dir == Endpoint::Direction::UP || dir == Endpoint::Direction::DOWN ?
				IntersectionFlags::COLUMN : IntersectionFlags::ROW)));
	}
	else _grid[x][y] = symbol | color;
}

void Panel::ClearGridSymbol(int x, int y)
{
	_grid[x][y] = 0;
}

void Panel::Resize(int width, int height)
{
	for (Point &s : _startpoints) {
		if (s.first == _width - 1) s.first = width - 1;
		if (s.second == _height - 1) s.second = height - 1;
	}
	for (Endpoint &e : _endpoints) {
		if (e.GetX() == _width - 1) e.SetX(width - 1);
		if (e.GetY() == _height - 1) e.SetY(height - 1);
	}
	if (_width != _height || width != height) {
		float maxDim = std::max(maxx - minx, maxy - miny);
		float unitSize = maxDim / std::max(width - 1, height - 1);
		minx = 0.5f - unitSize * (width - 1) / 2;
		maxx = 0.5f + unitSize * (width - 1) / 2;
		miny = 0.5f - unitSize * (height - 1) / 2;
		maxy = 0.5f + unitSize * (height - 1) / 2;
	}
	_width = width;
	_height = height;
	_grid.resize(width);
	for (auto& row : _grid) row.resize(height);
	_resized = true;
}

Color Panel::GetBackgroundColor() {
	return Memory::get()->ReadPanelData<Color>(0x0008F, BACKGROUND_REGION_COLOR);
}

void Panel::ReadAllData() {
	Memory* memory = Memory::get();
	Color pathColor = memory->ReadPanelData<Color>(id, PATH_COLOR);
	Color rpathColor = memory->ReadPanelData<Color>(id, REFLECTION_PATH_COLOR);
	Color successColor = memory->ReadPanelData<Color>(id, SUCCESS_COLOR_A);
	Color strobeColor = memory->ReadPanelData<Color>(id, STROBE_COLOR_A);
	Color errorColor = memory->ReadPanelData<Color>(id, ERROR_COLOR);
	int numDecorations = memory->ReadPanelData<int>(id, NUM_DECORATIONS);
	Color a = memory->ReadPanelData<Color>(id, SYMBOL_A);
	Color b = memory->ReadPanelData<Color>(id, SYMBOL_B);
	Color c = memory->ReadPanelData<Color>(id, SYMBOL_C);
	Color d = memory->ReadPanelData<Color>(id, SYMBOL_D);
	Color e = memory->ReadPanelData<Color>(id, SYMBOL_E);
	Color ppColor = memory->ReadPanelData<Color>(id, PATTERN_POINT_COLOR);
	Color ppColorA = memory->ReadPanelData<Color>(id, PATTERN_POINT_COLOR_A);
	Color ppColorB = memory->ReadPanelData<Color>(id, PATTERN_POINT_COLOR_B);
	int pushSymbolColors = memory->ReadPanelData<int>(id, PUSH_SYMBOL_COLORS);
	int numColored = memory->ReadPanelData<int>(id, NUM_COLORED_REGIONS);
	std::vector<int> colored = memory->ReadArray<int>(id, COLORED_REGIONS, numColored * 4);
	int numConnections = memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	int numDots = memory->ReadPanelData<int>(id, NUM_DOTS);
	int reflectionData = memory->ReadPanelData<int>(id, REFLECTION_DATA);
	std::vector<int> rdata; if (reflectionData) rdata = memory->ReadArray<int>(id, REFLECTION_DATA, numDots);
	int style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	std::vector<int> connections_a = memory->ReadArray<int>(id, DOT_CONNECTION_A, numConnections);
	std::vector<int> connections_b = memory->ReadArray<int>(id, DOT_CONNECTION_B, numConnections);
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<float> intersections = memory->ReadArray<float>(id, DOT_POSITIONS, numIntersections * 2);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<int> decorations = memory->ReadArray<int>(id, DECORATIONS, numDecorations);
	std::vector<int> decorationFlags = memory->ReadArray<int>(id, DECORATION_FLAGS, numDecorations);
	float width = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);
	int seqLen = memory->ReadPanelData<int>(id, SEQUENCE_LEN);
	std::vector<int> seq = memory->ReadArray<int>(id, SEQUENCE, seqLen);
	std::vector<float> power = memory->ReadPanelData<float>(id, POWER, 2);
	float openRate = memory->ReadPanelData<float>(id, OPEN_RATE);
	int cptr = memory->ReadPanelData<int>(id, DECORATION_COLORS);
	std::vector<Color> colors; if (cptr) colors = memory->ReadArray<Color>(id, DECORATION_COLORS, numDecorations);
	Color outerBackground = memory->ReadPanelData<Color>(id, OUTER_BACKGROUND);
	int outerBackgroundMode = memory->ReadPanelData<int>(id, OUTER_BACKGROUND_MODE);
	Color bgRegionColor = memory->ReadPanelData<Color>(id, BACKGROUND_REGION_COLOR);
	short metadata = memory->ReadPanelData<short>(id, METADATA);
	//void* specularTexture = memory->ReadPanelData<void*>(id, SPECULAR_TEXTURE);
	//std::vector<float> data = memory->ReadPanelData<float>(id, SPECULAR_TEXTURE, 1000);
	int dotSeqLen = memory->ReadPanelData<int>(id, DOT_SEQUENCE_LEN);
	std::vector<int> dotSeq = memory->ReadArray<int>(id, DOT_SEQUENCE, dotSeqLen);
	int dotSeqLenR = memory->ReadPanelData<int>(id, DOT_SEQUENCE_LEN_REFLECTION);
	std::vector<int> dotSeqR = memory->ReadArray<int>(id, DOT_SEQUENCE_REFLECTION, dotSeqLenR);
	void* target = memory->ReadPanelData<void*>(id, TARGET);
	void* panelTarget = memory->ReadPanelData<void*>(id, PANEL_TARGET);
	Color cableTarget = memory->ReadPanelData<Color>(id, CABLE_TARGET_2);
	//std::vector<int> targets = memory->ReadArray<int>(id, PANEL_TARGET, 6);
	int isPillar = memory->ReadPanelData<int>(id, IS_CYLINDER);
	int numTraced = memory->ReadPanelData<int>(id, TRACED_EDGES);
	int numSol = memory->ReadPanelData<int>(id, TRACED_EDGES + 4); //Don't know what this number is for yet
	int tracedptr = memory->ReadPanelData<int>(id, TRACED_EDGE_DATA);
	//float solved = memory->ReadPanelData<float>(id, SOLVED);
	float distance = memory->ReadPanelData<float>(id, MAX_BROADCAST_DISTANCE);
	std::vector<SolutionPoint> traced; if (tracedptr) traced = memory->ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, numTraced);
}

void Panel::ReadDecorations() {
	Memory* memory = Memory::get();
	int numDecorations = memory->ReadPanelData<int>(id, NUM_DECORATIONS);
	std::vector<int> decorations = memory->ReadArray<int>(id, DECORATIONS, numDecorations);
	std::vector<int> decorationFlags = memory->ReadArray<int>(id, DECORATION_FLAGS, numDecorations);

	for (int i=0; i<numDecorations; i++) {
		auto [x, y] = dloc_to_xy(i);
		_grid[x][y] = decorations[i];
	}
}

void Panel::WriteDecorations() {
	Memory* memory = Memory::get();
	std::vector<int> decorations;
	std::vector<Color> decorationColors;
	bool any = false;
	bool arrows = false;
	_style &= ~0x3fc0; //Remove all element flags
	for (int y=_height-2; y>0; y-=2) {
		for (int x=1; x<_width; x+=2) {
			if (colorMode == ColorMode::Treehouse || colorMode == ColorMode::TreehouseAlternate) {
				if ((_grid[x][y] & 0xf) == Decoration::Color::Green) {
					_grid[x][y] &= ~0xf; _grid[x][y] |= 6;
				}
				if ((_grid[x][y] & 0xf) == Decoration::Color::Orange) {
					_grid[x][y] &= ~0xf; _grid[x][y] |= 5;
				}
				if ((_grid[x][y] & 0xf) == Decoration::Color::Magenta) {
					_grid[x][y] &= ~0xf; _grid[x][y] |= 4;
				}
			}
			decorations.push_back(_grid[x][y]);
			decorationColors.push_back(get_color_rgb(_grid[x][y] & 0xf));
			if (_grid[x][y])
				any = true;
			if ((_grid[x][y] & 0x700) == Decoration::Shape::Stone) _style |= HAS_STONES;
			if ((_grid[x][y] & 0x700) == Decoration::Shape::Star) _style |= HAS_STARS;
			if ((_grid[x][y] & 0x700) == Decoration::Shape::Poly) _style |= HAS_SHAPERS;
			if ((_grid[x][y] & 0x700) == Decoration::Shape::Eraser) _style |= HAS_ERASERS;
			if ((_grid[x][y] & 0x700) == Decoration::Shape::Triangle) _style |= HAS_TRIANGLES;
			if ((_grid[x][y] & 0x700) == Decoration::Shape::Arrow) {
				_style |= HAS_TRIANGLES | HAS_STONES;
				arrows = true;
			}
		}
	}
	if (arrows) {
		for (int i = 0; i < decorations.size(); i++) {
			if (decorations[i] == 0) decorations[i] = Decoration::Triangle; //To force it to be unsolvable
		}
		memory->WritePanelData<int>(id, OUTER_BACKGROUND_MODE, { 1 });
	}
	if (!any) {
		memory->WritePanelData<int>(id, NUM_DECORATIONS, { 0 });
	}
	else {
		memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
		if (colorMode == ColorMode::WriteColors || colorMode == ColorMode::Treehouse || colorMode == ColorMode::TreehouseAlternate || memory->ReadPanelData<int>(id, DECORATION_COLORS))
			memory->WriteArray<Color>(id, DECORATION_COLORS, decorationColors);
		else if (colorMode == ColorMode::Reset || colorMode == ColorMode::Alternate) {
			memory->WritePanelData<int>(id, PUSH_SYMBOL_COLORS, { colorMode == ColorMode::Reset ? 0 : 1 });
		}
		if (colorMode == ColorMode::Treehouse) {
			memory->WritePanelData<int>(id, PUSH_SYMBOL_COLORS, { 1 });
			memory->WritePanelData<Color>(id, SYMBOL_A, { { 0, 0, 0, 1 } }); //Black
			memory->WritePanelData<Color>(id, SYMBOL_B, { { 1, 1, 1, 1 } }); //White
			memory->WritePanelData<Color>(id, SYMBOL_C, { { 1, 0.5, 0, 1 } }); //Orange
			memory->WritePanelData<Color>(id, SYMBOL_D, { { 1, 0, 1, 1 } }); //Magenta
			memory->WritePanelData<Color>(id, SYMBOL_E, { { 0, 1, 0, 1 } }); //Green
		}
		else if (colorMode == ColorMode::TreehouseAlternate) {
			memory->WritePanelData<int>(id, PUSH_SYMBOL_COLORS, { 1 });
			memory->WritePanelData<Color>(id, SYMBOL_A, { { 0, 0, 0, 1 } }); //Black
			memory->WritePanelData<Color>(id, SYMBOL_B, { { 0, 0, 1, 1 } }); //White->Blue
			memory->WritePanelData<Color>(id, SYMBOL_C, { { 1, 0.5, 0, 1 } }); //Orange
			memory->WritePanelData<Color>(id, SYMBOL_D, { { 1, 0, 1, 1 } }); //Magenta
			memory->WritePanelData<Color>(id, SYMBOL_E, { { 1, 1, 1, 1 } }); //Green->White
		}
	}
	if (any || memory->ReadPanelData<int>(id, DECORATIONS)) {
		memory->WriteArray<int>(id, DECORATIONS, decorations);
		for (int i = 0; i < decorations.size(); i++) decorations[i] = 0;
		memory->WriteArray<int>(id, DECORATION_FLAGS, decorations);
	}
	if (arrows) {
		arrowPuzzles.emplace_back(id, Point::pillarWidth);
	}
}

void Panel::StartArrowWatchdogs(const std::map<int, int>& shuffleMappings) {
	std::map<int, int> invertedMappings;
	for (const auto& [from, to] : shuffleMappings) {
		invertedMappings[to] = from;
	}
	for (const auto& [id, pillarWidth] : arrowPuzzles) {
		int realId = id;
		if (invertedMappings.count(realId)) realId = invertedMappings.at(realId);

		ArrowWatchdog* watchdog = new ArrowWatchdog(realId, pillarWidth);
		watchdog->start();
	}
}

void Panel::ReadIntersections() {
	Memory* memory = Memory::get();
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<float> intersections = memory->ReadArray<float>(id, DOT_POSITIONS, numIntersections * 2);
	int num_grid_points = this->get_num_grid_points();
	minx = intersections[0]; miny = intersections[1];
	maxx = intersections[num_grid_points * 2 - 2]; maxy = intersections[num_grid_points * 2 - 1];
	if (minx > maxx) std::swap(minx, maxx);
	if (miny > maxy) std::swap(miny, maxy);
	unitWidth = (maxx - minx) / (_width - 1);
	if (Point::pillarWidth) unitWidth = 1.0f / _width;
	unitHeight = (maxy - miny) / (_height - 1);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<int> symmetryData = memory->ReadPanelData<int>(id, REFLECTION_DATA) ? 
		memory->ReadArray<int>(id, REFLECTION_DATA, numIntersections) : std::vector<int>();
	if (symmetryData.size() == 0) symmetry = Symmetry::None;
	else if (symmetryData[0] == num_grid_points - 1) symmetry = Symmetry::Rotational;
	else if (symmetryData[0] == _width / 2 && intersections[1] == intersections[3]) symmetry = Symmetry::Vertical;
	else symmetry = Symmetry::Horizontal;

	for (int i = 0; i < num_grid_points; i++) {
		int x = static_cast<int>(std::round((intersections[i * 2] - minx) / unitWidth));
		int y = _height - 1 - static_cast<int>(std::round((intersections[i * 2 + 1] - miny) / unitHeight));
		_grid[x][y] = intersectionFlags[i];
		if (intersectionFlags[i] & IntersectionFlags::STARTPOINT) {
			_startpoints.push_back({x, y});
		}
	}

	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			if (x % 2 == y % 2) continue;
			_grid[x][y] = OPEN;
		}
	}
	int numConnections = memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	std::vector<int> connections_a = memory->ReadArray<int>(id, DOT_CONNECTION_A, numConnections);
	std::vector<int> connections_b = memory->ReadArray<int>(id, DOT_CONNECTION_B, numConnections);
	//Remove non-existent connections
	std::vector<std::string> out;
	for (int i = 0; i < connections_a.size(); i++) {
		out.push_back("(" + std::to_string(connections_a[i]) + ", " + std::to_string(connections_b[i]) + ")");
		if (connections_a[i] >= num_grid_points || connections_b[i] >= num_grid_points) continue;
		int x = static_cast<int>(std::round((intersections[connections_a[i] * 2] - minx) / unitWidth));
		int y = _height - 1 - static_cast<int>(std::round((intersections[connections_a[i] * 2 + 1] - miny) / unitHeight));
		int x2 = static_cast<int>(std::round((intersections[connections_b[i] * 2] - minx) / unitWidth));
		int y2 = _height - 1 - static_cast<int>(std::round((intersections[connections_b[i] * 2 + 1] - miny) / unitHeight));
		_grid[(x + x2) / 2][(y + y2) / 2] = 0;
	}

	// Iterate the remaining intersections (endpoints, dots, gaps)
	for (int i = num_grid_points; i < numIntersections; i++) {
		float xd = (intersections[i * 2] - minx) / unitWidth;
		float yd = (intersections[i * 2 + 1] - miny) / unitHeight;
		int x = std::clamp((int)std::round(xd), 0, _width - 1);
		int y = _height - 1 - std::clamp((int)std::round(yd), 0, _height - 1);
		if (intersectionFlags[i] & IntersectionFlags::GAP) {
			float xd2 = (intersections[i * 2 + 2] - minx) / unitWidth;
			float yd2 = (intersections[i * 2 + 3] - miny) / unitHeight;
			x = std::clamp((int)std::round((xd + xd2) / 2), 0, _width - 1);
			y = _height - 1 - std::clamp((int)std::round((yd + yd2) / 2), 0, _height - 1);
			bool fake = false;
			for (int j = 0; j < numConnections; j++) {
				if (connections_a[j] == i && connections_b[j] == i + 1 ||
					connections_a[j] == i + 1 && connections_b[j] == i) {
					//Fake symmetry wall
					fake = true;
					_grid[x][y] = 0;
					i++;
					break;
				}
			}
			if (fake) continue;
			i++;
		}
		if (intersectionFlags[i] & IntersectionFlags::ENDPOINT) {
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
					x = std::clamp((int)std::round(xd), 0, _width - 1);
					y = _height - 1 - std::clamp((int)std::round(yd), 0, _height - 1);
					_endpoints.emplace_back(Endpoint(x, y, dir, intersectionFlags[i]));
					break;
				}
			}
		}

		else {
			_grid[x][y] = intersectionFlags[i];
			//if (intersectionFlags[i] & IntersectionFlags::STARTPOINT) { //Glitchy right now
			//	_startpoints.push_back({ x, y });
			//}
		}
	}	
}

void Panel::WriteIntersections() {
	Memory* memory = Memory::get();
	std::vector<float> intersections;
	std::vector<int> intersectionFlags;
	std::vector<int> connections_a;
	std::vector<int> connections_b;
	std::vector<int> symmetryData;
	std::vector<int> polygons;

	unitWidth = (maxx - minx) / (_width - 1);
	if (Point::pillarWidth) unitWidth = 1.0f / _width;
	unitHeight = (maxy - miny) / (_height - 1);

	for (Point p : _startpoints) {
		_grid[p.first][p.second] |= STARTPOINT;
	}

	_style &= ~HAS_DOTS;

	for (int y = _height - 1; y >= 0; y -= 2) {
		for (int x = 0; x <_width; x += 2) {
			intersections.push_back(static_cast<float>(minx + x * unitWidth));
			intersections.push_back(static_cast<float>(miny + (_height - 1 - y) * unitHeight));
			if (_grid[x][y] & IntersectionFlags::NO_POINT) intersectionFlags.push_back(_grid[x][y]);
			else intersectionFlags.push_back(_grid[x][y] | IntersectionFlags::INTERSECTION);
			if (_grid[x][y] & DOT) {
				_style |= HAS_DOTS;
				if (_grid[x][y] & IntersectionFlags::DOT_IS_BLUE || _grid[x][y] & IntersectionFlags::DOT_IS_ORANGE)
					_style |= IS_2COLOR;
			}

			// Create connections for this intersection -- always write low -> high
			if (y > 0 && _grid[x][y - 1] != OPEN) {
				connections_a.push_back(xy_to_loc(x, y - 2));
				connections_b.push_back(xy_to_loc(x, y));
			}
			if (x > 0 && _grid[x - 1][y] != OPEN) {
				connections_a.push_back(xy_to_loc(x - 2, y));
				connections_b.push_back(xy_to_loc(x, y));
			}
			if (symmetry) {
				symmetryData.push_back(xy_to_loc(get_sym_point(x, y).first, get_sym_point(x, y).second));
			}
		}
		if (Point::pillarWidth) {
			connections_a.push_back(xy_to_loc(_width - 2, y));
			connections_b.push_back(xy_to_loc(0, y));
		}
	}

	std::vector<std::string> out;
	for (int i = 0; i < connections_a.size(); i++) {
		out.push_back(std::to_string(connections_a[i]) + " -> " + std::to_string(connections_b[i]));
	}

	if (symmetry) {
		//Rearrange exits to be in symmetric pairs
		for (int i = 0; i < _endpoints.size(); i += 2) {
			Point sp = get_sym_point(_endpoints[i].GetX(), _endpoints[i].GetY());
			for (int j = i + 1; j < _endpoints.size(); j++) {
				if (_endpoints[j].GetX() == sp.first && _endpoints[j].GetY() == sp.second) {
					std::swap(_endpoints[i + 1], _endpoints[j]);
					break;
				}
			}
		}
	}

	double endDist = Point::pillarWidth == 0 ? 0.05 : 0.03;

	for (int i = 0; i < _endpoints.size(); i++) {
		Endpoint endpoint = _endpoints[i];
		int x = endpoint.GetX(); int y = endpoint.GetY();
		if (x % 2 || y % 2) {
			break_segment(x, y, connections_a, connections_b, intersections, intersectionFlags);
			connections_a.push_back(static_cast<int>(intersectionFlags.size()) - 1); // Target to connect to
		}
		else {
			connections_a.push_back(xy_to_loc(endpoint.GetX(), endpoint.GetY())); // Target to connect to
		}
		connections_b.push_back(static_cast<int>(intersectionFlags.size()));  // This endpoint
		double xPos = minx + endpoint.GetX() * unitWidth;
		double yPos = miny + (_height - 1 - endpoint.GetY()) * unitHeight;
		if (endpoint.GetDir() & Endpoint::Direction::LEFT) {
			xPos -= endDist;
		}
		if (endpoint.GetDir() & Endpoint::Direction::RIGHT) {
			xPos += endDist;
		}
		if (endpoint.GetDir() & Endpoint::Direction::UP) {
			yPos += endDist;
		}
		if (endpoint.GetDir() & Endpoint::Direction::DOWN) {
			yPos -= endDist;
		}
		intersections.push_back(static_cast<float>(xPos));
		intersections.push_back(static_cast<float>(yPos));
		intersectionFlags.push_back(endpoint.GetFlags());
		if (symmetry) {
			Point sp = get_sym_point(endpoint.GetX(), endpoint.GetY());
			for (int j = 0; j < _endpoints.size(); j++) {
				if (_endpoints[j].GetX() == sp.first && _endpoints[j].GetY() == sp.second) {
					symmetryData.push_back(get_num_grid_points() + j);
					break;
				}
				if (j == _endpoints.size() - 1) symmetryData.push_back(get_num_grid_points() + i); //No exit matches up with it symmetrically
			}
		}
	}

	// Dots/Gaps
	for (int y = _height - 1; y >= 0; y--) {
		for (int x = 0; x < _width; x++) {
			if (x % 2 == y % 2) continue;
			if (_grid[x][y] == 0 || _grid[x][y] == OPEN) continue;
			if (_grid[x][y] & IntersectionFlags::DOT) {
				_style |= HAS_DOTS;
				if (_grid[x][y] & IntersectionFlags::DOT_IS_BLUE || _grid[x][y] & IntersectionFlags::DOT_IS_ORANGE)
					_style |= IS_2COLOR;
			}
			if (locate_segment(x, y, connections_a, connections_b) == -1)
				continue;
			if (_grid[x][y] & IntersectionFlags::GAP) {
				if (!break_segment_gap(x, y, connections_a, connections_b, intersections, intersectionFlags))
					continue;
				if (symmetry) {
					auto[sx, sy] = get_sym_point(x, y);
					break_segment_gap(sx, sy, connections_a, connections_b, intersections, intersectionFlags);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 1);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 2);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 3);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 4);
					if (x % 2 == 0 && get_sym_dir(Endpoint::Direction::UP, symmetry) == Endpoint::Direction::UP ||
						y % 2 == 0 && get_sym_dir(Endpoint::Direction::LEFT, symmetry) == Endpoint::Direction::LEFT || symmetry == Symmetry::FlipXY) {
						std::swap(symmetryData[symmetryData.size() - 1], symmetryData[symmetryData.size() - 2]);
						std::swap(symmetryData[symmetryData.size() - 3], symmetryData[symmetryData.size() - 4]);
					}
				}
			}
			else {
				if (_grid[x][y] == IntersectionFlags::COLUMN || _grid[x][y] == IntersectionFlags::ROW)
					continue;
				if (!break_segment(x, y, connections_a, connections_b, intersections, intersectionFlags))
					continue;
				if (symmetry) {
					auto[sx, sy] = get_sym_point(x, y);
					if (!break_segment(sx, sy, connections_a, connections_b, intersections, intersectionFlags))
						continue;
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 1);
					symmetryData.push_back(static_cast<int>(intersectionFlags.size()) - 2);
				}
			}
		}
	}

	//Arrows (if applicable)
	for (int y = 1; y < _height; y += 2) {
		for (int x = 1; x < _width; x += 2) {
			if ((_grid[x][y] & 0x700) == Decoration::Arrow)
				render_arrow(x, y, (_grid[x][y] & 0xf000) >> 12, (_grid[x][y] & 0xf0000) >> 16, intersections, intersectionFlags, polygons);
		}
	}

	//Symmetry Data
	if (id == 0x01D3F && symmetry == Symmetry::None || id == 0x00076 && symmetry == Symmetry::None) {
		_style &= ~Style::SYMMETRICAL;
		memory->WritePanelData<long long>(id, REFLECTION_DATA, { 0 });
	}
	else if (symmetryData.size() > 0) {
		_style |= Style::SYMMETRICAL;
		memory->WriteArray<int>(id, REFLECTION_DATA, symmetryData);
	}
	else {
		_style &= ~Style::SYMMETRICAL;
		memory->WritePanelData<long long>(id, REFLECTION_DATA, { 0 });
	}

	memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) });
	memory->WriteArray<float>(id, DOT_POSITIONS, intersections);
	memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags);
	memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connections_a.size()) });
	memory->WriteArray<int>(id, DOT_CONNECTION_A, connections_a);
	memory->WriteArray<int>(id, DOT_CONNECTION_B, connections_b);
	
	if (polygons.size() > 0) {
		memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(polygons.size()) / 4 });
		memory->WriteArray<int>(id, COLORED_REGIONS, polygons);
	}
}