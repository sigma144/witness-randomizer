#pragma once

#include <map>
#include <set>
#include <string>
#include <set>
#include <vector>

class APState;
struct LockablePuzzle;

class PanelLocker {
	public:
		void UpdatePPEPPuzzleLocks(const APState& state);
		void DisableNonRandomizedPuzzles(std::set<int> exemptDoorPanels);
		void UpdatePuzzleLock(const APState& state, const int& id);
		void UpdatePuzzleLocks(const APState& state, const int& itemIndex);
		bool PuzzleIsLocked(int id);
		void PermanentlyUnlockPuzzle(int id, const APState& state);
		void DisablePuzzle(int id);
		std::vector<int> getAndFlushRecentlyUnlockedPuzzles();

	private:
		std::vector<int> recentlyUnlockedPuzzles;
		std::set<int> neverLockAgain;
		std::set<int> disabledPuzzles;
		std::map<int, LockablePuzzle*> lockedPuzzles;

		void unlockPuzzle(LockablePuzzle* puzzle, const APState& state);
};

