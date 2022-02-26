#pragma once

#include <memory>
#include <set>
#include <map>
#include "..\Randomizer.h"
#include "..\Special.h"
#include "PuzzleData.h"

class APRandomizer {
public:
	int Seed = 12345;

	bool unlockedStones = false;
	bool unlockedColoredStones = false;
	bool unlockedStars = false;
	bool unlockedTetris = false;
	bool unlockedErasers = false;
	bool unlockedTriangles = false;
	bool unlockedDots = false;
	bool unlockedColoredDots = false;
	bool unlockedInvisibleDots = false;
	bool unlockedArrows = false;

	APRandomizer();

	void Connect();
	void Initialize(HWND loadingHandle);
	void GenerateNormal();

private:
	std::map<int, PuzzleData> disabledPuzzles;
	std::shared_ptr<Memory> _memory;

	void StartWatching();
	void PreventSnipes();

	void disablePuzzles(HWND loadingHandle);
	void disablePuzzle(int id);
	void enablePuzzles();

	static void addMissingSimbolsDisplay(std::vector<float>& newIntersections, std::vector<int>& newIntersectionFlags, std::vector<int>& newConnectionsA, std::vector<int>& newConnectionsB);
	static void addPuzzleSimbols(PuzzleData& panel, std::vector<int>& decorations, std::vector<int>& decorationsFlags, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& polygons);
};