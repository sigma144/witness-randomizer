#include "APWatchdog.h"
#include "..\Quaternion.h"
#include "PanelsUnlocks.h"
#include "PanelsChecks.h"
#include <thread>

// Completion manager test>
void APWatchdog::action() {
	//THIS IS HOW TO TEST SOLVE STATE!
	int hasEverBeenSolved = ReadPanelData<int>(0x00061, SOLVED);

	std::string test = std::to_string(hasEverBeenSolved);

	go++;

	for (auto panelId : doorUnlockPanels) {
		WritePanelData<float>(panelId.first, POWER, { (go % 2) * 1.0f, (go % 2) * 1.0f });
		WritePanelData<int>(panelId.first, NEEDS_REDRAW, { 1 });
	}

	int doneChecks = 0;

	for (auto id : standardChecks) {
		doneChecks += ReadPanelData<int>(id.first, SOLVED);
	}

	OutputDebugStringA(std::to_string(doneChecks).c_str());

	//this->_memory->WriteMovementSpeed(4.0f); We can manipulate movement speed with this - Potential "bonus" item?
}