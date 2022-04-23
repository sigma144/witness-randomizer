#pragma once

#include <memory>
#include <set>
#include <map>
#include "..\Randomizer.h"
#include "..\Special.h"
#include "PuzzleData.h"
#include "APState.h"

class PanelLocker {
	public:
		PanelLocker(std::shared_ptr<Memory> memory) {
			_memory = memory;
		};

		void DisableNonRandomizedPuzzles();
		void UpdatePuzzleLock(const APState& state, int id);
		void UpdatePuzzleLocks(const APState& state, int itemIndex);

	private:
		std::map<int, PuzzleData*> lockedPuzzles;
		std::shared_ptr<Memory> _memory;

		void disablePuzzle(int id);
		void addMissingSimbolsDisplay(std::vector<float>& newIntersections, std::vector<int>& newIntersectionFlags, std::vector<int>& newConnectionsA, std::vector<int>& newConnectionsB);
		void createText(int id, std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, float left, float right, float top, float bottom);
		void addPuzzleSimbols(const APState& state, PuzzleData* puzzle, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, std::vector<int>& decorations, std::vector<int>& decorationsFlags, std::vector<int>& polygons);
		void unlockPuzzle(PuzzleData* puzzle);
};

