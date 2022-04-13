#pragma once

#include <memory>
#include <set>
#include <map>
#include "nlohmann\json.hpp"
#include "..\Randomizer.h"
#include "..\Special.h"
#include "PuzzleData.h"
#include "APWatchdog.h"
#include "APSolvedPuzzlesTracker.h"
#include "..\Converty.h"
#include "..\DateTime.h"

class APRandomizer {
public:
	int Seed = 0;

	bool Hard = false;
	bool UnlockSymbols = false;
	bool DisableNonRandomizedPuzzles = false;

	bool connected = false;

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

	bool Connect(HWND& messageBoxHandle, std::string& server, std::string& user, std::string& password);
	void Initialize(HWND loadingHandle);
	void GenerateNormal();
	void GenerateHard();

private:
	std::map<int, PuzzleData> disabledPuzzles;
	std::map<int, int> panelIdToLocationId;
	std::shared_ptr<Memory> _memory;

	APClient* ap;
	APSolvedPuzzles* solvedPuzzlesTracker;
	
	void StartWatching();
	void PreventSnipes();

	void disablePuzzles(HWND loadingHandle);
	void disablePuzzle(int id);
	void enablePuzzles();

	static void addMissingSimbolsDisplay(std::vector<float>& newIntersections, std::vector<int>& newIntersectionFlags, std::vector<int>& newConnectionsA, std::vector<int>& newConnectionsB);
	static void addPuzzleSimbols(PuzzleData& panel, std::vector<int>& decorations, std::vector<int>& decorationsFlags, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& polygons);
};