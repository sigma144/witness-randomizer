#pragma once
#include "Memory.h"
#include "Panel.h"
#include "Randomizer.h"
#include "Generate.h"

class Watchdog
{
public:
	Watchdog(float time) {
		terminate = false;
		sleepTime = time;
		_memory = std::make_shared<Memory>("witness64_d3d11.exe");
	};
	void start();
	void run();
	virtual void action() = 0;
	float sleepTime;
	bool terminate;
protected:
	template <class T> std::vector<T> ReadPanelData(int panel, int offset, size_t size) {
		return _memory->ReadPanelData<T>(panel, offset, size);
	}
	template <class T> T ReadPanelData(int panel, int offset) {
		return _memory->ReadPanelData<T>(panel, offset);
	}
	template <class T> std::vector<T> ReadArray(int panel, int offset, int size) {
		return _memory->ReadArray<T>(panel, offset, size);
	}
	template <class T> void WritePanelData(int panel, int offset, const std::vector<T>& data) {
		return _memory->WritePanelData<T>(panel, offset, data);
	}
	template <class T> void WriteArray(int panel, int offset, const std::vector<T>& data) {
		return _memory->WriteArray<T>(panel, offset, data, false);
	}
	template <class T> void WriteArray(int panel, int offset, const std::vector<T>& data, bool force) {
		return _memory->WriteArray<T>(panel, offset, data, force);
	}
	std::shared_ptr<Memory> _memory;
};

class KeepWatchdog : public Watchdog {
public:
	KeepWatchdog() : Watchdog(10) { ready = false; }
	virtual void action();
	bool ready;
};

class TestWatchdog : public Watchdog {
public:
	TestWatchdog() : Watchdog(10) { ready = false; }
	virtual void action();
	bool ready;
	int go = 0;
};

class ArrowWatchdog : public Watchdog {
public:
	ArrowWatchdog(int id) : Watchdog(0.1f) {
		Panel panel(id);
		this->id = id;
		grid = backupGrid = panel._grid;
		width = static_cast<int>(grid.size());
		height = static_cast<int>(grid[0].size());
		pillarWidth = tracedLength = 0;
		complete = false;
		style = ReadPanelData<int>(id, STYLE_FLAGS);
		DIRECTIONS = { Point(0, 2), Point(0, -2), Point(2, 0), Point(-2, 0), Point(2, 2), Point(2, -2), Point(-2, -2), Point(-2, 2) };
		exitPos = panel.xy_to_loc(panel._endpoints[0].GetX(), panel._endpoints[0].GetY());
		exitPosSym = (width / 2 + 1) * (height / 2 + 1) - 1 - exitPos;
		exitPoint = (width / 2 + 1) * (height / 2 + 1);
	}
	ArrowWatchdog(int id, int pillarWidth) : ArrowWatchdog(id) {
		this->pillarWidth = pillarWidth;
		if (pillarWidth > 0) exitPoint = (width / 2) * (height / 2 + 1);
	}
	virtual void action();
	void initPath();
	bool checkArrow(int x, int y);
	bool checkArrowPillar(int x, int y);

	int id;
	std::vector<std::vector<int>> backupGrid;
	std::vector<std::vector<int>> grid;
	int width, height, pillarWidth;
	int tracedLength;
	bool complete;
	int style;
	int exitPos, exitPosSym, exitPoint;
	std::vector<Point> DIRECTIONS;
};

class BridgeWatchdog : public Watchdog {
public:
	BridgeWatchdog(int id1, int id2) : Watchdog(1) {
		solLength1 = false;
		solLength2 = false;
		this->id1 = id1; this->id2 = id2;
	}
	virtual void action();
	bool checkTouch(int id);
	int id1, id2, solLength1, solLength2;
};

class TreehouseWatchdog : public Watchdog {
public:
	TreehouseWatchdog(int id) : Watchdog(2) {
		this->id = id;
	}
	virtual void action();
	int id;
};