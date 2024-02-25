#include "PanelLocker.h"

#include "../Randomizer.h"
#include "LockablePuzzle.h"
#include "../Panel.h"
#include "../Panels.h"
#include "../Memory.h"
#include "APState.h"
#include "APGameData.h"
#include "../Special.h"
#include "SkipSpecialCases.h"

void PanelLocker::UpdatePPEPPuzzleLocks(const APState& state) {
	for (int ppEpID : { 0x033BE , 0x033BF , 0x033DD , 0x033E5 , 0x018B6 }) {
		if (lockedPuzzles.count(ppEpID)) {
			lockedPuzzles[ppEpID]->UpdateLock(state);
		}
	}
}

void PanelLocker::DisableNonRandomizedPuzzles(std::set<int> exemptDoorPanels)
{
	Special::copyTarget(0x00021, 0x19650);
	Special::copyTarget(0x00061, 0x09DE0);
	Special::copyTarget(0x17CFB, 0x28B39);
	Special::copyTarget(0x3C12B, 0x28B39);
	Special::copyTarget(0x17CE7, 0x17CA4);
	Special::copyTarget(0x00B8D, 0x28B39);
	Special::copyTarget(0x17FA9, 0x17CA4);
	Special::copyTarget(0x17FA0, 0x17CAB);
	Special::copyTarget(0x17D27, 0x17CAB);
	Special::copyTarget(0x17D28, 0x19650);
	Special::copyTarget(0x17D01, 0x09DE0);
	Special::copyTarget(0x17C71, 0x19650);
	Special::copyTarget(0x17CF7, 0x28B39);
	Special::copyTarget(0x17D01, 0x09DE0);
	Special::copyTarget(0x17F9B, 0x17CAB);
	Special::copyTarget(0x17C42, 0x09DE0);
	Special::copyTarget(0x00A5B, 0x17CA4);

	for (int id : {0x17CA4, 0x17CAB, 0x28B39, 0x00C92, 0x0A8DC}) {
		if (!exemptDoorPanels.count(id)) {
			Special::setPower(id, true);
		}
	}
}

void PanelLocker::UpdatePuzzleLocks(const APState& state, const int& itemIndex) {
	std::vector<LockablePuzzle*> puzzlesToUpdate;

	for (auto const& [id, puzzle] : lockedPuzzles) {
		switch (itemIndex) {
			//Puzzle Symbols
			case ITEM_DOTS:							if (puzzle->hasDots) puzzlesToUpdate.push_back(puzzle);							break;
			case ITEM_COLORED_DOTS:					if (puzzle->hasColoredDots) puzzlesToUpdate.push_back(puzzle);					break;
			case ITEM_FULL_DOTS:					if (puzzle->hasFullDots) puzzlesToUpdate.push_back(puzzle);						break;
			case ITEM_SOUND_DOTS:					if (puzzle->hasSoundDots) puzzlesToUpdate.push_back(puzzle);					break;
			case ITEM_SYMMETRY:						if (puzzle->hasSymmetry) puzzlesToUpdate.push_back(puzzle);						break;
			case ITEM_TRIANGLES:						if (puzzle->hasTriangles) puzzlesToUpdate.push_back(puzzle);					break;
			case ITEM_ERASOR:							if (puzzle->hasErasers) puzzlesToUpdate.push_back(puzzle);						break;
			case ITEM_TETRIS:							if (puzzle->hasTetris) puzzlesToUpdate.push_back(puzzle);						break;
			case ITEM_TETRIS_ROTATED:				if (puzzle->hasTetrisRotated) puzzlesToUpdate.push_back(puzzle);				break;
			case ITEM_TETRIS_NEGATIVE:				if (puzzle->hasTetrisNegative) puzzlesToUpdate.push_back(puzzle);				break;
			case ITEM_STARS:							if (puzzle->hasStars) puzzlesToUpdate.push_back(puzzle);							break;
			case ITEM_STARS_WITH_OTHER_SYMBOL:	if (puzzle->hasStarsWithOtherSymbol) puzzlesToUpdate.push_back(puzzle);		break;
			case ITEM_B_W_SQUARES:					if (puzzle->hasStones) puzzlesToUpdate.push_back(puzzle);						break;
			case ITEM_COLORED_SQUARES:				if (puzzle->hasColoredStones) puzzlesToUpdate.push_back(puzzle);				break;
			case ITEM_SQUARES: if (puzzle->hasStones || puzzle->hasColoredStones) puzzlesToUpdate.push_back(puzzle);			break;
			case ITEM_ARROWS: if (puzzle->hasArrows) puzzlesToUpdate.push_back(puzzle);			break;
			case LASER_CHECK: if (puzzle->id == 0x0A332 || puzzle->id == 0x3D9A9)  puzzlesToUpdate.push_back(puzzle); break;

			default:																																			break;
		}
	}

	for (auto& puzzle : puzzlesToUpdate)
		UpdatePuzzleLock(state, puzzle->id);
}

void PanelLocker::UpdatePuzzleLock(const APState& state, const int& id) {
	if (neverLockAgain.count(id)) return;

	Memory* memory = Memory::get();

	bool isLocked;
	LockablePuzzle* puzzle;

	if (lockedPuzzles.count(id) == 1) {
		isLocked = true;
		puzzle = lockedPuzzles[id];
	}
	else {
		if (allEPs.count(id)) {
			puzzle = new LockableEP(id);
		}
		else if (allObelisks.count(id)) {
			puzzle = new LockableObelisk(id);
		}
		else if (allPanels.count(id)) {
			puzzle = new LockablePanel(id);
		}
		else puzzle = new LockablePuzzle(id);

		isLocked = false;
		puzzle->Read();
	}

	if ((puzzle->hasStones && !state.unlockedStones)
		|| (puzzle->hasColoredStones && !state.unlockedColoredStones)
		|| (puzzle->hasStars && !state.unlockedStars)
		|| (puzzle->hasStarsWithOtherSymbol && !state.unlockedStarsWithOtherSimbol)
		|| (puzzle->hasTetris && !state.unlockedTetris)
		|| (puzzle->hasTetrisRotated && !state.unlockedTetrisRotated)
		|| (puzzle->hasTetrisNegative && !state.unlockedTetrisNegative)
		|| (puzzle->hasErasers && !state.unlockedErasers)
		|| (puzzle->hasTriangles && !state.unlockedTriangles)
		|| (puzzle->hasDots && !state.unlockedDots)
		|| (puzzle->hasColoredDots && !state.unlockedColoredDots)
		|| (puzzle->hasFullDots && !state.unlockedFullDots)
		|| (puzzle->hasSoundDots && !state.unlockedSoundDots)
		|| (puzzle->hasArrows && !state.unlockedArrows)
		|| (puzzle->hasSymmetry && !state.unlockedSymmetry)
		|| (puzzle->needsChallengeLasers && state.activeLasers < state.requiredChallengeLasers)
		|| (puzzle->needsMountainLasers && state.activeLasers < state.requiredMountainLasers)
		|| (state.keysInTheGame.count(puzzle->id) && !state.keysReceived.count(puzzle->id))
		|| disabledPuzzles.count(id) && allEPs.count(id))
	{
		//puzzle should be locked
		puzzle->UpdateLock(state);
		if (!isLocked) lockedPuzzles.insert({ id, puzzle });
	}
	else if (isLocked) {
		//puzzle is locked but should nolonger be locked
		unlockPuzzle(puzzle, state);
	}
	else {
		//puzzle isnt locked and should not be locked
		delete puzzle;
	}
}

void PanelLocker::PermanentlyUnlockPuzzle(int id, const APState& state) {
	if (disabledPuzzles.count(id) && allEPs.count(id)) {
		neverLockAgain.insert(id);
		return;
	}

	if (lockedPuzzles.count(id) == 1) {
		LockablePuzzle* puzzle = lockedPuzzles[id];
		unlockPuzzle(puzzle, state);
	}

	neverLockAgain.insert(id);
}

void PanelLocker::unlockPuzzle(LockablePuzzle* puzzle, const APState& state) {
	puzzle->Restore();
	lockedPuzzles.erase(puzzle->id);
	recentlyUnlockedPuzzles.push_back(puzzle->id);

	if (EPtoStartPoint.count(puzzle->id)) {
		for (int conjoinedPuzzle : startPointToEPs.find(EPtoStartPoint.find(puzzle->id)->second)->second) {
			if (lockedPuzzles.count(conjoinedPuzzle)) {
				puzzle->UpdateLock(state);
			}
		}
	}

	//delete puzzle;
}

std::vector<int> PanelLocker::getAndFlushRecentlyUnlockedPuzzles()
{
	auto returnValue = recentlyUnlockedPuzzles;
	recentlyUnlockedPuzzles = {};
	return returnValue;
}

bool PanelLocker::PuzzleIsLocked(int id) {
	return lockedPuzzles.count(id) != 0;
}

void PanelLocker::DisablePuzzle(int id) {
	disabledPuzzles.insert(id);

	if (allEPs.count(id)) {
		if (!lockedPuzzles.count(id)) {
			auto puzzle = new LockableEP(id);
			lockedPuzzles[id] = puzzle;
		}
		bool allDisabledForStartpoint = true;
		bool allDisabledForEndpoint = true;

		for (int associatedEP : startPointToEPs.find(EPtoStartPoint.find(id)->second)->second) {
			allDisabledForStartpoint = allDisabledForStartpoint && disabledPuzzles.count(associatedEP);
		}

		for (int associatedEP : endPointToEPs.find(EPtoEndPoint.find(id)->second)->second) {
			allDisabledForEndpoint = allDisabledForEndpoint && disabledPuzzles.count(associatedEP);
		}

		((LockableEP*)lockedPuzzles[id])->DisableEP(allDisabledForStartpoint, allDisabledForEndpoint);
	}

	neverLockAgain.insert(id);
}