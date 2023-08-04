#pragma once

#include "APState.h"

#include <map>
#include <set>
#include <string>
#include <vector>

class APRandomizer {
	public:
		APRandomizer();

		int Seed = 0;
		int FinalPanel = 0;
		bool SyncProgress = false;
		std::string Collect = "Unchanged";
		std::string DisabledPuzzlesBehavior = "Prevent Solve";

		int PuzzleRandomization = 2;
		bool UnlockSymbols = false;
		bool EarlyUTM = false;
		bool DisableNonRandomizedPuzzles = false;
		bool EPShuffle = false;
		int MountainLasers = 7;
		int ChallengeLasers = 11;
		bool DeathLink;
		bool ElevatorsComeToYou = false;

		int mostRecentItemId = -1;

		float solveModeSpeedFactor = 0.0f;

		bool connected = false;
		bool randomizationFinished = false;

		bool Connect(std::string& server, std::string& user, std::string& password);
		void PostGeneration();

		void GenerateNormal();
		void GenerateHard();

		void HighContrastMode();

		void SkipPuzzle();

		void SeverDoors();

		void unlockItem(int item);

		bool InfiniteChallenge(bool enable);

		void Init();

	private:
		std::map<int, int> panelIdToLocationId;
		std::map<int, std::set<int>> itemIdToDoorSet;
		std::set<int> doorsActuallyInTheItemPool;
		std::map<int, std::vector<int>> progressiveItems;
		std::map<int, std::pair<std::string, int64_t>> audioLogMessages;
		std::map<int, int> panelIdToLocationIdReverse;
		std::set<int> disabledEntities;

		std::map<int, std::set<int>> obeliskSideIDsToEPHexes;
		std::set<int> precompletedLocations;
		std::map<int, std::string> entityToName;

		class APClient* ap = NULL;
		class APWatchdog* async;
		class PanelLocker* panelLocker;

		APState state = APState();

		void PreventSnipes();

		void setPuzzleLocks();

		std::string buildUri(std::string& server);
};

#define DATAPACKAGE_CACHE "ap_datapackage.json"


