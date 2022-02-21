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

	void disablePuzzle(int id);

	static void addText(int id, std::string text, float left, float right, float top, float bottom);
	static void addToIntersections(int id, std::vector<float>& newIntersections, std::vector<int>& newIntersectionFlags, std::vector<int>& newConnectionsA, std::vector<int>& newConnectionsB);
};