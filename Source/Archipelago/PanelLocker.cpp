#include "PanelLocker.h"

void PanelLocker::DisableNonRandomizedPuzzles()
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

	Special::setPower(0x17CA4, true);
	Special::setPower(0x17CAB, true);
	Special::setPower(0x28B39, true);
	Special::setPower(0x00C92, true); //Monastary door right
	Special::setPower(0x1972A, true); //Shadows avoid 4

	disablePuzzle(0x1972A); //Shadows
	disablePuzzle(0x17C2E); //BNK3R door
	disablePuzzle(0x00B10); //Monastary door left
	disablePuzzle(0x00C92); //Monastary door right
	disablePuzzle(0x002C4); //Waves one
	disablePuzzle(0x18590); //Town transparent symmetry
	disablePuzzle(0x00143); //Orchard Apple Tree 1
	disablePuzzle(0x00139); //Keep Hedge Maze 1
	disablePuzzle(0x15ADD); //River Rhombic Avoid Vault
	disablePuzzle(0x0042D); //Mountaintop River Shape
	disablePuzzle(0x009B8); //Symmetry Island Scenery Outlines 1
}

void PanelLocker::EarlyUTM()
{
	Special::copyTarget(0x17C42, 0x021D7); // Mountain Discard Early UTM
}

void PanelLocker::disablePuzzle(int id) {
	if (lockedPuzzles.count(id) == 1)
		unlockPuzzle(lockedPuzzles[id]);

	disabledPuzzles.emplace_back(id);
	
	std::vector<float> intersections = { 0.0f, 0.0f, 1.0f, 1.0f };
	std::vector<int> intersectionFlags = { IntersectionFlags::STARTPOINT, IntersectionFlags::ENDPOINT };
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;	
	std::vector<int> decorations = { Decoration::Triangle }; //Invisable triangle to have a non empty decorations array
	std::vector<int> decorationsFlags = { 0 };

	createText(id, "disabled", intersections, intersectionFlags, connectionsA, connectionsB, 0.1f, 0.9f, 0.1f, 0.4f);

	_memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.5f });
	_memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
	_memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
	_memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
	_memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorationsFlags.size()) });
	_memory->WriteArray<int>(id, DECORATIONS, decorations);
	_memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);
	_memory->WritePanelData<int>(id, TRACED_EDGES, { 0 }); //removed the traced line
	_memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void PanelLocker::UpdatePuzzleLocks(const APState& state, const int& itemIndex) {
	std::vector<PuzzleData*> puzzlesToUpdate;

	for (auto const& [id, puzzle] : lockedPuzzles) {
		switch (itemIndex) {
			//Puzzle Symbols
			case ITEM_DOTS:							if (puzzle->hasDots) puzzlesToUpdate.push_back(puzzle);							break;
			case ITEM_COLORED_DOTS:					if (puzzle->hasColoredDots) puzzlesToUpdate.push_back(puzzle);					break;
			case ITEM_SOUND_DOTS:					if (puzzle->hasSoundDots) puzzlesToUpdate.push_back(puzzle);					break;
			case ITEM_INVISIBLE_DOTS:				if (puzzle->hasInvisibleDots) puzzlesToUpdate.push_back(puzzle);				break;
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

			default:																																			break;
		}
	}

	for (auto& puzzle : puzzlesToUpdate)
		UpdatePuzzleLock(state, puzzle->id);
}

void PanelLocker::UpdatePuzzleLock(const APState& state, const int& id) {
	if (find(disabledPuzzles.begin(), disabledPuzzles.end(), id) != disabledPuzzles.end())
		return;

	bool isLocked;
	PuzzleData* puzzle;

	if (lockedPuzzles.count(id) == 1) {
		isLocked = true;
		puzzle = lockedPuzzles[id];
	}
	else {
		isLocked = false;
		puzzle = new PuzzleData(id);
		puzzle->Read(_memory);
	}

	if ((puzzle->hasStones && !state.unlockedStones)
		|| (puzzle->hasStars && !state.unlockedStars)
		|| (puzzle->hasStarsWithOtherSymbol && !state.unlockedStarsWithOtherSimbol)
		|| (puzzle->hasTetris && !state.unlockedTetris)
		|| (puzzle->hasTetrisRotated && !state.unlockedTetrisRotated)
		|| (puzzle->hasTetrisNegative && !state.unlockedTetrisNegative)
		|| (puzzle->hasTetrisNegative && !state.unlockedTetrisNegative)
		|| (puzzle->hasErasers && !state.unlockedErasers)
		|| (puzzle->hasTriangles && !state.unlockedTriangles)
		|| (puzzle->hasDots && !state.unlockedDots)
		|| (puzzle->hasColoredDots && !state.unlockedColoredDots)
		//|| (puzzle->hasInvisibleDots && !state.unlockedInvisibleDots) //NYI
		|| (puzzle->hasSoundDots && !state.unlockedSoundDots)
		|| (puzzle->hasArrows && !state.unlockedArrows)
		|| (puzzle->hasSymmetry && !state.unlockedSymmetry))
	{
		//puzzle should be locked
		if (!isLocked)
			lockedPuzzles.insert({ id, puzzle });

		const int width = 4;
		const int height = 2;

		std::vector<float> intersections;
		std::vector<int> intersectionFlags;
		std::vector<int> connectionsA;
		std::vector<int> connectionsB;
		std::vector<int> decorations;
		std::vector<int> decorationsFlags;
		std::vector<int> polygons;

		addMissingSimbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB);

		addPuzzleSimbols(state, puzzle, intersections, intersectionFlags, connectionsA, connectionsB, decorations, decorationsFlags, polygons);

		createText(id, "missing", intersections, intersectionFlags, connectionsA, connectionsB, 0.1f, 0.9f, 0.1f, 0.4f);

		_memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
		_memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 });
		_memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.5f });
		_memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
		_memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
		_memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
		_memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
		_memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
		_memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
		_memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
		_memory->WriteArray<int>(id, DECORATIONS, decorations);
		_memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);
		if (polygons.size() > 0) { //TODO maybe just always write ?
			_memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(polygons.size()) / 4 }); //why devide by 4 tho?
			_memory->WriteArray<int>(id, COLORED_REGIONS, polygons);
			_memory->WritePanelData<int>(id, OUTER_BACKGROUND_MODE, { 1 });
		}
		_memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
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

void PanelLocker::SetItemReward(const int& id, const APClient::NetworkItem& item, const bool& receiving, const std::string& receivingPlayer, const std::string& itemName) {
	if (!_memory->ReadPanelData<int>(id, SOLVED)) //Setting item reward makes the puzzle unsolveable only do it on solved puzzles
		return;

	//Mountain combo panel & Mountain Bottom Layer Discard
	if (id == 0x09FD2 || id == 0x17FA2)
	{
		return;
		//Combo Panel: This fails as the last panel can be solved while yielding an error on one of the previus panels
		//maybe check if staircase is opened
		//Bottom Layer Discard: This panel opens a door on a timer. If you fail to go through the door in time, you will get locked.
	}

	std::vector<float> intersections = { 0.0f, 0.0f, 1.0f, 1.0f };
	std::vector<int> intersectionFlags = { IntersectionFlags::STARTPOINT, IntersectionFlags::ENDPOINT };
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;

	createCenteredText(id, receiving ? "received" : "send", intersections, intersectionFlags, connectionsA, connectionsB, 0.03f, 0.13f);

	std::vector<std::string> words = StringSplitter::split(itemName, ' ');
	for (std::size_t i = 0; i < words.size() && i < 5; ++i)
		createCenteredText(id, words[i], intersections, intersectionFlags, connectionsA, connectionsB, 0.22f + (0.1f * i), 0.3f + (0.1f * i));

	if (!receiving) {
		std::string player = receivingPlayer;
		createCenteredText(id, "to " + player, intersections, intersectionFlags, connectionsA, connectionsB, 0.87f, 0.97f);
	}

	Color backgroundColor;
	if (item.flags & APClient::ItemFlags::FLAG_ADVANCEMENT)
		backgroundColor = { 0.686f, 0.6f, 0.937f, 1.0f };
	else if (item.flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE)
		backgroundColor = { 0.427f, 0.545f, 0.91f, 1.0f };
	else if (item.flags & APClient::ItemFlags::FLAG_TRAP)
		backgroundColor = { 0.98f, 0.502f, 0.447f, 1.0f };
	else
		backgroundColor = { 0.0f , 0.933f, 0.933f, 1.0f };

	_memory->WritePanelData<Color>(id, PATH_COLOR, { { 0.0f, 0.0f, 0.0f, 1.0f } });
	_memory->WritePanelData<Color>(id, BACKGROUND_REGION_COLOR, { backgroundColor });
	_memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.3f });
	_memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
	_memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
	_memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
	_memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WritePanelData<int>(id, TRACED_EDGES, { 0 }); //removed the traced line

	std::vector<int> decorations; 
	std::vector<int> decorationsFlags;
	//override decorations with invisiables triangles of size 0
	int numDecorations = _memory->ReadPanelData<int>(id, NUM_DECORATIONS);
		for (int i = 0; i < numDecorations; i++) {
		decorations.emplace_back(Decoration::Triangle);
		decorationsFlags.emplace_back(0);
	}
	_memory->WriteArray<int>(id, DECORATIONS, decorations);
	_memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);

	_memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void PanelLocker::unlockPuzzle(PuzzleData* puzzle) {
	puzzle->Restore(_memory);
	lockedPuzzles.erase(puzzle->id);
	delete puzzle;
}

void PanelLocker::addMissingSimbolsDisplay(std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB) {
	//does not respect width/height but its hardcoded at a grid of 4/2

	//panel coordinates go from 0,0 in bottom left to 1,1 in top right
	//format vector of x,y,x,y,x,y,x,y,x,y
	std::vector<float> gridIntersections = {
		0.1f, 0.1f, 0.3f, 0.1f, 0.5f, 0.1f, 0.7f, 0.1f, 0.9f, 0.1f, //bottom row
		0.1f, 0.3f, 0.3f, 0.3f, 0.5f, 0.3f, 0.7f, 0.3f, 0.9f, 0.3f, //middle row
		0.1f, 0.5f, 0.3f, 0.5f, 0.5f, 0.5f, 0.7f, 0.5f, 0.9f, 0.5f,  //top row
		0.0f, 0.0f, 1.0f, 1.0f //start, exit needed to prevent crash on re-randomization
	};

	//flag for each point
	//must be half the size of gridIntersections as its one per intersection
	std::vector<int> gridIntersectionFlags = {
		0, 0, 0, 0, 0, //bottom row
		0, 0, 0, 0, 0, //middle row
		0, 0, 0, 0, 0, //top row
		IntersectionFlags::STARTPOINT, IntersectionFlags::ENDPOINT //start, exit
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

void PanelLocker::addPuzzleSimbols(const APState& state, PuzzleData* puzzle,
	std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB,
	std::vector<int>& decorations, std::vector<int>& decorationsFlags, std::vector<int>& polygons) {

	//does not respect width/height but its hardcoded at a grid of 4/2
	//stored the puzzle simnbols per grid section, from bottom to top from left to right so buttom left is decoration 0
	std::vector<int> gridDecorations(8, 0);
	std::vector<int> gridDecorationsFlags(8, 0);

	int column = 4;
	int secondRowOffset = -column;

	if (puzzle->hasStones && !state.unlockedStones && puzzle->hasColoredStones && !state.unlockedColoredStones) {
		gridDecorations[column] = Decoration::Stone | Decoration::Color::Black;
		gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::Blue;
		column++;
	}
	else if (puzzle->hasColoredStones && !state.unlockedColoredStones) {
		gridDecorations[column] = Decoration::Stone | Decoration::Color::Orange;
		gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::Blue;
		column++;
	}
	else if (puzzle->hasStones && !state.unlockedStones) {
		gridDecorations[column] = Decoration::Stone | Decoration::Color::Black;
		gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::White;
		column++;
	}

	if (puzzle->hasStarsWithOtherSymbol && !state.unlockedStarsWithOtherSimbol) {
		gridDecorations[column] = Decoration::Star | Decoration::Color::Green;
		gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::Green;
		column++;
	}
	else if (puzzle->hasStars && !state.unlockedStars) {
		gridDecorations[column] = Decoration::Star | Decoration::Color::Magenta;
		column++;
	}

	const int defaultLShape = 0x00170000;
	if ((puzzle->hasTetris && !state.unlockedTetris)
		|| (puzzle->hasTetrisRotated && !state.unlockedTetrisRotated)
		|| (puzzle->hasTetrisNegative && !state.unlockedTetrisNegative))
	{
		if (puzzle->hasTetrisRotated && !state.unlockedTetrisRotated)
			gridDecorations[column] = Decoration::Poly | Decoration::Color::Yellow | defaultLShape | Decoration::Can_Rotate;
		else if (puzzle->hasTetris && !state.unlockedTetris)
			gridDecorations[column] = Decoration::Poly | Decoration::Color::Yellow | defaultLShape;

		if (puzzle->hasTetrisNegative && !state.unlockedTetrisNegative)
			gridDecorations[column + secondRowOffset] = Decoration::Poly | Decoration::Color::Blue | defaultLShape | Decoration::Negative;

		column++;
	}

	if (puzzle->hasErasers && !state.unlockedErasers) {
		gridDecorations[column] = Decoration::Eraser;
		column++;
	}

	if (puzzle->hasTriangles && !state.unlockedTriangles) {
		gridDecorations[column] = Decoration::Triangle2;
		column++;
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
	else if (puzzle->hasInvisibleDots && !state.unlockedInvisibleDots) {
		intersectionFlags[11] = IntersectionFlags::DOT | IntersectionFlags::DOT_IS_INVISIBLE;
		intersectionFlags[12] = IntersectionFlags::DOT;
	}
	else if (puzzle->hasSoundDots && !state.unlockedSoundDots) {
		intersectionFlags[11] = IntersectionFlags::DOT | IntersectionFlags::DOT_SMALL;
		intersectionFlags[12] = IntersectionFlags::DOT | IntersectionFlags::DOT_MEDIUM;
		intersectionFlags[13] = IntersectionFlags::DOT | IntersectionFlags::DOT_LARGE;
	}
	else if (puzzle->hasDots && !state.unlockedDots) {
		intersectionFlags[11] = IntersectionFlags::DOT;
	}

	if (puzzle->hasSymmetry && !state.unlockedSymmetry) {
		float x = intersections[(column - 4) * 2];
		int intersectionOffset = intersectionFlags.size();

		std::vector<float> symmetryIntersections = {
			x + 0.05f, 0.35f, x + 0.08f, 0.35f, x + 0.12f, 0.35f, x + 0.15f, 0.35f, //bottom row
			x + 0.05f, 0.40f, x + 0.08f, 0.40f, x + 0.12f, 0.40f, x + 0.15f, 0.40f, //middle row
			x + 0.05f, 0.45f, x + 0.08f, 0.45f, x + 0.12f, 0.45f, x + 0.15f, 0.45f, //top row
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

		column++;
	}

	if (puzzle->hasArrows && !state.unlockedArrows) {
		float x = intersections[(column - 4) * 2];
		int intersectionOffset = intersectionFlags.size();

		std::vector<float> arrowIntersections = {
			x + 0.05f, 0.40f, x + 0.15f, 0.40f,
			x + 0.09f, 0.35f, x + 0.09f, 0.45f
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

		//TODO Fix meh
		//Panel panel;
		//panel.render_arrow(column, 1, 2, 0, intersections, intersectionFlags, polygons);
		column++;
	}

	decorations.insert(decorations.begin(), gridDecorations.begin(), gridDecorations.end());
	decorationsFlags.insert(decorationsFlags.begin(), gridDecorationsFlags.begin(), gridDecorationsFlags.end());
}
