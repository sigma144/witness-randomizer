#pragma once
#include "Generate.h"
#include <algorithm>
#include "Memory.h"
#include "Panel.h"
#include "Enums.h"

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
	Special() { }
	Special(Generate* generator);
	
	void generateSpecialSymMaze(PanelID id);
	void generateReflectionDotPuzzle(PanelID id1, PanelID id2, std::vector<std::pair<int, int>> symbols, Symmetry symmetry, bool split);
	void generateAntiPuzzle(PanelID id);
	void generateColorFilterPuzzle(PanelID id, Point size, const std::vector<std::pair<int, int>>& symbols, const Color& filter, bool colorblind);
	void generateSoundDotPuzzle(PanelID id, Point size, std::vector<int> dotSequence, bool writeSequence);
	void generateSoundDotPuzzle(PanelID id1, PanelID id2, std::vector<int> dotSequence, bool writeSequence);
	void generateSoundDotReflectionPuzzle(PanelID id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored, bool writeSequence);
	bool generateSoundDotReflectionSpecial(PanelID id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored);
	void generateRGBStonePuzzleN(PanelID id);
	void generateRGBStarPuzzleN(PanelID id);
	void generateRGBStonePuzzleH(PanelID id);
	void generateRGBDotPuzzleH(PanelID id);
	void generateJungleVault(PanelID id);
	void generateApplePuzzle(PanelID id, bool changeExit, bool flip);
	void generateKeepLaserPuzzle(PanelID id, const std::set<Point>& path1, const std::set<Point>& path2, const std::set<Point>& path3, const std::set<Point>& path4, std::vector<std::pair<int, int>> symbols);
	void generateMountaintop(PanelID id, const std::vector<std::pair<int, int>>& symbolVec);
	void generateMultiPuzzle(std::vector<PanelID> ids, const std::vector<std::vector<std::pair<int, int>>>& symbolVec, bool flip);
	bool generateMultiPuzzle(std::vector<PanelID> ids, std::vector<Generate>& gens, const std::vector<PuzzleSymbols>& symbols, const std::set<Point>& path);
	void generate2Bridge(PanelID id1, PanelID id2);
	bool generate2Bridge(PanelID id1, PanelID id2, std::vector<std::shared_ptr<Generate>> gens);
	void generate2BridgeH(PanelID id1, PanelID id2);
	bool generate2BridgeH(PanelID id1, PanelID id2, std::vector<std::shared_ptr<Generate>> gens);
	void generateMountainFloor();
	void generateMountainFloorH();
	void generatePivotPanel(PanelID id, Point gridSize, const std::vector<std::pair<int, int>>& symbolVec, bool colorblind); //Too slow right now, only used a couple times in hard mode
	void modifyGate(PanelID id);
	void addDecoyExits(int amount);
	void initSecretSymmetryGrid();
	void initRotateGrid();
	void initPillarSymmetry(PanelID id, Symmetry symmetry);
	void generateSymmetryGate(PanelID id);
	bool checkDotSolvability(Panel* panel1, Panel* panel2, Symmetry correctSym);
	void createArrowPuzzle(PanelID id, int x, int y, SymbolId symbolId, const std::vector<Point>& gaps);
	void createArrowSecretDoor(PanelID id);
	void generateCenterPerspective(PanelID id, const std::vector<std::pair<int, int>>& symbolVec, Symbol symbolType);

	static void createText(PanelID id, std::string text, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB,
		float left, float right, float top, float bottom);
	static void drawText(PanelID id, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB, const std::vector<float>& finalLine);
	static void drawSeedAndDifficulty(PanelID id, int seed, bool hard, bool setSeed, bool options);
	static void drawGoodLuckPanel(PanelID id);

	static void setTarget(PanelID puzzle, PanelID target);
	static void clearTarget(PanelID puzzle);
	static void copyTarget(PanelID puzzle, PanelID sourceTarget);
	static void setTargetAndDeactivate(PanelID puzzle, PanelID target);
	static void setPower(PanelID puzzle, bool power);

	static bool hasBeenPlayed();
	static bool hasBeenRandomized();

	void test(); //For testing/debugging purposes only

	Generate* gen;
	Memory* memory;
	Generate g; //For testing convenience ONLY, use "gen" for Special.cpp generation functions

private:

	template <class T> T pickRandom(std::vector<T>& vec) { return Random::pickRandom(vec); }
	template <class T> T pickRandom(std::set<T>& set) { return Random::pickRandom(set); }
	template <class T> T popRandom(std::vector<T>& vec) { return Random::popRandom(vec); }
	template <class T> T popRandom(std::set<T>& set) { return Random::popRandom(set); }
};