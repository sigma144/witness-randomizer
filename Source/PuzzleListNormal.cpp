#include "PuzzleList.h"
#include "Watchdog.h"

void PuzzleList::GenerateAllN() {
	g.setLoadingData(336);
	CopyTargets();
	GenerateTutorialN();
	GenerateSymmetryN();
	GenerateQuarryN();
	//GenerateBunkerN(); //Can't randomize because panels refuse to render the symbols
	GenerateSwampN();
	GenerateTreehouseN();
	GenerateTownN();
	GenerateVaultsN();
	GenerateTrianglePanelsN();
	GenerateOrchardN();
	GenerateDesertN();
	GenerateKeepN();
	GenerateJungleN();
	GenerateMountainN();
	GenerateCavesN();
	SetWindowText(_handle, L"Done!");
	//GenerateShadowsN(); //Can't randomize
	//GenerateMonasteryN(); //Can't randomize
}

void PuzzleList::GenerateAllH() {
	g.setLoadingData(349);
	CopyTargets();
	GenerateTutorialH();
	GenerateSymmetryH();
	GenerateQuarryH();
	//GenerateBunkerH(); //Can't randomize because panels refuse to render the symbols
	GenerateSwampH();
	GenerateTreehouseH();
	GenerateTownH();
	GenerateVaultsH();
	GenerateTrianglePanelsH();
	GenerateOrchardH();
	GenerateDesertH();
	GenerateKeepH();
	GenerateJungleH();
	GenerateMountainH();
	GenerateCavesH();
	SetWindowText(_handle, L"Done!");
	//GenerateShadowsH(); //Can't randomize
	//GenerateMonasteryH(); //Can't randomize
}

void PuzzleList::CopyTargets() {
	//TODO: Activate all these automatically.
	Special::copyTarget(TUT_STONE_9, SHADOWS_LASER);
	Special::copyTarget(TUT_DOT_5, BUNKER_LASER);
	Special::copyTarget(TUT_DISCARD, TOWN_RED);
	Special::copyTarget(SYM_DISCARD, TOWN_RED);
	Special::copyTarget(DESERT_DISCARD, MONASTERY_LASER);
	Special::copyTarget(SYM_ROCKS_5, TOWN_RED);
	Special::copyTarget(TREEHOUSE_GREEN_DISCARD, MONASTERY_LASER);
	Special::copyTarget(TREEHOUSE_ORANGE_DISCARD, JUNGLE_POPUP);
	Special::copyTarget(KEEP_DISCARD, JUNGLE_POPUP);
	Special::copyTarget(SHIPWRECK_DISCARD, SHADOWS_LASER);
	Special::copyTarget(TOWN_CRATE_DISCARD, BUNKER_LASER);
	Special::copyTarget(TOWN_ROOF_DISCARD, SHADOWS_LASER);
	Special::copyTarget(THEATER_DISCARD, TOWN_RED);
	Special::copyTarget(TOWN_CRATE_DISCARD, BUNKER_LASER);
	Special::copyTarget(JUNGLE_DISCARD, JUNGLE_POPUP);
	Special::copyTarget(MOUNTAIN_OUTSIDE_DISCARD, BUNKER_LASER);
	Special::copyTarget(SYM_YELLOW_3, MONASTERY_LASER);
	
	Special::setPower(MONASTERY_LASER, true);
	Special::setPower(JUNGLE_POPUP, true);
	Special::setPower(TOWN_RED, true);
}

void PuzzleList::GenerateTutorialN() {
	g.setLoadingData(L"Tutorial", 21);
	g.resetConfig();
	Special::drawSeedAndDifficulty(TUT_ENTER_1, seed, false, !seedIsRNG, false);
	Special::drawGoodLuckPanel(TUT_ENTER_2);
	//Mazes
	g.setConfig(FullGaps);
	g.setGridSize(6, 6);
	g.generateMaze(TUT_MAZE_1);
	g.setGridSize(9, 9);
	g.generateMaze(TUT_MAZE_2, 1, 1);
	g.setGridSize(12, 12);
	g.generateMaze(TUT_MAZE_3);
	g.resetConfig();
	//2 starts maze
	g.generateMaze(TUT_2START);
	//2 exits maze
	g.setSymbol(Exit, 12, 12);
	g.setConfigOnce(DisableWrite);
	g.generateMaze(TUT_2EXIT);
	g.panel.endpoints.emplace_back(Endpoint(12, 0, Endpoint::Direction::RIGHT, Exit));
	g.write(TUT_2EXIT);
	g.resetConfig();
	//Secret back area
	g.generate(TUT_OUTER_1, Dot_Intersection, 25, Gap, 4);
	g.generate(TUT_OUTER_2, Dot_Intersection, 25, Stone|Black, 2, Stone|White, 2);
	//Dots Tutorial
	g.setConfig(FullGaps);
	g.setGridSize(3, 3);
	g.generate(TUT_DOT_1, Start, 1, Exit, 1, Dot_Intersection, 7, Gap, 4);
	g.generate(TUT_DOT_2, Start, 2, Exit, 1, Dot_Intersection, 7, Gap, 4);
	g.setGridSize(4, 4);
	g.generate(TUT_DOT_3, Start, 2, Exit, 1, Dot_Intersection, 10, Gap, 8);
	g.generate(TUT_DOT_4, Start, 2, Exit, 1, Dot_Intersection, 10, Gap, 8);
	g.generate(TUT_DOT_5, Start, 3, Exit, 1, Dot_Intersection, 10, Gap, 8);
	//Stones Tutorial
	g.resetConfig();
	g.setGridSize(4, 4);
	g.generate(TUT_STONE_1, Exit, 1, Stone|Black, 7, Stone|White, 5);
	g.generate(TUT_STONE_2, Exit, 1, Stone|Black, 7, Stone|White, 5);
	g.generate(TUT_STONE_3, Exit, 1, Stone|Black, 7, Stone|White, 5, Start, 3);
	g.setGridSize(5, 5);
	g.generate(TUT_STONE_4, Exit, 1, Stone|Black, 11, Stone|White, 8);
	g.generate(TUT_STONE_5, Exit, 1, Stone|Black, 11, Stone|White, 8);
	g.generate(TUT_STONE_6, Exit, 1, Stone|Black, 11, Stone|White, 8, Start, 3);
	g.generate(TUT_STONE_7, Exit, 1, Stone|Black, 7, Stone|White, 5, Gap, 10);
	g.generate(TUT_STONE_8, Exit, 1, Stone|Black, 7, Stone|White, 5, Gap, 10);
	g.generate(TUT_STONE_9, Exit, 1, Stone|Black, 7, Stone|White, 5, Gap, 10, Start, 3);
}

void PuzzleList::GenerateSymmetryN() {
	g.setLoadingData(L"Symmetry", 33);
	g.resetConfig();
	g.setConfig(StartEdgeOnly);
	//Vertical Symmetry Mazes
	g.setSymmetry(Vertical);
	g.setGridSize(5, 5);
	g.generateMaze(SYM_MAZE_V1, 0, 1);
	g.generateMaze(SYM_MAZE_V2, 1, 1);
	g.setGridSize(0, 0);
	g.removeConfigOnce(StartEdgeOnly);
	g.generateMaze(SYM_MAZE_V3, 1, 0);
	g.generateMaze(SYM_MAZE_V4);
	special.generateSpecialSymMaze(SYM_MAZE_V5);
	//Rotational Symmetry Mazes
	g.setSymmetry(Rotational);
	g.setGridSize(5, 5);
	g.generateMaze(SYM_MAZE_R1, 0, 1);
	g.generateMaze(SYM_MAZE_R2, 1, 1);
	g.removeConfigOnce(StartEdgeOnly);
	g.generateMaze(SYM_MAZE_R3, 1, 1);
	g.setGridSize(6, 6);
	g.generateMaze(SYM_MAZE_M1);
	g.generateMaze(SYM_MAZE_M2);
	g.generateMaze(SYM_MAZE_M3);
	//Symmetry Island Door
	g.resetConfig();
	g.setGridSize(4, 4);
	g.setConfigOnce(FullGaps);
	g.generate(SYM_DOT_ENTER, Dot, 8, Gap, 5);
	//Black Dots
	g.setConfig(StartEdgeOnly);
	g.setGridSize(5, 5);
	g.setSymmetry(Horizontal);
	g.generate(SYM_DOT_1, Dot, 8, Start, 1, Exit, 1);
	g.generate(SYM_DOT_2, Dot, 8, Start, 1, Exit, 1);
	g.setSymmetry(Rotational);
	g.generate(SYM_DOT_3, Dot, 8, Start, 1, Exit, 1);
	g.generate(SYM_DOT_4, Dot, 8, Start, 1, Exit, 1);
	g.generate(SYM_DOT_5, Dot, 8, Start, 1, Exit, 1);
	//Colored Dots
	g.generate(SYM_COLOR_1, Dot|Blue, 3, Dot|Yellow, 3, Start, 1, Exit, 1);
	g.generate(SYM_COLOR_2, Dot|Blue, 3, Dot|Yellow, 3, Start, 1, Exit, 2);
	g.generate(SYM_COLOR_3, Dot|Blue, 2, Dot|Yellow, 2, Dot, 3, Start, 1, Exit, 1);
	g.generate(SYM_COLOR_4, Dot|Blue, 2, Dot|Yellow, 1, Dot, 5, Start, 1, Exit, 1);
	g.generate(SYM_COLOR_5, Dot|Blue, 1, Dot|Yellow, 2, Dot, 5, Start, 1, Exit, 1);
	g.generate(SYM_COLOR_6, Dot|Blue, 2, Dot|Yellow, 2, Dot, 4, Start, 2, Exit, 1);
	//Fading Lines
	g.setGridSize(6, 6);
	g.generate(SYM_INVISIBLE_1, Dot|Blue, 4, Dot|Yellow, 2, Start, 1, Exit, 1);
	g.generate(SYM_INVISIBLE_2, Dot|Blue, 3, Dot|Yellow, 3, Start, 1, Exit, 2);
	g.generate(SYM_INVISIBLE_3, Dot|Blue, 2, Dot|Yellow, 4, Start, 2, Exit, 1);
	g.setSymbol(Start, 0, 6);  g.setSymbol(Start, 12, 6);
	g.generate(SYM_INVISIBLE_4, Dot|Blue, 3, Dot|Yellow, 3, Exit, 1);
	g.setSymmetry(Vertical);
	g.setSymbol(Start, 0, 6); g.setSymbol(Start, 12, 6);
	g.generate(SYM_INVISIBLE_5, Dot|Blue, 3, Dot|Yellow, 3, Exit, 1);
	g.setSymmetry(Rotational);
	g.setSymbol(Start, 0, 6); g.setSymbol(Start, 12, 6);
	g.generate(SYM_INVISIBLE_6, Dot|Blue, 2, Dot|Yellow, 3, Start, 1, Exit, 1);
	g.generate(SYM_INVISIBLE_7, Dot|Yellow, 5, Start, 1, Exit, 1);
	//Dot Reflection Dual Panels (before laser)
	g.resetConfig();
	std::set<Symmetry> normalSym = { Horizontal, Rotational };
	std::set<Symmetry> weirdSym = { RotateLeft, RotateRight, FlipXY, FlipNegXY };
	special.generateReflectionDotPuzzle(SYM_YELLOW_1, SYM_CYAN_1, { {Dot, 10 }, {Exit, 1}, { Gap, 5 } }, Vertical, false);
	g.setSymbol(Start, 0, 8);
	special.generateReflectionDotPuzzle(SYM_YELLOW_2, SYM_CYAN_2, { { Dot, 12}, { Exit, 1}, {Gap, 1} }, Random::popRandom(normalSym), false);
	g.setSymbol(Start, 0, 8); g.setSymbol(Start, 8, 8); g.setSymbol(Start, 8, 0); g.setSymbol(Start, 0, 0);
	g.setSymbol(Exit, 8, 2);
	special.generateReflectionDotPuzzle(SYM_YELLOW_3, SYM_CYAN_3, { { Dot, 12} }, Random::popRandom(weirdSym), false);
}

void PuzzleList::GenerateQuarryN() {
	Memory* memory = Memory::get();
	g.setLoadingData(L"Quarry", 39);
	g.resetConfig();
	//Entry Gates
	g.setGridSize(5, 5);
	g.generate(QUARRY_ENTER_1, Stone|Black, 7, Stone|White, 5, Gap, 10);
	g.setGridSize(4, 4);
	g.setConfigOnce(RequireCombineShapes);
	g.generate(QUARRY_ENTER_2, Poly, 2, Gap, 5);
	//Mill Entry Door
	g.resetConfig();
	g.generate(MILL_ENTER_L, Stone|White, 5, Stone|Black, 7);
	g.setConfigOnce(PreserveStructure);
	g.generate(MILL_ENTER_R, Dot, 15);
	//Dots
	g.setGridSize(3, 3);
	g.generate(MILL_DOT_1, Dot, 5, Eraser|Green, 1);
	g.generate(MILL_DOT_2, Dot, 7, Eraser|Green, 1);
	g.setGridSize(4, 3);
	g.generate(MILL_DOT_3, Dot, 11, Eraser|Green, 1);
	g.generate(MILL_DOT_4, Dot, 13, Eraser|Green, 1);
	g.setGridSize(4, 4);
	g.generate(MILL_DOT_5, Dot, 15, Eraser|Green, 1);
	g.generate(MILL_DOT_6, Dot, 15, Eraser|Green, 1);
	//Stones
	g.setConfig(AlternateColors);
	g.setGridSize(3, 3);
	g.generate(MILL_STONE_1, Stone|White, 3, Stone|Black, 3, Eraser|Green, 1);
	g.generate(MILL_STONE_2, Stone|White, 4, Stone|Black, 4, Eraser|Green, 1);
	g.setConfig(MakeStonesUnsolvable);
	g.setGridSize(4, 4);
	g.generate(MILL_STONE_3, Stone|White, 7, Stone|Black, 6, Eraser|Green, 1);
	g.generate(MILL_STONE_4, Stone|White, 6, Stone|Black, 7, Eraser|Green, 1);
	g.generate(MILL_STONE_5, Stone|White, 7, Stone|Black, 6, Eraser|Green, 1);
	g.removeConfig(MakeStonesUnsolvable);
	g.setGridSize(3, 3);
	g.generate(MILL_STONE_6, Stone|White, 2, Stone|Black, 2, Stone|Red, 2, Eraser|Green, 1);
	g.setGridSize(4, 4);
	g.generate(MILL_STONE_7, Stone|Red, 4, Stone|White, 4, Stone|Black, 3, Eraser|Green, 1);
	g.generate(MILL_STONE_8, Stone|White, 4, Stone|Black, 4, Stone|Red, 3, Eraser|Green, 1);
	//Dots + Stones
	g.resetConfig();
	g.setSymbol(Start, 4, 4);
	g.setConfigOnce(AlternateColors);
	g.generate(MILL_FINAL, Stone|White, 5, Stone|Black, 5, Dot, 4, Eraser|Green, 1);
	g.generate(MILL_OPTIONAL, Stone|White, 10, Stone|Black, 10, Dot, 10, Eraser|Purple, 1, Start, 3);
	//Boathouse Ramp Activation
	g.generate(BOATHOUSE_BEGIN_L, Star|White, 8, Star|Black, 6);
	g.setConfigOnce(RequireCombineShapes);
	g.generate(BOATHOUSE_BEGIN_R, Poly, 1, Poly|Rotate, 1, Gap, 5);
	//Eraser + Shapes
	g.setConfig(ResetColors);
	g.generate(BOATHOUSE_SHAPE_1, Poly, 3, Eraser|White, 1);
	g.generate(BOATHOUSE_SHAPE_2, Poly, 3, Eraser|White, 1);
	g.generate(BOATHOUSE_SHAPE_3, Poly, 4, Eraser|White, 1);
	g.setConfigOnce(SmallShapes);
	g.generate(BOATHOUSE_SHAPE_4, Poly, 4, Eraser|White, 1);
	g.setConfigOnce(DisconnectShapes);
	g.generate(BOATHOUSE_SHAPE_5, Poly, 3, Eraser|White, 1);
	g.removeConfig(ResetColors);
	//Eraser + Stars
	g.setConfig(WriteColors);
	g.generate(BOATHOUSE_STAR_1, Star|Green, 3, Star|Magenta, 4, Eraser|Magenta, 1, Gap, 2);
	g.generate(BOATHOUSE_STAR_2, Star|Green, 3, Star|Magenta, 4, Eraser|Green, 1, Gap, 2);
	g.generate(BOATHOUSE_STAR_3, Star|Green, 6, Star|Orange, 5, Eraser|Orange, 1);
	g.generate(BOATHOUSE_STAR_4, Star|Magenta, 7, Star|Orange, 6, Eraser|Orange, 1);
	g.setConfigOnce(CombineErasers);
	g.generate(BOATHOUSE_STAR_5, Star|Green, 1, Eraser|Green, 1, Star|Magenta, 1, Eraser|Magenta, 1,
		Star|Orange, 1, Eraser|Orange, 1, Star|White, 1, Eraser|White, 1);
	g.generate(BOATHOUSE_STAR_6, Star|Orange, 9, Eraser|Orange, 1);
	g.generate(BOATHOUSE_STAR_7, Star|Orange, 6, Star|Magenta, 5, Star|Green, 4, Eraser|Magenta, 1);
	g.resetConfig();
	//Eraser + Stars + Shapes
	g.setConfigOnce(FixBackground);
	g.setGridSize(4, 4);
	g.setSymbol(Start, 0, 8);
	g.generate(BOATHOUSE_STAR_8, Star|White, 5, Poly|Green, 2, Eraser|Black, 1);
	g.lineThickness = 0.8f;
	g.generate(BOATHOUSE_STAR_9, Star|White, 6, Poly|Green, 2, Eraser|Black, 1);
	g.setGridSize(6, 3);
	g.lineThickness = 0.6f;
	g.generate(BOATHOUSE_FINAL_1, Star|Black, 3, Poly|Green, 3, Eraser|Cyan, 1);
	g.generate(BOATHOUSE_FINAL_2, Star|Black, 5, Poly|Green, 3, Eraser|Cyan, 1);
	g.generate(BOATHOUSE_FINAL_3, Star|Black, 6, Poly|Green, 3, Eraser|Cyan, 1);
}

void PuzzleList::GenerateBunkerN() {
	//I would randomize this, if I could get the panels to actually render the symbols.
	//Unfortunately, the path is rendered to a 3D model that doesn't have any geometry between the grid lines.
	//Somehow, I would either have to change the model, or make the puzzle render to the background texture instead.
}

void PuzzleList::GenerateSwampN() {
	g.setLoadingData(L"Swamp", 49);
	g.resetConfig();
	//First row
	g.setConfig(SplitShapes);
	g.setGridSize(3, 3);
	g.lineThickness = 0.7f;
	g.generate(SWAMP_ENTER, Poly, 1, Gap, 5);
	g.generate(SWAMP_TUT_A1, Poly, 2, Gap, 5);
	g.generate(SWAMP_TUT_A2, Poly, 2, Gap, 5);
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_TUT_A3, Poly, 2, Gap, 8);
	g.generate(SWAMP_TUT_A4, Poly, 2, Gap, 8);
	g.generate(SWAMP_TUT_A5, Poly, 2, Gap, 8);
	g.generate(SWAMP_TUT_A6, Poly, 3, Gap, 8);
	//Second Row
	g.resetConfig();
	g.setConfig(RequireCombineShapes);
	g.setGridSize(3, 3);
	g.lineThickness = 0.7f;
	g.generate(SWAMP_TUT_B1, Poly, 2, Gap, 3);
	g.generate(SWAMP_TUT_B2, Poly, 2, Gap, 3);
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_TUT_B3, Poly, 2, Gap, 5);
	g.generate(SWAMP_TUT_B4, Poly, 2, Gap, 5);
	g.generate(SWAMP_TUT_B5, Poly, 2, Gap, 5);
	g.generate(SWAMP_TUT_B6, Poly, 2, Gap, 5);
	g.generate(SWAMP_TUT_B7, Poly, 2, Gap, 5);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_TUT_B8, Poly, 2, Gap, 12);
	special.setTargetAndDeactivate(SWAMP_TUT_B8, SWAMP_SLIDING_BRIDGE);
	//Red Panels
	g.resetConfig();
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_RED_1, Poly, 3, Gap, 2);
	g.generate(SWAMP_RED_2, Poly, 3, Gap, 2);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.generate(SWAMP_RED_3, Poly, 3, Gap, 5);
	g.generate(SWAMP_RED_4, Poly, 4);
	//generator->setConfigOnce(DisableReset);
	g.setConfigOnce(LongPath);
	g.generate(SWAMP_RED_SHORTCUT_1, Poly, 3);
	g.placeGaps(12);
	g.write(SWAMP_RED_SHORTCUT_2);
	//Disconnected Shapes
	g.resetConfig();
	g.setConfig(DisconnectShapes);
	g.setGridSize(3, 3);
	g.lineThickness = 0.7f;
	g.generate(SWAMP_DISJOINT_1, Poly, 2);
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_DISJOINT_2, Poly, 2);
	g.generate(SWAMP_DISJOINT_3, Poly, 3);
	g.generate(SWAMP_DISJOINT_4, Poly, 3);
	//Rotating Shapes
	g.resetConfig();
	g.setGridSize(3, 3);
	g.lineThickness = 0.7f;
	g.generate(SWAMP_ROTATE_A1, Poly|Rotate, 1, Gap, 3);
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_ROTATE_A2, Poly|Rotate, 1, Poly, 1, Gap, 6);
	g.generate(SWAMP_ROTATE_A3, Poly|Rotate, 2, Gap, 6);
	g.setConfigOnce(BigShapes);
	g.setGridSize(5, 5);
	g.lineThickness = 0.6f;
	g.generate(SWAMP_ROTATE_A4, Poly|Rotate, 1, Gap, 15);
	//5x5 Rotating Shapes
	g.generate(SWAMP_ROTATE_B1, Poly|Rotate, 3, Gap, 2);
	g.generate(SWAMP_ROTATE_B2, Poly|Rotate, 4);
	g.generate(SWAMP_ROTATE_B3, Poly|Rotate, 2, Poly, 1, Gap, 2);
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_ROTATE_B4, Poly|Rotate, 3);
	//Optional Tetris
	g.resetConfig();
	g.generate(SWAMP_PURPLE, Poly, 5);
	//Negative Shapes 1
	g.resetConfig();
	g.setConfig(RequireCombineShapes);
	g.setConfig(DisableCancelShapes);
	g.setGridSize(3, 3);
	g.lineThickness = 0.7f;
	g.generate(SWAMP_NEG_A1, Poly, 2, Poly|Negative, 1);
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_NEG_A2, Poly, 2, Poly|Negative, 1);
	g.removeConfig(RequireCombineShapes);
	g.setConfig(BigShapes);
	g.generate(SWAMP_NEG_A3, Poly, 2, Poly|Negative, 1);
	g.generate(SWAMP_NEG_A4, Poly, 2, Poly|Negative, 1);
	g.removeConfig(BigShapes);
	g.setConfigOnce(DisconnectShapes);
	g.generate(SWAMP_NEG_A5, Poly, 2, Poly|Negative, 1);
	//Negative Shapes 2
	g.setConfigOnce(RequireCombineShapes);
	g.generate(SWAMP_NEG_B1, Poly, 2, Poly|Negative, 2);
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_NEG_B2, Poly, 2, Poly|Negative, 2);
	g.generate(SWAMP_NEG_B3, Poly, 3, Poly|Negative, 1);
	g.generate(SWAMP_NEG_B4, Poly, 3, Poly|Negative, 2);
	g.setConfigOnce(DisconnectShapes);
	g.setConfigOnce(BigShapes);
	g.generate(SWAMP_NEG_B5, Poly, 2, Poly|Negative, 1);
	//Negative Shapes 3
	g.resetConfig();
	g.setConfig(RequireCancelShapes);
	g.setGridSize(2, 3);
	g.lineThickness = 0.7f;
	g.generate(SWAMP_NEG_C1, Poly, 1, Poly|Negative, 2);
	g.setGridSize(4, 4);
	g.lineThickness = 0.65f;
	g.generate(SWAMP_NEG_C2, Poly, 3, Poly|Negative, 2);
	g.generate(SWAMP_NEG_C3, Poly, 3, Poly|Negative, 3);
	g.generate(SWAMP_NEG_C4, Poly, 3, Poly|Negative, 4);
	//Exit Shortcut
	g.resetConfig();
	g.generate(SWAMP_EXIT_1, Poly|Rotate, 2);
	special.generateAntiPuzzle(SWAMP_EXIT_2);
}

void PuzzleList::GenerateTreehouseN() {
	g.setLoadingData(L"Treehouse", 57);
	g.resetConfig();
	g.setConfig(WriteColors);
	g.setGridSize(2, 2);
	g.generate(TREEHOUSE_ENTER_2, Star|Orange, 4);
	g.setConfig(TreehouseLayout);
	//Yellow Bridge
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_YELLOW_1, Star|Orange, 4, Gap, 5);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_YELLOW_2, Star|Orange, 4, Gap, 10);
	g.generate(TREEHOUSE_YELLOW_3, Star|Orange, 4, Gap, 10);
	g.generate(TREEHOUSE_YELLOW_4, Star|Orange, 6, Gap, 5);
	g.generate(TREEHOUSE_YELLOW_5, Star|Orange, 6, Gap, 5);
	g.generate(TREEHOUSE_YELLOW_6, Star|Orange, 6, Gap, 5);
	g.generate(TREEHOUSE_YELLOW_7, Star|Orange, 8, Gap, 2);
	g.generate(TREEHOUSE_YELLOW_8, Star|Orange, 8, Gap, 2);
	g.generate(TREEHOUSE_YELLOW_9, Star|Orange, 8, Gap, 2);
	g.removeConfigOnce(TreehouseLayout);
	g.setGridSize(5, 5);
	g.generate(TREEHOUSE_YELLOW_DOOR, Star|Orange, 10, Gap, 3);
	//Pink Bridge 1
	g.setConfig(FullGaps);
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_PURPLE_A1, Star|Magenta, 4, Dot_Intersection, 4, Gap, 2);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_PURPLE_A2, Star|Magenta, 6, Dot_Intersection, 6, Gap, 3);
	g.generate(TREEHOUSE_PURPLE_A3, Star|Magenta, 6, Dot_Intersection, 6, Gap, 3);
	g.setGridSize(5, 5);
	g.lineThickness = 0.84f;
	g.generate(TREEHOUSE_PURPLE_A4, Star|Magenta, 6, Dot_Intersection, 9, Gap, 8);
	g.generate(TREEHOUSE_PURPLE_A5, Star|Magenta, 8, Dot_Intersection, 9, Gap, 5);
	g.removeConfig(FullGaps);
	g.lineThickness = 1;
	//Pink Bridge 2
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_PURPLE_B1, Star|Magenta, 4, Stone|Black, 2, Stone|White, 2);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_PURPLE_B2, Star|Magenta, 4, Stone|Black, 4, Stone|White, 4);
	g.generate(TREEHOUSE_PURPLE_B3, Star|Magenta, 6, Stone|Black, 3, Stone|White, 3);
	g.generate(TREEHOUSE_PURPLE_B4, Star|White, 4, Stone|Black, 4, Stone|Magenta, 4);
	g.removeConfig(WriteColors);
	g.setConfig(WriteColors);
	g.generate(TREEHOUSE_PURPLE_B5, Star|Orange, 6, Stone|Magenta, 3, Stone|Green, 3);
	g.setGridSize(5, 4);
	//generator->setConfigOnce(DisableReset);
	g.setConfigOnce(LongPath);
	g.generate(TREEHOUSE_PURPLE_B6, Star|Magenta, 6, Stone|Orange, 4, Stone|Green, 4);
	g.placeGaps(8);
	g.write(TREEHOUSE_PURPLE_B7);
	g.removeConfig(WriteColors);
	g.setConfig(WriteColors);
	//Orange Bridge 1		
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_ORANGE_L1, Star|Black, 2, Stone|Black, 2, Stone|White, 2);
	g.generate(TREEHOUSE_ORANGE_L2, Star|White, 2, Stone|Black, 2, Stone|White, 2);
	g.generate(TREEHOUSE_ORANGE_L3, Star|Black, 1, Stone|Black, 2, Stone|White, 2);
	g.generate(TREEHOUSE_ORANGE_L4, Star|Black, 1, Star|White, 1, Stone|Black, 1, Stone|White, 2);
	g.setGridSize(4, 4);
	g.removeConfig(WriteColors);
	g.setConfig(AlternateColors); //Black -> Green, White -> Pink, Purple -> White
	g.generate(TREEHOUSE_ORANGE_L5, Star|White, 1, Stone|Black, 2, Stone|White, 2, Stone|Purple, 2);
	g.generate(TREEHOUSE_ORANGE_L6, Star|White, 2, Stone|Black, 2, Stone|White, 2, Stone|Purple, 2);
	g.generate(TREEHOUSE_ORANGE_L7, Star|White, 3, Stone|Black, 2, Stone|White, 2, Stone|Purple, 2);
	g.generate(TREEHOUSE_ORANGE_L8, Star|White, 1, Star|Black, 1, Stone|White, 2, Stone|Black, 2);
	g.setGridSize(5, 5);
	g.setObstructions({ { 1, 2 },{ 1, 4 },{ 9, 2 },{ 9, 4 },{ 2, 1 },{ 4, 1 },{ 6, 1 },{ 8, 1 } });
	g.generate(TREEHOUSE_ORANGE_L9, Star|White, 2, Stone|Black, 4, Stone|White, 4);
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_ORANGE_L10, Star|Black, 2, Star|White, 2, Stone|Black, 2, Stone|White, 2);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_ORANGE_L11, Star|Black, 2, Star|White, 2, Stone|Black, 3, Stone|White, 3);
	g.removeConfig(AlternateColors);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_ORANGE_L12, Star|Black, 3, Star|White, 2, Stone|Black, 2, Stone|White, 2);
	g.generate(TREEHOUSE_ORANGE_L13, Star|Black, 3, Star|White, 3, Stone|Black, 2, Stone|White, 2);
	g.generate(TREEHOUSE_ORANGE_L14, Star|Black, 3, Star|White, 4, Stone|Black, 2, Stone|White, 2);
	g.generate(TREEHOUSE_ORANGE_L15, Star|Black, 4, Star|White, 4, Stone|Black, 2, Stone|White, 2);
	//Orange Bridge 2
	g.setConfig(WriteColors);
	g.setGridSize(3, 3);
	g.generate(TREEHOUSE_ORANGE_R1, Star|Orange, 4, Star|Magenta, 4, Gap, 2);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_ORANGE_R2, Star|Orange, 4, Star|Magenta, 4, Gap, 8);
	g.generate(TREEHOUSE_ORANGE_R3, Star|Orange, 6, Star|Magenta, 4, Gap, 6);
	g.setObstructions({ { 1, 2 },{ 1, 4 },{ 7, 2 },{ 7, 4 },{ 2, 1 },{ 4, 1 },{ 6, 1 } });
	g.generate(TREEHOUSE_ORANGE_R4, Star|Orange, 4, Star|Magenta, 4);
	g.generate(TREEHOUSE_ORANGE_R5, Star|Magenta, 6, Star|Orange, 6, Gap, 3);
	g.generate(TREEHOUSE_ORANGE_R6, Star|Magenta, 6, Star|Orange, 6, Gap, 3);
	g.generate(TREEHOUSE_ORANGE_R7, Star|Orange, 8, Star|Magenta, 4);
	g.generate(TREEHOUSE_ORANGE_R8, Star|Magenta, 8, Star|Orange, 6);
	g.generate(TREEHOUSE_ORANGE_R9, Star|Orange, 4, Star|Magenta, 2, Star|Green, 2, Gap, 8);
	g.setObstructions({ { 1, 2 },{ 1, 4 },{ 7, 2 },{ 7, 4 },{ 2, 1 },{ 4, 1 },{ 6, 1 } });
	g.setConfigOnce(FixBackground);
	g.generate(TREEHOUSE_ORANGE_R10, Star|Orange, 4, Star|Magenta, 4, Star|Green, 4);
	g.setGridSize(4, 3);
	g.generate(TREEHOUSE_ORANGE_R11, Star|Orange, 6, Star|Magenta, 4, Star|Green, 2);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_ORANGE_R12, Star|Orange, 6, Star|Magenta, 6, Star|Green, 4);
	//Green Bridge
	g.removeConfig(WriteColors);
	g.setConfig(AlternateColors);
	g.setGridSize(4, 4);
	g.generate(TREEHOUSE_GREEN_1, Poly|White, 2, Star|Black, 4);
	g.generate(TREEHOUSE_GREEN_2, Poly|White, 2, Star|Black, 4);
	g.generate(TREEHOUSE_GREEN_3, Poly|White, 1, Poly|Rotate|White, 1, Star|Black, 6);
	g.setGridSize(5, 5);
	g.setObstructions({ { { 1, 2 },{ 1, 4 },{ 9, 2 },{ 9, 4 },{ 2, 1 },{ 4, 1 },{ 6, 1 },{ 8, 1 } },
		{ { 1, 2 },{ 1, 4 },{ 1, 6 },{ 0, 7 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 7 },{ 4, 1 },{ 6, 1 },{ 8, 1 } },
		{ { 1, 2 },{ 1, 4 },{ 1, 6 },{ 0, 7 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 7 },{ 2, 1 },{ 4, 1 },{ 6, 1 } } });
	g.setConfigOnce(BigShapes);
	g.generate(TREEHOUSE_GREEN_4, Poly|Rotate|White, 1, Star|Black, 6);
	g.setGridSize(5, 5);
	g.generate(TREEHOUSE_GREEN_5, Poly|White, 2, Poly|Black, 1, Star|Black, 3);
	g.setGridSize(4, 4);
	g.setConfig(RequireCombineShapes);
	g.generate(TREEHOUSE_GREEN_6, Poly|White, 2, Poly|Negative|Black, 1, Star|Black, 5);
	g.setGridSize(5, 5);
	g.lineThickness = 0.8f;
	g.generate(TREEHOUSE_GREEN_7, Poly|White, 1, Poly|Rotate|White, 1, Star|Black, 4, Star|Purple, 4);
}

void PuzzleList::GenerateTownN() {
	g.setLoadingData(L"Town", 20);
	g.resetConfig();
	//Full Dots + Shapes
	g.generate(TOWN_DOT_1, Dot_Intersection, 25, Poly|Rotate, 1);
	g.generate(TOWN_DOT_2, Dot_Intersection, 25, Poly, 2);
	g.generate(TOWN_DOT_3, Dot_Intersection, 25, Poly|Rotate, 1, Poly, 1);
	g.generate(TOWN_DOT_4, Dot_Intersection, 25, Poly|Rotate, 2);
	g.generate(TOWN_DOT_5, Dot_Intersection, 25, Poly|Rotate, 2);
	g.generate(TOWN_DOT_FINAL, Dot_Intersection, 25, Poly|Rotate, 2, Eraser|White, 1);
	//Blue Symmetry
	g.setSymmetry(Rotational);
	g.generate(TOWN_BLUE_1, Stone|Black, 6, Stone|White, 6);
	g.generate(TOWN_BLUE_2, Stone|Black, 6, Stone|White, 6);
	g.generate(TOWN_BLUE_3, Stone|Black, 5, Stone|White, 5, Dot, 3);
	g.generate(TOWN_BLUE_4, Stone|Black, 5, Stone|White, 5, Dot, 3);
	g.generate(TOWN_BLUE_5, Stone|Black, 5, Stone|White, 5, Dot, 3);
	g.setSymmetry(NoSymmetry);
	//Glass Door
	g.setConfigOnce(SmallShapes);
	g.generate(TOWN_GLASS, Poly|Rotate, 4, Star|White, 6);
	//Church Star Door
	g.setConfig(DisableFlash);
	special.generateColorFilterPuzzle(TOWN_STAR, { 4, 4 }, { { Star|1, 6 },{ Star|2, 6 }, { Star|3, 4 } }, { 1, 1, 0, 0 }, colorblind);
	special.setPower(TOWN_LATTICE, false); (new TownDoorWatchdog())->start();
	//Soundproof Room
	std::vector<int> allPitches = { DOT_SMALL, DOT_SMALL, DOT_MEDIUM, DOT_MEDIUM, DOT_LARGE, DOT_LARGE };
	std::vector<int> pitches;
	for (int i = 0; i < 4; i++) pitches.push_back(Random::popRandom(allPitches));
	special.generateSoundDotPuzzle(TOWN_SOUND_DOT, { 4, 4 }, pitches, false);
	g.resetConfig();
	//3-color Room
	special.generateRGBStonePuzzleN(TOWN_RGB_L);
	special.generateRGBStarPuzzleN(TOWN_RGB_R);
	//Orange Crate
	g.generate(TOWN_CRATE, Poly|Orange, 3, Stone|White, 2, Stone|Black, 2);
	//Windmill Puzzles
	//The Witness has a weird issue with these particular puzzles where the edge at (3, 4) gets bypassed by the region calculator if you don't draw over it.
	//Because of this, polyominoes can't be used with these puzzles unless Config::PreserveStructure flag is turned off.
	g.setConfigOnce(PreserveStructure);
	g.generate(THEATER_ENTER, Stone|Black, 5, Stone|White, 5, Gap, 7);
	g.setConfig(FixBackground);
	g.generate(THEATER_EXIT_L, Stone|White, 8, Stone|Black, 8, Eraser|White, 2);
	g.setConfigOnce(WriteColors);
	g.generate(THEATER_EXIT_R, Poly|Orange, 2, Stone|White, 4, Stone|Black, 4, Gap, 4);
}

void PuzzleList::GenerateVaultsN() {
	g.setLoadingData(L"Vaults", 5);
	g.resetConfig();
	g.setConfig(DisableFlash);
	//Tutorial Vault
	g.generate(TUT_VAULT, Stone|White, 10, Stone|Black, 10, Dot, 10, Start, 4);
	//Tetris Vault
	g.setConfigOnce(RequireCancelShapes);
	g.generate(DESERT_VAULT, Dot_Intersection, 49, Poly, 1, Poly|Rotate, 2, Poly|Negative, 3);
	//Symmetry Vault
	g.setSymmetry(Rotational);
	g.generate(MOUNTAIN_VAULT, Stone|White, 4, Stone|Black, 4, Dot, 2, Dot|Blue, 2, Dot|Yellow, 2);
	g.resetConfig();
	//Shipwreck Vault
	special.generateSoundDotReflectionPuzzle(SHIPWRECK_VAULT, { 7, 7 }, { DOT_MEDIUM, DOT_LARGE, DOT_MEDIUM, DOT_SMALL }, { DOT_LARGE, DOT_SMALL, DOT_MEDIUM }, 3, false);
	g.resetConfig();
	//Jungle Vault
	special.generateJungleVault(JUNGLE_VAULT);
}

void PuzzleList::GenerateTrianglePanelsN() {
	g.setLoadingData(L"Triangles", 12);
	g.resetConfig();
	g.setGridSize(3, 3);
	g.setSymbol(Start, 0, 0);
	g.setSymbol(Start, 0, 6);
	g.setSymbol(Start, 6, 6);
	g.generate(SHIPWRECK_DISCARD, Triangle|Orange, 6);
	g.setGridSize(4, 4);
	g.generate(SYM_DISCARD, Triangle|Orange, 6, Gap, 2);
	g.generate(MILL_DISCARD, Triangle|Orange, 6, Gap, 2);
	g.generate(TREEHOUSE_GREEN_DISCARD, Triangle|Orange, 4, Gap, 6);
	g.generate(TREEHOUSE_ORANGE_DISCARD, Triangle|Orange, 6);
	g.generate(KEEP_DISCARD, Triangle|Orange, 4, Gap, 6);
	g.generate(TUT_DISCARD, Triangle|Orange, 7);
	g.generate(TOWN_CRATE_DISCARD, Triangle|Orange, 5, Gap, 4);
	g.generate(TOWN_ROOF_DISCARD, Triangle|Orange, 7);
	g.generate(THEATER_DISCARD, Triangle|Orange, 5, Gap, 4);
	g.generate(MOUNTAIN_OUTSIDE_DISCARD, Triangle|Orange, 8);
	g.generate(DESERT_DISCARD, Triangle|Orange, 8);
}

void PuzzleList::GenerateMountainN() {
	std::wstring text = L"Mountain Perspective";
	SetWindowText(_handle, text.c_str());
	special.generateMountaintop(MOUNTAIN_TOP, { { Stone|Black, 2 },{ Stone|White, 1, }, { Star|Black, 1, },{ Star|White, 1 } });
	
	g.setLoadingData(L"Mountain", 39);
	g.resetConfig();
	//Purple Bridge
	g.setConfigOnce(PreserveStructure);
	g.setConfigOnce(DecorationsOnly);
	std::set<Point> bpoints1 = { { 6, 3 },{ 5, 4 },{ 7, 4 } };
	std::set<Point> bpoints2 = { { 6, 5 },{ 5, 6 },{ 6, 7 },{ 7, 6 } };
	std::set<Point> bpoints3 = { { 3, 6 },{ 4, 7 } };
	if (Random::rand() % 2 == 0) g.hitPoints = { Random::pickRandom(bpoints1), Random::pickRandom(bpoints2), Random::pickRandom(bpoints3) };
	else g.hitPoints = { Random::pickRandom(bpoints3), Random::pickRandom(bpoints2), Random::pickRandom(bpoints1) };
	g.setObstructions({ { 4, 1 },{ 6, 1 },{ 8, 1 } });
	g.blockPos = { { 1, 1 },{ 11, 1 },{ 1, 11 },{ 11, 11 } };
	g.setSymbol(Gap_Row, 3, 4);
	g.generate(MOUNTAIN_PURPLE_BRIDGE, Stone|Black, 3, Stone|White, 3, Stone|Purple, 3, Eraser|White, 1);
	
	//Orange Row
	g.resetConfig();
	g.generate(MOUNTAIN_ORANGE_1, Dot, 4, Stone|Black, 2, Stone|White, 2, Star|Black, 2, Star|White, 2);
	g.generate(MOUNTAIN_ORANGE_2, Dot, 4, Stone|Black, 1, Stone|White, 3, Star|Black, 3, Star|White, 1);
	g.generate(MOUNTAIN_ORANGE_3, Poly, 2, Dot, 3);
	g.generate(MOUNTAIN_ORANGE_4, Poly|Rotate, 1, Poly, 2, Gap, 2);
	g.generate(MOUNTAIN_ORANGE_5, Stone|Black, 4, Stone|White, 4, Star|Purple, 4, Gap, 2);
	g.generate(MOUNTAIN_ORANGE_6, Poly, 1, Poly|Rotate, 1, Star|Magenta, 4);
	g.generate(MOUNTAIN_ORANGE_7, Dot, 8, Star|Magenta, 4);
	//Green Row
	g.generate(MOUNTAIN_GREEN_1, Dot, 6, Stone|Black, 3, Stone|White, 3);
	g.blockPos = { { 0, 0 },{ 3, 8 },{ 8, 3 },{ 0, 5 },{ 5, 0 },{ 8, 8 } };
	g.generate(MOUNTAIN_GREEN_2, Dot, 6, Stone|Black, 4, Stone|White, 2);
	g.blockPos = { { 0, 0 },{ 3, 8 },{ 8, 3 },{ 0, 5 },{ 5, 0 },{ 8, 8 } };
	g.generate(MOUNTAIN_GREEN_3, Dot, 4, Stone|Black, 3, Stone|White, 2, Poly, 1);
	g.blockPos = { { 0, 3 },{ 3, 0 },{ 5, 8 },{ 8, 5 } };
	g.generate(MOUNTAIN_GREEN_4, Dot, 6, Stone|Black, 3, Stone|White, 3);
	g.generate(MOUNTAIN_GREEN_5, Dot, 10, Stone|Black, 3, Stone|White, 2);
	//Purple Panels
	g.setConfig(RequireCombineShapes);
	g.generate(MOUNTAIN_PURPLE_1, Poly, 2, Stone|Black, 4, Stone|White, 3);
	g.generate(MOUNTAIN_PURPLE_2, Poly, 2, Stone|Black, 4, Stone|White, 4);
	g.resetConfig();
	//Blue Row
	g.setSymmetry(Rotational);
	g.setObstructions({ { 4, 3 },{ 5, 4 },{ 5, 6 },{ 5, 8 },{ 5, 10 },{ 6, 9 },{ 7, 10 } });
	g.generate(MOUNTAIN_BLUE_1, Dot, 3, Stone | Black, 4, Stone | White, 4);
	g.setSymmetry(NoSymmetry);
	if (Random::rand() % 2 == 0) g.setObstructions({ { 5, 4 },{ 5, 6 },{ 5, 8 },{ 5, 10 },{ 9, 4 },{ 9, 6 },{ 9, 8 },{ 9, 10 },{ 7, 0 },{ 7, 2 } });
	else g.setObstructions({ { 3, 4 },{ 3, 6 },{ 3, 8 },{ 3, 10 },{ 7, 4 },{ 7, 6 },{ 7, 8 },{ 7, 10 },{ 5, 0 },{ 5, 2 },{ 9, 0 },{ 9, 2 } });
	g.generate(MOUNTAIN_BLUE_2, Stone | Black, 7, Stone | White, 5, Star | Orange, 4);
	g.setSymmetry(Rotational);
	g.setObstructions({ { 0, 1 },{ 0, 3 },{ 0, 5 },{ 0, 7 },{ 9, 4 },{ 1, 4 },{ 1, 6 },{ 1, 8 },{ 2, 7 },{ 2, 9 },{ 3, 8 },{ 3, 10 },{ 4, 9 },{ 5, 8 },{ 5, 10 },
		{ 6, 7 },{ 6, 9 },{ 7, 6 },{ 7, 8 },{ 7, 10 },{ 8, 5 },{ 8, 7 },{ 8, 9 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 9, 8 },{ 10, 1 },{ 10, 3 },{ 10, 5 } });
	g.blockPos = { { 5, 0 }, { 6, 0 }, { 4, 2 }, { 5, 2 }, { 4, 3 }, { 3, 4 } };
	g.generate(MOUNTAIN_BLUE_3, Dot, 6);

	//Rainbow Row
	g.resetConfig();
	g.setConfig(WriteColors);
	g.generate(MOUNTAIN_RAINBOW_1, Stone|Magenta, 2, Stone|Green, 2, Star|Magenta, 2, Star|Green, 2);
	g.generate(MOUNTAIN_RAINBOW_2, Stone|Magenta, 2, Stone|Green, 2, Star|Magenta, 2, Star|Green, 3);
	g.generate(MOUNTAIN_RAINBOW_3, Stone|Cyan, 2, Stone|Yellow, 2, Star|Cyan, 3, Star|Yellow, 3);
	g.setGridSize(5, 5);
	g.generate(MOUNTAIN_RAINBOW_4, Stone|Cyan, 2, Stone|Magenta, 2, Star|Cyan, 2, Star|Magenta, 1, Poly|Cyan, 1, Poly|Magenta, 1);
	g.setConfigOnce(CyanAndYellowLines);
	g.setSymbol(Start, 10, 0);
	g.setSymbol(Start, 0, 10);
	g.setSymbol(Exit, 0, 0);
	g.setSymbol(Exit, 10, 10);
	g.setSymmetry(Rotational);
	g.generate(MOUNTAIN_RAINBOW_5, Dot_Intersection|Blue, 4, Dot_Intersection|Orange, 4);
	g.resetConfig();

	special.generateMultiPuzzle({ MOUNTAIN_MULTI_1, MOUNTAIN_MULTI_2, MOUNTAIN_MULTI_3, MOUNTAIN_MULTI_4, MOUNTAIN_MULTI_5, MOUNTAIN_MULTI_6 }, {
		{ { Dot_Intersection, 5 } },
		{ { Stone|Black, 2 },{ Stone|White, 2 } },
		{ { Star|Orange, 2 },{ Star|Magenta, 2 },{ Star|Green, 2 } },
		{ { Poly|Rotate, 1 } },
		{ { Stone|Cyan, 2 },{ Stone|Yellow, 1 },{ Star|Cyan, 1 },{ Star|Yellow, 1 } },
		{ { Poly, 2 } } }, false);

	special.generate2Bridge(MOUNTAIN_BLUE_BRIDGE, MOUNTAIN_ORANGE_BRIDGE);

	special.generateMountainFloor();

	//Pillar Puzzles
	g.setGridSize(6, 3);
	g.generate(MOUNTAIN_PILLAR_L1, Dot, 15, Gap, 6);
	g.resetConfig();
	g.setConfig(WriteColors);
	g.setConfig(DecorationsOnly);
	g.generate(MOUNTAIN_PILLAR_R1, Star|Orange, 6, Star|Magenta, 4);
	g.generate(MOUNTAIN_PILLAR_L2, Stone|Black, 4, Stone|White, 4); //TODO: Buff?
	g.removeConfig(DecorationsOnly);
	g.generate(MOUNTAIN_PILLAR_R2, Star|Orange, 4, Dot, 8);
	g.removeConfig(WriteColors);
	g.setConfigOnce(RequireCombineShapes);
	g.setConfigOnce(DecorationsOnly);
	g.generate(MOUNTAIN_PILLAR_L3, Poly, 3);
	g.setSymbol(Start, 4, 8);
	g.generate(MOUNTAIN_PILLAR_R3, Dot_Intersection, 30, Gap, 10);
	g.setConfigOnce(WriteColors);
	g.setSymmetry(PillarRotational);
	g.setConfigOnce(DecorationsOnly);
	g.generate(MOUNTAIN_PILLAR_L4, Star|Orange, 4, Stone|Black, 2, Stone|White, 2);
	g.setSymmetry(PillarParallel);
	g.setConfigOnce(DisableDotIntersection);
	g.generate(MOUNTAIN_PILLAR_R4, Dot, 8);
}

void PuzzleList::GenerateCavesN() {
	g.setLoadingData(L"Caves", 51);
	g.resetConfig();
	g.generate(MOUNTAIN_SECRET_DOOR, Triangle|Orange, 10);
	g.generate(CAVES_ENTER, Stone|Black, 4, Stone|White, 3, Triangle|Orange, 4);

	//Blue Symmetry/Tetris
	g.setSymmetry(Rotational);
	g.setConfig(RequireCombineShapes);
	g.setGridSize(5, 5);
	g.generate(CAVES_SHAPE_SYM1, Triangle|Orange, 6);
	g.generate(CAVES_SHAPE_SYM2, Poly, 3);
	g.resetConfig();
	g.setConfig(DisconnectShapes);
	g.setConfigOnce(BigShapes);
	g.generate(CAVES_SHAPE_DISJOINT, Poly, 3);
	g.generate(CAVES_SHAPE_NEG, Poly, 3, Poly|Negative, 1);
	g.resetConfig();

	//Triangle Puzzles
	g.generate(CAVES_SHAPE_R1, Dot, 25, Triangle|Orange, 4);
	g.generate(CAVES_SHAPE_R2, Dot, 25, Triangle|Orange, 5);
	g.generate(CAVES_SHAPE_R3, Dot, 25, Triangle|Orange, 5);
	g.generate(CAVES_SHAPE_R4, Dot, 25, Triangle|Orange, 6);
	g.resetConfig();

	g.generate(CAVES_SHAPE_L1, Stone|Black, 3, Stone|White, 3, Triangle|Orange, 4);
	g.generate(CAVES_SHAPE_L2, Star|Black, 4, Star|White, 4, Triangle|Orange, 4);
	g.generate(CAVES_SHAPE_L3, Star|Orange, 3, Triangle|Orange, 4);
	g.generate(CAVES_SHAPE_L4, Star|Black, 2, Star|White, 2, Stone|Black, 1, Stone|White, 1, Triangle|Orange, 4);
	g.generate(CAVES_SHAPE_L5, Star|Black, 2, Star|Orange, 2, Stone|Black, 1, Stone|Orange, 1, Triangle|Orange, 4);

	g.setGridSize(4, 4);
	g.generate(CAVES_SHAPE_B1, Poly|Rotate, 1, Triangle|Orange, 4);
	g.setConfigOnce(BigShapes);
	g.generate(CAVES_SHAPE_B2, Poly|Rotate, 1, Triangle|Orange, 3);
	g.setGridSize(5, 5);
	g.setConfigOnce(BigShapes);
	g.generate(CAVES_SHAPE_B3, Poly|Rotate, 1, Triangle|Orange, 6);
	g.generate(CAVES_SHAPE_B4, Poly, 2, Triangle|Orange, 5);
	g.setConfigOnce(BigShapes);
	g.generate(CAVES_SHAPE_B5, Poly, 2, Triangle|Orange, 4);
	g.resetConfig();

	//First alcove
	g.setConfigOnce(DisconnectShapes);
	g.generate(CAVES_SWAMP, Poly|Rotate, 3);
	g.setConfigOnce(WriteColors);
	g.generate(CAVES_TREEHOUSE, Star|Black, 4, Star|White, 4, Star|Orange, 4, Stone|Black, 1, Stone|White, 1);
	g.setConfigOnce(WriteColors);
	g.setConfigOnce(DisableFlash);
	g.generate(CAVES_RAINBOW, Stone|Cyan, 16, Stone|Yellow, 14);
	g.setConfigOnce(AlternateColors);
	g.generate(CAVES_QUARRY, Stone|White, 2, Stone|Black, 1, Star|White, 2, Star|Black, 3, Eraser|White, 1);
	g.resetConfig();

	//Perspective
	g.setConfig(DisableFlash);
	g.setConfig(DecorationsOnly);
	std::vector<std::vector<Point>> obstructions = { { { 5, 0 },{ 5, 2 },{ 5, 4 } },{ { 5, 6 },{ 5, 8 },{ 5, 10 } },{ { 0, 5 },{ 2, 5 },{ 4, 5 } },{ { 6, 5 },{ 8, 5 },{ 10, 5 } } };
	g.setObstructions(obstructions);
	g.blockPos = { { 5, 5 } };
	g.generate(CAVES_PERSPECTIVE_1, Poly, 3);
	g.setObstructions(obstructions);
	g.blockPos = { { 5, 5 } };
	g.generate(CAVES_PERSPECTIVE_2, Poly, 1, Poly|Rotate, 1, Stone|Black, 3, Stone|White, 3);
	g.setObstructions(obstructions);
	g.blockPos = { { 5, 5 } };
	g.generate(CAVES_PERSPECTIVE_3, Stone|Black, 4, Stone|White, 3, Star|Orange, 6);
	g.setObstructions(obstructions);
	g.blockPos = { { 5, 5 } };
	g.setConfigOnce(RequireCombineShapes);
	g.generate(CAVES_PERSPECTIVE_4, Poly, 2, Star|Black, 6, Star|White, 4);
	g.resetConfig();

	//Full Dots
	g.generate(CAVES_DOT_1, Dot_Intersection, 33);
	g.generate(CAVES_DOT_2, Dot_Intersection, 25, Star|Black, 6, Star|White, 2);
	g.generate(CAVES_DOT_3, Dot_Intersection, 25, Star|Black, 1, Star|White, 2, Stone|Black, 2, Stone|White, 1);
	g.generate(CAVES_DOT_4, Dot_Intersection, 25, Poly|Black, 3);
	g.generate(CAVES_DOT_5, Dot_Intersection, 25, Poly|Rotate|Black, 2);
	g.setSymbol(Start, 0, 0); g.setSymbol(Start, 6, 6); g.setSymbol(Start, 0, 6);
	g.generate(CAVES_DOT_6, Dot_Intersection, 16, Poly|Black, 2, Poly|Negative|Black, 1);
	g.resetConfig();

	//Invisible Dots
	g.generate(CAVES_INVISIBLE_1, Dot_Intersection, 2);
	g.generate(CAVES_INVISIBLE_2, Dot, 3);
	g.generate(CAVES_INVISIBLE_3, Dot, 6);
	g.generate(CAVES_INVISIBLE_4, Dot, 6);
	g.generate(CAVES_INVISIBLE_5, Dot, 9);
	g.generate(CAVES_INVISIBLE_6, Dot, 9);
	g.generate(CAVES_INVISIBLE_7, Dot, 9);
	g.generate(CAVES_INVISIBLE_8, Dot, 12);
	g.setSymmetry(Vertical);
	g.generate(CAVES_INVISIBLE_SYM1, Dot, 3);
	g.generate(CAVES_INVISIBLE_SYM2, Dot, 7);
	g.setSymmetry(Rotational);
	g.generate(CAVES_INVISIBLE_SYM3, Dot, 7);
	g.resetConfig();
	
	//Triangle Exit
	g.generate(CAVES_TRIANGLE_EXIT, Triangle1|Orange, 6);
	//Stars Exit
	g.generate(CAVES_STAR_EXIT, Triangle|Cyan, 2, Triangle|Yellow, 2, Star|Cyan, 2, Star|Yellow, 2);

	//Challenge Pillar
	g.generate(CAVES_PILLAR, Triangle|Orange, 10);
	//Challenge Entrance
	g.generate(CAVES_CHALLENGE_ENTER, Poly|Green, 4, Star|Green, 3);
	//Theater Exit
	g.generate(CAVES_THEATER_WALK, Triangle2|Orange, 12);
	//Town Exit
	g.generate(CAVES_TOWN_EXIT, Triangle|Orange, 12, Start, 3);
}

void PuzzleList::GenerateOrchardN() {
	special.generateApplePuzzle(APPLE_5, true, false);
}

void PuzzleList::GenerateDesertN() {
	Randomizer().RandomizeDesert();
}

void PuzzleList::GenerateKeepN() {
	g.setLoadingData(L"Keep", 5);
	g.resetConfig();

	g.setSymbol(Gap_Column, 8, 3);
	g.setSymbol(Gap_Column, 4, 5);
	g.setSymbol(Gap_Row, 3, 0);
	g.setSymbol(Gap_Row, 3, 2);
	g.setSymbol(Gap_Row, 5, 6);
	g.setConfigOnce(DisableWrite);
	g.setObstructions({ { 1, 4 },{ 2, 3 },{ 5, 4 },{ 5, 8 } });
	g.generate(KEEP_PRESSURE_1);
	std::set<Point> path1 = g.path;
	std::vector<std::vector<Point>> sets = { { { 7, 8 },{ 8, 7 },{ 7, 6 },{ 6, 7 } },{ { 6, 5 },{ 7, 4 },{ 8, 5 } },{ { 7, 0 },{ 7, 2 },{ 6, 1 },{ 8, 1 },{ 5, 2 } },
	{ { 2, 7 },{ 4, 7 },{ 3, 8 },{ 3, 6 },{ 1, 6 } },{ { 0, 1 },{ 1, 0 },{ 2, 1 },{ 1, 2 } } };
	for (std::vector<Point> set : sets) {
		Point p = g.pickRandom(set);
		while (!path1.count(p)) p = g.pickRandom(set);
		g.set(p, p.x % 2 == 0 ? Dot_Column : Dot_Row);
	}
	g.write(KEEP_PRESSURE_1);

	g.resetConfig();
	g.setObstructions({ { 3, 2 },{ 8, 5 } });
	g.setConfigOnce(DisableWrite);
	g.generate(KEEP_PRESSURE_2, Star|Black, 2, Star|White, 2, Stone|Black, 6, Stone|White, 6);
	std::set<Point> path2 = g.path;
	g.write(KEEP_PRESSURE_2);

	g.resetConfig();
	std::vector<std::vector<Point>> validHitPoints = {
		{ { 5, 8 },{ 3, 4 },{ 7, 2 },{ 3, 2 } },{ { 8, 7 },{ 7, 2 },{ 6, 7 },{ 5, 8 } },{ { 5, 8 },{ 4, 5 },{ 7, 2 },{ 2, 5 } },
	{ { 5, 8 },{ 3, 6 },{ 7, 2 },{ 3, 4 } },{ { 5, 8 },{ 1, 6 },{ 7, 2 },{ 1, 4 } },{ { 5, 8 },{ 4, 3 },{ 7, 2 },{ 2, 3 } },
	{ { 5, 8 },{ 3, 4 },{ 7, 2 },{ 3, 2 } },{ { 5, 8 },{ 1, 4 },{ 7, 2 },{ 1, 2 } },{ { 5, 8 },{ 3, 2 },{ 7, 2 },{ 3, 0 } },
	{ { 5, 8 },{ 1, 2 },{ 7, 2 },{ 1, 0 } } };
	g.hitPoints = validHitPoints[Random::rand() % validHitPoints.size()];
	g.setConfigOnce(DisableWrite);
	g.generate(KEEP_PRESSURE_3, Poly, 2, Stone|Black, 1, Stone|White, 1, Stone|Cyan, 1, Stone|Magenta, 1);
	std::set<Point> path3 = g.path;
	g.write(KEEP_PRESSURE_3);

	g.resetConfig();
	g.setSymmetry(Rotational);
	g.setConfigOnce(DisableWrite);
	g.generate(KEEP_PRESSURE_4, Poly, 3);
	if (g.panel.endpoints[0].x == 0) {
		std::swap(g.panel.endpoints[0], g.panel.endpoints[1]); //Need to have endpoints in right order to associate with pressure plates correctly
	}
	std::set<Point> path4 = (g.path1.count(Point(0, 8)) ? g.path2 : g.path1);
	if (g.path.count({ 7, 0 })) g.set(7, 0, Dot_Row);
	else g.set(8, 1, Dot_Column);
	g.write(KEEP_PRESSURE_4);

	special.generateKeepLaserPuzzle(KEEP_PRESSURE_LASER, path1, path2, path3, path4,
		{ { Dot, 4 },{ Star|Black, 4 },{ Star|White, 4 },
		{ Stone|Black, 7 },{ Stone|White, 5 },{ Stone|Cyan, 1 },
		{ Stone|Magenta, 1 },{ Poly, 5 } });

	special.clearTarget(KEEP_HEDGE_LASER); //Must solve pressure plate side
}

void PuzzleList::GenerateJungleN() {
	g.setLoadingData(L"Jungle", 4);
	g.resetConfig();
	special.generateSoundDotPuzzle(JUNGLE_DOT_1, { 2, 2 }, { DOT_SMALL, DOT_LARGE }, false);
	special.generateSoundDotPuzzle(JUNGLE_DOT_2, { 2, 2 }, { DOT_SMALL, DOT_LARGE }, false);
	special.generateSoundDotPuzzle(JUNGLE_DOT_3, { 4, 4 }, { DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_MEDIUM, DOT_LARGE }, false);
	if (Random::rand() % 2) special.generateSoundDotPuzzle(JUNGLE_DOT_4, { 4, 4 }, { DOT_SMALL, DOT_MEDIUM, DOT_SMALL, DOT_LARGE }, true);
	else special.generateSoundDotPuzzle(JUNGLE_DOT_4, { 4, 4 }, { DOT_LARGE, DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_LARGE }, true);
	if (Random::rand() % 2) special.generateSoundDotPuzzle(JUNGLE_DOT_5, { 4, 4 }, { DOT_SMALL, DOT_SMALL, DOT_LARGE, DOT_MEDIUM, DOT_LARGE }, true);
	else special.generateSoundDotPuzzle(JUNGLE_DOT_5, { 4, 4 }, { DOT_MEDIUM, DOT_MEDIUM, DOT_SMALL, DOT_MEDIUM, DOT_LARGE }, true);
	if (Random::rand() % 2) special.generateSoundDotPuzzle(JUNGLE_DOT_6, { 4, 4 }, { DOT_SMALL, DOT_LARGE, DOT_SMALL, DOT_LARGE, DOT_MEDIUM }, true);
	else special.generateSoundDotPuzzle(JUNGLE_DOT_6, { 4, 4 }, { DOT_LARGE, DOT_MEDIUM, DOT_SMALL, DOT_LARGE, DOT_SMALL }, true);
}


