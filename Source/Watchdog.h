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
	KeepWatchdog() : Watchdog(10) { }
	virtual void action();
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
	TreehouseWatchdog(int id) : Watchdog(1) { }
	virtual void action();
};

class JungleWatchdog : public Watchdog {
public:
	JungleWatchdog(int id, std::vector<int> correctSeq1, std::vector<int> correctSeq2) : Watchdog(0.5f) {
		this->id = id;
		int size = ReadPanelData<int>(id, NUM_DOTS);
		sizes = ReadArray<int>(id, DOT_FLAGS, ReadPanelData<int>(id, NUM_DOTS));
		this->correctSeq1 = correctSeq1;
		this->correctSeq2 = correctSeq2;
		state = false;
		tracedLength = 0;
		ptr1 = ReadPanelData<long>(id, DOT_SEQUENCE);
		ptr2 = ReadPanelData<long>(id, DOT_SEQUENCE_REFLECTION);
	}
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

#define SW_WAITING 0
#define SW_PLAYING 1
class SoundWatchdog : public Watchdog {
public:
	enum Flags : int { HIGH = 0x1, MEDIUM = 0x2, LOW = 0x3, LONG = 0x8, HIDDEN = 0x10, HIDDEN2 = 0x20 };
	SoundWatchdog() : Watchdog(2.0f) {
		this->sequence = sequence;
		soundTime = 0; seqTime = 0; seqIndex = 0; state = SW_WAITING;
		_memory = std::make_shared<Memory>("witness64_d3d11.exe");
	}
	void playSound(std::string filename) {
		OutputDebugStringW(L"Playing sound\n");
		OutputDebugStringW(std::wstring(filename.begin(), filename.end()).c_str());
		PlaySound(std::wstring(filename.begin(), filename.end()).c_str(), NULL, SND_FILENAME | SND_ASYNC);
	}
	std::vector<std::vector<int>> getClosestPanelSounds();
	virtual void action();
	int seqIndex, state;
	std::vector<int> sequence;
	double soundTime, seqTime;
	std::shared_ptr<Memory> _memory;
	static std::map<int, std::vector<std::vector<int>>> puzzleSounds;
};

