#pragma once
#include "..\Watchdog.h"

class APWatchdog : public Watchdog {
public:
	APWatchdog() : Watchdog(1) { ready = false; }

	virtual void action();

	bool ready;
	int go = 0;
};