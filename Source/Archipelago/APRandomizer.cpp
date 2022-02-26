#include "APRandomizer.h"
#include "APWatchdog.h"

int Seed;

int AllPuzzles[]{
		0x0A171, 0x04CA4, // Tutorial Secret back area 
		0x0005D, 0x0005E, 0x0005F, 0x00060, 0x00061, // Tutorial Dots Tutorial
		0x018AF, 0x0001B, 0x012C9, 0x0001C, 0x0001D, 0x0001E, 0x0001F, 0x00020, 0x00021,	// Tutorial Stones Tutorial
		0x00086, 0x00087, 0x00059, 0x00062, 0x0005C, // Symmetry Vertical Symmetry Mazes
		0x0008D, 0x00081, 0x00083, 0x00084, 0x00082, 0x0343A, // Symmetry Rotational Symmetry Mazes
		0x000B0, // Symmetry Island Door
		0x00022, 0x00023, 0x00024, 0x00025, 0x00026, // Symmetry Black Dots
		0x0007C, 0x0007E, 0x00075, 0x00073, 0x00077, 0x00079, // Symmetry Colored Dots
		0x00065, 0x0006D, 0x00072, 0x0006F, 0x00070, 0x00071, 0x00076, // Symmetry Fading Lines
		0x00A52, 0x00A61, 0x00A57, 0x00A64, 0x00A5B, 0x00A68, // Symmetry Dot Reflection Dual Panels (before laser)
		0x09E57, 0x17C09, // Quarry Entry Gates
		0x01E5A, 0x01E59, // Quarry Mill Entry Door
		0x00E0C, 0x01489, 0x0148A, 0x014D9, 0x014E7, 0x014E8, // Quarry Dots
		0x00557, 0x005F1, 0x00620, 0x009F5, 0x0146C, 0x3C12D,0x03686, 0x014E9, // Quarry Stones
		0x0367C, 0x3C125, // Quarry Dots + Stones
		0x034D4, 0x021D5, // Quarry Boathouse Ramp Activation
		0x021B3, 0x021B4, 0x021B0, 0x021AF, 0x021AE, // Quarry Eraser + Shapes
		0x021B5, 0x021B6, 0x021B7, 0x021BB, 0x09DB5, 0x09DB1, 0x3C124, // Quarry + Stars
		0x09DB3, 0x09DB4, 0x0A3CB, 0x0A3CC, 0x0A3D0, // Quarry Eraser + Stars + Shapes
		0x0056E, 0x00469, 0x00472, 0x00262, 0x00474, 0x00553, 0x0056F, // Swamp First row
		0x00390, 0x010CA, 0x00983, 0x00984, 0x00986, 0x00985, 0x00987, 0x181A9, // Swamp Second Row
		//0x00609, // Swamp Bridge controll 
		0x00982, 0x0097F, 0x0098F, 0x00990, 0x17C0D, 0x17C0E, // Swamp Red Panels
		0x00999, 0x0099D, 0x009A0, 0x009A1, // Swamp Disconnected Shapes
		0x00007, 0x00008, 0x00009, 0x0000A, 0x003B2, 0x00A1E, 0x00C2E, 0x00E3A, // Swamp Rotating Shapes
		0x009A6, // Swamp Optional Tetris
		0x009AB, 0x009AD, 0x009AE, 0x009AF, 0x00006, // Swamp Negative Shapes 1
		0x00002, 0x00004, 0x00005, 0x013E6, 0x00596, // Swamp Negative Shapes 2
		0x00001, 0x014D2, 0x014D4, 0x014D1, // Swamp Negative Shapes 3
		0x17C05, 0x17C02, // Swamp Exit Shortcut
		0x02886, // Treehouse Entry door
		0x17D72, 0x17D8F, 0x17D74, 0x17DAC, 0x17D9E, 0x17DB9, 0x17D9C, 0x17DC2, 0x17DC4, 0x0A182, // Treehouse Yellow Bridge
		0x17DC8, 0x17DC7, 0x17CE4, 0x17D2D, 0x17D6C, // Treehouse Pink Bridge 1
		0x17D9B, 0x17D99, 0x17DAA, 0x17D97, 0x17BDF, 0x17D91, 0x17DC6, // Treehouse Pink Bridge 2
		0x17DB3, 0x17DB5, 0x17DB6, 0x17DC0, 0x17DD7, 0x17DD9, 0x17DB8, 0x17DDC, 0x17DD1, 0x17DDE, 0x17DE3, 0x17DEC, 0x17DAE, 0x17DB0, 0x17DDB, // Treehouse Orange Bridge 1
		0x17D88, 0x17DB4, 0x17D8C, 0x17CE3, 0x17DCD, 0x17DB2, 0x17DCC, 0x17DCA, 0x17D8E, 0x17DB7, 0x17DB1, 0x17DA2, // Treehouse Orange Bridge 2
		0x17E3C, 0x17E4D, 0x17E4F, 0x17E52, 0x17E5B, 0x17E5F, 0x17E61, // Treehouse Green Bridge
		0x2899C, 0x28A33, 0x28ABF, 0x28AC0, 0x28AC1, 0x28AD9, // Town Full Dots + Shapes
		0x28AC7, 0x28AC8, 0x28ACA, 0x28ACB, 0x28ACC, // Town Blue Symmetry
		0x28998, // Town Glass Door
		0x28A69, // Town Church Star Door
		0x034E3, // Town Soundproof Room
		0x03C0C, 0x03C08, // Town 3-color Room
		0x0A0C8, // Town Orange
		0x17F89, 0x0A168, 0x33AB2, //Windmill Puzzles
		0x033D4, 0x0CC7B, 0x002A6, 0x00AFB, 0x15ADD, // Vaults
		0x17D28, 0x3C12B, 0x17CF0, 0x17FA9, 0x17FA0, 0x17D27, 0x17CFB, 0x17D01, 0x17C71, 0x17CF7, 0x17C42, 0x17CE7, // Triangle Discards
		0x17C34, // Mountain
		0x09E39,	// Mountain Purple Bridge
		0x09E73, 0x09E75, 0x09E78, 0x09E79, 0x09E6C, 0x09E6F, 0x09E6B, 0x09E7A, 0x09E71, 0x09E72, 0x09E69, 0x09E7B, // Mountain Orange Row
		0x09E7A, 0x09E71, 0x09E72, 0x09E69, 0x09E7B, // Mountain Green Row
		0x09EAD, 0x09EAF, 0x33AF5, 0x33AF7, 0x09F6E, // Mountain Purple Panels
		0x09FD3, 0x09FD4, 0x09FD6, 0x09FD7, 0x09FD8, // Mountain Rainbow Row
		0x09FCC, 0x09FCE, 0x09FCF, 0x09FD0, 0x09FD1, 0x09FD2, // Mountain multi puzzle
		0x09E86, 0x09ED8, // Mountain double bridge
		0x09EFF, 0x09F01, 0x09FC1, 0x09F8E, 0x09FDA, // Mountain floor
		0x0383D, 0x0383A, 0x0383F, 0x09E56, 0x03859, 0x09E5A, 0x339BB, 0x33961, // Mountain pillar puzzles
		/* Disabled caves as its beyond the final puzzles and thus makes no sense to include in AP
		0x17FA2, 0x00FF8, // Caves
		0x01A0D, 0x018A0, 0x009A4, 0x00A72, // Caves Blue Symmetry/Tetris
		0x00190, 0x00558, 0x00567, 0x006FE, 0x008B8, 0x00973, 0x0097B, 0x0097D, 0x0097E, 0x00994, 0x334D5, 0x00995, 0x00996, 0x00998, // Caves riangle Puzzles
		0x32962, 0x32966, 0x01A31, 0x00B71, // Caves First alcove
		0x288EA, 0x288FC, 0x289E7, 0x288AA, // Caves Perspective
		0x0A16B, 0x0A2CE, 0x0A2D7, 0x0A2DD, 0x0A2EA, 0x17FB9, // Caves Full Dots
		0x0008F, 0x0006B, 0x0008B, 0x0008C, 0x0008A, 0x00089, 0x0006A, 0x0006C, 0x00027, 0x00028, 0x00029,	// Caves Invisible Dots
		0x17CF2, 0x021D7 // Caves Exit
		0x09DD5, 0x0A16E, 0x039B4, 0x09E85 // Caves Deep challenge caves
		*/
		//0x032FF, //Why a single apple tree?
		0x00698,	0x0048F, 0x09F92,	0x0A036,	0x09DA6,	0x0A049, 0x0A053, 0x09F94, // Desert Surface
		0x00422, 0x006E3, 0x0A02D, // Desert Light
		0x00C72, 0x0129D,	0x008BB, 0x0078D, 0x18313, // Desert Pond
		0x04D18,	0x01205, 0x181AB, 0x0117A, 0x17ECA, // Desert Flood
		//	0x18076, 0x0A15C, 0x09FFF, 0x0A15F, 0x012D7 // Desert Final and exit
		0x033EA, 0x01BE9, 0x01BE9, 0x01CD3, 0x01CD3, 0x01D3F, 0x01D3F, 0x03317, 0x0360E, // Keep
		0x0026F, 0x00C3F, 0x00C3F, 0x00C41, 0x00C41, 0x014B2, 0x014B2 // Jungle
};

APRandomizer::APRandomizer() {
	_memory = std::make_shared<Memory>("witness64_d3d11.exe");
}

void APRandomizer::Connect() {

}

void APRandomizer::Initialize(HWND loadingHandle) {
	disablePuzzles(loadingHandle);
}

void APRandomizer::disablePuzzles(HWND loadingHandle) {
	for (int i = 0; i < sizeof(AllPuzzles) / sizeof(AllPuzzles[0]); i++)
	{
		std::wstring text = L"Locking puzzles: " + std::to_wstring(i) + L"/" + std::to_wstring(sizeof(AllPuzzles));
		SetWindowText(loadingHandle, text.c_str());

		disablePuzzle(AllPuzzles[i]);
	}
}

void APRandomizer::enablePuzzles() {
	std::vector<PuzzleData> puzzlesToEnable;

	for (auto const& [id, puzzle] : disabledPuzzles)
	{
		if (
				(!unlockedStones && puzzle.hasStones)
				|| (!unlockedColoredStones && puzzle.hasColoredStones)
				|| (!unlockedStars && puzzle.hasStars)
				|| (!unlockedTetris && puzzle.hasTetris)
				|| (!unlockedErasers && puzzle.hasErasers)
				|| (!unlockedTriangles && puzzle.hasTriangles)
				|| (!unlockedDots && puzzle.hasDots)
				|| (!unlockedColoredDots && puzzle.hasColoredDots)
				|| (!unlockedInvisibleDots && puzzle.hasInvisibleDots)
				|| (!unlockedArrows && puzzle.hasArrows)
			)
			continue;

		puzzlesToEnable.push_back(puzzle);
	}

	for (auto& puzzle : puzzlesToEnable) {
		puzzle.Restore(_memory);
		disabledPuzzles.erase(puzzle.id);
	}
}

void APRandomizer::GenerateNormal() {
	StartWatching(); //From now on, we check completion.
	PreventSnipes(); //Prevents Snipes to preserve progression randomizer experience
}

void APRandomizer::StartWatching() {
	//test
	(new APWatchdog())->start();
	//holy shit it works
}

void APRandomizer::PreventSnipes()
{
	// Distance-gate shadows laser to prevent sniping through the bars
	_memory->WritePanelData<float>(0x19650, MAX_BROADCAST_DISTANCE, { 2.5 });
}

void APRandomizer::disablePuzzle(int id) {
	if (disabledPuzzles.count(id) == 1)
		return;

	PuzzleData puzzle = PuzzleData(_memory, id);

	disabledPuzzles.insert({ id, puzzle });

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

	Special::createText(id, "missing", intersections, connectionsA, connectionsB, 0.1f, 0.9f, 0.1f, 0.4f);

	addPuzzleSimbols(puzzle, decorations, decorationsFlags, intersections, intersectionFlags, polygons);

	_memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
	_memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 });
	_memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.6f });
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
	}
	_memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void APRandomizer::addMissingSimbolsDisplay(std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB) {
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
	//must be half the size of gridIntersections as its one per intersections
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

void APRandomizer::addPuzzleSimbols(PuzzleData& puzzle, std::vector<int>& decorations, std::vector<int>& decorationsFlags,
	std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& polygons) {

	//does not respect width/height but its hardcoded at a grid of 4/2
	//stored the puzzle simnbols per grid section, from bottom to top from left to right so buttom left is decoration 0
	std::vector<int> gridDecorations(8, 0);
	std::vector<int> gridDecorationsFlags(8, 0);

	int column = 4;
	int secondRowOffset = -column;

	if (puzzle.hasStones)
	{
		if (puzzle.hasColoredStones)
		{
			gridDecorations[column] = Decoration::Stone | Decoration::Color::Yellow;
			gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::Blue;
			column++;
		}
		else {
			gridDecorations[column] = Decoration::Stone | Decoration::Color::Black;
			gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::White;
			column++;
		}
	}
	if (puzzle.hasStars)
	{
		gridDecorations[column] = Decoration::Star | Decoration::Color::Magenta;
		column++;
	}
	if (puzzle.hasTetris) //no idea how
	{
		//gridDecorations[column] = Decoration::Poly | Decoration::Color::Yellow; 
		//column++;
	}
	if (puzzle.hasErasers)
	{
		gridDecorations[column] = Decoration::Eraser;
		column++;
	}
	if (puzzle.hasTriangles)
	{
		gridDecorations[column] = Decoration::Triangle2;
		column++;
	}
	if (puzzle.hasDots) {  //no idea how
		if (puzzle.hasColoredDots) {

		}
		if (puzzle.hasInvisibleDots) {

		}
		else {

		}
	}
	if (puzzle.hasArrows)
	{
		Panel panel;
		panel.render_arrow(column, 1, 2, 0, intersections, intersectionFlags, polygons); //BUG: should either pass decorations or gridDecorations along
		column++;
	}

	decorations.insert(decorations.begin(), gridDecorations.begin(), gridDecorations.end());
	decorationsFlags.insert(decorationsFlags.begin(), gridDecorationsFlags.begin(), gridDecorationsFlags.end());
}