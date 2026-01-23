#include "PuzzleList.h"
#include "Watchdog.h"

void PuzzleList::GenerateTutorialH() {
	g.setLoadingData(L"Tutorial", 21);
	g.resetConfig();
	Special::drawSeedAndDifficulty(TUT_ENTER_1, seed, true, !seedIsRNG, false);
	Special::drawGoodLuckPanel(TUT_ENTER_2);
	g.setConfig(LongestPath);
	g.setConfig(FixDotColor);
	//Mazes
	g.setConfig(FullGaps);
	g.setGridSize(4, 4);
	g.generate(TUT_MAZE_1, Dot_Intersection, 8, Gap, 8);
	g.setGridSize(6, 6);
	g.generate(TUT_MAZE_2, Dot_Intersection, 12, Gap, 18);
	g.setGridSize(8, 8);
	g.generate(TUT_MAZE_3, Dot_Intersection, 21, Gap, 32, Start, 3);
	g.removeConfig(FullGaps);
	//2 starts maze
	g.setGridSize(5, 5);
	g.setConfigOnce(DisableWrite);
	g.generate(TUT_2START, Dot_Intersection, 36, Gap, 12);
	g.set(0, 9, 0); g.set(1, 10, 0); g.set(9, 10, 0); g.set(10, 9, 0);
	g.write(TUT_2START);
	//2 exits maze
	g.setGridSize(5, 5);
	g.setSymbol(Start, 0, 0);
	g.setConfigOnce(DisableWrite);
	g.generate(TUT_2EXIT, Dot_Intersection, 36, Gap, 12);
	g.set(9, 0, 0); g.set(10, 1, 0); g.set(9, 10, 0); g.set(10, 9, 0);
	g.write(TUT_2EXIT);
	g.resetConfig();
	//Exit Gate
	special.modifyGate(TUT_GATE);
	//Secret back area
	g.generate(TUT_OUTER_1, Dot_Intersection, 25, Triangle|Orange, 6);
	g.generate(TUT_OUTER_2, Dot_Intersection, 25, Poly|Yellow|Rotate, 1, Poly|Yellow, 1);
	//Dots Tutorial
	g.setGridSize(4, 4);
	g.setConfigOnce(DisableWrite);
	g.generate(TUT_DOT_1, Start, 2, Exit, 1, Dot_Intersection, 25);
	for (int x = 0; x <= 4; x++) for (int y = 0; y <= 4; y++) if ((x + y) % 2 != g.parity)
		g.set({ x * 2, y * 2 }, Start|Dot);
	g.write(TUT_DOT_1);
	g.setGridSize(5, 5);
	g.setConfigOnce(DisableWrite);
	g.generate(TUT_DOT_2, Start, 2, Exit, 1, Dot_Intersection, 36);
	for (int x = 0; x <= 5; x++) for (int y = 0; y <= 5; y++) if ((x + y) % 2 != g.parity && g.exits.count({ x * 2, y * 2 }) == 0)
		g.set({ x * 2, y * 2 }, Start|Dot);
	g.write(TUT_DOT_2);
	g.generate(TUT_DOT_3, Start, 8, Exit, 1, Dot_Intersection, 36);
	g.generate(TUT_DOT_4, Start, 8, Exit, 1, Dot_Intersection, 36);
	g.setGridSize(6, 6);
	g.generate(TUT_DOT_5, Start, 10, Exit, 1, Dot_Intersection, 49);
	//Stones Tutorial
	g.resetConfig();
	g.setGridSize(5, 5);
	g.generate(TUT_STONE_1, Exit, 1, Stone|Black, 11, Stone|White, 8, Start, 3);
	g.generate(TUT_STONE_2, Exit, 1, Stone|Black, 7, Stone|White, 5, Gap, 10, Start, 3);
	g.setGridSize(4, 4);
	g.generate(TUT_STONE_3, Exit, 1, Dot, 6, Stone|Black, 5, Stone|White, 3, Start, 1);
	g.generate(TUT_STONE_4, Exit, 1, Dot, 6, Stone|Black, 5, Stone|White, 3, Start, 3);
	g.setGridSize(5, 5);
	g.generate(TUT_STONE_5, Exit, 1, Dot, 10, Stone|Black, 7, Stone|White, 5, Start, 3);
	g.generate(TUT_STONE_6, Exit, 1, Dot, 10, Stone|Black, 7, Stone|White, 5, Start, 3);
	g.setGridSize(4, 4);
	g.generate(TUT_STONE_7, Exit, 1, Dot_Intersection, 25, Stone|Black, 3, Stone|White, 3, Start, 1);
	g.setGridSize(5, 5);
	g.generate(TUT_STONE_8, Exit, 1, Dot_Intersection, 36, Stone|Black, 6, Stone|White, 4, Start, 1);
	g.generate(TUT_STONE_9, Exit, 1, Dot_Intersection, 36, Stone|Black, 5, Stone|White, 5, Start, 1);
}

void PuzzleList::GenerateSymmetryH() {
	g.setLoadingData(L"Symmetry", 34);
	g.resetConfig();
	g.setConfig(StartEdgeOnly);
	//Symmetry Mazes
	g.setConfig(MatchDotColor);
	g.setSymmetry(Vertical);
	g.setGridSize(7, 7);
	g.lineThickness = 0.8f;
	g.generate(SYM_MAZE_V1, Gap, 12, Dot_Intersection, 8, Start, 1, Exit, 1);
	g.removeConfigOnce(StartEdgeOnly);
	g.generate(SYM_MAZE_V2, Gap, 12, Dot_Intersection, 8, Start, 3, Exit, 1);
	g.setSymmetry(Rotational);
	g.generate(SYM_MAZE_V3, Gap, 12, Dot_Intersection, 10, Start, 1, Exit, 1);
	g.removeConfigOnce(StartEdgeOnly);
	g.generate(SYM_MAZE_V4, Gap, 12, Dot_Intersection, 10, Start, 1, Exit, 1);
	g.setGridSize(11, 8);
	g.lineThickness = 1;
	g.setSymbol(Start, 0, 8);
	g.setSymbol(Start, 22, 8);
	g.setSymbol(Exit, 0, 6);
	g.setSymbol(Exit, 22, 10);
	g.setObstructions({ { 0, 15 },{ 1, 16 },{ 11, 16 },{ 12, 15 },{ 13, 16 } });
	g.hitPoints = { { 22, 7 },{ 22, 5 },{ 22, 3 },{ 22, 1 },{ 21, 0 },{ 10, 1 } };
	g.setConfigOnce(DisableWrite);
	g.generate(SYM_MAZE_V5, Gap, 30, Dot_Intersection, 1);
	g.set(12, 16, Dot_Intersection);
	g.set(0, 16, Dot_Intersection);
	g.write(SYM_MAZE_V5);
	//Decoy Exits
	g.setGridSize(7, 7);
	g.lineThickness = 0.8f;
	g.setConfig(DisableWrite);
	g.generateMaze(SYM_MAZE_R1, 0, 1);
	special.addDecoyExits(4);
	g.write(SYM_MAZE_R1);
	g.generateMaze(SYM_MAZE_R2, 2, 1);
	special.addDecoyExits(12);
	g.write(SYM_MAZE_R2);
	g.generateMaze(SYM_MAZE_R3, 4, 0);
	special.addDecoyExits(12);
	g.write(SYM_MAZE_R3);
	g.removeConfig(DisableWrite);
	//Weird Symmetry
	g.setSymmetry(ParallelV);
	g.generate(SYM_MAZE_M1, Gap, 12, Dot_Intersection, 10, Start, 2, Exit, 1);
	g.setSymmetry(ParallelHFlip);
	g.generate(SYM_MAZE_M2, Gap, 12, Dot_Intersection, 10, Start, 2, Exit, 1);
	g.setSymmetry(FlipNegXY);
	g.generate(SYM_MAZE_M3, Gap, 10, Dot_Intersection, 8, Start, 2, Exit, 1);
	g.resetConfig();
	//Symmetry Island Door
	g.setGridSize(4, 4);
	g.generate(SYM_DOT_ENTER, Triangle|Cyan, 8);
	//Triangle Symmetry
	g.setConfig(StartEdgeOnly);
	g.setGridSize(6, 6);
	g.setSymmetry(Horizontal);
	g.generate(SYM_DOT_1, Triangle|Cyan, 12, Start, 1, Exit, 1);
	g.setSymmetry(Rotational);
	g.generate(SYM_DOT_2, Triangle|Cyan, 12, Start, 1, Exit, 1);
	g.setGridSize(7, 7);
	g.lineThickness = 0.7f;
	g.setSymmetry(ParallelH);
	g.generate(SYM_DOT_3, Triangle|Cyan, 12, Start, 1, Exit, 1);
	g.setSymmetry(ParallelVFlip);
	g.generate(SYM_DOT_4, Triangle|Cyan, 12, Start, 1, Exit, 1);
	g.setSymmetry(RotateLeft);
	g.setSymbol(Start, 4, 4); g.setSymbol(Start, 10, 4); g.setSymbol(Start, 4, 10); g.setSymbol(Start, 10, 10);
	g.setSymbol(Exit, 4, 0); g.setSymbol(Exit, 14, 4); g.setSymbol(Exit, 0, 10); g.setSymbol(Exit, 10, 14);
	g.generate(SYM_DOT_5, Triangle4|Cyan, 1, Triangle|Cyan, 4);
	//2-color Mechanics
	g.setSymmetry(Rotational);
	g.setConfig(InvisibleSymmetryLine);
	g.generate(SYM_COLOR_1, Dot|Blue, 4, Dot|Yellow, 4, Dot, 7, Start, 1, Exit, 1);
	g.generate(SYM_COLOR_2, Stone|Cyan, 6, Stone|Yellow, 6, Start, 1, Exit, 1);
	g.generate(SYM_COLOR_3, Star|Cyan, 6, Star|Yellow, 6, Start, 1, Exit, 1);
	g.setGridSize(5, 5);
	g.setConfigOnce(RequireCombineShapes);
	g.generate(SYM_COLOR_4, Poly|Yellow, 3, Start, 1, Exit, 1);
	g.setGridSize(7, 7);
	g.setConfig(ResetColors);
	g.generate(SYM_COLOR_5, Triangle|Yellow, 8, Start, 1, Exit, 1);
	g.setSymmetry(FlipXY);
	g.setSymbol(Exit, 0, 8);
	g.setSymbol(Exit, 8, 0);
	g.generate(SYM_COLOR_6, Dot|Blue, 2, Dot|Yellow, 2, Dot, 8, Eraser|White, 1, Start, 1);
	//Secret Symmetry
	g.setConfig(DisableDotIntersection);
	std::vector<PanelID> ids = { SYM_INVISIBLE_1, SYM_INVISIBLE_2, SYM_INVISIBLE_3, SYM_INVISIBLE_4, SYM_INVISIBLE_5, SYM_INVISIBLE_6 };
	std::vector<Symmetry> sym1 = { Vertical, Horizontal, Rotational, ParallelH, ParallelV };
	std::vector<Symmetry> sym2 = { ParallelHFlip, ParallelVFlip, Random::rand() % 2 == 0 ? ParallelV : ParallelH };
	Symmetry lastChoice = NoSymmetry;
	for (int i = 0; i < ids.size(); i++) {
		Symmetry choice = (i < 4 ? Random::popRandom(sym1) : Random::popRandom(sym2));
		if (choice == lastChoice) { i--; continue; }
		lastChoice = choice;
		special.initSecretSymmetryGrid();
		g.setSymmetry(choice);
		if (i < 5) g.generate(ids[i], Dot|Cyan, 2, Dot|Yellow, 2, Triangle|Orange, 4);
		else g.generate(ids[i], Triangle|Orange, 6);
	}
	special.initSecretSymmetryGrid();
	g.setSymmetry(NoSymmetry);
	g.generate(SYM_INVISIBLE_7, Triangle|Orange, 6);
	//Symmetry Island Door 2
	special.generateSymmetryGate(SYM_LASER_ENTER);
	//Dot Reflection Dual Panels (before laser)
	g.resetConfig();
	g.setConfig(MatchDotColor);
	std::set<Symmetry> normalSym = { Vertical, Rotational };
	std::set<Symmetry> weirdSym = { RotateLeft, RotateRight, FlipXY, FlipNegXY };
	g.setSymbol(Start, 0, 8);
	g.setSymbol(Exit, 8, 0);
	special.generateReflectionDotPuzzle(SYM_YELLOW_1, SYM_CYAN_1, { { Dot, 15 } }, Random::popRandom(normalSym), true);
	g.setSymbol(Start, 0, 8);
	g.setSymbol(Exit, 8, 0);
	special.generateReflectionDotPuzzle(SYM_YELLOW_2, SYM_CYAN_2, { { Dot, 15 } }, Random::popRandom(weirdSym), true);
	if (!weirdSym.count(RotateLeft)) weirdSym.erase(RotateRight);
	else if (!weirdSym.count(RotateRight)) weirdSym.erase(RotateLeft);
	else if (!weirdSym.count(FlipXY)) weirdSym.erase(FlipNegXY);
	else if (!weirdSym.count(FlipNegXY)) weirdSym.erase(FlipXY);
	g.setSymbol(Start, 0, 8); g.setSymbol(Start, 8, 8);
	g.setSymbol(Start, 8, 0); g.setSymbol(Start, 0, 0);
	g.setSymbol(Exit, 0, 4); g.setSymbol(Exit, 8, 4);
	g.setConfig(DisableDotIntersection);
	special.generateReflectionDotPuzzle(SYM_YELLOW_3, SYM_CYAN_3, { { Dot, 15 } }, g.pickRandom(weirdSym), true);
}

void PuzzleList::GenerateQuarryH() {
	Memory* memory = Memory::get();
	g.setLoadingData(L"Quarry", 40);
	g.resetConfig();
	//Entry Gates
	g.setGridSize(4, 4);
	g.generate(QUARRY_ENTER_1, Stone|Black, 2, Stone|White, 2, Triangle|Orange, 4);
	g.setConfigOnce(RequireCombineShapes);
	g.generate(QUARRY_ENTER_2, Poly, 2, Triangle|Orange, 2);
	//Mill Entry Door
	g.resetConfig();
	g.generate(MILL_ENTER_L, Stone|White, 3, Stone|Black, 3, Star|White, 3, Star|Black, 2, Star|Orange, 4);
	g.setConfigOnce(PreserveStructure);
	g.blockPos = { { 1, 7 },{ 1, 9 },{ 3, 7 },{ 3, 9 },{ 7, 1 },{ 9, 1 },{ 7, 3 },{ 9, 3 } };
	g.generate(MILL_ENTER_R, Triangle|Black, 7);
	//Triangles
	g.lineThickness = 0.8f;
	g.setGridSize(3, 3);
	g.generate(MILL_DOT_1, Triangle|Black, 4, Eraser|Green, 1);
	g.generate(MILL_DOT_2, Triangle|Black, 8, Eraser|Green, 1);
	g.setGridSize(4, 3);
	g.generate(MILL_DOT_3, Triangle|Black, 8, Eraser|Green, 1);
	g.generate(MILL_DOT_4, Triangle|Black, 8, Eraser|Green, 1);
	g.setGridSize(4, 4);
	g.generate(MILL_DOT_5, Triangle|Black, 12, Eraser|Green, 1);
	g.generate(MILL_DOT_6, Triangle|Black, 15, Eraser|Green, 1);
	g.lineThickness = 1;
	//Stones + Stars
	g.setGridSize(3, 3);
	g.setConfig(AlternateColors);
	g.generate(MILL_STONE_1, Stone|White, 2, Stone|Black, 2, Star|White, 2, Star|Black, 2, Eraser|Green, 1);
	g.generate(MILL_STONE_2, Stone|White, 2, Stone|Black, 1, Star|White, 2, Star|Black, 3, Eraser|Green, 1);
	g.setGridSize(4, 4);
	g.generate(MILL_STONE_3, Stone|White, 6, Stone|Black, 5, Star|White, 2, Star|Black, 2, Eraser|Green, 1);
	g.generate(MILL_STONE_4, Stone|White, 5, Stone|Black, 4, Star|White, 3, Star|Black, 3, Eraser|Green, 1);
	g.generate(MILL_STONE_5, Stone|White, 4, Stone|Black, 4, Star|White, 4, Star|Black, 3, Eraser|Green, 1);
	g.setGridSize(3, 3);
	g.generate(MILL_STONE_6, Stone|White, 2, Stone|Black, 1, Star|White, 1, Star|Black, 1, Stone|Red, 1, Star|Red, 2, Eraser|Green, 1);
	g.setGridSize(4, 4);
	g.generate(MILL_STONE_7, Stone|White, 2, Stone|Black, 3, Stone|Red, 2, Star|White, 4, Star|Black, 2, Star|Red, 2, Eraser|Green, 1);
	g.generate(MILL_STONE_8, Stone|White, 1, Stone|Black, 2, Stone|Red, 2, Star|White, 4, Star|Black, 3, Star|Red, 3, Eraser|Green, 1);
	//Stones + Stars + Triangles
	g.setSymbol(Start, 0, 8);
	g.generate(MILL_FINAL, Stone|White, 2, Stone|Black, 1, Star|White, 1, Star|Black, 2, Triangle|White, 3, Triangle|Black, 3, Eraser|Green, 1);
	//Optional Puzzle
	g.resetConfig();
	g.setConfigOnce(FalseParity);
	g.setGridSize(8, 8);
	g.setSymbol(Exit, 0, 0);
	g.setSymbol(Exit, 16, 0);
	g.setSymbol(Exit, 0, 16);
	g.setSymbol(Exit, 16, 16);
	g.generate(MILL_OPTIONAL, Stone|White, 10, Stone|Black, 10, Dot_Intersection, 81, Start, 7, Eraser|Purple, 1);
	//Boathouse Ramp Activation
	g.resetConfig();
	g.generate(BOATHOUSE_BEGIN_L, Star|White, 3, Star|Black, 8, Star|Magenta, 6, Eraser|White, 1);
	g.setConfigOnce(DisconnectShapes);
	g.generate(BOATHOUSE_BEGIN_R, Poly, 4, Eraser|White, 1);
	//Eraser + Shapes
	g.setConfig(ResetColors);
	g.generate(BOATHOUSE_SHAPE_1, Poly, 3, Poly|Negative, 1, Eraser|White, 1);
	g.generate(BOATHOUSE_SHAPE_2, Poly, 3, Poly|Negative, 1, Eraser|White, 1);
	g.generate(BOATHOUSE_SHAPE_3, Poly, 4, Poly|Negative, 1, Eraser|White, 1);
	g.setGridSize(4, 4);
	g.generate(BOATHOUSE_SHAPE_4, Poly, 3, Poly|Negative, 2, Eraser|White, 1);
	g.setConfigOnce(DisconnectShapes);
	g.generate(BOATHOUSE_SHAPE_5, Poly, 3, Poly|Negative, 1, Eraser|White, 1);
	g.resetConfig();
	//Eraser + Stars
	g.setConfig(WriteColors);
	g.initPanel(BOATHOUSE_STAR_1);
	g.clear();
	g.set(1, 1, Eraser|Green);
	g.set(3, 5, Eraser|Magenta);
	g.set(3, 1, Star|Green);
	g.set(5, 5, Star|Green);
	g.setConfigOnce(DecorationsOnly);
	g.write(BOATHOUSE_STAR_1);
	g.initPanel(BOATHOUSE_STAR_2);
	g.clear();
	g.set(3, 1, Eraser|Magenta);
	g.set(1, 3, Star|Magenta);
	g.set(3, 3, Star|Magenta);
	g.set(5, 3, Star|Magenta);
	g.set(3, 5, Eraser|Magenta);
	g.setConfigOnce(DecorationsOnly);
	g.write(BOATHOUSE_STAR_2);
	g.generate(BOATHOUSE_STAR_3, Star|Green, 7, Star|Orange, 5, Eraser|Green, 1, Eraser|Orange, 1);
	g.setGridSize(5, 5);
	g.generate(BOATHOUSE_STAR_4, Star|Magenta, 8, Star|Orange, 6, Star|Green, 4, Eraser|Magenta, 1, Eraser|Orange, 1);
	g.generate(BOATHOUSE_STAR_5, Star|Magenta, 7, Star|Orange, 6, Star|Green, 5, Eraser|Magenta, 1, Eraser|Green, 1);
	//Eraser + Stars + Shapes
	g.removeConfig(WriteColors);
	g.setConfig(WriteColors);
	g.setGridSize(4, 4);
	g.generate(BOATHOUSE_STAR_6, Poly|Green, 4, Eraser|White, 2);
	g.setConfigOnce(DisconnectShapes);
	g.generate(BOATHOUSE_STAR_7, Poly|Green, 4, Eraser|White, 2);
	g.setSymbol(Start, 0, 8);
	g.generate(BOATHOUSE_STAR_8, Poly|Green, 3, Star|Green, 3, Eraser|White, 2); //TODO: Change
	g.generate(BOATHOUSE_STAR_9, Poly|Green, 2, Poly|Magenta, 1, Star|Green, 2, Star|Magenta, 2, Eraser|White, 2);
	//All together
	g.removeConfig(WriteColors);
	g.setConfig(ResetColors);
	g.setGridSize(6, 3);
	g.lineThickness = 0.6f;
	g.generate(BOATHOUSE_FINAL_1, Star|Orange, 4, Poly|Orange, 2, Poly|Negative|Magenta, 2, Eraser|White, 2);
	g.generate(BOATHOUSE_FINAL_2, Star|Magenta, 4, Poly|Orange, 2, Poly|Negative|Magenta, 2, Eraser|White, 2);
	g.generate(BOATHOUSE_FINAL_3, Star|Magenta, 2, Star|Orange, 3, Poly|Orange, 2, Poly|Negative|Magenta, 2, Eraser|White, 2);
	//Laser Puzzle
	g.resetConfig();
	g.setConfigOnce(DecorationsOnly);
	g.initPanel(QUARRY_LASER);
	g.clear();
	g.set(1, 5, Eraser|White);
	g.set(5, 5, Eraser|Green);
	g.set(1, 3, Eraser|White);
	g.set(3, 1, Triangle|0x10000|White);
	g.set(5, 3, Star|Green);
	g.write(QUARRY_LASER);
}

void PuzzleList::GenerateBunkerH() {
	//I would randomize this, if I could get the panels to actually render the symbols.
	//Unfortunately, the path is rendered to a 3D model that doesn't have any geometry between the grid lines.
	//Somehow, I would either have to change the model, or make the puzzle render to the background texture instead.
}

void PuzzleList::GenerateSwampH() {
	g.setLoadingData(L"Swamp", 55);
	g.resetConfig();
	g.setGridSize(3, 3);
	g.setConfigOnce(SplitShapes);
	g.generate(SWAMP_ENTER, Poly|Rotate, 2, Stone|Black, 1, Stone|White, 1, Triangle|Orange, 1);
	//First row
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_TUT_A1, Poly, 2, Stone|Black, 3, Stone|White, 3);
	g.generate(SWAMP_TUT_A2, Poly, 1, Poly|Rotate, 1, Stone|Black, 3, Stone|White, 3);
	g.generate(SWAMP_TUT_A3, Poly|Rotate, 2, Stone|Black, 3, Stone|White, 3);
	g.generate(SWAMP_TUT_A4, Poly, 2, Poly|Rotate, 1, Stone|Black, 2, Stone|White, 2);
	g.generate(SWAMP_TUT_A5, Poly, 1, Poly|Rotate, 2, Stone|Black, 2, Stone|White, 2);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.generate(SWAMP_TUT_A6, Poly, 2, Poly|Rotate, 1, Stone|Black, 5, Stone|White, 3);
	//Second Row
	g.resetConfig();
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_TUT_B1, Poly, 2, Triangle|Orange, 3);
	g.generate(SWAMP_TUT_B2, Poly, 1, Poly|Rotate, 1, Triangle|Orange, 3);
	g.generate(SWAMP_TUT_B3, Poly|Rotate, 2, Triangle|Orange, 3);
	g.generate(SWAMP_TUT_B4, Poly, 2, Poly|Rotate, 1, Triangle|Orange, 2);
	g.generate(SWAMP_TUT_B5, Poly, 1, Poly|Rotate, 2, Triangle|Orange, 2);
	g.generate(SWAMP_TUT_B6, Poly|Rotate, 2, Triangle|Orange, 2, Stone|Black, 2, Stone|White, 2);
	g.generate(SWAMP_TUT_B7, Poly, 2, Poly|Rotate, 1, Triangle|Orange, 2, Stone|Black, 2, Stone|White, 2);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_TUT_B8, Poly|Rotate, 2, Triangle|Orange, 3, Stone|Black, 3, Stone|White, 3);
	//Modify bridge
	g.resetConfig();
	g.initPanel(SWAMP_SLIDING_BRIDGE);
	g.set(5, 1, Stone|White);
	g.set(11, 1, Stone|White);
	g.set(3, 5, Stone|Black);
	g.set(9, 5, Stone|Black);
	g.setConfig(DecorationsOnly);
	g.write(SWAMP_SLIDING_BRIDGE);
	g.write(SWAMP_SLIDING_BRIDGE_2);
	g.resetConfig();
	//Turn off bridge control until all previous puzzles are solved
	special.setTargetAndDeactivate(SWAMP_TUT_B8, SWAMP_SLIDING_BRIDGE);
	//Red Panels
	g.resetConfig();
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.setSymbol(Start, 4, 4);
	g.generate(SWAMP_RED_1, Poly|Rotate, 2, Gap, 3);
	g.setSymbol(Start, 4, 4);
	g.generate(SWAMP_RED_2, Poly|Rotate, 3, Gap, 3);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.setSymbol(Start, 4, 6);
	g.generate(SWAMP_RED_3, Poly|Rotate, 3, Gap, 3);
	g.setSymbol(Start, 4, 6);
	g.generate(SWAMP_RED_4, Poly|Rotate, 4, Gap, 3);
	g.setConfigOnce(LongPath);
	g.setSymbol(Start, 4, 6);
	g.generate(SWAMP_RED_SHORTCUT_1, Poly|Rotate, 3);
	g.placeGaps(6);
	g.write(SWAMP_RED_SHORTCUT_2);
	//Disconnected Shapes
	g.resetConfig();
	g.setConfig(DisconnectShapes);
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_DISJOINT_1, Poly|Rotate, 2);
	g.generate(SWAMP_DISJOINT_2, Poly|Rotate, 3);
	g.setSymbol(Start, 4, 4);
	g.generate(SWAMP_DISJOINT_3, Poly|Rotate, 3);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.setSymbol(Start, 4, 6);
	g.generate(SWAMP_DISJOINT_4, Poly|Rotate, 3);
	//Modify rotating bridge
	g.initPanel(SWAMP_ROTATING_BRIDGE);
	g.set(1, 1, Star|Black);
	g.set(9, 3, Triangle3|Black);
	g.set(9, 7, Triangle3|Black);
	g.set(1, 9, Stone|Black);
	g.setConfigOnce(DecorationsOnly);
	g.write(SWAMP_ROTATING_BRIDGE);
	//Full Dot Shapes
	g.resetConfig();
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_ROTATE_A1, Dot_Intersection, 25, Poly|Rotate, 2, Start, 1);
	g.generate(SWAMP_ROTATE_A2, Dot_Intersection, 25, Poly|Rotate, 2, Start, 1);
	g.generate(SWAMP_ROTATE_A3, Dot_Intersection, 25, Poly, 2, Poly|Rotate, 1, Start, 1);
	g.generate(SWAMP_ROTATE_A4, Dot_Intersection, 25, Poly, 1, Poly|Rotate, 2, Start, 1);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.generate(SWAMP_ROTATE_B1, Dot_Intersection, 36, Poly, 3, Start, 1);
	g.generate(SWAMP_ROTATE_B2, Dot_Intersection, 36, Poly, 2, Poly|Rotate, 1, Start, 1);
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_ROTATE_B3, Dot_Intersection, 36, Poly, 3, Start, 1);
	g.setConfigOnce(DisconnectShapes);
	g.generate(SWAMP_ROTATE_B4, Dot_Intersection, 36, Poly, 3, Start, 1);
	//Optional Tetris
	g.resetConfig();
	g.generate(SWAMP_PURPLE, Dot_Intersection, 42, Poly, 2, Poly|Rotate, 1, Triangle|Orange, 3, Stone|Black, 2, Stone|White, 2);
	//Negative Shapes 1
	g.resetConfig();
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.setConfigOnce(RequireCancelShapes);
	g.generate(SWAMP_NEG_A1, Poly, 3, Poly|Negative, 3);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.generate(SWAMP_NEG_A2, Poly, 4, Poly|Negative, 1);
	g.setConfigOnce(RequireCancelShapes);
	g.generate(SWAMP_NEG_A3, Poly, 4, Poly|Negative, 3);
	g.setConfig(BigShapes);
	g.generate(SWAMP_NEG_A4, Poly, 3, Poly|Negative, 1);
	g.setConfigOnce(RequireCancelShapes);
	g.generate(SWAMP_NEG_A5, Poly, 3, Poly|Negative, 3);
	g.removeConfig(BigShapes);
	//Negative Shapes 2
	g.generate(SWAMP_NEG_B1, Poly, 3, Poly|Negative, 1, Stone|Black, 4, Stone|White, 3);
	g.generate(SWAMP_NEG_B2, Poly, 3, Poly|Negative, 2, Triangle|Orange, 5);
	g.generate(SWAMP_NEG_B3, Poly, 3, Poly|Negative, 1, Triangle|Orange, 3, Stone|Black, 2, Stone|White, 2);
	g.generate(SWAMP_NEG_B4, Poly, 3, Poly|Negative, 2, Triangle|Orange, 3, Stone|Black, 2, Stone|White, 2);
	g.setConfigOnce(DisconnectShapes);
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_NEG_B5, Poly, 2, Poly|Negative, 1, Triangle|Orange, 3, Stone|Black, 2, Stone|White, 2);
	//Negative Shapes 3
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.setConfig(BigShapes);
	g.generate(SWAMP_NEG_C1, Dot_Intersection, 36, Poly, 2, Poly|Negative, 1, Start, 1);
	g.generate(SWAMP_NEG_C2, Dot_Intersection, 36, Poly, 2, Poly|Negative, 2, Start, 1);
	g.removeConfig(BigShapes);
	g.generate(SWAMP_NEG_C3, Dot_Intersection, 36, Poly, 3, Poly|Negative, 1, Start, 1);
	g.generate(SWAMP_NEG_C4, Dot_Intersection, 36, Poly, 3, Poly|Negative, 2, Start, 1);
	//Modify Swamp Island
	g.resetConfig();
	g.initPanel(SWAMP_ISLAND_1);
	g.set(5, 3, 0);
	g.set(3, 1, Poly|Rotate|Negative|0x00130000);
	g.setConfig(DecorationsOnly);
	g.write(SWAMP_ISLAND_1);
	g.write(SWAMP_ISLAND_2);
	//Exit Shortcut
	g.resetConfig();
	g.setGridSize(4, 4);
	g.lineThickness = 0.7f;
	g.generate(SWAMP_EXIT_1, Poly|Orange, 3, Poly|Negative|Blue, 3, Star|Orange, 3);
	g.generate(SWAMP_EXIT_2, Poly|Blue, 3, Poly|Negative|Orange, 3, Star|Orange, 3);
}

void PuzzleList::GenerateTreehouseH() {
	g.setLoadingData(L"Treehouse", 58);
	g.resetConfig();
	g.setConfig(WriteColors);
	g.initPanel(TREEHOUSE_ENTER_1);
	g.set(1, 1, Triangle1|Orange);
	g.set(1, 3, Star|Orange);
	g.write(TREEHOUSE_ENTER_1);
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_ENTER_2, Star|Orange, 3, Triangle|Orange, 2);
	g.setConfig(TreehouseLayout);
	//Yellow Bridge
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_YELLOW_1, Star|Orange, 1, Triangle|Orange, 7);
	g.generate(TREEHOUSE_YELLOW_2, Star|Orange, 2, Triangle|Orange, 6);
	g.generate(TREEHOUSE_YELLOW_3, Star|Orange, 2, Triangle|Orange, 6);
	g.generate(TREEHOUSE_YELLOW_4, Star|Orange, 3, Triangle|Orange, 5);
	g.generate(TREEHOUSE_YELLOW_5, Star|Orange, 3, Triangle|Orange, 5);
	g.generate(TREEHOUSE_YELLOW_6, Star|Orange, 4, Triangle|Orange, 4);
	g.setGridSize(5, 5);
	g.lineThickness = 0.8f;
	g.generate(TREEHOUSE_YELLOW_7, Star|Orange, 5, Triangle|Orange, 6);
	g.generate(TREEHOUSE_YELLOW_8, Star|Orange, 4, Triangle|Orange, 6);
	g.generate(TREEHOUSE_YELLOW_9, Star|Orange, 6, Triangle|Orange, 5);
	g.removeConfigOnce(TreehouseLayout);
	g.generate(TREEHOUSE_YELLOW_DOOR, Star|Orange, 5, Triangle|Orange, 3, Stone|Orange, 3);
	//Pink Bridge 1
	g.setGridSize(4, 4);
	g.lineThickness = 1;
	g.generate(TREEHOUSE_PURPLE_A1, Star|Magenta, 4, Dot_Intersection, 25);
	g.setGridSize(5, 5);
	g.lineThickness = 0.8f;
	g.generate(TREEHOUSE_PURPLE_A2, Star|Magenta, 4, Dot_Intersection, 36);
	g.generate(TREEHOUSE_PURPLE_A3, Star|Magenta, 6, Dot_Intersection, 36);
	g.generate(TREEHOUSE_PURPLE_A4, Star|Magenta, 4, Star|Orange, 4, Dot_Intersection, 36);
	g.generate(TREEHOUSE_PURPLE_A5, Star|Magenta, 4, Star|Orange, 6, Dot_Intersection, 36);
	//Pink Bridge 2
	g.setGridSize(4, 4);
	g.lineThickness = 1;
	g.removeConfig(WriteColors);
	g.setConfig(WriteColors);
	g.generate(TREEHOUSE_PURPLE_B1, Triangle|Magenta, 4, Stone|Black, 2, Stone|White, 2, Star|Black, 1, Star|White, 1);
	g.generate(TREEHOUSE_PURPLE_B2, Triangle|Magenta, 4, Stone|Black, 1, Stone|White, 1, Star|Black, 2, Star|White, 2);
	g.generate(TREEHOUSE_PURPLE_B3, Triangle|Magenta, 4, Stone|Black, 2, Stone|White, 2, Star|Magenta, 2);
	g.generate(TREEHOUSE_PURPLE_B4, Triangle|Magenta, 4, Stone|Black, 1, Star|Black, 1, Stone|Magenta, 2, Star|White, 2);
	g.removeConfig(WriteColors);
	g.setConfig(WriteColors);
	g.generate(TREEHOUSE_PURPLE_B5, Triangle|Orange, 2, Triangle|Magenta, 2, Star|Magenta, 2, Star|Green, 2, Stone|Orange, 2, Stone|Green, 2);
	g.setGridSize(5, 4);
	g.lineThickness = 0.8f;
	g.generate(TREEHOUSE_PURPLE_B6, Triangle|Orange, 2, Triangle|Magenta, 2, Star|Magenta, 3, Star|Green, 4, Stone|Orange, 2, Stone|Green, 2);
	g.generate(TREEHOUSE_PURPLE_B7, Triangle|Green, 3, Triangle|Magenta, 2, Star|Magenta, 3, Star|Orange, 3, Stone|Orange, 2, Stone|Green, 2);
	//Orange Bridge 1	
	g.removeConfig(WriteColors);
	g.setConfig(WriteColors);
	g.setGridSize(4, 4);
	g.lineThickness = 1;
	g.generate(TREEHOUSE_ORANGE_L1, Star|Black, 1, Star|White, 2, Stone|Black, 2, Stone|White, 1, Poly|Rotate|Orange, 1, Poly|Orange, 1);
	g.generate(TREEHOUSE_ORANGE_L2, Star|Black, 1, Star|White, 2, Stone|Black, 2, Stone|White, 1, Poly|Rotate|Black, 1, Poly|Black, 1);
	g.generate(TREEHOUSE_ORANGE_L3, Star|Black, 1, Star|White, 2, Stone|Black, 2, Stone|White, 1, Poly|Rotate|White, 1, Poly|White, 1);
	g.generate(TREEHOUSE_ORANGE_L4, Star|Black, 1, Star|White, 2, Stone|Black, 2, Stone|White, 1, Poly|Rotate|Black, 1, Poly|White, 1);
	g.removeConfig(WriteColors);
	g.setConfig(AlternateColors); //Black -> Green, White -> Pink, Purple -> White, Green -> Black
	g.setGridSize(5, 5);
	g.lineThickness = 0.8f;
	g.generate(TREEHOUSE_ORANGE_L5, Star|Black, 2, Star|White, 4, Stone|Black, 2, Stone|White, 2, Poly|Rotate|Black, 1, Poly|Black, 1);
	g.generate(TREEHOUSE_ORANGE_L6, Star|Black, 3, Star|White, 3, Stone|Black, 2, Stone|White, 2, Poly|Black, 1, Poly|Rotate|White, 1);
	g.setConfig(BigShapes);
	g.generate(TREEHOUSE_ORANGE_L7, Star|Black, 4, Star|White, 3, Stone|Black, 2, Stone|White, 2, Poly|Rotate|White, 1, Poly|White, 1);
	g.generate(TREEHOUSE_ORANGE_L8, Star|Black, 3, Star|White, 4, Stone|Black, 2, Stone|White, 2, Poly|Rotate|Black, 1, Poly|White, 1);
	g.removeConfig(BigShapes);
	g.setObstructions({ { 1, 2 },{ 1, 4 },{ 9, 2 },{ 9, 4 },{ 2, 1 },{ 4, 1 },{ 6, 1 },{ 8, 1 } });
	g.generate(TREEHOUSE_ORANGE_L9, Star|Black, 3, Star|White, 3, Stone|White, 2, Stone|Purple, 2, Poly|Rotate|Purple, 1, Poly|Black, 1);
	g.generate(TREEHOUSE_ORANGE_L10, Star|Purple, 2, Star|Black, 5, Stone|Black, 2, Stone|Purple, 1, Poly|Purple, 1, Poly|Rotate|White, 1);
	g.setConfig(BigShapes);
	g.generate(TREEHOUSE_ORANGE_L11, Star|Purple, 4, Star|White, 4, Stone|Black, 2, Stone|Purple, 1, Poly|Rotate|Purple, 1, Poly|Black, 1);
	g.removeConfig(BigShapes);
	g.removeConfig(AlternateColors);
	g.generate(TREEHOUSE_ORANGE_L12, Star|Black, 2, Star|White, 3, Stone|Black, 2, Stone|White, 1, Triangle|Black, 2, Triangle|White, 1, Poly|Rotate|Black, 1, Poly|White, 1);
	special.clearTarget(TREEHOUSE_ORANGE_L12);
	g.generate(TREEHOUSE_ORANGE_L13, Star|Black, 3, Star|White, 3, Stone|Black, 2, Stone|White, 1, Triangle|Black, 1, Triangle|White, 2, Poly|Rotate|Black, 1, Poly|White, 1);
	g.generate(TREEHOUSE_ORANGE_L14, Star|Black, 4, Star|White, 2, Stone|Black, 1, Stone|White, 2, Triangle|Black, 1, Triangle|White, 1, Poly|Black, 1, Poly|Rotate|White, 1);
	g.generate(TREEHOUSE_ORANGE_L15, Star|Black, 3, Star|White, 3, Stone|Black, 1, Stone|White, 1, Triangle|Black, 1, Triangle|White, 1, Poly|Rotate|Black, 1, Poly|Rotate|White, 1);
	(new TreehouseWatchdog(TREEHOUSE_ORANGE_L13))->start();
	//Orange Bridge 2
	g.setConfig(WriteColors);
	g.lineThickness = 1;
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_ORANGE_R1, Star|Orange, 2, Star|Magenta, 1, Triangle|Orange, 1, Triangle|Magenta, 1);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_ORANGE_R2, Star|Orange, 3, Star|Magenta, 2, Triangle|Orange, 2, Triangle|Magenta, 2);
	g.generate(TREEHOUSE_ORANGE_R3, Star|Orange, 1, Star|Magenta, 2, Triangle|Orange, 3, Triangle|Magenta, 2);
	special.generatePivotPanel(TREEHOUSE_ORANGE_R4, { 4, 4 }, { { Triangle|Orange, 3 },{ Triangle|Magenta, 2 } });
	g.setGridSize(5, 4);
	g.lineThickness = 0.8f;
	g.generate(TREEHOUSE_ORANGE_R5, Star|Orange, 3, Star|Magenta, 3, Triangle|Orange, 3, Triangle|Magenta, 3);
	g.generate(TREEHOUSE_ORANGE_R6, Star|Orange, 4, Star|Magenta, 4, Triangle|Orange, 2, Triangle|Magenta, 2);
	g.generate(TREEHOUSE_ORANGE_R7, Star|Orange, 3, Star|Magenta, 3, Star|Green, 6, Triangle|Orange, 2, Triangle|Magenta, 2);
	g.generate(TREEHOUSE_ORANGE_R8, Star|Orange, 4, Star|Magenta, 4, Star|Green, 6, Triangle|Orange, 1, Triangle|Magenta, 1);
	g.generate(TREEHOUSE_ORANGE_R9, Star|Orange, 3, Star|Magenta, 3, Star|Green, 2, Triangle|Orange, 2, Triangle|Magenta, 2, Triangle|Green, 2);
	special.generatePivotPanel(TREEHOUSE_ORANGE_R10, { 4, 4 }, { { Triangle|Orange, 2 },{ Triangle|Magenta, 2 },{ Triangle|Green, 1 } });
	g.generate(TREEHOUSE_ORANGE_R11, Star|Orange, 3, Star|Magenta, 3, Star|White, 3, Star|Green, 3, Triangle|Orange, 1, Triangle|Magenta, 1, Triangle|White, 1, Triangle|Green, 1);
	g.generate(TREEHOUSE_ORANGE_R12, Star|Orange, 2, Star|Magenta, 2, Star|White, 2, Star|Green, 2, Star|Black, 2,
		Triangle|Orange, 1, Triangle|Magenta, 1, Triangle|White, 1, Triangle|Green, 1, Triangle|Black, 1);
	//Green Bridge
	g.removeConfig(WriteColors);
	g.setConfig(AlternateColors);
	g.setGridSize(5, 5);
	g.lineThickness = 0.7f;
	g.generate(TREEHOUSE_GREEN_1, Poly|White, 3, Poly|Negative|Green, 1, Star|White, 5, Star|Green, 3);
	g.generate(TREEHOUSE_GREEN_2, Poly|Green, 3, Poly|Negative|White, 2, Star|White, 5, Star|Green, 3);
	g.generate(TREEHOUSE_GREEN_3, Poly|White, 2, Poly|Negative|Green, 1, Poly|Green, 1, Poly|Negative|White, 1, Star|White, 5, Star|Green, 4);
	g.setObstructions({ { { 1, 2 },{ 1, 4 },{ 9, 2 },{ 9, 4 },{ 2, 1 },{ 4, 1 },{ 6, 1 },{ 8, 1 } },
		{ { 1, 2 },{ 1, 4 },{ 1, 6 },{ 0, 7 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 7 },{ 4, 1 },{ 6, 1 },{ 8, 1 } },
		{ { 1, 2 },{ 1, 4 },{ 1, 6 },{ 0, 7 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 7 },{ 2, 1 },{ 4, 1 },{ 6, 1 } } });
	g.generate(TREEHOUSE_GREEN_4, Poly|Green, 1, Poly|Black, 1, Poly|Negative|White, 3, Star|White, 3, Star|Green, 2, Star|Black, 3);
	g.generate(TREEHOUSE_GREEN_5, Poly|White, 1, Poly|Black, 2, Poly|Negative|White, 2, Poly|Negative|Green, 2, Star|White, 3, Star|Green, 3, Star|Black, 3);
	g.generate(TREEHOUSE_GREEN_6, Poly|Black, 2, Poly|Negative|Green, 2, Star|Black, 3, Star|Green, 3, Triangle|Black, 2, Triangle|Green, 2);
	g.generate(TREEHOUSE_GREEN_7, { { Poly|White, 1 },{ Poly|Green, 1 },{ Poly|Cyan, 1 }, { Poly|Negative|Green, 1 },{ Poly|Negative|Black, 2 },
		{ Star|Cyan, 2 },{ Star|Black, 1 },{ Star|White, 1 },{ Star|Green, 2 }, { Triangle|Black, 2 },{ Triangle|White, 1 } ,{ Triangle|Cyan, 2 } });
}

void PuzzleList::GenerateTownH() {
	g.setLoadingData(L"Town", 21);
	g.resetConfig();
	//Full Dots + Triangles
	g.setGridSize(5, 5);
	g.lineThickness = 0.9f;
	g.generate(TOWN_DOT_1, Dot_Intersection, 36, Triangle1|Orange, 4, Start, 1);
	g.generate(TOWN_DOT_2, Dot_Intersection, 36, Triangle3|Orange, 6, Start, 1);
	g.generate(TOWN_DOT_3, Dot_Intersection, 36, Triangle2|Orange, 10, Start, 1);
	g.generate(TOWN_DOT_4, Dot_Intersection, 36, Triangle|Orange, 6, Start, 1);
	g.generate(TOWN_DOT_5, Dot_Intersection, 36, Triangle|Orange, 8, Start, 1);
	g.setSymbol(Start, 0, 10);
	g.setConfigOnce(FalseParity);
	g.generate(TOWN_DOT_FINAL, Dot_Intersection, 36, Triangle|Orange, 7, Eraser|White, 1);
	//Blue Symmetry
	g.setConfig(RequireCombineShapes);
	g.setGridSize(7, 7);
	g.lineThickness = 0.6f;
	g.setSymmetry(Vertical);
	g.setSymbol(Start, 0, 14); g.setSymbol(Start, 14, 14);
	g.setSymbol(Exit, 0, 0); g.setSymbol(Exit, 14, 0);
	g.generate(TOWN_BLUE_1, Poly|Orange, 5);
	g.setSymmetry(ParallelV);
	g.setSymbol(Start, 0, 14); g.setSymbol(Start, 8, 14);
	g.setSymbol(Exit, 6, 0); g.setSymbol(Exit, 14, 0);
	g.generate(TOWN_BLUE_2, Poly|Orange, 5);
	g.setSymmetry(ParallelHFlip);
	g.setSymbol(Start, 0, 14); g.setSymbol(Start, 14, 6);
	g.setSymbol(Exit, 0, 0); g.setSymbol(Exit, 14, 8);
	g.generate(TOWN_BLUE_3, Poly|Orange, 5);
	g.setSymmetry(ParallelVFlip);
	g.setSymbol(Start, 0, 14); g.setSymbol(Start, 8, 0);
	g.setSymbol(Exit, 6, 0); g.setSymbol(Exit, 14, 14);
	g.generate(TOWN_BLUE_4, Poly|Orange, 5);
	g.setSymmetry(Rotational);
	g.setSymbol(Start, 0, 14); g.setSymbol(Start, 14, 0);
	g.setSymbol(Exit, 0, 0); g.setSymbol(Exit, 14, 14);
	g.generate(TOWN_BLUE_5, Poly|Orange, 5);
	g.resetConfig();
	//Glass Door
	g.setConfigOnce(SmallShapes);
	g.generate(TOWN_GLASS, Poly|Rotate|Black, 3, Poly|Rotate|White, 2, Star|Black, 4, Star|White, 5);
	//Church Star Door
	special.generateColorFilterPuzzle(TOWN_STAR, { 5, 5 }, { { Star|1, 6 }, { Star|2, 6 }, { Star|3, 6 }, { Star|4, 6 } }, { 1, 1, 0, 0 }, false);
	//Mess with targets
	special.copyTarget(TOWN_RGB_R, TOWN_STAR); special.copyTarget(TOWN_STAR, TOWN_GLASS);
	special.setTargetAndDeactivate(TOWN_GLASS, TOWN_STAR); special.setTargetAndDeactivate(TOWN_RGB_L, TOWN_RGB_R);
	special.setPower(TOWN_LATTICE, false); (new TownDoorWatchdog())->start();
	//Soundproof Room
	std::vector<int> allPitches = { DOT_SMALL, DOT_SMALL, DOT_MEDIUM, DOT_MEDIUM, DOT_LARGE, DOT_LARGE };
	std::vector<int> pitches;
	for (int i = 0; i < 4; i++) pitches.push_back(Random::popRandom(allPitches));
	special.generateSoundDotPuzzle(TOWN_SOUND_DOT, { 4, 4 }, pitches, false);
	g.resetConfig();
	//3-color Room
	//Modify switch to remove green
	g.initPanel(TOWN_RGB_CONTROL);
	g.set(7, 5, Triangle3|Orange);
	g.set(9, 5, Stone|Red);
	g.set(1, 5, Stone|Blue);
	g.set(5, 1, Stone|Green);
	g.setConfigOnce(DecorationsOnly);
	g.setConfigOnce(WriteColors);
	g.setConfigOnce(DisableFlash);
	g.write(TOWN_RGB_CONTROL);
	special.generateRGBStonePuzzleH(TOWN_RGB_L);
	special.generateRGBDotPuzzleH(TOWN_RGB_R);
	//Orange Crate
	g.setGridSize(5, 5);
	g.generate(TOWN_CRATE, Poly|Yellow, 7, Stone|White, 3, Stone|Black, 3, Triangle|Orange, 6);
	//Windmill Puzzles
	g.resetConfig();
	g.setConfigOnce(PreserveStructure);
	g.generate(THEATER_ENTER, Stone|Black, 3, Stone|White, 2, Triangle|Orange, 5, Eraser|White, 1);
	g.generate(THEATER_EXIT_L, Stone|White, 3, Stone|Black, 3, Star|White, 3, Star|Black, 3, Eraser|White, 2);
	g.setConfigOnce(WriteColors);
	g.generate(THEATER_EXIT_R, Poly|Yellow, 3, Triangle|Orange, 5, Eraser|White, 1);
}

void PuzzleList::GenerateVaultsH() {
	g.setLoadingData(L"Vaults", 5);
	g.resetConfig();
	g.setConfig(DisableFlash);
	//Tutorial Vault
	g.setGridSize(8, 8);
	g.setSymbol(Exit, 0, 0);
	g.setSymbol(Exit, 16, 0);
	g.setSymbol(Exit, 0, 16);
	g.setSymbol(Exit, 16, 16);
	g.generate(TUT_VAULT, Stone|White, 10, Stone|Black, 10, Dot_Intersection, 81, Start, 8);
	//Desert Vault
	g.resetConfig();
	g.setConfig(DisableFlash);
	g.generate(DESERT_VAULT, { { Dot_Intersection, 49 },{ Poly|Orange, 1 },{ Poly|Blue, 1 }, { Poly|Negative|Blue, 2 },{ Poly|Negative|Orange, 1 },
		{ Star|Orange, 4 },{ Star|Blue, 3 }, { Triangle|Orange, 2 },{ Triangle|Blue, 1 }, { Stone|Orange, 1 },{ Stone|Blue, 2 }, { Eraser|White, 1 } });
	//Symmetry Vault
	g.resetConfig();
	g.setConfig(DisableFlash);
	g.setGridSize(8, 8);
	g.setSymmetry(FlipXY);
	g.setSymbol(Start, 0, 16); g.setSymbol(Start, 16, 0);
	g.setSymbol(Exit, 8, 0); g.setSymbol(Exit, 8, 16);
	g.setSymbol(Exit, 0, 8); g.setSymbol(Exit, 16, 8);
	g.generate(MOUNTAIN_VAULT, Triangle|Cyan, 3, Triangle|Yellow, 2, Star|Cyan, 3, Star|Yellow, 3, Stone|Cyan, 2, Stone|Yellow, 3);
	//Shipwreck Vault
	g.resetConfig();
	special.generateSoundDotReflectionPuzzle(SHIPWRECK_VAULT, { 6, 6 }, { DOT_MEDIUM, DOT_LARGE, DOT_MEDIUM, DOT_SMALL }, { DOT_MEDIUM, DOT_LARGE, DOT_MEDIUM, DOT_SMALL }, 2, true);
	//Jungle Vault
	g.resetConfig();
	special.generateJungleVault(JUNGLE_VAULT);
}

void PuzzleList::GenerateTrianglePanelsH() {
	g.setLoadingData(L"Arrows", 14);
	g.resetConfig();
	g.backgroundColor = { 0.5f, 0.5f, 0.5f, 1 };
	g.activeColor = { 0.85f, 0.7f, 1, 1 };
	g.successColor = { 0.6f, 0, 1, 1 };
	g.setConfig(WriteColors);
	special.createArrowPuzzle(TUT_DISCARD, 5, 3, SymbolID::ARROW1N, { { 0, 3}, {6, 1} });
	special.createArrowPuzzle(SYM_DISCARD, 1, 1, SymbolID::ARROW2N, { { 1, 4 },{ 6, 3 },{ 5, 0 } });
	special.createArrowPuzzle(DESERT_DISCARD, 1, 3, SymbolID::ARROW1SE, { { 0, 5 },{ 6, 3 } });
	special.createArrowPuzzle(MILL_DISCARD, 5, 5, SymbolID::ARROW1SW, { { 1, 4}, {1, 6}, { 3, 0 },{ 3, 2 } });
	special.createArrowPuzzle(TOWN_ROOF_DISCARD, 5, 1, SymbolID::ARROW1NW, { { 0, 1}, {3, 6} });
	g.setSymbol(Start, 0, 0);
	special.createArrowPuzzle(THEATER_DISCARD, 5, 5, SymbolID::ARROW1W, { { 4, 1 },{ 1, 2 },{3, 6}, {6, 1} });
	special.createArrowPuzzle(TOWN_CRATE_DISCARD, 1, 1, SymbolID::ARROW2NE, { { 4, 5 },{ 6, 1 } });
	g.setGridSize(2, 1);
	special.createArrowPuzzle(JUNGLE_DISCARD, 1, 1, SymbolID::ARROW2E, { });
	g.setGridSize(3, 3);
	special.createArrowPuzzle(KEEP_DISCARD, 1, 5, SymbolID::ARROW2SE, { { 3, 0 },{ 0, 5 } });
	special.createArrowPuzzle(SHIPWRECK_DISCARD, 5, 1, SymbolID::ARROW2N, { { 2, 1 },{ 4, 1 },{3, 4} });
	special.createArrowPuzzle(TREEHOUSE_GREEN_DISCARD, 1, 3, SymbolID::ARROW2E, { { 3, 4 },{ 5, 6 } });
	g.setSymbol(Start, 6, 6);
	special.createArrowPuzzle(TREEHOUSE_ORANGE_DISCARD, 1, 3, SymbolID::ARROW1E, { { 4, 1 },{ 6, 1 },{ 3, 2 },{ 4, 5 },{ 5, 4 } });
	special.createArrowPuzzle(MOUNTAIN_OUTSIDE_DISCARD, 1, 5, SymbolID::ARROW2SE, { { 4, 5 },{ 2, 3 },{ 3, 2 },{ 5, 2 },{ 5, 4 } });
	g.setGridSize(2, 1);
	special.createArrowPuzzle(MOUNTAIN_INSIDE_DISCARD, 1, 1, SymbolID::ARROW1E, {});
	g.resetConfig();
}

void PuzzleList::GenerateMountainH() {
	std::wstring text = L"Mountain Perspective";
	SetWindowText(_handle, text.c_str());
	special.generateMountaintop(MOUNTAIN_TOP, { { Triangle|White, 2 },{ Triangle|Black, 1 }, { Star|White, 1 },{ Star|Black, 1 }, { Stone|White, 1 },{ Stone|Black, 1 } });
	g.setLoadingData(L"Mountain", 39);
	g.resetConfig();
	//Purple Bridge
	g.setConfigOnce(PreserveStructure);
	g.setConfigOnce(DecorationsOnly);
	std::set<Point> bpoints1 = { { 6, 3 },{ 5, 4 },{ 7, 4 } };
	std::set<Point> bpoints2 = { { 6, 5 },{ 5, 6 },{ 6, 7 },{ 7, 6 } };
	std::set<Point> bpoints3 = { { 3, 6 },{ 4, 7 } };
	if (Random::rand() % 2 == 0) g.hitPoints = { g.pickRandom(bpoints1), g.pickRandom(bpoints2), g.pickRandom(bpoints3) };
	else g.hitPoints = { g.pickRandom(bpoints3), g.pickRandom(bpoints2), g.pickRandom(bpoints1) };
	g.setObstructions({ { 4, 1 },{ 6, 1 },{ 8, 1 } });
	g.blockPos = { { 3, 1 },{ 5, 1 },{ 7, 1 },{ 9, 1 },{ 1, 1 },{ 11, 1 },{ 1, 11 },{ 11, 11 } };
	g.setSymbol(Gap_Row, 3, 4);
	g.generate(MOUNTAIN_PURPLE_BRIDGE, Triangle|Purple, 8, Eraser|Purple, 1);
	//Orange Row
	g.resetConfig();
	g.setGridSize(5, 5);
	g.lineThickness = 0.9f;
	g.setConfig(ResetColors);
	g.generate(MOUNTAIN_ORANGE_1, Start, 1, Dot, 6, Triangle|Yellow, 5, Stone|Black, 4, Stone|White, 4);
	g.generate(MOUNTAIN_ORANGE_2, Start, 1, Dot, 6, Poly|Blue, 1, Poly|Rotate|Blue, 1, Stone|Black, 2, Stone|White, 2);
	g.generate(MOUNTAIN_ORANGE_3, Start, 1, Exit, 1, Poly|Blue, 1, Poly|Rotate|Blue, 1, Triangle|Yellow, 3, Star|Yellow, 3);
	g.generate(MOUNTAIN_ORANGE_4, Start, 1, Triangle|Purple, 2, Triangle|Yellow, 2, Stone|Purple, 1, Stone|Yellow, 2, Star|Purple, 2, Star|Yellow, 1, Eraser|Purple, 1);
	g.generate(MOUNTAIN_ORANGE_5, Start, 1, Exit, 1, Poly|Blue, 3, Poly|Negative|Blue, 3, Star|Blue, 3, Eraser|Blue, 1);
	g.setGridSize(6, 6);
	g.lineThickness = 0.8f;
	g.setSymmetry(Rotational);
	g.generate(MOUNTAIN_ORANGE_6, Start, 1, Exit, 1, Star|White, 4, Star|Black, 4, Star|Purple, 2, Stone|Purple, 1, Stone|Black, 1, Stone|White, 1);
	g.setGridSize(7, 7);
	g.setSymmetry(Horizontal);
	g.setSymbol(Start, 0, 0); g.setSymbol(Start, 0, 14);
	g.setSymbol(Exit, 14, 0); g.setSymbol(Exit, 14, 14);
	g.generate(MOUNTAIN_ORANGE_7, Dot_Intersection, 64, Triangle|Blue, 8);
	//Green Row
	g.resetConfig();
	g.setGridSize(5, 5);
	g.lineThickness = 0.9f;
	g.generate(MOUNTAIN_GREEN_1, Dot, 5, Stone|Black, 2, Stone|White, 2, Star|Black, 3, Star|White, 4);
	g.blockPos = { { 1, 1 },{ 3, 3 },{ 7, 7 },{ 9, 9 },{ 1, 7 },{ 7, 1 },{ 3, 9 },{ 9, 3 } };
	g.generate(MOUNTAIN_GREEN_2, Stone|Black, 3, Stone|White, 3, Triangle|Black, 3, Triangle|White, 3);
	g.blockPos = { { 1, 1 },{ 3, 3 },{ 7, 7 },{ 9, 9 },{ 1, 7 },{ 7, 1 },{ 3, 9 },{ 9, 3 } };
	g.setConfigOnce(RequireCombineShapes);
	g.generate(MOUNTAIN_GREEN_3, Star|Black, 3, Star|White, 4, Triangle|Black, 1, Triangle|White, 1, Poly|Black, 1, Poly|White, 1);
	g.blockPos = { { 7, 3 },{ 3, 7 },{ 1, 9 },{ 9, 1 },{ 1, 3 },{ 3, 1 },{ 7, 9 },{ 9, 7 } };
	g.generate(MOUNTAIN_GREEN_4, Stone|Black, 2, Stone|White, 2, Star|Black, 2, Star|White, 3, Poly|Rotate|Black, 1);
	g.generate(MOUNTAIN_GREEN_5, Dot, 4, Star|Black, 2, Stone|Black, 2, Triangle|Black, 2, Eraser|Black, 1, Poly|Black, 2);
	//Purple Panels
	g.setConfig(DisconnectShapes);
	g.generate(MOUNTAIN_PURPLE_1, Poly|Rotate, 2, Star|Magenta, 6);
	g.generate(MOUNTAIN_PURPLE_2, Poly|Rotate, 2, Triangle|Orange, 3);
	//Blue Row
	g.resetConfig();
	g.setSymmetry(RotateLeft);
	g.setObstructions({ { 4, 3 },{ 5, 4 },{ 5, 6 },{ 5, 8 },{ 5, 10 },{ 6, 9 },{ 7, 10 } });
	special.initRotateGrid();
	g.generate(MOUNTAIN_BLUE_1, Triangle|Orange, 2, Stone|Black, 4, Stone|White, 4);
	if (Random::rand() % 2 == 0) g.setObstructions({ { 5, 4 },{ 5, 6 },{ 5, 8 },{ 5, 10 },{ 9, 4 },{ 9, 6 },{ 9, 8 },{ 9, 10 },{ 7, 0 },{ 7, 2 } });
	else g.setObstructions({ { 3, 4 },{ 3, 6 },{ 3, 8 },{ 3, 10 },{ 7, 4 },{ 7, 6 },{ 7, 8 },{ 7, 10 },{ 5, 0 },{ 5, 2 },{ 9, 0 },{ 9, 2 } });
	special.initRotateGrid();
	g.generate(MOUNTAIN_BLUE_2, Triangle|Orange, 2, Star|Orange, 3);
	g.setObstructions({ { 0, 5 },{ 3, 0 },{ 3, 2 },{ 3, 10 },{ 4, 1 },{ 4, 3 },{ 4, 9 },{ 5, 0 },{ 5, 2 },{ 5, 4 },{ 5, 6 },{ 5, 8 },{ 5, 10 },
		{ 6, 1 },{ 6, 3 },{ 6, 5 },{ 6, 7 },{ 6, 9 },{ 7, 0 },{ 7, 2 },{ 7, 4 },{ 7, 6 },{ 7, 8 },{ 7, 10 },{ 8, 1 },{ 8, 3 },{ 8, 5 },{ 8, 7 },{ 8, 9 },
		{ 9, 0 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 9, 8 },{ 9, 10 },{ 10, 1 },{ 10, 3 },{ 10, 5 },{ 10, 7 },{ 10, 9 } });
	g.blockPos = { { 5, 9 },{ 7, 7 } };
	special.initRotateGrid();
	g.setConfig(ShortPath);
	g.generate(MOUNTAIN_BLUE_3, Poly|Yellow, 3, Star|Yellow, 3);

	//Rainbow Row
	g.resetConfig();
	g.setGridSize(5, 5);
	g.setConfig(WriteColors);
	g.generate(MOUNTAIN_RAINBOW_1, Stone|Cyan, 3, Stone|Magenta, 3, Star|Cyan, 3, Star|Magenta, 4, Star|Yellow, 6);
	g.generate(MOUNTAIN_RAINBOW_2, Star|Cyan, 3, Star|Magenta, 2, Star|Yellow, 2, Triangle|Cyan, 2, Triangle|Magenta, 1, Triangle|Yellow, 2);
	g.generate(MOUNTAIN_RAINBOW_3, Poly|Cyan, 1, Poly|Yellow, 2, Poly|Negative|Cyan, 1, Poly|Negative|Yellow, 1, Poly|Negative|Purple, 1,
		Star|Cyan, 4, Star|Purple, 3, Star|Yellow, 2);
	g.setGridSize(6, 6);
	g.generate(MOUNTAIN_RAINBOW_4, Star|Yellow, 6, Star|Green, 6, Star|Cyan, 6, Star|Magenta, 6);
	g.setGridSize(5, 5);
	g.blockPos = { {1, 1}, {1, 5}, {3, 3}, {5, 3}, {5, 5}, {5, 7}, {7, 1}, {7, 5}, {7, 9}, {9, 1}, {9, 5}, {9, 9} };
	g.generate(MOUNTAIN_RAINBOW_5, Star|Green, 4, Star|Magenta, 5, Poly|Rotate|Green, 1, Poly|Rotate|Magenta, 1, Eraser|Magenta, 1);
	g.resetConfig();

	special.generateMultiPuzzle({ MOUNTAIN_MULTI_1, MOUNTAIN_MULTI_2, MOUNTAIN_MULTI_3, MOUNTAIN_MULTI_4, MOUNTAIN_MULTI_5, MOUNTAIN_MULTI_6 }, {
	{ { Triangle|Orange, 2 } },
	{ { Stone|Black, 1 },{ Stone|White, 1 },
	{ Star|Black, 1 },{ Star|White, 2 } },
	{ { Star|Magenta, 2 },{ Star|Cyan, 1 }, { Triangle|Cyan, 1 } },
	{ { { Poly|Rotate, 1 },{ Poly|Negative|Blue|Rotate, 2 } } },
	{ { Dot_Intersection, 8 } },
	{ { Poly|Rotate, 1 } } }, true);

	special.generate2BridgeH(MOUNTAIN_BLUE_BRIDGE, MOUNTAIN_ORANGE_BRIDGE);

	g.resetConfig();
	special.generateMountainFloorH();

	//Pillar Puzzles
	g.resetConfig();
	g.setGridSize(6, 4);
	g.setConfig(ResetColors);
	g.generate(MOUNTAIN_PILLAR_L1, Star|Black, 2, Star|White, 3, Stone|Black, 2, Stone|White, 2);
	g.removeConfigOnce(ResetColors);
	g.setConfigOnce(WriteColors);
	g.generate(MOUNTAIN_PILLAR_R1, Star|Orange, 4, Star|Magenta, 4, Star|Green, 4, Star|White, 1, Triangle|White, 1, Eraser|White, 1);
	g.setGridSize(6, 5);
	special.initPillarSymmetry(MOUNTAIN_PILLAR_L2, PillarParallel);
	g.generate(MOUNTAIN_PILLAR_L2, Triangle|Orange, 8);
	special.initPillarSymmetry(MOUNTAIN_PILLAR_R2, PillarHorizontal);
	g.setConfigOnce(WriteColors);
	g.generate(MOUNTAIN_PILLAR_R2, Triangle|Orange, 4, Dot_Intersection, 36); 
	g.setConfigOnce(RequireCombineShapes);
	g.setGridSize(6, 4);
	special.initPillarSymmetry(MOUNTAIN_PILLAR_L3, PillarRotational);
	g.generate(MOUNTAIN_PILLAR_L3, Poly, 3, Stone|Black, 2, Stone|White, 2);
	g.setConfigOnce(RequireCancelShapes);
	g.setConfigOnce(DisableDotIntersection);
	g.setGridSize(6, 5);
	special.initPillarSymmetry(MOUNTAIN_PILLAR_R3, PillarVertical); //TODO: Change?
	g.generate(MOUNTAIN_PILLAR_R3, Poly|Orange, 1, Poly|Negative|Blue, 2, Poly|Blue, 1, Poly|Negative|Orange, 2, Star|Orange, 1, Star|Blue, 1, Dot, 4, Start, 3);
	g.setConfig(CyanAndYellowLines);
	g.setConfig(InvisibleSymmetryLine);
	special.initPillarSymmetry(MOUNTAIN_PILLAR_L4, PillarHorizontal);
	g.generate(MOUNTAIN_PILLAR_L4, Star|Black, 2, Star|White, 1, Stone|Black, 1, Stone|White, 1, Triangle|Orange, 2, Dot_Intersection|Cyan, 2, Dot_Intersection|Yellow, 2);
	special.initPillarSymmetry(MOUNTAIN_PILLAR_R4, PillarParallel);
	g.generate(MOUNTAIN_PILLAR_R4, Triangle|Orange, 2, Triangle|Magenta, 1, Star|Orange, 1, Star|Magenta, 1, Poly|Orange, 1, Poly|Negative|Magenta, 2, Eraser|Magenta, 1);
}

void PuzzleList::GenerateCavesH() {
	g.setLoadingData(L"Caves", 51);
	g.resetConfig();

	special.createArrowSecretDoor(MOUNTAIN_SECRET_DOOR);

	g.setGridSize(5, 5);
	g.backgroundColor = { 0, 0, 0.5f, 1 };
	g.foregroundColor = { 0, 0, 0.5f, 1 };
	g.successColor = { 0, 1, 1, 1 };
	g.generate(CAVES_ENTER, Stone|Black, 5, Stone|White, 4, Arrow, 6);

	//Arrow Puzzles
	g.setSymmetry(Rotational);
	g.setConfig(WriteColors);
	g.backgroundColor = { 0, 0, 0.1f, 1 };
	g.foregroundColor = { 0, 0, 0.1f, 1 };
	g.successColor = { 0.6f, 0, 1, 1 };
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.generate(CAVES_SHAPE_SYM1, Arrow|Purple, 6);
	g.setGridSize(6, 6);
	g.lineThickness = 0.7f;
	g.generate(CAVES_SHAPE_SYM2, Arrow|Purple, 8);
	g.setSymmetry(NoSymmetry);
	g.lineThickness = 1;
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.generate(CAVES_SHAPE_DISJOINT, Arrow|Purple, 6, Star|Cyan, 6, Star|Magenta, 6);
	g.generate(CAVES_SHAPE_NEG, Arrow|Purple, 4, Poly, 3, Poly|Negative, 1);

	g.setGridSize(4, 4);
	g.lineThickness = 1.0f;
	g.generate(CAVES_SHAPE_R1, Arrow|Purple, 8);
	g.generate(CAVES_SHAPE_R2, Arrow|Purple, 8);
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.generate(CAVES_SHAPE_R3, Arrow|Purple, 12);
	g.generate(CAVES_SHAPE_R4, Arrow|Purple, 12);

	g.setGridSize(4, 4);
	g.lineThickness = 1.0f;
	g.generate(CAVES_SHAPE_L1, Arrow|Purple, 4, Triangle|Orange, 4);
	g.generate(CAVES_SHAPE_L2, Arrow|Purple, 5, Triangle|Orange, 3);
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.generate(CAVES_SHAPE_L3, Arrow|Purple, 4, Triangle|Orange, 8);
	g.generate(CAVES_SHAPE_L4, Arrow|Purple, 6, Triangle|Orange, 6);
	g.generate(CAVES_SHAPE_L5, Arrow|Purple, 8, Triangle|Orange, 4);
	g.setGridSize(4, 4);
	g.lineThickness = 1.0f;
	g.generate(CAVES_SHAPE_B1, Arrow|Purple, 2, Poly, 1, Poly|Rotate, 1);
	g.generate(CAVES_SHAPE_B2, Arrow|Purple, 2, Poly|Rotate, 2);
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.generate(CAVES_SHAPE_B3, Arrow|Purple, 6, Poly, 1, Poly|Rotate, 1);
	g.setConfigOnce(BigShapes);
	g.generate(CAVES_SHAPE_B4, Arrow|Purple, 4, Poly, 1, Poly|Rotate, 1);
	g.generate(CAVES_SHAPE_B5, Arrow|Purple, 4, Poly, 3);

	//First alcove
	g.resetConfig();
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.setConfigOnce(DisconnectShapes);
	g.generate(CAVES_SWAMP, Dot_Intersection, 36, Poly|Rotate, 3, Start, 1);
	g.setConfigOnce(WriteColors);
	g.setConfigOnce(TreehouseLayout);
	g.setConfigOnce(BigShapes);
	g.generate(CAVES_TREEHOUSE, Star|Black, 2, Star|White, 2, Stone|Black, 1, Stone|White, 1,
		Triangle|Black, 1, Triangle|White, 1, Poly|Rotate|Black, 1, Poly|Rotate|White, 1);
	g.setGridSize(6, 6);
	g.lineThickness = 1.0f;
	g.setConfigOnce(WriteColors);
	g.setConfigOnce(DisableFlash);
	g.generate(CAVES_RAINBOW, Star|Cyan, 8, Star|Yellow, 6, Star|Magenta, 6);
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.setConfigOnce(RequireCancelShapes);
	g.generate(CAVES_QUARRY, Stone|Black, 1, Star|Black, 1, Star|White, 2, Poly|White, 1, Poly|Negative|White, 2, Dot, 36, Eraser|Black, 1, Start, 1);

	//Perspective
	g.resetConfig();
	g.setConfig(DisableFlash);
	g.setConfig(DecorationsOnly);
	special.generateCenterPerspective(CAVES_PERSPECTIVE_1, { { Star|Black, 8 }, { Star|White, 6 } }, Star);
	special.generateCenterPerspective(CAVES_PERSPECTIVE_2, { { Poly, 4 }, { Eraser|White, 1 } }, Eraser);
	special.generateCenterPerspective(CAVES_PERSPECTIVE_3, { { Triangle|Orange, 10}, { Eraser|White, 1 } }, Triangle);
	g.removeConfig(DecorationsOnly);
	g.setConfigOnce(OnlyCancelShapes);
	special.generateCenterPerspective(CAVES_PERSPECTIVE_4, { { Dot_Intersection, 36 },{ Poly, 2 }, { Poly|Negative|Blue, 4 } }, Poly);

	//Full Dots + Arrows
	g.resetConfig();
	g.backgroundColor = { 0, 0.7f, 0.2f, 1 };
	g.successColor = { 0.6f, 1, 0.25f, 1 };
	g.generate(CAVES_DOT_1, Dot_Intersection, 25, Arrow|Black, 4);
	g.generate(CAVES_DOT_2, Dot_Intersection, 25, Arrow|Black, 4, Start, 1);
	g.setGridSize(5, 5);
	g.lineThickness = 0.85f;
	g.generate(CAVES_DOT_3, Dot_Intersection, 36, Arrow|Black, 4, Start, 1);
	g.generate(CAVES_DOT_4, Dot_Intersection, 36, Arrow|Black, 6, Start, 1);
	g.generate(CAVES_DOT_5, Dot_Intersection, 36, Arrow|Black, 8, Start, 1);
	g.lineThickness = 0.7f;
	g.generate(CAVES_DOT_6, Dot_Intersection, 36, Arrow|Black, 4, Start, 1, Stone|Black, 4, Stone|White, 3);

	//Invisible Symbols
	g.resetConfig();
	g.setConfig(WriteColors);
	g.setGridSize(3, 3);
	g.generate(CAVES_INVISIBLE_1, Dot_Intersection, 3, Stone|Black, 2, Stone|Invisible, 3);
	g.setGridSize(4, 4);
	g.generate(CAVES_INVISIBLE_2, Stone|Black, 2, Stone|Invisible, 3, Star|Black, 3, Star|Invisible, 2);
	g.generate(CAVES_INVISIBLE_3, Stone|Black, 3, Stone|Invisible, 2, Triangle|Black, 2, Star|Invisible, 3);
	g.generate(CAVES_INVISIBLE_4, Star|Black, 4, Star|Invisible, 4, Poly|Rotate|Invisible, 1, Poly|Invisible, 1);
	g.generate(CAVES_INVISIBLE_5, Star|Black, 2, Triangle|Black, 2, Poly|Rotate|Black, 1, Poly|Invisible, 1, Eraser|Invisible, 1);
	g.setGridSize(0, 0);
	g.setConfigOnce(OnlyCancelShapes);
	g.generate(CAVES_INVISIBLE_6, Poly|Invisible, 2, Poly|Negative|Invisible, 4, Dot, 8);
	g.generate(CAVES_INVISIBLE_7, Dot, 4, Star|Black, 4, Star|Orange, 4, Star|Invisible, 6);
	g.generate(CAVES_INVISIBLE_8, Dot, 6, Star|Black, 4, Star|Orange, 4, Star|Invisible, 6, Eraser|Invisible, 1);
	//TODO: Buff the symmetry ones?
	g.setSymmetry(Vertical);
	g.generate(CAVES_INVISIBLE_SYM1, Stone|Black, 2, Stone|Invisible, 2, Eraser|Black, 1);
	g.generate(CAVES_INVISIBLE_SYM2, Poly|Black, 2, Poly|Invisible, 2);
	g.setSymmetry(Rotational);
	g.generate(CAVES_INVISIBLE_SYM3, Triangle|Black, 8, Eraser|Invisible, 1);

	//Arrows Exit
	g.resetConfig();
	g.setConfig(WriteColors);
	g.setGridSize(4, 4);
	g.backgroundColor = { 0.5f, 0.5f, 0.5f, 1 };
	g.activeColor = { 0.85f, 0.7f, 1, 1 };
	g.successColor = { 0.6f, 0, 1, 1 };
	g.generate(CAVES_TRIANGLE_EXIT, Arrow1|Purple, 12);
	//Stars Exit
	g.resetConfig();
	g.setGridSize(4, 4);
	g.generate(CAVES_STAR_EXIT, Triangle|Cyan, 3, Triangle|Yellow, 3, Star|Cyan, 3, Star|Yellow, 3, Eraser|Cyan, 1);
	//Arrow Pillar
	g.resetConfig();
	g.setConfig(WriteColors);
	g.gridColor = { 0.01f, 0, 0.02f, 1 };
	g.activeColor = { 0.85f, 0.7f, 1, 1 };
	g.backgroundColor = { 0, 0, 0, 1 };
	g.successColor = { 1, 1, 1, 1 };
	g.lineThickness = 0.5f;
	g.generate(CAVES_PILLAR, Arrow|Purple, 8);
	g.lineThickness = 1;
	g.successColor = { 0, 0, 0, 0 }; //Reset
	//Challenge Entrance
	g.resetConfig();
	g.backgroundColor = { 0.1f, 0, 0, 1 };
	g.generate(CAVES_CHALLENGE_ENTER, Star|Green, 5, Arrow|Green, 6);
	//Theater Exit
	g.backgroundColor = { 0.5f, 0.5f, 0.5f, 1 };
	g.activeColor = { 0.85f, 0.7f, 1, 1 };
	g.successColor = { 0.6f, 0, 1, 1 };
	g.setConfig(WriteColors);
	g.generate(CAVES_THEATER_WALK, Arrow2|Purple, 12);
	//Town Exit
	g.generate(CAVES_TOWN_EXIT, Arrow|Purple, 16, Start, 4, Exit, 1);
}

void PuzzleList::GenerateOrchardH() {
	special.generateApplePuzzle(APPLE_1, false, true);
	special.generateApplePuzzle(APPLE_2, false, Random::rand() % 2 == 0);
	special.generateApplePuzzle(APPLE_3, false, Random::rand() % 2 == 0);
	special.generateApplePuzzle(APPLE_4, false, Random::rand() % 2 == 0);
	special.generateApplePuzzle(APPLE_5, true, true);
}

void PuzzleList::GenerateDesertH() {
	Randomizer().RandomizeDesert();
}

void PuzzleList::GenerateKeepH() {
	g.setLoadingData(L"Keep", 5);
	g.resetConfig();

	g.setObstructions({ { 8, 3 },{ 4, 5 },{ 3, 0 },{ 3, 2 },{ 5, 6 } });
	g.setConfigOnce(DisableWrite);
	g.generate(KEEP_PRESSURE_1, Triangle|Yellow, 4, Star|Yellow, 3, Stone|Yellow, 2);
	std::set<Point> path1 = g.path;
	g.write(KEEP_PRESSURE_1);

	g.resetConfig();
	g.setObstructions({ { 8, 5 },{ 6, 5 },{ 1, 8 },{ 1, 6 },{ 1, 4 },{ 2, 3 },{ 4, 3 },{ 6, 3 } });
	g.hitPoints = { { 3, 2 },{ 1, 0 } };
	g.setConfigOnce(DisableWrite);
	g.generate(KEEP_PRESSURE_2, Star|Black, 3, Star|White, 3, Stone|Black, 2, Stone|White, 2, Poly|Rotate|Black, 1, Poly|White, 1);
	std::set<Point> path2 = g.path;
	g.write(KEEP_PRESSURE_2);

	g.resetConfig();
	std::vector<std::vector<Point>> validHitPoints = {
		{ { 3, 4 },{ 7, 2 },{ 3, 2 } },{ { 4, 5 },{ 7, 2 },{ 2, 5 } },{ { 4, 3 },{ 7, 2 },{ 2, 3 } },
	{ { 3, 4 },{ 7, 2 },{ 3, 2 } },{ { 1, 4 },{ 7, 2 },{ 1, 2 } },{ { 3, 2 },{ 7, 2 },{ 3, 0 } },{ { 1, 2 },{ 7, 2 },{ 1, 0 } } };
	g.hitPoints = validHitPoints[Random::rand() % validHitPoints.size()];
	g.setObstructions({ { 5, 8 } });
	g.setConfigOnce(SplitShapes);
	g.setConfigOnce(DisableWrite);
	g.generate(KEEP_PRESSURE_3, Stone|Black, 3, Stone|White, 2, Triangle|Yellow, 5, Poly, 1, Poly|Rotate, 1);
	std::set<Point> path3 = g.path;
	g.write(KEEP_PRESSURE_3);

	g.resetConfig();
	g.setSymmetry(NoSymmetry);
	g.setConfigOnce(DisableWrite);
	g.setObstructions({ { 0, 7 },{ 2, 7 },{ 4, 7 },{ 6, 7 },{ 8, 7 },{ 8, 5 },{ 7, 6 } });
	g.setSymbol(Start, 8, 0);
	g.setSymbol(Exit, 8, 4);
	g.setConfigOnce(ShortPath);
	g.generate(KEEP_PRESSURE_4, Poly|Yellow, 2, Poly|Blue, 1, Star|Yellow, 2, Star|Blue, 2, Triangle|Yellow, 3, Triangle|Blue, 3);
	g.panel.startpoints.push_back({ 0, 8 });
	g.panel.endpoints.push_back(Endpoint(0, 4, Endpoint::Direction::LEFT, Exit));
	if (g.panel.endpoints[0].x == 0) {
		std::swap(g.panel.endpoints[0], g.panel.endpoints[1]); //Need to have endpoints in right order to associate with pressure plates correctly
	}
	std::set<Point> path4 = g.path;
	g.write(KEEP_PRESSURE_4);

	special.generateKeepLaserPuzzle(KEEP_PRESSURE_LASER, path1, path2, path3, path4,
		{ { Triangle|Yellow, 10 },{ Star|Yellow, 4 },{ Stone|Yellow, 2 },
		{ Star|Black, 3 },{ Star|White, 3 },
		{ Stone|Black, 5 },{ Stone|White, 4 },
		{ Poly|Rotate|Black, 1 },{ Poly|Rotate|White, 1 },
		{ Poly|Rotate|Yellow, 2 },{ Poly|Yellow, 3 },{ Poly|Blue, 3 },
		{ Star|Blue, 2 },{ Triangle|Yellow, 2 },{ Triangle|Blue, 2 } });

	special.clearTarget(KEEP_HEDGE_LASER); //Must solve pressure plate side
}

void PuzzleList::GenerateJungleH() {
	g.setLoadingData(L"Jungle", 6);
	g.resetConfig();
	special.generateSoundDotPuzzle(JUNGLE_DOT_1, { 3, 3 }, { DOT_SMALL, DOT_LARGE }, false);
	special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_2, { 5, 5 }, { DOT_SMALL, DOT_LARGE }, { DOT_SMALL, DOT_LARGE }, 0, true);
	special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_3, { 7, 7 }, { DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_MEDIUM, DOT_LARGE },
		{ DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_MEDIUM, DOT_LARGE }, 0, true);
	special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_4, { 7, 7 }, { DOT_SMALL, DOT_MEDIUM, DOT_SMALL, DOT_LARGE },
		{ DOT_LARGE, DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_LARGE }, 0, true);
	if (Random::rand() % 2) special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_5, { 7, 7 }, { DOT_SMALL, DOT_SMALL, DOT_LARGE, DOT_MEDIUM, DOT_LARGE },
		{ DOT_SMALL, DOT_SMALL, DOT_LARGE, DOT_MEDIUM, DOT_LARGE }, 0, true);
	else special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_5, { 7, 7 }, { DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_MEDIUM, DOT_LARGE },
		{ DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_MEDIUM, DOT_LARGE }, 0, true);
	switch (Random::rand() % 4) {
	case 0: special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_6, { 7, 7 }, { DOT_SMALL, DOT_LARGE, DOT_SMALL, DOT_LARGE, DOT_MEDIUM },
		{ DOT_SMALL, DOT_LARGE, DOT_SMALL, DOT_LARGE, DOT_MEDIUM }, 0, true); break;
	case 1: special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_6, { 7, 7 }, { DOT_LARGE, DOT_MEDIUM, DOT_SMALL, DOT_LARGE, DOT_SMALL },
		{ DOT_LARGE, DOT_MEDIUM, DOT_SMALL, DOT_LARGE, DOT_SMALL }, 0, true); break;
	case 2: special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_6, { 7, 7 }, { DOT_LARGE, DOT_MEDIUM, DOT_SMALL, DOT_LARGE, DOT_SMALL },
		{ DOT_SMALL, DOT_LARGE, DOT_SMALL, DOT_LARGE, DOT_MEDIUM }, 0, true); break;
	case 3: special.generateSoundDotReflectionPuzzle(JUNGLE_DOT_6, { 7, 7 }, { DOT_SMALL, DOT_LARGE, DOT_SMALL, DOT_LARGE, DOT_MEDIUM },
		{ DOT_LARGE, DOT_MEDIUM, DOT_SMALL, DOT_LARGE, DOT_SMALL }, 0, true); break;
	}
}