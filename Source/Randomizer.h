#pragma once

#include <memory>
#include <set>
#include <map>
#include <vector>
#include <windows.h>
#include "Enums.h"

class Randomizer {
public:
	void GenerateNormal(HWND loadingHandle);
	void GenerateHard(HWND loadingHandle);
	void StartWatchdogs();

	void AdjustSpeed();

	enum SWAP {
		NONE = 0,
		TARGETS = 1,
		PATHS = 2,
		AUDIO_NAMES = 4,
		COLORS = 8,
		DRAWN_LINES = 16,
	};

	int seed = 0;
	bool seedIsRNG = false;
	bool colorblind = false;
	bool doubleMode = false;
	bool watchdogsStarted = false;

private:
	void RandomizeDesert();

	void Randomize(std::vector<PanelID>& panels, int flags);
	void RandomizeRange(std::vector<PanelID> panels, int flags, int startIndex, int endIndex);
	void RandomizeAudiologs();
	void SwapPanels(PanelID panel1, PanelID panel2, int flags);
	void ReassignTargets(const std::vector<PanelID>& panels, const std::vector<int>& order, std::vector<PanelID> targets = {});
	void SwapWithRandomPanel(PanelID panel1, const std::vector<PanelID>& possiblePanels, int flags);
	void ShuffleRange(std::vector<int>& order, int startIndex, int endIndex);
	void ShufflePanels(bool hard);
	bool HasBeenPlayed();
	bool HasBeenRandomized();

	std::set<PanelID> _alreadySwapped;
	std::map<int, int> _shuffleMapping;

	friend class Panel;
	friend class PuzzleList;
	friend class Special;
};
