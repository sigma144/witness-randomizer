#include "PanelLocker.h"

#include "../Randomizer.h"
#include "PuzzleData.h"
#include "../Panel.h"
#include "../Memory.h"
#include "APState.h"
#include "../Special.h"
#include "SkipSpecialCases.h"

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
	std::vector<PuzzleData*> puzzlesToUpdate;

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
	if (find(disabledPuzzles.begin(), disabledPuzzles.end(), id) != disabledPuzzles.end())
		return;

	Memory* memory = Memory::get();

	bool isLocked;
	PuzzleData* puzzle;

	if (lockedPuzzles.count(id) == 1) {
		isLocked = true;
		puzzle = lockedPuzzles[id];
	}
	else {
		isLocked = false;
		puzzle = new PuzzleData(id);
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
		|| (state.keysInTheGame.count(puzzle->id) && !state.keysReceived.count(puzzle->id)))
	{
		//puzzle should be locked
		if (!isLocked)
			lockedPuzzles.insert({ id, puzzle });

		if (id == 0x01983) {
			if (memory->ReadPanelData<float>(0x0C179, 0xE0) < 0.0001f) return;

			while (memory->ReadPanelData<int>(0x0C179, 0x1e4)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			memory->WritePanelData<float>(0x0C179, 0xE0, { 0.000001f });
			memory->WritePanelData<float>(id, MAX_BROADCAST_DISTANCE, { 0.000001f });
			return;
		}

		if (id == 0x01987) {
			if (memory->ReadPanelData<float>(0x0C19D, 0xE0) < 0.0001f) return;

			while (memory->ReadPanelData<int>(0x0C19D, 0x1e4)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			memory->WritePanelData<float>(0x0C19D, 0xE0, { 0.000001f });
			memory->WritePanelData<float>(id, MAX_BROADCAST_DISTANCE, { 0.000001f });
			return;
		}

		const int width = 4;
		const int height = 2;
		int extraRows = 0;

		float pattern_scale = 0.5f;

		std::vector<float> intersections;
		std::vector<int> intersectionFlags;
		std::vector<int> connectionsA;
		std::vector<int> connectionsB;
		std::vector<int> decorations;
		std::vector<int> decorationsFlags;
		std::vector<int> polygons;

		std::vector<float> backgroundColor = { 0.5f, 0.0f, 0.0f, 1.0f };
		std::string text = "missing";

		bool puzzleIsMissingSymbols = ((puzzle->hasStones && !state.unlockedStones)
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
			|| (puzzle->needsMountainLasers && state.activeLasers < state.requiredMountainLasers));

		bool puzzleIsMissingKey = state.keysInTheGame.count(puzzle->id) && !state.keysReceived.count(puzzle->id);

		if (puzzleIsMissingKey) {
			text = "locked";
			backgroundColor = { 0.6f, 0.55f, 0.2f, 1.0f };
			if (puzzleIsMissingSymbols) {
				backgroundColor = { 0.5f, 0.25f, 0.0f, 1.0f };
			}
		}
		

 		if (id == 0x3D9A9) {
			std::string laserText = std::to_string(state.activeLasers);
			laserText += "/";
			laserText += std::to_string(state.requiredMountainLasers);
			std::string laserText2 = "Lasers:";

			pattern_scale = 0.3f;

			createText(id, laserText2, intersections, intersectionFlags, connectionsA, connectionsB, 0.515f - laserText2.size() * 0.029f, 0.515f + laserText2.size() * 0.029f, 0.38f, 0.47f);
			createText(id, laserText, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f - laserText.size() * 0.029f, 0.5f + laserText.size() * 0.029f, 0.53f, 0.62f);
		}
		else if (id == 0x09D9B) { // Monastery Shutters
			std::string text1 = "Needs Dots   Needs Dots   Needs Dots";
			std::string text2 = "Needs Dots   Needs Dots   Needs Dots";

			if (puzzleIsMissingKey){
				if (puzzleIsMissingSymbols) {
					text1 = "Needs Dots    Locked    Needs Dots";
					text2 = "Locked    Needs Dots    Locked";
				}
				else {
					text1 = "Locked    Locked    Locked    Locked";
					text2 = "Locked    Locked    Locked    Locked";

					backgroundColor = { 0.5f, 0.46f, 0.15f, 1.0f };
				}
			}
			pattern_scale = 0.05f;
			for (int i = 0; i < 4; i++) {
				std::string text = (i % 2) ? text1 : text2;

				int currentIntersections = intersections.size();
				createText(id, text, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f - text.size() * 0.01f, 0.5f + text.size() * 0.01f, 0.915f, 0.943f);
				int newIntersections = intersections.size();

				if (i % 2) {
					for (int i = currentIntersections; i < newIntersections; i += 2) {
						float x = intersections[i];
						intersections[i] = intersections[i + 1];
						intersections[i + 1] = 1.0f - x;
					}
				}

				if (i >= 2) {
					for (int i = currentIntersections; i < newIntersections; i += 2) {
						intersections[i + 1] = 1.0f - intersections[i + 1];
						intersections[i] = 1.0f - intersections[i];
					}
				}
			}
		}
		else if (very_thin_panels.count(id)) {
			float yOffset = 0.0f;

			if (id == 0x34BC5) yOffset = 0.25f;
			if (id == 0x34BC6) yOffset = -0.25f;

			int currentIntersections = intersections.size();

			std::string text = "Locked";

			createText(id, text, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f + text.size() * 0.037f, 0.5f - text.size() * 0.037f, 0.455f + yOffset, 0.545f + yOffset);
		
			int newIntersections = intersections.size();

			for (int i = currentIntersections; i < newIntersections; i += 2) {
				float x = intersections[i];
				intersections[i] = intersections[i + 1];
				intersections[i + 1] = x;
			}
		}
		else if (fairly_thin_panels.count(id) || id == 0x09EEB) {
			addMissingSimbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB, id);
			extraRows = addPuzzleSimbols(state, puzzle, intersections, intersectionFlags, connectionsA, connectionsB, decorations, decorationsFlags, polygons, id);
			memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
			memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 + extraRows });
		}
		else if (wide_panels.count(id)) {
			if (puzzleIsMissingSymbols) {
				addMissingSimbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB, id);
				extraRows = addPuzzleSimbols(state, puzzle, intersections, intersectionFlags, connectionsA, connectionsB, decorations, decorationsFlags, polygons, id);
				memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
				memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 + extraRows });
			}
			else
			{
				std::string text = "Locked";
				createText(id, text, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f - text.size() * 0.037f, 0.5f + text.size() * 0.037f, 0.455f, 0.545f);
			}
		}
		else {
			addMissingSimbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB, id);

			extraRows = addPuzzleSimbols(state, puzzle, intersections, intersectionFlags, connectionsA, connectionsB, decorations, decorationsFlags, polygons, id);
		
			if (id == 0x0A332) {
				int currentIntersections = intersections.size();

				std::string laserText = std::to_string(state.activeLasers);
				laserText += "/";
				laserText += std::to_string(state.requiredChallengeLasers);

				createText(id, laserText, intersections, intersectionFlags, connectionsA, connectionsB, 0.31f, 0.05f, 0.42f, 0.58f);

				int newIntersections = intersections.size();

				for (int i = currentIntersections; i < newIntersections; i += 2) {
					float x = intersections[i];
					intersections[i] = intersections[i + 1];
					intersections[i + 1] = x;
				}
			}
			else
			{
				createText(id, text, intersections, intersectionFlags, connectionsA, connectionsB, 0.1f, 0.9f, 0.1f, 0.4f);
			}

			memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
			memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 + extraRows });
		}

		memory->WritePanelData<float>(id, PATTERN_SCALE, { pattern_scale / puzzle->path_width_scale });
		memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
		memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
		memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
		memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
		memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
		memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
		memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
		memory->WriteArray<int>(id, DECORATIONS, decorations);
		memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);
		memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(polygons.size()) / 4 }); //why devide by 4 tho?
		memory->WriteArray<int>(id, COLORED_REGIONS, polygons, true);
		memory->WritePanelData<float>(id, PATH_COLOR, { 0.75f, 0.75f, 0.75f, 1.0f });
		memory->WritePanelData<__int64>(id, DECORATION_COLORS, { 0 });
		if (id != 0x28998) {
			memory->WritePanelData<float>(id, OUTER_BACKGROUND, backgroundColor);
			memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, backgroundColor);
			memory->WritePanelData<int>(id, OUTER_BACKGROUND_MODE, { 1 }); //Tinted glass door needs to stay transparent
		}
		memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
	}
	else if (isLocked) {
		//puzzle is locked but should nolonger be locked
		unlockPuzzle(puzzle);
	}
	else {
		//puzzle isnt locked and should not be locked
		delete puzzle;
	}
}

void PanelLocker::PermanentlyUnlockPuzzle(int id) {
	if (lockedPuzzles.count(id) == 1) {
		PuzzleData* puzzle = lockedPuzzles[id];
		unlockPuzzle(puzzle);
	}
	else {
		
	}
}

void PanelLocker::unlockPuzzle(PuzzleData* puzzle) {
	Memory* memory = Memory::get();

	if (puzzle->id == 0x01983) {
		while (memory->ReadPanelData<int>(0x0C179, 0x1d4)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		memory->WritePanelData<float>(0x0C179, 0xE0, { 0.2300000042f });
		memory->WritePanelData<float>(puzzle->id, MAX_BROADCAST_DISTANCE, { -1.0f });
	}

	else if (puzzle->id == 0x01987) {
		while (memory->ReadPanelData<int>(0x0C19D, 0x1d4)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		memory->WritePanelData<float>(0x0C19D, 0xE0, { 0.2300000042f });
		memory->WritePanelData<float>(puzzle->id, MAX_BROADCAST_DISTANCE, { -1.0f });
	}

	else {
		puzzle->Restore();
		memory->UpdatePanelJunctions(puzzle->id);
	}
	lockedPuzzles.erase(puzzle->id);
	//delete puzzle;
}

void PanelLocker::addMissingSimbolsDisplay(std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, int id) {
	//does not respect width/height but its hardcoded at a grid of 4/2

	//panel coordinates go from 0,0 in bottom left to 1,1 in top right
	//format vector of x,y,x,y,x,y,x,y,x,y
	std::vector<float> gridIntersections = {
		0.1f, 0.1f, 0.3f, 0.1f, 0.5f, 0.1f, 0.7f, 0.1f, 0.9f, 0.1f, //bottom row
		0.1f, 0.3f, 0.3f, 0.3f, 0.5f, 0.3f, 0.7f, 0.3f, 0.9f, 0.3f, //middle row
		0.1f, 0.5f, 0.3f, 0.5f, 0.5f, 0.5f, 0.7f, 0.5f, 0.9f, 0.5f,  //top row
	};

	//starts of a connection, the value refers to the position of an intersection point the intersections array
//works together with gridConnectionsB
	std::vector<int> gridConnectionsA = {
		0, 1, 2, 3, //horizontal bottom row
		5, 6, 7, 8, //horizontal middle row
		10,11,12,13,//horizontal top row
		0, 5, //vertical left column
		1, 6, //vertical 2 column
		2, 7, //vertical 3 column
		3, 8, //vertical 4 column
		4, 9  //vertical right column
	};

	//end of a connection, the value refers to the position of an intersection point the intersections array
	//works together with gridConnectionsA
	std::vector<int> gridConnectionsB = {
		1, 2, 3, 4, //horizontal bottom row
		6, 7, 8, 9, //horizontal middle row
		11,12,13,14,//horizontal top row
		5, 10, //vertical left column
		6, 11, //vertical 2 column
		7, 12, //vertical 3 column
		8, 13, //vertical 4 column
		9, 14  //vertical right column
	};

	//flag for each point
	//must be half the size of gridIntersections as its one per intersection
	std::vector<int> gridIntersectionFlags = {
		0, 0, 0, 0, 0, //bottom row
		0, 0, 0, 0, 0, //middle row
		0, 0, 0, 0, 0, //top row
	};

	if (id == 0x0A332) {
		gridIntersections = {
			0.35f, 0.37f, 0.35f, 0.52f, 0.35f, 0.67f, 0.35f, 0.82f, 0.35f, 0.97f, //bottom row
			0.495f, 0.37f, 0.495f, 0.52f, 0.495f, 0.67f, 0.495f, 0.82f, 0.495f, 0.97f, //middle row
			0.64f, 0.37f, 0.64f, 0.52f, 0.64f, 0.67f, 0.64f, 0.82f, 0.64f, 0.97f,  //top row
		};
	}
	else if (fairly_thin_panels.count(id)) {
		gridIntersections = {
				0.3f, 0.1f, 0.3f, 0.3f, 0.3f, 0.5f, 0.3f, 0.7f, 0.3f, 0.9f, //bottom row
				0.5f, 0.1f, 0.5f, 0.3f, 0.5f, 0.5f, 0.5f, 0.7f, 0.5f, 0.9f, //middle row
				0.7f, 0.1f, 0.7f, 0.3f, 0.7f, 0.5f, 0.7f, 0.7f, 0.7f, 0.9f,  //top row
		};
	}
	else if (id == 0x09EEB) {
		gridIntersections = {
				0.4f, 0.1f, 0.4f, 0.3f, 0.4f, 0.5f, 0.4f, 0.7f, 0.4f, 0.9f, //bottom row
				0.6f, 0.1f, 0.6f, 0.3f, 0.6f, 0.5f, 0.6f, 0.7f, 0.6f, 0.9f, //middle row
		};

		gridIntersectionFlags = {
			0, 0, 0, 0, 0, //bottom row
			0, 0, 0, 0, 0, //middle row
		};

		gridConnectionsA = {
			0, 1, 2, 3, //horizontal bottom row
			5, 6, 7, 8, //horizontal middle row
			0, //vertical left column
			1, //vertical 2 column
			2, //vertical 3 column
			3, //vertical 4 column
			4, //vertical right column
		};

		gridConnectionsB = {
			1, 2, 3, 4, //horizontal bottom row
			6, 7, 8, 9, //horizontal middle row
			5, //vertical left column
			6, //vertical 2 column
			7, //vertical 3 column
			8, //vertical 4 column
			9, //vertical right column
		};
	}
	else if (wide_panels.count(id)) {
		gridIntersections = {
			0.1f, 0.3f, 0.3f, 0.3f, 0.5f, 0.3f, 0.7f, 0.3f, 0.9f, 0.3f, //bottom row
			0.1f, 0.5f, 0.3f, 0.5f, 0.5f, 0.5f, 0.7f, 0.5f, 0.9f, 0.5f, //middle row
			0.1f, 0.7f, 0.3f, 0.7f, 0.5f, 0.7f, 0.7f, 0.7f, 0.9f, 0.7f,  //top row
		};
	}

	intersections.insert(intersections.begin(), gridIntersections.begin(), gridIntersections.end());
	intersectionFlags.insert(intersectionFlags.begin(), gridIntersectionFlags.begin(), gridIntersectionFlags.end());
	connectionsA.insert(connectionsA.begin(), gridConnectionsA.begin(), gridConnectionsA.end());
	connectionsB.insert(connectionsB.begin(), gridConnectionsB.begin(), gridConnectionsB.end());
}

void PanelLocker::createCenteredText(int id, std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags,
	std::vector<int>& connectionsA, std::vector<int>& connectionsB, float top, float bottom) {

	float left, right;

	if (text.size() > 20)
		text.erase(20);

	if (text.size() > 12)
		createText(id, text, intersections, intersectionFlags, connectionsA, connectionsB, 0.03f, 0.97f, top, bottom);
	else if (text.size() > 6)
		createText(id, text, intersections, intersectionFlags, connectionsA, connectionsB, 0.3f, 0.7f, top, bottom);
	else
		createText(id, text, intersections, intersectionFlags, connectionsA, connectionsB, 0.4f, 0.6f, top, bottom);
}

void PanelLocker::createText(int id, std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags,
	std::vector<int>& connectionsA, std::vector<int>& connectionsB, float left, float right, float top, float bottom) {

	int currentIntersections = intersections.size();

	Special::createText(id, text, intersections, connectionsA, connectionsB, left, right, top, bottom);

	int newIntersections = intersections.size();

	for (int i = 0; i < (newIntersections - currentIntersections) / 2; i++)
		intersectionFlags.emplace_back(0);
}

int PanelLocker::addPuzzleSimbols(const APState& state, PuzzleData* puzzle,
	std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB,
	std::vector<int>& decorations, std::vector<int>& decorationsFlags, std::vector<int>& polygons, int id) {

	//does not respect width/height but its hardcoded at a grid of 4/2
	//stored the puzzle simnbols per grid section, from bottom to top from left to right so buttom left is decoration 0
	std::vector<int> gridDecorations(8, 0);
	std::vector<int> gridDecorationsFlags(8, 0);

	int i = 0;

	if (id == 0x09EEB) i = 1;

	int position = 0;

	int extraRows = 0;

	if (puzzle->hasStones && !state.unlockedStones) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);;
		gridDecorations[position] = Decoration::Stone | Decoration::Color::White;
		i++; // 1
	}

	if (puzzle->hasColoredStones && !state.unlockedColoredStones) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Stone | Decoration::Color::Orange;
		i++; // 2
	}

	if (puzzle->hasStars && !state.unlockedStars) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Star | Decoration::Color::Green;

		i++; // 3
	}

	if (puzzle->hasStarsWithOtherSymbol && !state.unlockedStarsWithOtherSimbol) {
		int intersectionOffset = intersectionFlags.size();

		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		int intersectionIndex = position;

		if (position > 3) intersectionIndex++;

		float x = intersections[intersectionIndex * 2];
		float y = intersections[intersectionIndex * 2 + 1];

		float factor = 1.0f;
		if (id == 0x0A332) {
			factor = 0.75f;
		}

		std::vector<float> newIntersections = {
			x - 0.03f, y + 0.03f,	x + 0.17f, y + 0.03f,					0, 0, 0 ,0,	0, 0,
			x - 0.03f, y + 0.23f,	x + 0.17f, y + 0.23f,					0, 0, 0 ,0,	0, 0,

			x + 0.03f, y - 0.03f,	x + 0.23f, y - 0.03f,					0, 0, 0 ,0,	0, 0,
			x + 0.03f, y + 0.17f,	x + 0.23f, y + 0.17f,					0, 0, 0 ,0,	0, 0,
		};

		std::vector<int> newIntersectionFlags = {
			0x8, 0x8, 0x8, 0x8, 0x8,
			0x8, 0x8, 0x8, 0x8, 0x8,
			0x8, 0x8, 0x8, 0x8, 0x8,
			0x8, 0x8, 0x8, 0x8, 0x8,
		};

		gridDecorations.emplace_back(0); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0);
		gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);	gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);

		gridDecorations.emplace_back(Decoration::Star | Decoration::Color::Green); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0);
		gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);	gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);
		
		gridDecorations.emplace_back(0); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0);
		gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);	gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);

		gridDecorations.emplace_back(Decoration::Stone | Decoration::Color::Green); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0); gridDecorations.emplace_back(0);
		gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);	gridDecorationsFlags.emplace_back(0); gridDecorationsFlags.emplace_back(0);


		extraRows = 4;

		intersections.insert(intersections.end(), newIntersections.begin(), newIntersections.end());
		intersectionFlags.insert(intersectionFlags.end(), newIntersectionFlags.begin(), newIntersectionFlags.end());

		i++; // 4
	}

	if (puzzle->hasErasers && !state.unlockedErasers) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Eraser;
		i++; // 5
	}

	if (puzzle->hasTriangles && !state.unlockedTriangles) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Triangle2 | Decoration::Yellow;
		i++; // 6
	}

	const int defaultLShape = 0x00170000;

	if (puzzle->hasTetris && !state.unlockedTetris) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Poly | Decoration::Color::Yellow | defaultLShape;
		i++; // 7
	}

	if (puzzle->hasTetrisRotated && !state.unlockedTetrisRotated){
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Poly | Decoration::Color::Yellow | defaultLShape | Decoration::Can_Rotate;
		i++; // 8 - Careful from here on!
	}

	if (puzzle->hasTetrisNegative && !state.unlockedTetrisNegative && i != gridDecorations.size()) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Poly | Decoration::Color::Blue | defaultLShape | Decoration::Negative;
		i++;
	}

	if (puzzle->hasSymmetry && !state.unlockedSymmetry && i != gridDecorations.size()) {
		int intersectionOffset = intersectionFlags.size();

		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		int intersectionIndex = position;
		if (position > 3) intersectionIndex++;

		float x = intersections[intersectionIndex * 2];
		float y = intersections[intersectionIndex * 2 + 1];

		float factor = 1.0f;
		if (id == 0x0A332) {
			factor = 0.75f;
		}

		std::vector<float> symmetryIntersections = {
			x + 0.05f * factor, y + 0.05f * factor, x + 0.08f * factor, y + 0.05f * factor, x + 0.12f * factor, y + 0.05f * factor, x + 0.15f * factor, y + 0.05f * factor, //bottom row
			x + 0.05f * factor, y + 0.10f * factor, x + 0.08f * factor, y + 0.10f * factor, x + 0.12f * factor, y + 0.10f * factor, x + 0.15f * factor, y + 0.10f * factor, //middle row
			x + 0.05f * factor, y + 0.15f * factor, x + 0.08f * factor, y + 0.15f * factor, x + 0.12f * factor, y + 0.15f * factor, x + 0.15f * factor, y + 0.15f * factor, //top row
		};

		std::vector<int> symmetryIntersectionFlags = {
			0, 0, 0, 0, //bottom row
			0, 0, 0, 0, //middle row
			0, 0, 0, 0, //top row
		};
		std::vector<int> symmetryConnectionsA = {
			intersectionOffset + 0, intersectionOffset + 2, //horizontal bottom row
			intersectionOffset + 4, intersectionOffset + 6, //horizontal middle row
			intersectionOffset + 8, intersectionOffset + 11,//horizontal top row
			intersectionOffset + 4, // vertical left column
			intersectionOffset + 1, // vertical 2 column
			intersectionOffset + 2, // vertical 3 column
			intersectionOffset + 7, // vertical right column
		};
		std::vector<int> symmetryConnectionsB = {
			intersectionOffset + 1, intersectionOffset + 3, //horizontal bottom row
			intersectionOffset + 5, intersectionOffset + 7, //horizontal middle row
			intersectionOffset + 9, intersectionOffset + 10,//horizontal top row
			intersectionOffset + 8, // vertical left column
			intersectionOffset + 5, // vertical 2 column
			intersectionOffset + 6, // vertical 3 column
			intersectionOffset + 11,// vertical right column
		};

		intersections.insert(intersections.end(), symmetryIntersections.begin(), symmetryIntersections.end());
		intersectionFlags.insert(intersectionFlags.end(), symmetryIntersectionFlags.begin(), symmetryIntersectionFlags.end());
		connectionsA.insert(connectionsA.end(), symmetryConnectionsA.begin(), symmetryConnectionsA.end());
		connectionsB.insert(connectionsB.end(), symmetryConnectionsB.begin(), symmetryConnectionsB.end());

		i++;
	}

	if (puzzle->hasArrows && !state.unlockedArrows && i != gridDecorations.size()) {
		int intersectionOffset = intersectionFlags.size();
		
		position = (1 - (i % 2)) * 4 + ((int) i / 2);

		int intersectionIndex = position;
		if (position > 3) intersectionIndex++;

		float x = intersections[intersectionIndex * 2];
		float y = intersections[intersectionIndex * 2 + 1];

		std::vector<float> arrowIntersections = {
			x + 0.05f, y + 0.1f, x + 0.15f, y + 0.1f,
			x + 0.09f, y + 0.05f, x + 0.09f, y + 0.15f,
		};
		std::vector<int> arrowIntersectionFlags = { 0, 0, 0, 0 };
		std::vector<int> arrowConnectionsA = {
			intersectionOffset + 0,	intersectionOffset + 0,	intersectionOffset + 0,
		};
		std::vector<int> arrowConnectionsB = {
			intersectionOffset + 1, intersectionOffset + 2, intersectionOffset + 3,
		};

		intersections.insert(intersections.end(), arrowIntersections.begin(), arrowIntersections.end());
		intersectionFlags.insert(intersectionFlags.end(), arrowIntersectionFlags.begin(), arrowIntersectionFlags.end());
		connectionsA.insert(connectionsA.end(), arrowConnectionsA.begin(), arrowConnectionsA.end());
		connectionsB.insert(connectionsB.end(), arrowConnectionsB.begin(), arrowConnectionsB.end());

		i++;
	}

	if (state.keysInTheGame.count(puzzle->id) && !state.keysReceived.count(puzzle->id)) {
		int intersectionOffset = intersectionFlags.size();

		position = (1 - (i % 2)) * 4 + ((int)i / 2);

		int intersectionIndex = position;
		if (position > 3) intersectionIndex++;

		float x = intersections[intersectionIndex * 2];
		float y = intersections[intersectionIndex * 2 + 1];

		std::vector<float> keyIntersections = {
			x + 0.05f, y + 0.153f, x + 0.05f, y + 0.1f, x + 0.15f, y + 0.1f, x + 0.15f, y + 0.153f, 
			
			x + 0.1f, y + 0.1f, x + 0.1f, y + 0.045f,

			x + 0.1f, y + 0.061f, x + 0.12f, y + 0.061f,
		};
		std::vector<int> keyIntersectionFlags = { 0, 0, 0, 0, 0, 0, 0, 0 };
		std::vector<int> keyConnectionsA = {
			intersectionOffset + 0,	intersectionOffset + 1,	intersectionOffset + 2, intersectionOffset + 3,
			intersectionOffset + 4, intersectionOffset + 6,
		};
		std::vector<int> keyConnectionsB = {
			intersectionOffset + 1,	intersectionOffset + 2,	intersectionOffset + 3, intersectionOffset + 0,
			intersectionOffset + 5, intersectionOffset + 7,
		};

		intersections.insert(intersections.end(), keyIntersections.begin(), keyIntersections.end());
		intersectionFlags.insert(intersectionFlags.end(), keyIntersectionFlags.begin(), keyIntersectionFlags.end());
		connectionsA.insert(connectionsA.end(), keyConnectionsA.begin(), keyConnectionsA.end());
		connectionsB.insert(connectionsB.end(), keyConnectionsB.begin(), keyConnectionsB.end());

		i++;
	}

	if (puzzle->hasColoredDots && !state.unlockedColoredDots) {
		intersectionFlags[11] = IntersectionFlags::DOT | IntersectionFlags::DOT_IS_BLUE;
		intersectionFlags[12] = IntersectionFlags::DOT | IntersectionFlags::DOT_IS_ORANGE;

		if (puzzle->hasDots && !state.unlockedDots)
			intersectionFlags[13] = IntersectionFlags::DOT;
		else if (puzzle->hasSoundDots && !state.unlockedSoundDots) {
			intersectionFlags[6] = IntersectionFlags::DOT | IntersectionFlags::DOT_SMALL;
			intersectionFlags[7] = IntersectionFlags::DOT | IntersectionFlags::DOT_MEDIUM;
			intersectionFlags[8] = IntersectionFlags::DOT | IntersectionFlags::DOT_LARGE;
		}
	}

	if (puzzle->hasFullDots && !state.unlockedFullDots) {
		for (int i = 0; i < 15; i++) {
			intersectionFlags[i] |= IntersectionFlags::DOT;
		}
	}
	else if (puzzle->hasSoundDots && !state.unlockedSoundDots) {
		intersectionFlags[11] = IntersectionFlags::DOT | IntersectionFlags::DOT_SMALL;
		intersectionFlags[12] = IntersectionFlags::DOT | IntersectionFlags::DOT_MEDIUM;
		intersectionFlags[13] = IntersectionFlags::DOT | IntersectionFlags::DOT_LARGE;
	}
	else if (puzzle->hasDots && !state.unlockedDots) {
		if (id == 0x09EEB) intersectionFlags[0] = IntersectionFlags::DOT;
		else intersectionFlags[11] = IntersectionFlags::DOT;
	}

	decorations.insert(decorations.begin(), gridDecorations.begin(), gridDecorations.end());
	decorationsFlags.insert(decorationsFlags.begin(), gridDecorationsFlags.begin(), gridDecorationsFlags.end());

	return extraRows;
}

bool PanelLocker::PuzzleIsLocked(int id) {
	return lockedPuzzles.count(id) != 0;
}