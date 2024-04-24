#pragma once

#include "Memory.h"

class Watchdog
{
public:
	Watchdog(float time) {
		terminate = false;
		sleepTime = time;
	};
	void start();
	void run();
	virtual void action() = 0;
	float sleepTime;
	bool terminate;
protected:
	template <class T> std::vector<T> ReadPanelData(int panel, int offset, size_t size) const {
		return ReadPanelData<T>(panel, offset, size, false);
	}

	template <class T> std::vector<T> ReadPanelData(int panel, int offset, size_t size, bool forceRecalculate) const {
		try {
			return Memory::get()->ReadPanelData<T>(panel, offset, size, forceRecalculate);
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Watchdog Read Problem");
			return {};
		}
	}

	template <class T> T ReadPanelData(int panel, int offset) const {
		try {
			return Memory::get()->ReadPanelData<T>(panel, offset);
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Watchdog Read Problem");
			return T();
		}
	}

	template <class T> T ReadPanelDataIntentionallyUnsafe(int panel, int offset) const {
		return Memory::get()->ReadPanelData<T>(panel, offset);
	}

	template <class T> std::vector<T> ReadArray(int panel, int offset, int size) const {
		try {
			return Memory::get()->ReadArray<T>(panel, offset, size);
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Watchdog Read Array Problem");
			return {};
		}
	}

	template <class T> bool WritePanelData(int panel, int offset, const std::vector<T>& data, bool forceRecalculatePointer) const {
		try {
			Memory::get()->WritePanelData<T>(panel, offset, data, forceRecalculatePointer);
			return true;
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Watchdog Write Problem");
			return false;
		}
	}

	template <class T> bool WritePanelData(int panel, int offset, const std::vector<T>& data) const {
		return WritePanelData(panel, offset, data, false);
	}

	template <class T> bool WriteArray(int panel, int offset, const std::vector<T>& data) const {
		try {
			 Memory::get()->WriteArray<T>(panel, offset, data, false);
			 return true;
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Watchdog Write Problem");
			return false;
		}
	}

	template <class T> bool WriteArray(int panel, int offset, const std::vector<T>& data, bool force) const {
		try {
			Memory::get()->WriteArray<T>(panel, offset, data, force);
			return true;
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Watchdog Write Problem");
			return false;
		}
	}
};

class KeepWatchdog : public Watchdog {
public:
	KeepWatchdog() : Watchdog(10) { }
	virtual void action();
};
class ArrowWatchdog : public Watchdog {
public:
	ArrowWatchdog(int id);
	ArrowWatchdog(int id, int pillarWidth);

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
	std::vector<struct Point> DIRECTIONS;
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
	TreehouseWatchdog(int id) : Watchdog(1) { }
	virtual void action();
};

class JungleWatchdog : public Watchdog {
public:
	JungleWatchdog(int id, std::vector<int> correctSeq1, std::vector<int> correctSeq2);
	virtual void action();
	int id;
	std::vector<int> sizes;
	long ptr1, ptr2;
	std::vector<int> correctSeq1, correctSeq2;
	bool state;
	int tracedLength;
};

class TownDoorWatchdog : public Watchdog {
public:
	TownDoorWatchdog() : Watchdog(0.2f) { }
	virtual void action();
};