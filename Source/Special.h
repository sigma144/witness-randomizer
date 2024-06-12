#pragma once

#include <algorithm>

#include "Generate.h"
#include "Memory.h"
#include "Panel.h"

typedef std::set<Point> Shape;

//Functions to handle special case puzzles

template <class T> struct MemoryWrite {
	int id;
	int offset;
	std::vector<T> data;
	MemoryWrite(int id, int offset, const std::vector<T>& data) { this->id = id; this->offset = offset; this->data = data; }
};

class Special {

public:
	static std::map<int, int> correctShapesById;

	Special(std::shared_ptr<Generate> generator) {
		this->generator = generator;
	}
	
	void generateSpecialSymMaze(std::shared_ptr<Generate> gen, int id);
	void generateReflectionDotPuzzle(std::shared_ptr<Generate> gen, int id1, int id2, std::vector<std::pair<int, int>> symbols, Panel::Symmetry symmetry, bool split);
	void generateAntiPuzzle(int id);
	void generateColorFilterPuzzle(int id, Point size, const std::vector<std::pair<int, int>>& symbols, const Color& filter, bool colorblind);
	void generateSoundDotPuzzle(int id, Point size, std::vector<int> dotSequence, bool writeSequence);
	void generateSoundDotPuzzle(int id1, int id2, std::vector<int> dotSequence, bool writeSequence);
	void generateSoundDotReflectionPuzzle(int id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored, bool writeSequence);
	bool generateSoundDotReflectionSpecial(int id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored);
	void generateRGBStonePuzzleN(int id);
	void generateRGBStarPuzzleN(int id);
	void generateRGBStonePuzzleH(int id);
	void generateRGBDotPuzzleH(int id);
	void generateJungleVault(int id);
	void generateApplePuzzle(int id, bool changeExit, bool flip);
	void generateKeepLaserPuzzle(int id, const std::set<Point>& path1, const std::set<Point>& path2, const std::set<Point>& path3, const std::set<Point>& path4, std::vector<std::pair<int, int>> symbols);
	void generateMountaintop(int id, const std::vector<std::pair<int, int>>& symbolVec);
	void generateMultiPuzzle(std::vector<int> ids, const std::vector<std::vector<std::pair<int, int>>>& symbolVec, bool flip);
	bool generateMultiPuzzle(std::vector<int> ids, std::vector<Generate>& gens, const std::vector<PuzzleSymbols>& symbols, const std::set<Point>& path);
	void generate2Bridge(int id1, int id2);
	bool generate2Bridge(int id1, int id2, std::vector<std::shared_ptr<Generate>> gens);
	void generate2BridgeH(int id1, int id2);
	bool generate2BridgeH(int id1, int id2, std::vector<std::shared_ptr<Generate>> gens);
	void generateMountainFloor();
	void generateMountainFloorH();
	void generatePivotPanel(int id, Point gridSize, const std::vector<std::pair<int, int>>& symbolVec, bool colorblind); //Too slow right now, only used a couple times in hard mode
	void modifyGate(int id);
	void addDecoyExits(std::shared_ptr<Generate> gen, int amount);
	void initSSGrid(std::shared_ptr<Generate> gen);
	void initRotateGrid(std::shared_ptr<Generate> gen);
	void initPillarSymmetry(std::shared_ptr<Generate> gen, int id, Panel::Symmetry symmetry);
	void generateSymmetryGate(int id);
	bool checkDotSolvability(std::shared_ptr<Panel> panel1, std::shared_ptr<Panel> panel2, Panel::Symmetry correctSym);
	void createArrowPuzzle(int id, int x, int y, int dir, int ticks, const std::vector<Point>& gaps);
	void createArrowSecretDoor(int id);
	void generateCenterPerspective(int id, const std::vector<std::pair<int, int>>& symbolVec, int symbolType);
	static void createText(int id, std::string text, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB,
		float left, float right, float top, float bottom);
	static void drawText(int id, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB, const std::vector<float>& finalLine);
	static void drawSeedAndDifficulty(int id, int seed, bool hard, bool setSeed, bool options);
	static void drawGoodLuckPanel(int id);
    static void SetRequiredLasers(int mountain, int challenge);

	static void SkipPanel(int id) {
		SkipPanel(id, "Skipped", false);
	}

	static void SkipPanel(int id, bool kickOut) {
		SkipPanel(id, "Skipped", kickOut);
	}

	static void SkipPanel(int id, std::string text) {
		SkipPanel(id, text, false);
	}

	static void SkipPanel(int id, std::string text, bool kickOut);

	static void SetVanillaMetapuzzleShapes();

	static void SkipMetapuzzle(int id, std::string text, bool kickOut);

	static void ColorPanel(int id, std::string text);

	static void DrawSimplePanel(int id, std::string text, bool kickOut);

	static void writeGoalCondition(int id, std::string goal1, std::string goal2, int mountain_lasers, int challenge_lasers);

	void test(); //For testing/debugging purposes only

	static void setTarget(int puzzle, int target)
	{
		Memory* memory = Memory::get();
		memory->WritePanelData(puzzle, TARGET, target + 1);
	}

	static void clearTarget(int puzzle)
	{
		Memory* memory = Memory::get();
		memory->WritePanelData(puzzle, TARGET, 0);
	}

	static void copyTarget(int puzzle, int sourceTarget)
	{
		Memory* memory = Memory::get();
		memory->WritePanelData(puzzle, TARGET, memory->ReadPanelData<int>(sourceTarget, TARGET));
	}
	static bool hasBeenPlayed() {
		Memory* memory = Memory::get();
		return memory->ReadPanelData<float>(0x00295, POWER) > 0 ||
			memory->ReadPanelData<int>(0x00064, TRACED_EDGES) > 0;
	}
	static bool hasBeenRandomized() {
		Memory* memory = Memory::get();
		return memory->ReadPanelData<int>(0x00064, BACKGROUND_REGION_COLOR + 12) > 0;
	}
	static void setTargetAndDeactivate(int puzzle, int target)
	{
		Memory* memory = Memory::get();
		if (!hasBeenRandomized()) //Only deactivate on a fresh save file (since power state is preserved)
			memory->WritePanelData<float>(target, POWER, { 0.0, 0.0 });
		memory->WritePanelData(puzzle, TARGET, target + 1);
	}

	static void setPower(int puzzle, bool power) {
		Memory* memory = Memory::get();
		if (!power && hasBeenRandomized()) return; //Only deactivate on a fresh save file (since power state is preserved)
		if (power) memory->WritePanelData<float>(puzzle, POWER, { 1.0, 1.0 });
		else memory->WritePanelData<float>(puzzle, POWER, { 0.0, 0.0 });
	}

	static std::string readStringFromPanels(std::vector<int> panelIDs);
	static void writeStringToPanels(std::string string, std::vector<int> panelIDs);

	static void testSwap(int id1, int id2) {
		Memory* memory = Memory::get();
		std::vector<byte> bytes1 = memory->ReadPanelData<byte>(id1, 0, 0x600);
		std::vector<byte> bytes2 = memory->ReadPanelData<byte>(id2, 0, 0x600);
		memory->WritePanelData<byte>(id1, TRACED_EDGES, bytes2);
		memory->WritePanelData<byte>(id2, TRACED_EDGES, bytes1);
		memory->WritePanelData<int>(id1, NEEDS_REDRAW, { 1 });
		memory->WritePanelData<int>(id2, NEEDS_REDRAW, { 1 });
	}

	template <class T> static std::vector<T> testRead(int address, int numItems) {
		Memory* memory = Memory::get();
		std::vector<int> offsets = { address };
		return memory->ReadData<T>(offsets, numItems);
	}

	static void testPanel(int id) {
		std::shared_ptr<Panel> panel = std::make_shared<Panel>(id);
		panel->Write();
	}

	template <class T> static uintptr_t testFind(uintptr_t startAddress, int length, T item) {
		Memory* memory = Memory::get();
		uintptr_t address;
		std::vector<byte> bytes;
		bytes.resize(1024, 0);
		std::vector<byte> itemb;
		itemb.resize(sizeof(T));
		std::memcpy(&itemb[0], &item, sizeof(T));
		for (address = startAddress; address < startAddress + length; address += 1024) {
			if (!memory->ReadAbsolute(reinterpret_cast<LPCVOID>(address), &bytes[0], 1024))
				continue;
			for (int i = 0; i < bytes.size() - itemb.size(); i += sizeof(T)) {
				if (std::equal(bytes.begin() + i, bytes.begin() + i + sizeof(T), itemb.begin()))
					return address + i;
			}
		}
		return -1;
	}

	template <class T> static uintptr_t testFind2(uintptr_t startAddress, int length, T item) {
		Memory memory("witness64_d3d11.exe");
		uintptr_t address;
		std::vector<byte> bytes;
		bytes.resize(1024, 0);
		std::vector<byte> itemb;
		itemb.resize(sizeof(T) - 1);
		std::memcpy(&itemb[0], &item, sizeof(T) - 1);
		for (address = startAddress; address < startAddress + length; address += 1024) {
			if (!memory.ReadAbsolute(reinterpret_cast<LPCVOID>(address), &bytes[0], 1024))
				continue;
			for (int i = 0; i < bytes.size() - itemb.size() + 1; i += sizeof(T)) {
				if (std::equal(bytes.begin() + i, bytes.begin() + i + sizeof(T) - 1, itemb.begin()))
					return address + i;
			}
		}
		return -1;
	}

private:

	std::shared_ptr<Generate> generator;

	template <class T> T pick_random(std::vector<T>& vec) { return vec[Random::rand() % vec.size()]; }
	template <class T> T pick_random(std::set<T>& set) { auto it = set.begin(); std::advance(it, Random::rand() % set.size()); return *it; }
	template <class T> T pop_random(std::vector<T>& vec) {
		int i = Random::rand() % vec.size();
		T item = vec[i];
		vec.erase(vec.begin() + i);
		return item;
	}
	template <class T> T pop_random(std::set<T>& set) {
		auto it = set.begin();
		std::advance(it, Random::rand() % set.size());
		T item = *it;
		set.erase(item);
		return item;
	}
};