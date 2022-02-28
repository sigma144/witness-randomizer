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

	Panel panel;

	//panel.Read(firstStoner);

	//panel.Read(0x0056E);

	int tetrisDoor = 0x0056E;

	//panel._memory->WritePanelData<float>(tetrisDoor, SOLVED, { 1 });

	redrawText = false;

	/*int doneChecks = 0;

	for (auto id : standardChecks) {
		doneChecks += ReadPanelData<int>(id.first, SOLVED);
	}

	OutputDebugStringA(std::to_string(doneChecks).c_str());*/

	//this->_memory->WriteMovementSpeed(4.0f); We can manipulate movement speed with this - Potential "bonus" item?
	this->_memory->WriteMovementSpeed(2.0f + (go % 2) *8.0f); //Change run speed back and forth between 2 (default) and 10 (very fast)
}

void APServerPoller::action() {
	ap->poll();
}
