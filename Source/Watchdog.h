#pragma once
#include "Randomizer.h"
#include "Generate.h"
#include "Memory.h"
#include "SymbolData.h"

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
	template <class T> std::vector<T> ReadPanelData(PanelID panel, PanelVar offset, size_t size) {
		return Memory::get()->ReadPanelData<T>(panel, offset, size);
	}
	template <class T> T ReadPanelData(PanelID panel, PanelVar offset) {
		return Memory::get()->ReadPanelData<T>(panel, offset);
	}
	template <class T> std::vector<T> ReadArray(PanelID panel, PanelVar offset, int size) {
		return Memory::get()->ReadArray<T>(panel, offset, size);
	}
	template <class T> void WritePanelData(PanelID panel, PanelVar offset, const T data) {
		return Memory::get()->WritePanelData<T>(panel, offset, data);
	}
	template <class T> void WritePanelDataVector(PanelID panel, PanelVar offset, const std::vector<T>& data) {
		return Memory::get()->WritePanelDataVector<T>(panel, offset, data);
	}
	template <class T> void WriteArray(PanelID panel, PanelVar offset, const std::vector<T>& data) {
		return Memory::get()->WriteArray<T>(panel, offset, data, false);
	}
	template <class T> void WriteArray(PanelID panel, PanelVar offset, const std::vector<T>& data, bool force) {
		return Memory::get()->WriteArray<T>(panel, offset, data, force);
	}
};

class SymbolsWatchdog : public Watchdog {
public:
	SymbolsWatchdog();
	virtual void action();
	void initPath();
	int get(int x, int y);
	void set(int x, int y, int val);
	int getCustomSymbol(int x, int y) { return SymbolData::GetSymbolFromVal(get(x, y)); }

	bool checkSymbol(int x, int y);
	bool checkArrow(int x, int y);
	bool checkAntiTriangle(int x, int y);

	PanelID id;
	std::vector<SolutionPoint> traced;
	int tracedLength;
private:
	Memory* memory;
	Panel panel;
	uintptr_t sequenceArray = 0;
};

//TODO: Replace these with a more generic "TargetWatchdog" and addTarget method.
class KeepWatchdog : public Watchdog {
public:
	KeepWatchdog() : Watchdog(10) { }
	virtual void action();
};

class BridgeWatchdog : public Watchdog {
public:
	BridgeWatchdog() : Watchdog(1) {
		solLength1 = false;
		solLength2 = false;
	}
	virtual void action();
	bool checkTouch(PanelID id);
	int solLength1, solLength2;
};

class TreehouseWatchdog : public Watchdog {
public:
	TreehouseWatchdog(PanelID id) : Watchdog(1) { }
	virtual void action();
};

class JungleWatchdog : public Watchdog {
public:
	JungleWatchdog(PanelID id, std::vector<int> correctSeq1, std::vector<int> correctSeq2);
	virtual void action();
	PanelID id;
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

//TODO: Add Desert latch watchdog