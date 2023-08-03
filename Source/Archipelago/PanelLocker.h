#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

class APState;
struct PuzzleData;

class PanelLocker {
	public:
		void DisableNonRandomizedPuzzles(std::set<int> exemptDoorPanels);
		void UpdatePuzzleLock(const APState& state, const int& id);
		void UpdatePuzzleLocks(const APState& state, const int& itemIndex);
		bool PuzzleIsLocked(int id);
		void PermanentlyUnlockPuzzle(int id);

	private:
		std::vector<int> disabledPuzzles;
		std::map<int, PuzzleData*> lockedPuzzles;

		void addMissingSimbolsDisplay(std::vector<float>& newIntersections, std::vector<int>& newIntersectionFlags, std::vector<int>& newConnectionsA, std::vector<int>& newConnectionsB, int id);
		void createText(int id, std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, float left, float right, float top, float bottom);
		void createCenteredText(int id, std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, float top, float bottom);
		int addPuzzleSimbols(const APState& state, PuzzleData* puzzle, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, std::vector<int>& decorations, std::vector<int>& decorationsFlags, std::vector<int>& polygons, int id);
		void unlockPuzzle(PuzzleData* puzzle);
};

