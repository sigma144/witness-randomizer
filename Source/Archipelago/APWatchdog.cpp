#include "APWatchdog.h"
#include "..\Quaternion.h"
#include "..\Special.h"
#include "PanelsUnlocks.h"
#include "PanelsChecks.h"
#include <thread>

// Completion manager test>
void APWatchdog::action() {
	//THIS IS HOW TO TEST SOLVE STATE!
	int hasEverBeenSolved = ReadPanelData<int>(0x00061, SOLVED);

	std::string test = std::to_string(hasEverBeenSolved);

	go++;

	int firstStoner = 0x018AF;

	//WritePanelData<float>(firstStoner, POWER, { (go % 10) * 0.1f, (go % 10) * 0.1f });
	//WritePanelData<int>(firstStoner, NEEDS_REDRAW, { 1 });

	/*for (auto panelId : doorUnlockPanels) {
		WritePanelData<float>(panelId.first, POWER, { (go % 2) * 1.0f, (go % 2) * 1.0f });
		WritePanelData<int>(panelId.first, NEEDS_REDRAW, { 1 });
	}*/

	if (redrawText)
	{
		disablePuzzle(firstStoner);

		redrawText = false;
	}

	/*int doneChecks = 0;

	for (auto id : standardChecks) {
		doneChecks += ReadPanelData<int>(id.first, SOLVED);
	}

	OutputDebugStringA(std::to_string(doneChecks).c_str());*/

	//this->_memory->WriteMovementSpeed(4.0f); We can manipulate movement speed with this - Potential "bonus" item?
}


void APWatchdog::disablePuzzle(int id) {
	// not verry optimised, can be improved later by skipping generation and handling it all by custom code
	//		first generated a 5x5 empty puzzle
	//		then makes the the top part invisiable
	//		then inserts the missing simbols
	//		then add missing text

	const int height = 5;
	const int width = 5;

	// generated a 5x5 empty puzzle
	generator->resetConfig();
	generator->setGridSize(width, height);
	generator->setFlagOnce(Generate::Config::DisableWrite);
	generator->generate(id);

	// then makes the the top part invisiable
	for (int x = 0; x <= width * 2; x++)
	{
		for (int y = 0; y <= height; y++)
		{
			if ((x % 2 == 0 && y % 2 == 1) || (x % 2 == 1 && y % 2 == 0))
				generator->set(x, y, IntersectionFlags::OPEN);
			else if (x % 2 == 0 && y % 2 == 0)
				generator->set(x, y, IntersectionFlags::NO_POINT);
		}
	}

	// then inserts the missing simbols
	// TODO detect used simbols by puzzle and render them rather than hardcoded list
	generator->set(1, 7, Decoration::Stone | Decoration::Color::Black);
	generator->set(1, 8, Decoration::Gap_Row);
	generator->set(1, 9, Decoration::Stone | Decoration::Color::White);

	generator->write(id);
	generator->resetConfig();

	// then add missing text
	std::vector<float> intersections;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;
	
	addText(id, "missing", 0.1f, 0.9f, 0.1f, 0.4f);
}

void APWatchdog::addText(int id, std::string text, float left, float right, float top, float bottom) {
	std::vector<float> newIntersections;
	std::vector<int> newIntersectionFlags;
	std::vector<int> newConnectionsA;
	std::vector<int> newConnectionsB;

	Special::createText(id, text, newIntersections, newConnectionsA, newConnectionsB, left, right, top, bottom);

	for (int i = 0; i < newIntersections.size() / 2; i++) {
		newIntersectionFlags.emplace_back(0);
	}

	addToIntersections(id, newIntersections, newIntersectionFlags, newConnectionsA, newConnectionsB);
}

void APWatchdog::addToIntersections(int id, std::vector<float>& newIntersections, std::vector<int>& newIntersectionFlags, std::vector<int>& newConnectionsA, std::vector<int>& newConnectionsB) {
	Panel panel;

	int sourceIntersectionCount = panel._memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<int> sourceIntersectionFlags = panel._memory->ReadArray<int>(id, DOT_FLAGS, sourceIntersectionCount);
	std::vector<float> sourceIntersections = panel._memory->ReadArray<float>(id, DOT_POSITIONS, sourceIntersectionCount * 2);

	int sourceNumberOfConnections = panel._memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	std::vector<int> sourceConnectionsA = panel._memory->ReadArray<int>(id, DOT_CONNECTION_A, sourceNumberOfConnections);
	std::vector<int> sourceConnectionsB = panel._memory->ReadArray<int>(id, DOT_CONNECTION_B, sourceNumberOfConnections);

	for (int i = 0; i < newConnectionsA.size(); i++) {
		newConnectionsA[i] += sourceIntersectionCount;
		newConnectionsB[i] += sourceIntersectionCount;
	}

	sourceIntersections.insert(sourceIntersections.end(), newIntersections.begin(), newIntersections.end());
	sourceIntersectionFlags.insert(sourceIntersectionFlags.end(), newIntersectionFlags.begin(), newIntersectionFlags.end());
	sourceConnectionsA.insert(sourceConnectionsA.end(), newConnectionsA.begin(), newConnectionsA.end());
	sourceConnectionsB.insert(sourceConnectionsB.end(), newConnectionsB.begin(), newConnectionsB.end());

	panel._memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.6f });
	panel._memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(sourceIntersectionFlags.size()) });
	panel._memory->WriteArray<float>(id, DOT_POSITIONS, sourceIntersections); //position of each point as array of x,y,x,y,x,y
	panel._memory->WriteArray<int>(id, DOT_FLAGS, sourceIntersectionFlags); //flags for each point such as entrance or exit
	panel._memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(sourceConnectionsA.size()) });
	panel._memory->WriteArray<int>(id, DOT_CONNECTION_A, sourceConnectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
	panel._memory->WriteArray<int>(id, DOT_CONNECTION_B, sourceConnectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
	panel._memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}