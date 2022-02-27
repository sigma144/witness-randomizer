#pragma once
#include "..\Watchdog.h"
#include "..\Generate.h"

class APWatchdog : public Watchdog {
public:
	APWatchdog() : Watchdog(1) { 
		ready = false;

		generator = std::make_shared<Generate>();
	}

	virtual void action();

	bool ready;
	int go = 0;

	bool redrawText = true;
private:
	std::shared_ptr<Generate> generator;

};


/*class PowedPuzzleWatchdog : public Watchdog {
public:
	PowedPuzzleWatchdog() : Watchdog(0.5f) {
		//std::vector<int> allPanels
	}

	virtual void action();
};*/

class APServerPoller : public Watchdog {
public:
	APServerPoller(APClient* client) : Watchdog(0.1f) {
		ap = client;
	}

	virtual void action();

private:
		APClient* ap;
};