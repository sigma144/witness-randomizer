#pragma once

#include <map>
#include "..\Watchdog.h"
#include "..\Generate.h"
#include "APRandomizer.h"

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

class APServerPoller : public Watchdog {
public:
	APServerPoller(APClient* client) : Watchdog(0.1f) {
		ap = client;
	}

	virtual void action();

private:
	APClient* ap;
};