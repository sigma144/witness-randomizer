#pragma once

#include "..\Watchdog.h"

class APSolvedPuzzles : public Watchdog {
public:
	APSolvedPuzzles(APClient* client, std::map<int, int> mapping) : Watchdog(1.0f) {
		ap = client;
		panelIdToLocationId = mapping;
	}

	virtual void action();
	void MarkLocationChecked(int locationId);

private:
	APClient* ap;
	std::map<int, int> panelIdToLocationId;

	void eraseByLocationId(int value);
};