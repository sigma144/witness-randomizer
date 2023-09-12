#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

class APState;
struct LockablePuzzle;

class PanelLocker {
	public:
		void DisableNonRandomizedPuzzles(std::set<int> exemptDoorPanels);
		void UpdatePuzzleLock(const APState& state, const int& id);
		void UpdatePuzzleLocks(const APState& state, const int& itemIndex);
		bool PuzzleIsLocked(int id);
		void PermanentlyUnlockPuzzle(int id);

	private:
		std::vector<int> disabledPuzzles;
		std::map<int, LockablePuzzle*> lockedPuzzles;

		void unlockPuzzle(LockablePuzzle* puzzle);
};

