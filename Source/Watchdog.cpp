// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Watchdog.h"
#include "Quaternion.h"
#include <thread>
#include "Panel.h"
#include "Randomizer.h"

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

//Symbols Watchdog - For custom symbols
SymbolsWatchdog::SymbolsWatchdog() : Watchdog(0.1f) {
	id = TUT_DOT_1;
	tracedLength = 0;
	memory = Memory::get();
	// The sequence array is used to indicate panel validity. A panel with a nonnull SEQUENCE will always fail as a 0-length solution does not exist.
	sequenceArray = memory->AllocArray<int>(1); //Don't have to actually initialize with a value as SEQUENCE_LEN is set to 0.
}

void SymbolsWatchdog::action() {
	PanelID active = memory->GetActivePanel();
	if (active != id) {
		id = active;
		if (active == -1 || (ReadPanelData<int>(active, STYLE_FLAGS) & HAS_CUSTOM) == 0) {
			sleepTime = 0.1f;
			return;
		}
		panel = Panel(id);
		sleepTime = 0.01f;
	}
	if (sleepTime == 0.1f) return;
	int length = ReadPanelData<int>(id, TRACED_EDGES);
	if (length == tracedLength)
		return;
	tracedLength = length;
	
	std::vector<std::vector<int>> backupGrid = panel.getGrid();
	initPath();
	//TODO: Have watchdog check for exits

	bool success = true;
	
	for (int x = 1; x < panel.width; x++) {
		for (int y = 1; y < panel.height; y++) {
			int symbol = get(x, y);
			if ((symbol & 0xF00) != 0x700) continue; //Skip non-custom symbols
			if (!checkSymbol(x, y)) {
				//LOG_DEBUG("Symbol at %d, %d NOT valid", x, y);
				memory->WriteToArray(id, DECORATION_FLAGS, panel.pointToDecorationIndex(x, y), 1);
				success = false;
			} else {
				//LOG_DEBUG("Symbol at %d, %d NOT valid", x, y);
				memory->WriteToArray(id, DECORATION_FLAGS, panel.pointToDecorationIndex(x, y), 0);
			}
		}
	}
	WritePanelData<uintptr_t>(id, SEQUENCE, success ? 0 : sequenceArray);
	panel.setGrid(backupGrid);
	//LOG_DEBUG("Puzzle is overall %s", (success ? "VALID" : "INVALID"));
}

void SymbolsWatchdog::initPath() {
	int tracedptr = ReadPanelData<int>(id, TRACED_EDGE_DATA);
	if (!tracedptr) return;
	int numTraced = ReadPanelData<int>(id, TRACED_EDGES);
	int numPoints = panel.getNumGridPoints();
	int width = panel.width; int height = panel.height;
	std::vector<SolutionPoint> tracedData = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, numTraced);
	traced.clear();
	for (int i = 0; i < numTraced; i++) { //Remove mid-segment points
		if (tracedData[i].pointA < 0 || tracedData[i].pointA >= numPoints) continue;
		if (tracedData[i].pointB < 0 || tracedData[i].pointB >= numPoints) {
			if (i+1 < numTraced)
				tracedData[i].pointB = tracedData[i + 1].pointB;
		}
		traced.emplace_back(tracedData[i]);
	}
	if (panel.style & SYMMETRICAL) {
		for (int i = 0; i < numTraced; i++) {
			SolutionPoint sp;
			//TODO: Other types of symmetry
			sp.pointA = (width / 2 + 1) * (height / 2 + 1) - 1 - traced[i].pointA;
			sp.pointB = (width / 2 + 1) * (height / 2 + 1) - 1 - traced[i].pointB;
			traced.push_back(sp);
		}
	}
	for (const SolutionPoint& p : traced) {
		int p1 = p.pointA, p2 = p.pointB;
		if (p1 < 0 || p2 < 0 || p1 >= numPoints || p2 >= numPoints) {
			continue;
		}
		int x1 = (p1 % (width / 2 + 1)) * 2, y1 = height - 1 - (p1 / (width / 2 + 1)) * 2;
		int x2 = (p2 % (width / 2 + 1)) * 2, y2 = height - 1 - (p2 / (width / 2 + 1)) * 2;
		if (panel.isCylinder) {
			x1 = (p1 % (width / 2)) * 2, y1 = height - 1 - (p1 / (width / 2)) * 2;
			x2 = (p2 % (width / 2)) * 2, y2 = height - 1 - (p2 / (width / 2)) * 2;
			set(x1, y1, PATH);
			set(x2, y2, PATH);
			if (x1 == x2 || x1 == x2 + 2 || x1 == x2 - 2)
				set((x1 + x2) / 2, (y1 + y2) / 2, PATH);
			else set(width - 1, (y1 + y2) / 2, PATH);
		}
		else {
			set(x1, y1, PATH);
			set(x2, y2, PATH);
			set((x1 + x2) / 2, (y1 + y2) / 2, PATH);
		}
	}
}

int SymbolsWatchdog::get(int x, int y) {
	return panel.get(x, y);
}

void SymbolsWatchdog::set(int x, int y, int val) {
	panel.set(x, y, val);
}

bool SymbolsWatchdog::checkSymbol(int x, int y) {
	int type = GetWitnessSymbolId(get(x, y));
	if (type >= SymbolId::Arrow1E && type <= SymbolId::Arrow3NE) {
		if (!checkArrow(x, y)) return false;
	}
	return true;
}

bool SymbolsWatchdog::checkArrow(int x, int y) {
	int symbol = get(x, y);
	int targetCount = (symbol >> 19);
	Point dir = Panel::DIRECTIONS8_2[(symbol >> 16) & 0x7];
	x += dir.x / 2; y += dir.y / 2;
	int count = 0;
	while (get(x, y) != OFF_GRID) {
		if (get(x, y) == PATH) {
			if (++count > targetCount) {
				break;
			}
		}
		x += dir.x; y += dir.y;
	}
	return count == targetCount;
}

//Keep Watchdog - Keep the big panel off until all panels are solved
void KeepWatchdog::action() {
	if (ReadPanelData<int>(KEEP_PRESSURE_2, SOLVED)) {
		WritePanelDataVector<float>(KEEP_PRESSURE_LASER, POWER, { 1, 1 });
		WritePanelData<int>(KEEP_PRESSURE_LASER, NEEDS_REDRAW, 1);
		terminate = true;
	}
	else {
		WritePanelDataVector<float>(KEEP_PRESSURE_LASER, POWER, { 0, 0 });
		WritePanelData<int>(KEEP_PRESSURE_LASER, NEEDS_REDRAW, 1);
	}
}

void BridgeWatchdog::action() {
	Memory* memory = Memory::get();
	PanelID id1 = MOUNTAIN_BLUE_BRIDGE;
	PanelID id2 = MOUNTAIN_ORANGE_BRIDGE;
	if (memory->GetActivePanel() != id1 && memory->GetActivePanel() != id2) return;
	int length1 = memory->ReadPanelData<int>(id1, TRACED_EDGES);
	int length2 = memory->ReadPanelData<int>(id2, TRACED_EDGES);
	if (solLength1 > 0 && length1 == 0) {
		memory->WritePanelData<int>(id2, STYLE_FLAGS, memory->ReadPanelData<int>(id2, STYLE_FLAGS) | HAS_DOTS);
	}
	if (solLength2 > 0 && length2 == 0) {
		memory->WritePanelData<int>(id1, STYLE_FLAGS, memory->ReadPanelData<int>(id1, STYLE_FLAGS) | HAS_DOTS);
	}
	if (length1 != solLength1 && length1 > 0 && !checkTouch(id2)) {
		memory->WritePanelData<int>(id2, STYLE_FLAGS, memory->ReadPanelData<int>(id2, STYLE_FLAGS) & ~HAS_DOTS);
	}
	if (length2 != solLength2 && length2 > 0 && !checkTouch(id1)) {
		memory->WritePanelData<int>(id1, STYLE_FLAGS, memory->ReadPanelData<int>(id1, STYLE_FLAGS) & ~HAS_DOTS);
	}
	solLength1 = length1;
	solLength2 = length2;
}

bool BridgeWatchdog::checkTouch(PanelID id) {
	Memory* memory = Memory::get();
	int length = memory->ReadPanelData<int>(id, TRACED_EDGES);
	if (length == 0) return false;
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<SolutionPoint> edges = memory->ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, length);
	for (const SolutionPoint& sp : edges) if (intersectionFlags[sp.pointA] == Dot_Intersection || intersectionFlags[sp.pointB] == Dot_Intersection)
		return true;
	return false;
}

void TreehouseWatchdog::action() {
	if (ReadPanelData<int>(TREEHOUSE_LASER, SOLVED)) {
		WritePanelDataVector<float>(TREEHOUSE_ORANGE_L13, POWER, { 1.0f, 1.0f });
		WritePanelData<int>(TREEHOUSE_ORANGE_L13, NEEDS_REDRAW, 1);
		terminate = true;
	}
}

JungleWatchdog::JungleWatchdog(PanelID id, std::vector<int> correctSeq1, std::vector<int> correctSeq2) : Watchdog(0.5f) {
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

void JungleWatchdog::action() {
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
			WritePanelData<long>(id, DOT_SEQUENCE, state ? ptr1 : ptr2 );
			WritePanelData<long>(id, DOT_SEQUENCE_REFLECTION, state ? ptr2 : ptr1);
			WritePanelData<int>(id, DOT_SEQUENCE_LEN, state ? (int)correctSeq1.size() : (int)correctSeq2.size());
			WritePanelData<int>(id, DOT_SEQUENCE_LEN_REFLECTION, state ? (int)correctSeq2.size() : (int)correctSeq1.size());
			state = !state;
			return;
		}
	}
}

void TownDoorWatchdog::action() {
	if (ReadPanelData<Quaternion>(static_cast<PanelID>(0x03BB0), ORIENTATION).z > 0) {
		WritePanelDataVector<float>(TOWN_LATTICE, POWER, { 1.0f, 1.0f });
		WritePanelData<int>(TOWN_LATTICE, NEEDS_REDRAW, 1);
		terminate = true;
	}
}
