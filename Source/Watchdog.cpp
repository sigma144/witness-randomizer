// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Watchdog.h"
#include "Quaternion.h"
#include <thread>
#include "Panel.h"
#include "Randomizer.h"

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

void KeepWatchdog::action() {
	if (ReadPanelData<int>(0x01BE9, SOLVED)) {
		WritePanelData<float>(0x03317, POWER, { 1, 1 });
		WritePanelData<int>(0x03317, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
	else {
		WritePanelData<float>(0x03317, POWER, { 0, 0 });
		WritePanelData<int>(0x03317, NEEDS_REDRAW, { 1 });
	}
}

//Arrow Watchdog - To run the arrow puzzles

ArrowWatchdog::ArrowWatchdog(int id) : Watchdog(0.1f) {
	Panel panel(id);
	this->id = id;
	grid = backupGrid = panel._grid;
	width = static_cast<int>(grid.size());
	height = static_cast<int>(grid[0].size());
	pillarWidth = tracedLength = 0;
	complete = false;
	style = ReadPanelData<int>(id, STYLE_FLAGS);
	DIRECTIONS = { Point(0, 2), Point(0, -2), Point(2, 0), Point(-2, 0), Point(2, 2), Point(2, -2), Point(-2, -2), Point(-2, 2) };
	exitPos = panel.xy_to_loc(panel._endpoints[0].GetX(), panel._endpoints[0].GetY());
	exitPosSym = (width / 2 + 1) * (height / 2 + 1) - 1 - exitPos;
	exitPoint = (width / 2 + 1) * (height / 2 + 1);
}

ArrowWatchdog::ArrowWatchdog(int id, int pillarWidth) : ArrowWatchdog(id) {
	this->pillarWidth = pillarWidth;
	if (pillarWidth > 0) exitPoint = (width / 2) * (height / 2 + 1);
}

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
					Memory::get()->WriteArray<int>(id, SEQUENCE, { 69 }, true);
					WritePanelData<int>(id, SEQUENCE_LEN, { 1 });
					return;
				}
			}
		}
		WritePanelData<uint64_t>(id, SEQUENCE, { 0 });
		WritePanelData<int>(id, SEQUENCE_LEN, { 0 });
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
	for (const SolutionPoint& p : traced) {
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
	int length1 = ReadPanelData<int>(id1, TRACED_EDGES);
	int length2 = ReadPanelData<int>(id2, TRACED_EDGES);
	if (solLength1 > 0 && length1 == 0) {
		WritePanelData<int>(id2, STYLE_FLAGS, { ReadPanelData<int>(id2, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (solLength2 > 0 && length2 == 0) {
		WritePanelData<int>(id1, STYLE_FLAGS, { ReadPanelData<int>(id1, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (length1 != solLength1 && length1 > 0 && !checkTouch(id2)) {
		WritePanelData<int>(id2, STYLE_FLAGS, { ReadPanelData<int>(id2, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	if (length2 != solLength2 && length2 > 0 && !checkTouch(id1)) {
		WritePanelData<int>(id1, STYLE_FLAGS, { ReadPanelData<int>(id1, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	solLength1 = length1;
	solLength2 = length2;
}

bool BridgeWatchdog::checkTouch(int id)
{
	int length = ReadPanelData<int>(id, TRACED_EDGES);
	if (length == 0) return false;
	int numIntersections = ReadPanelData<int>(id, NUM_DOTS);
	std::vector<int> intersectionFlags = ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<SolutionPoint> edges = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, length);
	for (const SolutionPoint& sp : edges) if (intersectionFlags[sp.pointA] == Decoration::Dot_Intersection || intersectionFlags[sp.pointB] == Decoration::Dot_Intersection) return true;
	return false;
}

void TreehouseWatchdog::action()
{
	if (ReadPanelData<int>(0x03613, SOLVED)) {
		WritePanelData<float>(0x17DAE, POWER, { 1.0f, 1.0f });
		WritePanelData<int>(0x17DAE, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
}

// Jungle watchdog

JungleWatchdog::JungleWatchdog(int id, std::vector<int> correctSeq1, std::vector<int> correctSeq2) : Watchdog(0.5f) {
	this->id = id;
	int size = ReadPanelData<int>(id, NUM_DOTS);
	sizes = ReadArray<int>(id, DOT_FLAGS, ReadPanelData<int>(id, NUM_DOTS));
	this->correctSeq1 = correctSeq1;
	this->correctSeq2 = correctSeq2;
	state = false;
	tracedLength = 0;
	ptr1 = ReadPanelData<long>(id, DOT_SEQUENCE);
	ptr2 = ReadPanelData<long>(id, DOT_SEQUENCE_REFLECTION);
}

void JungleWatchdog::action()
{
	int numTraced = ReadPanelData<int>(id, TRACED_EDGES);
	if (numTraced == tracedLength) return;
	tracedLength = numTraced;
	int tracedptr = ReadPanelData<int>(id, TRACED_EDGE_DATA);
	if (!tracedptr) return;
	std::vector<SolutionPoint> traced = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, numTraced);
	int seqIndex = 0;
	for (const SolutionPoint& p : traced) {
		if ((sizes[p.pointA] & IntersectionFlags::DOT) == 0) continue;
		if (sizes[p.pointA] & (0x1000 << (state ? correctSeq1[seqIndex] : correctSeq2[seqIndex])))
			seqIndex++;
		else return;
		if (seqIndex >= 1) {
			WritePanelData<long>(id, DOT_SEQUENCE, { state ? ptr1 : ptr2 } );
			WritePanelData<long>(id, DOT_SEQUENCE_REFLECTION, { state ? ptr2 : ptr1 });
			WritePanelData<int>(id, DOT_SEQUENCE_LEN, { state ? (int)correctSeq1.size() : (int)correctSeq2.size() });
			WritePanelData<int>(id, DOT_SEQUENCE_LEN_REFLECTION, { state ? (int)correctSeq2.size() : (int)correctSeq1.size() });
			state = !state;
			return;
		}
	}
}

// Town door watchdog

void TownDoorWatchdog::action()
{
	if (ReadPanelData<Quaternion>(0x03BB0, ORIENTATION).z > 0) {
		WritePanelData<float>(0x28A69, POWER, { 1.0f, 1.0f });
		WritePanelData<int>(0x28A69, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
}
