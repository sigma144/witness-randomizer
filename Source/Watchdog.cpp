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

ArrowWatchdog::ArrowWatchdog(int id) : Watchdog(0.1f), _panel(Panel(id)) {
	this->id = id;
	grid = backupGrid = _panel._grid;
	width = static_cast<int>(grid.size());
	height = static_cast<int>(grid[0].size());
	pillarWidth = tracedLength = 0;
	complete = false;
	style = ReadPanelData<int>(id, STYLE_FLAGS);
	exitPos = _panel.xy_to_loc(_panel._endpoints[0].GetX(), _panel._endpoints[0].GetY());
	exitPosSym = (width / 2 + 1) * (height / 2 + 1) - 1 - exitPos;
	exitPoint = (width / 2 + 1) * (height / 2 + 1);

	// The sequence array is used to indicate panel validity. A panel with a nonnull SEQUENCE will always fail (since the first two indices cannot both be 0).
	Memory* memory = Memory::get();
	memory->WritePanelData<int>(id, SEQUENCE_LEN, { 2 });
	memory->WriteArray<int>(id, SEQUENCE, { 0, 0 }, true);
	_sequenceArray = memory->ReadPanelData<uintptr_t>(id, SEQUENCE);
}

ArrowWatchdog::ArrowWatchdog(int id, int pillarWidth) : ArrowWatchdog(id) {
	this->pillarWidth = pillarWidth;
	if (pillarWidth > 0) exitPoint = (width / 2) * (height / 2 + 1);
}

#define LOG_DEBUG(fmt, ...) LogDebug(__FILE__, __LINE__, fmt, __VA_ARGS__)
void LogDebug(const char* function, int line, const char* fmt, ...) {
	char message[1024] = "\0";

	va_list args;
	va_start(args, fmt);
	vsprintf_s(&message[0], sizeof(message) / sizeof(message[0]), fmt, args);
	va_end(args);

	char message2[1024] = "\0";
	snprintf(&message2[0], sizeof(message2) / sizeof(message2[0]), "[%s:%d] %s\n", function, line, message);
	OutputDebugStringA(message2);
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
		bool success = true;

		Memory* memory = Memory::get();
		for (int x = 1; x < width; x++) {
			for (int y = 1; y < height; y++) {
				int symbol = grid[x][y];
				if ((symbol & 0xF00) != Decoration::Arrow) continue;

				if (!checkArrow(x, y)) {
					// LOG_DEBUG("Arrow at %d, %d NOT valid", x, y);
					memory->WriteToArray(id, DECORATION_FLAGS, _panel.xy_to_dloc(x, y), 1);
					success = false;
				} else {
					// LOG_DEBUG("Arrow at %d, %d IS valid", x, y);
					memory->WriteToArray(id, DECORATION_FLAGS, _panel.xy_to_dloc(x, y), 0);
				}
			}
		}
		// LOG_DEBUG("Puzzle is overall %s", (success ? "VALID" : "INVALID"));
		WritePanelData<uintptr_t>(id, SEQUENCE, { success ? 0 : _sequenceArray });
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
	int orig_x = x;
	int orig_y = y;
	if (pillarWidth > 0) return checkArrowPillar(x, y);
	int symbol = grid[x][y];
	if ((symbol & 0xF00) != Decoration::Arrow)
		return true;
	int targetCount = (symbol >> 19);
	Point dir = Generate::ArrowDirections[(symbol >> 16) & 0x07];
	x += dir.first / 2; y += dir.second / 2;
	int count = 0;
	while (x >= 0 && x < width && y >= 0 && y < height) {
		if (grid[x][y] == PATH) {
			if (++count > targetCount) { 
				// The arrow already sees too many lines, no need to continue counting
				break;
			}
		}
		x += dir.first; y += dir.second;
	}
	return count == targetCount;
}

bool ArrowWatchdog::checkArrowPillar(int x, int y)
{
	int orig_x = x;
	int orig_y = y;
	int symbol = grid[x][y];
	if ((symbol & 0xF00) != Decoration::Arrow)
		return true;
	int targetCount = (symbol >> 19);
	Point dir = Generate::ArrowDirections[(symbol >> 16) & 0x07];
	x = (x + (dir.first > 2 ? -2 : dir.first) / 2 + pillarWidth) % pillarWidth; y += dir.second / 2;
	int count = 0;
	while (y >= 0 && y < height) {
		if (grid[x][y] == PATH) {
			if (++count > targetCount) {
				// The arrow already sees too many lines, no need to continue counting
				break;
			}
		}
		x = (x + dir.first + pillarWidth) % pillarWidth; y += dir.second;
	}
	return count == targetCount;
}

void BridgeWatchdog::action()
{
	Memory* memory = Memory::get();
	int length1 = memory->ReadPanelData<int>(id1, TRACED_EDGES);
	int length2 = memory->ReadPanelData<int>(id2, TRACED_EDGES);
	if (solLength1 > 0 && length1 == 0) {
		memory->WritePanelData<int>(id2, STYLE_FLAGS, { memory->ReadPanelData<int>(id2, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (solLength2 > 0 && length2 == 0) {
		memory->WritePanelData<int>(id1, STYLE_FLAGS, { memory->ReadPanelData<int>(id1, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (length1 != solLength1 && length1 > 0 && !checkTouch(id2)) {
		memory->WritePanelData<int>(id2, STYLE_FLAGS, { memory->ReadPanelData<int>(id2, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	if (length2 != solLength2 && length2 > 0 && !checkTouch(id1)) {
		memory->WritePanelData<int>(id1, STYLE_FLAGS, { memory->ReadPanelData<int>(id1, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	solLength1 = length1;
	solLength2 = length2;
}

bool BridgeWatchdog::checkTouch(int id)
{
	Memory* memory = Memory::get();
	int length = memory->ReadPanelData<int>(id, TRACED_EDGES);
	if (length == 0) return false;
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<SolutionPoint> edges = memory->ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, length);
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

void TownDoorWatchdog::action()
{
	if (ReadPanelData<Quaternion>(0x03BB0, ORIENTATION).z > 0) {
		WritePanelData<float>(0x28A69, POWER, { 1.0f, 1.0f });
		WritePanelData<int>(0x28A69, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
}
