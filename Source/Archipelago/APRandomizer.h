#pragma once

#include <memory>
#include <set>
#include <map>
#include "nlohmann\json.hpp"
#include "..\Randomizer.h"
#include "..\Special.h"
#include "PuzzleData.h"
#include "APWatchdog.h"
#include "..\Converty.h"
#include "..\DateTime.h"

class APRandomizer {
	public:
		APRandomizer() {
			_memory = std::make_shared<Memory>("witness64_d3d11.exe");
			panelLocker = new PanelLocker(_memory);
		};

		int Seed = 0;
		int FinalPanel = 0;
		bool Collect = false;

		bool Hard = false;
		bool UnlockSymbols = false;
		bool EarlyUTM = false;
		bool DisableNonRandomizedPuzzles = false;
		int MountainLasers = 7;
		int ChallengeLasers = 11;

		int mostRecentItemId = -1;

		bool connected = false;
		bool randomizationFinished = false;

		bool Connect(HWND& messageBoxHandle, std::string& server, std::string& user, std::string& password);
		void PostGeneration(HWND loadingHandle);

		void GenerateNormal(HWND skipButton, HWND availableSkips);
		void GenerateHard(HWND skipButton, HWND availableSkips);

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
		std::map<int, std::pair<std::vector<std::string>, int64_t>> audioLogMessages;
		std::map<int, int> panelIdToLocationIdReverse;

		std::map<int, std::set<int>> obeliskSideIDsToEPHexes;
		std::set<int> precompletedLocations;

		std::shared_ptr<Memory> _memory;

		APClient* ap;
		APWatchdog* async;
		PanelLocker* panelLocker;

		APState state = APState();

		void PreventSnipes();

		void setPuzzleLocks(HWND loadingHandle);

		std::string buildUri(std::string& server);

		std::array<float, 3> getColorForItem(const APClient::NetworkItem& item);
};

#define DATAPACKAGE_CACHE "ap_datapackage.json"


