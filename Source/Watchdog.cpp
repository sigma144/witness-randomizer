// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Watchdog.h"
#include "PanelsUnlocks.h"
#include "PanelsChecks.h"
#include <thread>

void Watchdog::start()
{
	std::thread{ &Watchdog::run, this }.detach();
}

void Watchdog::run()
{
	while (!terminate) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime * 1000)));
		action();
	}
}

//Keep Watchdog - Keep the big panel off until all panels are solved
//The condition is not entirely correct, need to figure out how to check for sure if the puzzle is solved. Until then, the purple pressure plate panel (0x01BE9) is unskippable

void KeepWatchdog::action() {
	int numTraced = ReadPanelData<int>(0x01BE9, TRACED_EDGES);
	int tracedptr = ReadPanelData<int>(0x01BE9, TRACED_EDGE_DATA);
	std::vector<int> counts(26); std::fill(counts.begin(), counts.end(), 0);
	std::vector<SolutionPoint> traced; if (tracedptr) traced = ReadArray<SolutionPoint>(0x01BE9, TRACED_EDGE_DATA, numTraced);
	if (traced.size() < 12) {
		WritePanelData<float>(0x03317, POWER, { 0, 0 });
		return;
	}
	for (SolutionPoint p : traced) {
		if (p.pointA == p.pointB) continue;
		if (p.pointA > 25 || p.pointB > 25) continue;
		counts[p.pointA]++;
		counts[p.pointB]++;
	}
	for (int i = 0; i < 26; i++) {
		if (i == 4 || i == 25) {
			if (counts[i] != 1) {
				WritePanelData<float>(0x03317, POWER, { 0, 0 });
				return;
			}
		}
		else if (counts[i] != 0 && counts[i] != 2) {
			WritePanelData<float>(0x03317, POWER, { 0, 0 });
			return;
		}
	}
	WritePanelData<float>(0x03317, POWER, { 1, 1 });
	WritePanelData<int>(0x03317, NEEDS_REDRAW, { 1 });
}


// Completion manager test>
void TestWatchdog::action() {
	//THIS IS HOW TO TEST SOLVE STATE!
	int hasEverBeenSolved = ReadPanelData<int>(0x00061, 0x298);

	std::string test = std::to_string(hasEverBeenSolved);

	go++;

	for (int panelId : doorUnlockPanels) {
		WritePanelData<float>(panelId, POWER, { (go % 2) * 1.0f, (go % 2) * 1.0f });
		WritePanelData<int>(panelId, NEEDS_REDRAW, { 1 });
	}

	int doneChecks = 0;

	for (int id : standardChecks) {
		doneChecks += ReadPanelData<int>(id, 0x298);
	}

	OutputDebugStringA(std::to_string(doneChecks).c_str());
}

//Arrow Watchdog - To run the arrow puzzles

void ArrowWatchdog::action() {
	int length = ReadPanelData<int>(id, TRACED_EDGES);
	if (length != tracedLength) {
		complete = false;
	}
	if (length == 0 || complete) {
		sleepTime = 0.1f;
		return;
	}
	sleepTime = 0.01f;
	if (length == tracedLength) return;
	initPath();
	if (complete) {
		for (int x = 1; x < width; x++) {
			for (int y = 1; y < height; y++) {
				if (!checkArrow(x, y)) {
					//OutputDebugStringW(L"No");
					WritePanelData<int>(id, STYLE_FLAGS, { style | Panel::Style::HAS_TRIANGLES });
					return;
				}
			}
		}
		//OutputDebugStringW(L"Yes");
		WritePanelData<int>(id, STYLE_FLAGS, { style & ~Panel::Style::HAS_TRIANGLES });
	}
}

void ArrowWatchdog::initPath()
{
	int numTraced = ReadPanelData<int>(id, TRACED_EDGES);
	int tracedptr = ReadPanelData<int>(id, TRACED_EDGE_DATA);
	if (!tracedptr) return;
	std::vector<SolutionPoint> traced = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, numTraced);
	if (style & Panel::Style::SYMMETRICAL) {
		for (int i = 0; i < numTraced; i++) {
			SolutionPoint sp;
			if (traced[i].pointA >= exitPoint || traced[i].pointB >= exitPoint) {
				sp.pointA = sp.pointB = exitPoint;
			}
			else {
				sp.pointA = (width / 2 + 1) * (height / 2 + 1) - 1 - traced[i].pointA;
				sp.pointB = (width / 2 + 1) * (height / 2 + 1) - 1 - traced[i].pointB;
			}
			traced.push_back(sp);
		}
	}
	grid = backupGrid;
	tracedLength = numTraced;
	complete = false;
	if (traced.size() == 0) return;
	for (SolutionPoint p : traced) {
		int p1 = p.pointA, p2 = p.pointB;
		if (p1 == exitPoint || p2 == exitPoint) {
			complete = true;
			continue;
		}
		else if (p1 > exitPoint || p2 > exitPoint) continue;
		if (p1 == 0 && p2 == 0 || p1 < 0 || p2 < 0) {
			return;
		}
		int x1 = (p1 % (width / 2 + 1)) * 2, y1 = height - 1 - (p1 / (width / 2 + 1)) * 2;
		int x2 = (p2 % (width / 2 + 1)) * 2, y2 = height - 1 - (p2 / (width / 2 + 1)) * 2;
		if (pillarWidth > 0) {
			x1 = (p1 % (width / 2)) * 2, y1 = height - 1 - (p1 / (width / 2)) * 2;
			x2 = (p2 % (width / 2)) * 2, y2 = height - 1 - (p2 / (width / 2)) * 2;
			grid[x1][y1] = PATH;
			grid[x2][y2] = PATH;
			if (x1 == x2 || x1 == x2 + 2 || x1 == x2 - 2) grid[(x1 + x2) / 2][(y1 + y2) / 2] = PATH;
			else grid[width - 1][(y1 + y2) / 2] = PATH;
		}
		else {
			grid[x1][y1] = PATH;
			grid[x2][y2] = PATH;
			grid[(x1 + x2) / 2][(y1 + y2) / 2] = PATH;
		}
		if (p1 == exitPos || p2 == exitPos || (style & Panel::Style::SYMMETRICAL) && (p1 == exitPosSym || p2 == exitPosSym)) {
			complete = !complete;
		}
		else complete = false;
	}
}

bool ArrowWatchdog::checkArrow(int x, int y)
{
	if (pillarWidth > 0) return checkArrowPillar(x, y);
	int symbol = grid[x][y];
	if ((symbol & 0x700) == Decoration::Triangle && (symbol & 0xf0000) != 0) {
		int count = 0;
		if (grid[x - 1][y] == PATH) count++;
		if (grid[x + 1][y] == PATH) count++;
		if (grid[x][y - 1] == PATH) count++;
		if (grid[x][y + 1] == PATH) count++;
		return count == (symbol >> 16);
	}
	if ((symbol & 0x700) != Decoration::Arrow)
		return true;
	int targetCount = (symbol & 0xf000) >> 12;
	Point dir = DIRECTIONS[(symbol & 0xf0000) >> 16];
	x += dir.first / 2; y += dir.second / 2;
	int count = 0;
	while (x >= 0 && x < width && y >= 0 && y < height) {
		if (grid[x][y] == PATH) {
			if (++count > targetCount)
				return false;
		}
		x += dir.first; y += dir.second;
	}
	return count == targetCount;
}

bool ArrowWatchdog::checkArrowPillar(int x, int y)
{
	int symbol = grid[x][y];
	if ((symbol & 0x700) == Decoration::Triangle && (symbol & 0xf0000) != 0) {
		int count = 0;
		if (grid[x - 1][y] == PATH) count++;
		if (grid[x + 1][y] == PATH) count++;
		if (grid[x][y - 1] == PATH) count++;
		if (grid[x][y + 1] == PATH) count++;
		return count == (symbol >> 16);
	}
	if ((symbol & 0x700) != Decoration::Arrow)
		return true;
	int targetCount = (symbol & 0xf000) >> 12;
	Point dir = DIRECTIONS[(symbol & 0xf0000) >> 16];
	x = (x + (dir.first > 2 ? -2 : dir.first) / 2 + pillarWidth) % pillarWidth; y += dir.second / 2;
	int count = 0;
	while (y >= 0 && y < height) {
		if (grid[x][y] == PATH) {
			if (++count > targetCount) return false;
		}
		x = (x + dir.first + pillarWidth) % pillarWidth; y += dir.second;
	}
	return count == targetCount;
}

void BridgeWatchdog::action()
{
	int length1 = _memory->ReadPanelData<int>(id1, TRACED_EDGES);
	int length2 = _memory->ReadPanelData<int>(id2, TRACED_EDGES);
	if (solLength1 > 0 && length1 == 0) {
		_memory->WritePanelData<int>(id2, STYLE_FLAGS, { _memory->ReadPanelData<int>(id2, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (solLength2 > 0 && length2 == 0) {
		_memory->WritePanelData<int>(id1, STYLE_FLAGS, { _memory->ReadPanelData<int>(id1, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (length1 != solLength1 && length1 > 0 && !checkTouch(id2)) {
		_memory->WritePanelData<int>(id2, STYLE_FLAGS, { _memory->ReadPanelData<int>(id2, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	if (length2 != solLength2 && length2 > 0 && !checkTouch(id1)) {
		_memory->WritePanelData<int>(id1, STYLE_FLAGS, { _memory->ReadPanelData<int>(id1, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	solLength1 = length1;
	solLength2 = length2;
}

bool BridgeWatchdog::checkTouch(int id)
{
	int length = _memory->ReadPanelData<int>(id, TRACED_EDGES);
	if (length == 0) return false;
	int numIntersections = _memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<int> intersectionFlags = _memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<SolutionPoint> edges = _memory->ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, length);
	for (SolutionPoint sp : edges) if (intersectionFlags[sp.pointA] == Decoration::Dot_Intersection || intersectionFlags[sp.pointB] == Decoration::Dot_Intersection) return true;
	return false;
}

void TreehouseWatchdog::action()
{
	if (_memory->ReadPanelData<int>(0x03613, TRACED_EDGES) > 0) {
		_memory->WritePanelData<float>(0x17DAE, POWER, { 1.0f, 1.0f });
		_memory->WritePanelData<int>(0x17DAE, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
}
