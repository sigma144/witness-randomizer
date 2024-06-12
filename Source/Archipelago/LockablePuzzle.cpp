#include "LockablePuzzle.h"
#include "../Memory.h"
#include "../Panel.h"
#include "../Randomizer.h"
#include "APGameData.h"
#include "APState.h"
#include "SkipSpecialCases.h"
#include "../Special.h"
#include "PanelRestore.h"
#include "ASMPayloadManager.h"

void LockablePuzzle::Read() {}
void LockablePuzzle::UpdateLock(APState state) {}
void LockablePuzzle::Restore() {}

void LockablePanel::addMissingSymbolsDisplay(std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, int id) {
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

void LockablePanel::createCenteredText(std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags,
	std::vector<int>& connectionsA, std::vector<int>& connectionsB, float top, float bottom) {

	float left, right;

	if (text.size() > 20)
		text.erase(20);

	if (text.size() > 12)
		createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.03f, 0.97f, top, bottom);
	else if (text.size() > 6)
		createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.3f, 0.7f, top, bottom);
	else
		createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.4f, 0.6f, top, bottom);
}

void LockablePanel::createText(std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags,
	std::vector<int>& connectionsA, std::vector<int>& connectionsB, float left, float right, float top, float bottom) {

	int currentIntersections = intersections.size();

	Special::createText(id, text, intersections, connectionsA, connectionsB, left, right, top, bottom);

	int newIntersections = intersections.size();

	for (int i = 0; i < (newIntersections - currentIntersections) / 2; i++)
		intersectionFlags.emplace_back(0);
}

int LockablePanel::addPuzzleSymbols(const APState& state,
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

	if (hasStones && !state.unlockedStones) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);;
		gridDecorations[position] = Decoration::Stone | Decoration::Color::White;
		i++; // 1
	}

	if (hasColoredStones && !state.unlockedColoredStones) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Stone | Decoration::Color::Orange;
		i++; // 2
	}

	if (hasStars && !state.unlockedStars) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Star | Decoration::Color::Green;

		i++; // 3
	}

	if (hasStarsWithOtherSymbol && !state.unlockedStarsWithOtherSimbol) {
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

	if (hasErasers && !state.unlockedErasers) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Eraser;
		i++; // 5
	}

	if (hasTriangles && !state.unlockedTriangles) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Triangle2 | Decoration::Yellow;
		i++; // 6
	}

	const int defaultLShape = 0x00170000;

	if (hasTetris && !state.unlockedTetris) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Poly | Decoration::Color::Yellow | defaultLShape;
		i++; // 7
	}

	if (hasTetrisRotated && !state.unlockedTetrisRotated) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Poly | Decoration::Color::Yellow | defaultLShape | Decoration::Can_Rotate;
		i++; // 8 - Careful from here on!
	}

	if (hasTetrisNegative && !state.unlockedTetrisNegative && i != gridDecorations.size()) {
		position = (1 - (i % 2)) * 4 + ((int)i / 2);
		gridDecorations[position] = Decoration::Poly | Decoration::Color::Blue | defaultLShape | Decoration::Negative;
		i++;
	}

	if (hasSymmetry && !state.unlockedSymmetry && i != gridDecorations.size()) {
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

	if (hasArrows && !state.unlockedArrows && i != gridDecorations.size()) {
		int intersectionOffset = intersectionFlags.size();

		position = (1 - (i % 2)) * 4 + ((int)i / 2);

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

	if (state.keysInTheGame.count(id) && !state.keysReceived.count(id)) {
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

	if (hasColoredDots && !state.unlockedColoredDots) {
		intersectionFlags[11] = IntersectionFlags::DOT | IntersectionFlags::DOT_IS_BLUE;
		intersectionFlags[12] = IntersectionFlags::DOT | IntersectionFlags::DOT_IS_ORANGE;

		if (hasDots && !state.unlockedDots)
			intersectionFlags[13] = IntersectionFlags::DOT;
		else if (hasSoundDots && !state.unlockedSoundDots) {
			intersectionFlags[6] = IntersectionFlags::DOT | IntersectionFlags::DOT_SMALL;
			intersectionFlags[7] = IntersectionFlags::DOT | IntersectionFlags::DOT_MEDIUM;
			intersectionFlags[8] = IntersectionFlags::DOT | IntersectionFlags::DOT_LARGE;
		}
	}

	if (hasFullDots && !state.unlockedFullDots) {
		for (int i = 0; i < 15; i++) {
			intersectionFlags[i] |= IntersectionFlags::DOT;
		}
	}
	else if (hasSoundDots && !state.unlockedSoundDots) {
		intersectionFlags[11] = IntersectionFlags::DOT | IntersectionFlags::DOT_SMALL;
		intersectionFlags[12] = IntersectionFlags::DOT | IntersectionFlags::DOT_MEDIUM;
		intersectionFlags[13] = IntersectionFlags::DOT | IntersectionFlags::DOT_LARGE;
	}
	else if (hasDots && !state.unlockedDots) {
		if (id == 0x09EEB) intersectionFlags[0] = IntersectionFlags::DOT;
		else intersectionFlags[11] = IntersectionFlags::DOT;
	}

	decorations.insert(decorations.begin(), gridDecorations.begin(), gridDecorations.end());
	decorationsFlags.insert(decorationsFlags.begin(), gridDecorationsFlags.begin(), gridDecorationsFlags.end());

	return extraRows;
}

void LockablePanel::Read() {
	Memory* memory = Memory::get();

	grid_size_x = memory->ReadPanelData<int>(id, GRID_SIZE_X);
	grid_size_y = memory->ReadPanelData<int>(id, GRID_SIZE_Y);
	path_width_scale = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);
	pattern_scale = memory->ReadPanelData<float>(id, PATTERN_SCALE);

	int numberOfDots = memory->ReadPanelData<int>(id, NUM_DOTS);
	dot_positions = memory->ReadArray<float>(id, DOT_POSITIONS, numberOfDots * 2);
	dot_flags = memory->ReadArray<int>(id, DOT_FLAGS, numberOfDots);

	int numberOfConnections = memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	dot_connections_a = memory->ReadArray<int>(id, DOT_CONNECTION_A, numberOfConnections);
	dot_connections_b = memory->ReadArray<int>(id, DOT_CONNECTION_B, numberOfConnections);

	int numberOfDecorations = memory->ReadPanelData<int>(id, NUM_DECORATIONS);
	decorations = memory->ReadArray<int>(id, DECORATIONS, numberOfDecorations);
	decoration_flags = memory->ReadArray<int>(id, DECORATION_FLAGS, numberOfDecorations);

	int numberColoredRegions = memory->ReadPanelData<int>(id, NUM_COLORED_REGIONS);
	colored_regions = memory->ReadArray<int>(id, COLORED_REGIONS, numberColoredRegions * 4);

	outer_background_mode = memory->ReadPanelData<int>(id, OUTER_BACKGROUND_MODE);
	outer_background_color = memory->ReadPanelData<float>(id, OUTER_BACKGROUND, 4);
	background_region_color = memory->ReadPanelData<float>(id, BACKGROUND_REGION_COLOR, 4);
	path_color = memory->ReadPanelData<float>(id, PATH_COLOR, 4);
	decorationsColorsPointer = memory->ReadPanelData<__int64>(id, DECORATION_COLORS);

	if (id == 0x01983) {
		hasTetris = true;
		hasStars = true;
		return;
	}
	if (id == 0x01987) {
		hasDots = true;
		hasColoredStones = true;
		return;
	}

	std::vector<float> decorationColors = {};

	if (decorationsColorsPointer) {
		decorationColors = memory->ReadArray<float>(id, DECORATION_COLORS, 4 * numberOfDecorations);
	}

	for (int i = 0; i < numberOfDecorations; i++) {
		if ((decorations[i] & 0x700) == Decoration::Shape::Stone) {
			if (decorationsColorsPointer) {
				std::vector<float> color = { decorationColors[4 * i], decorationColors[4 * i + 1], decorationColors[4 * i + 2], decorationColors[4 * i + 3] };

				if (color[0] == color[1] && color[1] == color[2]) {
					hasStones = true;
				}
				else {
					hasColoredStones = true;
				}
			}
			else if (memory->ReadPanelData<int>(id, PUSH_SYMBOL_COLORS) == 1) { // Quarry type: Symbol_A through Symbol_E are relevant
				std::vector<float> color;
				switch (decorations[i] & 0x00F) {
				case 1:
					color = memory->ReadPanelData<float>(id, SYMBOL_A, 3);
					break;
				case 2:
					color = memory->ReadPanelData<float>(id, SYMBOL_B, 3);
					break;
				case 3:
					color = { 0.0f, 1.0f, 1.0f };
					break;
				case 4:
					color = memory->ReadPanelData<float>(id, SYMBOL_C, 3);
					break;
				case 5:
					color = memory->ReadPanelData<float>(id, SYMBOL_D, 3);
					break;
				case 6:
					color = memory->ReadPanelData<float>(id, SYMBOL_E, 3);
					break;
				default:
					color = { 0.0f, 1.0f, 1.0f };
				}

				if (color[0] == color[1] && color[1] == color[2] || (color[0] < 0.035 && color[1] < 0.05 && color[2] < 0.065)) { // vanilla left pillar 2 lol
					hasStones = true;
				}
				else {
					hasColoredStones = true;
				}
			}
			else {
				if ((decorations[i] & 0x00F) != Decoration::Color::White && (decorations[i] & 0x00F) != Decoration::Color::Black)
					hasColoredStones = true;
				else
					hasStones = true;
			}
		}
		else if ((decorations[i] & 0xF00) == Decoration::Shape::Star) {
			hasStars = true;

			bool sharesColorWithOtherShape = false;

			for (int j = 0; j < numberOfDecorations; j++) {
				if (((decorations[j] & 0xF00) != 0) //check if any other simbol shares its color
					&& ((decorations[j] & 0xF00) != Decoration::Shape::Star)
					&& ((decorations[j] & 0x00F) == (decorations[i] & 0x00F))) {

					sharesColorWithOtherShape = true;
					break;
				}
			}

			if (sharesColorWithOtherShape)
				hasStarsWithOtherSymbol = true;
		}
		else if ((decorations[i] & 0x700) == Decoration::Shape::Poly) {
			bool isRotated = (decorations[i] & 0x1000) == Decoration::Can_Rotate;

			if ((decorations[i] & 0x2000) == Decoration::Negative)
				hasTetrisNegative = true;
			else if (!isRotated)
				hasTetris = true;

			if (isRotated)
				hasTetrisRotated = true;
		}
		else if ((decorations[i] & 0x700) == Decoration::Shape::Eraser) {
			hasErasers = true;
		}
		else if ((decorations[i] & 0x700) == Decoration::Shape::Triangle && (decorations[i] & 0x70000) != 0) { //triangle with size 0 are used by arrows
			hasTriangles = true;
		}
		else if ((decorations[i] & 0x700) == Decoration::Shape::Arrow) {
			hasArrows = true;
		}
	}

	if (memory->ReadPanelData<int>(id, REFLECTION_DATA))
		hasSymmetry = true;

	int dotAmount = 0;
	int pointsToConsider = 0;

	for (int i = 0; i < numberOfDots; i++) {
		if (dot_flags[i] & DOT) {
			if (dot_flags[i] & DOT_IS_BLUE || dot_flags[i] & DOT_IS_ORANGE)
				hasColoredDots = true;
			else if (dot_flags[i] & DOT_SMALL || dot_flags[i] & DOT_MEDIUM || dot_flags[i] & DOT_LARGE)
				hasSoundDots = true;
			else
				hasDots = true;
		}

		if (dot_flags[i] & GAP || dot_flags[i] & STARTPOINT || dot_flags[i] & ENDPOINT || dot_flags[i] & NO_POINT) {
			continue;
		}
		if ((dot_flags[i] & ROW) && !(dot_flags[i] & COLUMN) || (dot_flags[i] & COLUMN) && !(dot_flags[i] & ROW)) {
			continue;
		}

		pointsToConsider++;

		if (dot_flags[i] & DOT) {
			dotAmount++;
		}
	}

	if (pointsToConsider && ((float)dotAmount / (float)pointsToConsider >= 0.9)) {
		hasFullDots = true;
	}

	else if (id == 0x00A52 || id == 0x00A61 || id == 0x00A57 || id == 0x00A64 || id == 0x00A5B || id == 0x00A68) // These dots seem to get picked up as normal dots, not colored ones. Also, these panels should require Symmetry despite not having Symmetry on them.
	{
		hasDots = false;
		hasColoredDots = true;
		hasSymmetry = true;
	}
	else if (id == 0x15ADD) { // River Vault: The squares are drawn on and validated separately!
		hasStones = true;
	}
	else if (id == 0x0A332) { // Challenge timer (can't lock the individual challenge panels)
		hasDots = true;
		hasColoredDots = true;
		hasStones = true;
		hasColoredStones = true;
		hasTriangles = true;
		hasSymmetry = true;
		hasStars = true;
		hasTetris = true;
		needsChallengeLasers = true;
	}
	else if (id == 0x3D9A9) { // Elevator
		needsMountainLasers = true;
	}
	else if (id == 0x09FDC || id == 0x09FF7 || id == 0x09F82 || id == 0x09FF8 || id == 0x0A01B || id == 0x0A01F || id == 0x17E67 || id == 0x0A079) { // Bunker Panels don't pick up their white squares
		hasColoredStones = true;
		hasStones = true;
	}
	else if (id == 0x09F7D || id == 0x09D9F || id == 0x09DA1 || id == 0x09DA2 || id == 0x09DAF || id == 0x0A010 || id == 0x17E63) {
		hasColoredStones = true;
		hasStones = false;
	}
}

void LockablePanel::UpdateLock(APState state) {
	auto memory = Memory::get();

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

	bool puzzleIsMissingSymbols = ((hasStones && !state.unlockedStones)
		|| (hasColoredStones && !state.unlockedColoredStones)
		|| (hasStars && !state.unlockedStars)
		|| (hasStarsWithOtherSymbol && !state.unlockedStarsWithOtherSimbol)
		|| (hasTetris && !state.unlockedTetris)
		|| (hasTetrisRotated && !state.unlockedTetrisRotated)
		|| (hasTetrisNegative && !state.unlockedTetrisNegative)
		|| (hasErasers && !state.unlockedErasers)
		|| (hasTriangles && !state.unlockedTriangles)
		|| (hasDots && !state.unlockedDots)
		|| (hasColoredDots && !state.unlockedColoredDots)
		|| (hasFullDots && !state.unlockedFullDots)
		|| (hasSoundDots && !state.unlockedSoundDots)
		|| (hasArrows && !state.unlockedArrows)
		|| (hasSymmetry && !state.unlockedSymmetry)
		|| (needsChallengeLasers && state.activeLasers < state.requiredChallengeLasers)
		|| (needsMountainLasers && state.activeLasers < state.requiredMountainLasers));

	bool puzzleIsMissingKey = state.keysInTheGame.count(id) && !state.keysReceived.count(id);

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

		createText(laserText2, intersections, intersectionFlags, connectionsA, connectionsB, 0.515f - laserText2.size() * 0.029f, 0.515f + laserText2.size() * 0.029f, 0.38f, 0.47f);
		createText(laserText, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f - laserText.size() * 0.029f, 0.5f + laserText.size() * 0.029f, 0.53f, 0.62f);
	}
	else if (id == 0x09D9B) { // Monastery Shutters
		std::string text1 = "Needs Dots   Needs Dots   Needs Dots";
		std::string text2 = "Needs Dots   Needs Dots   Needs Dots";

		if (puzzleIsMissingKey) {
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
			createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f - text.size() * 0.01f, 0.5f + text.size() * 0.01f, 0.915f, 0.943f);
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
	else if (very_thin_panels.count(id) && id != 0x0A332) {
		float yOffset = 0.0f;

		if (id == 0x34BC5) yOffset = 0.25f;
		if (id == 0x34BC6) yOffset = -0.25f;

		int currentIntersections = intersections.size();

		std::string text = "Locked";

		if (id == 0x09F86) {
			createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f - text.size() * 0.037f, 0.5f + text.size() * 0.037f, 0.545f + yOffset, 0.455f + yOffset);
		}
		else {
			createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f + text.size() * 0.037f, 0.5f - text.size() * 0.037f, 0.455f + yOffset, 0.545f + yOffset);
		}

		int newIntersections = intersections.size();

		for (int i = currentIntersections; i < newIntersections; i += 2) {
			float x = intersections[i];
			intersections[i] = intersections[i + 1];
			intersections[i + 1] = x;
		}
	}
	else if (fairly_thin_panels.count(id) || id == 0x09EEB) {
		addMissingSymbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB, id);
		extraRows = addPuzzleSymbols(state, intersections, intersectionFlags, connectionsA, connectionsB, decorations, decorationsFlags, polygons, id);
		memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
		memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 + extraRows });
	}
	else if (wide_panels.count(id)) {
		if (puzzleIsMissingSymbols) {
			addMissingSymbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB, id);
			extraRows = addPuzzleSymbols(state, intersections, intersectionFlags, connectionsA, connectionsB, decorations, decorationsFlags, polygons, id);
			memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
			memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 + extraRows });
		}
		else
		{
			std::string text = "Locked";
			createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.5f - text.size() * 0.037f, 0.5f + text.size() * 0.037f, 0.455f, 0.545f);

			std::vector<int> decorations(memory->ReadPanelData<int>(id, NUM_DECORATIONS), 0);
			std::vector<int> decorationsFlags(memory->ReadPanelData<int>(id, NUM_DECORATIONS), 0);

			memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
			memory->WriteArray<int>(id, DECORATIONS, decorations);
			memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);
		}
	}
	else {
		addMissingSymbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB, id);

		extraRows = addPuzzleSymbols(state, intersections, intersectionFlags, connectionsA, connectionsB, decorations, decorationsFlags, polygons, id);

		if (id == 0x0A332) {
			int currentIntersections = intersections.size();

			std::string laserText = std::to_string(state.activeLasers);
			laserText += "/";
			laserText += std::to_string(state.requiredChallengeLasers);

			createText(laserText, intersections, intersectionFlags, connectionsA, connectionsB, 0.31f, 0.05f, 0.42f, 0.58f);

			int newIntersections = intersections.size();

			for (int i = currentIntersections; i < newIntersections; i += 2) {
				float x = intersections[i];
				intersections[i] = intersections[i + 1];
				intersections[i + 1] = x;
			}
		}
		else
		{
			createText(text, intersections, intersectionFlags, connectionsA, connectionsB, 0.1f, 0.9f, 0.1f, 0.4f);
		}

		memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
		memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 + extraRows });
	}

	memory->WritePanelData<float>(id, PATTERN_SCALE, { pattern_scale / path_width_scale });
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

void LockablePanel::Restore() {
	Memory* memory = Memory::get();

	if (id == 0x01983) {
		while (memory->ReadPanelData<int>(0x0C179, 0x1d4)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		memory->WritePanelData<float>(0x0C179, 0xE0, { 0.2300000042f });
		memory->WritePanelData<float>(id, MAX_BROADCAST_DISTANCE, { -1.0f });

		return;
	}

	else if (id == 0x01987) {
		while (memory->ReadPanelData<int>(0x0C19D, 0x1d4)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		memory->WritePanelData<float>(0x0C19D, 0xE0, { 0.2300000042f });
		memory->WritePanelData<float>(id, MAX_BROADCAST_DISTANCE, { -1.0f });

		return;
	}

	memory->WritePanelData<int>(id, GRID_SIZE_X, { grid_size_x });
	memory->WritePanelData<int>(id, GRID_SIZE_Y, { grid_size_y });
	memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { path_width_scale });
	memory->WritePanelData<float>(id, PATTERN_SCALE, { pattern_scale });
	memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(dot_flags.size()) }); //amount of intersections
	memory->WriteArray<float>(id, DOT_POSITIONS, dot_positions); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
	memory->WriteArray<int>(id, DOT_FLAGS, dot_flags); //flags for each point such as entrance or exit
	memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(dot_connections_a.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
	memory->WriteArray<int>(id, DOT_CONNECTION_A, dot_connections_a); //start of a connection between points, contains position of point in sourceIntersectionFlags
	memory->WriteArray<int>(id, DOT_CONNECTION_B, dot_connections_b); //end of a connection between points, contains position of point in sourceIntersectionFlags
	memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
	memory->WriteArray<int>(id, DECORATIONS, decorations);
	memory->WriteArray<int>(id, DECORATION_FLAGS, decoration_flags);
	memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(colored_regions.size() / 4) });
	memory->WriteArray<int>(id, COLORED_REGIONS, colored_regions, true);
	memory->WritePanelData<float>(id, OUTER_BACKGROUND, outer_background_color);
	memory->WritePanelData<int>(id, OUTER_BACKGROUND_MODE, { outer_background_mode });
	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
	memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, background_region_color);
	memory->WritePanelData<float>(id, PATH_COLOR, path_color);
	memory->WritePanelData<__int64>(id, DECORATION_COLORS, { decorationsColorsPointer });

	memory->UpdatePanelJunctions(id);
}

void LockableEP::Read(){
	// nothing. All restore data for EPs is taken from PanelRestore.h
}

void LockableEP::UpdateLock(APState state)
{
	if (!state.keysInTheGame.count(id) || state.keysReceived.count(id)) return;

	LockEP(false, true, true);
}

void LockableEP::LockEP(bool disabled, bool recolor, bool disableEndPoint) {
	auto memory = Memory::get();

	int startPointID = EPtoStartPoint.find(id)->second;

	// Disable Endpoints
	int endPoint = EPtoEndPoint.find(id)->second;

	if (startPointID == 0x1AE8 || startPointID == 0x01C0D || startPointID == 0x01DC7 || startPointID == 0x01E12 || startPointID == 0x01E52) { // Pressure Plate 
		endPoint = memory->ReadPanelData<int>(endPoint, PRESSURE_PLATE_PATTERN_POINT_ID) - 1;
	}

	if (disableEndPoint && endPoint != -1) memory->WritePanelData<int>(endPoint, EP_PATTERN_POINT_IS_ENDPOINT, { 0 });

	if (!recolor) return;

	// Recolor stuff
	if (startPointID == 0x1AE8 || startPointID == 0x01C0D || startPointID == 0x01DC7 || startPointID == 0x01E12 || startPointID == 0x01E52) { // Pressure Plate 
		startPointID = memory->ReadPanelData<int>(startPointID, PRESSURE_PLATE_PATTERN_POINT_ID) - 1;
		if (startPointID == -1) return;
	}

	// Color the trail red / yellow
	
	std::vector<float> colors = { 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, };
	if (disabled) colors = { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, };
	

	memory->WritePanelData<float>(startPointID, EP_TRAIL_COLORS, colors);
	memory->WritePanelData<float>(startPointID, EP_PARTICLE_SIZE_SCALE, 2.0f);
}

void LockableEP::Restore(){
	auto memory = Memory::get();

	// Reenable Endpoints
	int startPointID = EPtoStartPoint.find(id)->second;
	int endPoint = EPtoEndPoint.find(id)->second;
	if (endPoint == 0x018f4 || endPoint == 0x018DA || endPoint == 0x17DF || endPoint == 0x18E7 || endPoint == 0x1786) { // Pressure Plate 
		endPoint = memory->ReadPanelData<int>(endPoint, PRESSURE_PLATE_PATTERN_POINT_ID) - 1;
		if (endPoint != -1)	memory->WritePanelData<int>(endPoint, EP_PATTERN_POINT_IS_ENDPOINT, { 1 });
	}
	else {
		memory->WritePanelData<int>(endPoint, EP_PATTERN_POINT_IS_ENDPOINT, { 1 });
	}

	// Restore Trail Color
	std::vector<float> colors = PanelRestore::GetEPColors(startPointID);
	float particleSize = PanelRestore::GetEPParticleSize(startPointID);

	if (startPointID == 0x1AE8 || startPointID == 0x01C0D || startPointID == 0x01DC7 || startPointID == 0x01E12 || startPointID == 0x01E52) { // Pressure Plate 
		startPointID = memory->ReadPanelData<int>(startPointID, PRESSURE_PLATE_PATTERN_POINT_ID) - 1;
		if (startPointID == -1) return;
	}

	memory->WritePanelData<float>(startPointID, EP_TRAIL_COLORS, colors);

	memory->WritePanelData<float>(startPointID, EP_PARTICLE_SIZE_SCALE, particleSize);
}

void LockableEP::DisableEP(bool recolor, bool disableEndPoint)
{
	LockEP(true, recolor, disableEndPoint);
}

void LockableObelisk::Read()
{
	// nothing to do here
	return;
}

void LockableObelisk::UpdateLock(APState state)
{	
	if (!state.keysInTheGame.count(id) || state.keysReceived.count(id)) return;

	auto memory = Memory::get();

	memory->WritePanelData<float>(id, SCALE, { 2.0f });
	memory->WritePanelData<float>(id, ORIENTATION, { 0.0f, 0.0f, 0.7f, 0.2f });
	ASMPayloadManager::get()->UpdateEntityPosition(id);
}

void LockableObelisk::Restore()
{
	auto memory = Memory::get();

	memory->WritePanelData<float>(id, SCALE, { 1.0f });
	memory->WritePanelData<float>(id, ORIENTATION, PanelRestore::GetObeliskOrientation(id));
	ASMPayloadManager::get()->UpdateEntityPosition(id);
}
