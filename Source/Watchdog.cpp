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

//Symbols Watchdog - For custom symbols
SymbolsWatchdog::SymbolsWatchdog() : Watchdog(0.1f) {
	id = TUT_DOT_1;
	endpoint = -1;
	memory = Memory::get();
	// The sequence array is used to indicate panel validity. A panel with a nonnull SEQUENCE will always fail as a 0-length solution does not exist.
	sequenceArray = memory->AllocArray<int>(1); //Don't have to actually initialize with a value as SEQUENCE_LEN is set to 0.
}

void SymbolsWatchdog::action() {
	PanelID active = memory->GetActivePanel();
	if (active != id) {
		if (active == -1 && (ReadPanelData<int>(id, STYLE_FLAGS) & HAS_CUSTOM)
			&& ReadPanelData<int>(id, ERASER_ACTIVE)) {
			initPath();
			(new EraserWatchdog(id, panel, sequenceArray))->start();
		}
		id = active;
		if (active == -1 || (ReadPanelData<int>(active, STYLE_FLAGS) & HAS_CUSTOM) == 0) {
			sleepTime = 0.1f;
			endpoint = -1;
			return;
		}
		panel = Panel(id);
		endpoints.clear();
		for (Endpoint& e : panel.endpoints) {
			endpoints.insert(panel.pointToIndex(e.x, e.y));
		}
		sleepTime = 0.01f;
	}
	if (sleepTime == 0.1f) return;

	int length = ReadPanelData<int>(id, TRACED_EDGES);
	if (length == 0) return;
	std::vector<SolutionPoint> tracedData = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, length);
	if (tracedData.size() > 1 && endpoints.count(tracedData[tracedData.size() - 1].pointA))
		tracedData.pop_back();
	SolutionPoint end = tracedData[tracedData.size() - 1];
	if (end.pointB == endpoint) return;
	endpoint = end.pointB;
	if (!endpoints.count(endpoint)) return;
	
	std::vector<std::vector<int>> backupGrid = panel.getGrid();
	initPath();

	bool success = panel.checkCustomSymbols(true);
	WritePanelData<uintptr_t>(id, SEQUENCE, success ? 0 : sequenceArray);
	panel.setGrid(backupGrid);
	//memory->LogDebug("Puzzle is overall %s", (success ? "VALID" : "INVALID"));
}

void SymbolsWatchdog::initPath() {
	int tracedptr = ReadPanelData<int>(id, TRACED_EDGE_DATA);
	if (!tracedptr) return;
	int numTraced = ReadPanelData<int>(id, TRACED_EDGES);
	int numPoints = panel.getNumGridPoints();
	int width = panel.width; int height = panel.height;
	std::vector<SolutionPoint> tracedData = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, numTraced);
	std::vector<SolutionPoint> traced;
	for (int i = 0; i < numTraced; i++) { //Remove mid-segment points
		if (tracedData[i].pointA < 0 || tracedData[i].pointA >= numPoints) continue;
		if (tracedData[i].pointB < 0 || tracedData[i].pointB >= numPoints) {
			if (i+1 < numTraced)
				tracedData[i].pointB = tracedData[i + 1].pointB;
		}
		traced.emplace_back(tracedData[i]);
	}
	if (traced.size() > 1 && endpoints.count(traced[traced.size() - 1].pointA)) {
		traced.pop_back();
		numTraced--;
	}
	for (int i = 0; i < traced.size(); i++) {
		SolutionPoint p = traced[i];
		int p1 = p.pointA, p2 = p.pointB;
		int x1 = (p1 % (width / 2 + 1)) * 2, y1 = height - 1 - (p1 / (width / 2 + 1)) * 2;
		int x2 = (p2 % (width / 2 + 1)) * 2, y2 = height - 1 - (p2 / (width / 2 + 1)) * 2;
		if (panel.isCylinder) {
			x1 = (p1 % (width / 2)) * 2, y1 = height - 1 - (p1 / (width / 2)) * 2;
			x2 = (p2 % (width / 2)) * 2, y2 = height - 1 - (p2 / (width / 2)) * 2;
		}
		if (i == 0) setPath(x1, y1);
		if (panel.isCylinder) {
			if (x1 == x2 || x1 == x2 + 2 || x1 == x2 - 2)
				setPath((x1 + x2) / 2, (y1 + y2) / 2);
			else setPath(width - 1, (y1 + y2) / 2);
			setPath(x2, y2);
		}
		else {
			setPath((x1 + x2) / 2, (y1 + y2) / 2);
			setPath(x2, y2);
		}
	}
}

EraserWatchdog::EraserWatchdog(PanelID id, Panel& panel, uintptr_t sequenceArray) : Watchdog(0.1f) {
	memory = Memory::get();
	this->id = id;
	this->panel = panel;
	this->sequenceArray = sequenceArray;
	checked = false;
}

void EraserWatchdog::action() { //TODO: Multi-eraser support, fix dot cancellation
	if (!memory->ReadPanelData<int>(id, ERASER_ACTIVE)) {
		terminate = true;
		return;
	}
	if (checked) {
		if (memory->ReadPanelData<float>(id, ERASER_TIME_TO_REJUDGE) == -1) {
			panel.checkCustomSymbols(true);
			terminate = true;
		}
		return;
	}
	std::vector<int> erasedDecorations, erasedDots;
	int numErasers = 0;
	bool success = true;
	for (int x = 1; x < panel.width; x++) {
		for (int y = 1; y < panel.height; y++) {
			int symbol = get(x, y);
			if (getType(symbol) == Eraser) {
				numErasers++;
				int eraser = panel.pointToDecorationIndex(x, y);
				Point erasedPos = getErasedSymbol({ x, y });
				if (erasedPos.x == -1) continue;
				int erased = -1;
				if (erasedPos.x % 2 == 1 && erasedPos.y % 2 == 1) //Grid block point
					erased = panel.pointToDecorationIndex(erasedPos.x, erasedPos.y);
				else if (erasedPos.x % 2 == 0 && erasedPos.y % 2 == 0) //Intersection point
					erased = panel.pointToIndex(erasedPos.x, erasedPos.y);
				else { //Edge point
					float x = panel.minx + erasedPos.x * panel.unitWidth;
					float y = panel.maxy - erasedPos.y * panel.unitHeight;
					std::vector<float> positions = memory->ReadArray<float>(id, DOT_POSITIONS, memory->ReadPanelData<int>(id, NUM_DOTS)*2);
					for (int i = panel.getNumGridPoints(); i < positions.size() / 2; i++) {
						if (abs(x - positions[i * 2]) < 0.001f && abs(y - positions[i * 2 + 1]) < 0.001f) {
							erased = i;
							break;
						}
					}
				}
				if (erased != -1) {
					erasedDecorations.emplace_back(eraser);
					if (erasedPos.x % 2 == 1 && erasedPos.y % 2 == 1)
						erasedDecorations.emplace_back(erased);
					else erasedDots.emplace_back(erased);
				}
			}
		}
	}
	memory->WritePanelData<int>(id, NUM_ERASED_DECORATIONS, static_cast<int>(erasedDecorations.size()));
	memory->WritePanelData<int>(id, NUM_ERASED_DOTS, static_cast<int>(erasedDots.size()));
	//Pre-allocate arrays to always be big enough so that the game doesn't try to free this memory
	erasedDecorations.resize(numErasers * 2);
	erasedDots.resize(numErasers);
	memory->WritePanelData<int>(id, ERASED_DECORATIONS_LEN, static_cast<int>(erasedDecorations.size()) + 1);
	memory->WriteArray<int>(id, ERASED_DECORATIONS, erasedDecorations);
	memory->WritePanelData<int>(id, ERASED_DOTS_LEN, static_cast<int>(erasedDots.size()) + 1);
	memory->WriteArray<int>(id, ERASED_DOTS, erasedDots);
	success = panel.checkCustomSymbols(false);
	WritePanelData<uintptr_t>(id, SEQUENCE, success ? 0 : sequenceArray);
	checked = true;
}

Point EraserWatchdog::getErasedSymbol(Point eraserPos) {
	std::set<Point> errors;
	std::set<Point> region = panel.getRegion(eraserPos);
	for (Point p : region) {
		if (!panel.checkSymbol(p)) {
			errors.insert(p);
		}
	}
	std::set<Point> edges = panel.getEdgesInRegion(region);
	for (Point p : edges) {
		if (panel.get(p) & Dot) {
			errors.insert(p);
		}
	}
	if (errors.size() == 0) return { -1, -1 };
	panel.set(eraserPos, None);
	panel.preCalcResult.clear();
	for (Point p : errors) {
		int symbol = panel.get(p);
		panel.set(p, None);
		bool valid = true;
		for (Point p2 : region) {
			if (!panel.checkSymbol(p2)) {
				valid = false;
				break;
			}
		}
		panel.preCalcResult.clear();
		if (valid)
			return p;
		panel.set(p, symbol);
	}
	Point p = Random::pickRandom(errors);
	panel.set(p, None);
	return p;
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
