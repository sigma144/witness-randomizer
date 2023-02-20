#include "PuzzleData.h"

void PuzzleData::Read(std::shared_ptr<Memory> _memory) {
	grid_size_x = _memory->ReadPanelData<int>(id, GRID_SIZE_X);
	grid_size_y = _memory->ReadPanelData<int>(id, GRID_SIZE_Y);
	path_width_scale = _memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);
	pattern_scale = _memory->ReadPanelData<float>(id, PATTERN_SCALE);
	
	int numberOfDots = _memory->ReadPanelData<int>(id, NUM_DOTS);
	dot_positions = _memory->ReadArray<float>(id, DOT_POSITIONS, numberOfDots * 2);
	dot_flags = _memory->ReadArray<int>(id, DOT_FLAGS, numberOfDots);

	int numberOfConnections = _memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	dot_connections_a = _memory->ReadArray<int>(id, DOT_CONNECTION_A, numberOfConnections);
	dot_connections_b = _memory->ReadArray<int>(id, DOT_CONNECTION_B, numberOfConnections);
	
	int numberOfDecorations = _memory->ReadPanelData<int>(id, NUM_DECORATIONS);
	decorations = _memory->ReadArray<int>(id, DECORATIONS, numberOfDecorations);
	decoration_flags = _memory->ReadArray<int>(id, DECORATION_FLAGS, numberOfDecorations);

	int numberColoredRegions = _memory->ReadPanelData<int>(id, NUM_COLORED_REGIONS);
	colored_regions = _memory->ReadArray<int>(id, COLORED_REGIONS, numberColoredRegions * 4);

	outer_background_mode = _memory->ReadPanelData<int>(id, OUTER_BACKGROUND_MODE);
	outer_background_color = _memory->ReadPanelData<float>(id, OUTER_BACKGROUND, 4);
	background_region_color = _memory->ReadPanelData<float>(id, BACKGROUND_REGION_COLOR, 4);
	path_color = _memory->ReadPanelData<float>(id, PATH_COLOR, 4);
	decorationsColorsPointer = _memory->ReadPanelData<__int64>(id, DECORATION_COLORS);


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

	for (int i = 0; i < numberOfDecorations; i++) {
		if ((decorations[i] & 0x700) == Decoration::Shape::Stone) {
			if ((decorations[i] & 0x00F) != Decoration::Color::White && (decorations[i] & 0x00F) != Decoration::Color::Black)
				hasColoredStones = true;
			else
				hasStones = true;
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
			if ((decorations[i] & 0x2000) == Decoration::Negative)
				hasTetrisNegative = true;
			else
				hasTetris = true;

			if ((decorations[i] & 0x1000) == Decoration::Can_Rotate)
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

  	if (_memory->ReadPanelData<int>(id, REFLECTION_DATA))
		hasSymmetry = true;

	int dotAmount = 0;
	int pointsToConsider = numberOfDots;

	for (int i = 0; i < numberOfDots; i++)	{
		if (dot_flags[i] & GAP || dot_flags[i] & STARTPOINT || dot_flags[i] & ENDPOINT || dot_flags[i] & NO_POINT) {
			pointsToConsider -= 1;
		}

		if (dot_flags[i] & DOT) {
			if (!(dot_flags[i] & STARTPOINT || dot_flags[i] & ENDPOINT || dot_flags[i] & NO_POINT)) {
				dotAmount++;
			}

			if (dot_flags[i] & DOT_IS_BLUE || dot_flags[i] & DOT_IS_ORANGE)
				hasColoredDots = true;
			else if (dot_flags[i] & DOT_IS_INVISIBLE)
				hasInvisibleDots = true;
			else if (dot_flags[i] & DOT_SMALL || dot_flags[i] & DOT_MEDIUM || dot_flags[i] & DOT_LARGE)
				hasSoundDots = true;
			else
				hasDots = true;
		}
	}

	if ((float) dotAmount / (float) pointsToConsider >= 0.9) {
		hasFullDots = true;
	}

	if (id == 0x17C2E) { //Greenhouse bunker door
		hasStones = true;
		hasColoredStones = true;
	}
	else if (id == 0x03677 || id == 0x00557 || id == 0x005F1 || id == 0x00620 || id == 0x009F5 || id == 0x0146C || id == 0x3C12D || id == 0x03686 || id == 0x014E9 || id == 0x0367C || id == 0x334D8 || id == 0x00B71) { // quarry erazor + stones (stones are incorrectly detected as b/w)
		hasStones = false;
		hasColoredStones = true;
	} 
	else if (id == 0x17D9B || id == 0x17D99 || id == 0x17DAA) {
		hasStones = true;
		hasColoredStones = false;
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
	else if (id == 0x09FDC || id == 0x09FF7 || id == 0x09F82 || id == 0x09FF8 || id == 0x09D9F || id == 0x0A01B || id == 0x0A01F || id == 0x17E67 || id == 0x0A079) { // Bunker Panels don't pick up their white squares
		hasStones = true;
	}
}

void PuzzleData::Restore(std::shared_ptr<Memory> _memory) {
	_memory->WritePanelData<int>(id, GRID_SIZE_X, { grid_size_x });
	_memory->WritePanelData<int>(id, GRID_SIZE_Y, { grid_size_y });
	_memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { path_width_scale });
	_memory->WritePanelData<float>(id, PATTERN_SCALE, { pattern_scale });
	_memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(dot_flags.size()) }); //amount of intersections
	_memory->WriteArray<float>(id, DOT_POSITIONS, dot_positions); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_FLAGS, dot_flags); //flags for each point such as entrance or exit
	_memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(dot_connections_a.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
	_memory->WriteArray<int>(id, DOT_CONNECTION_A, dot_connections_a); //start of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_CONNECTION_B, dot_connections_b); //end of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
	_memory->WriteArray<int>(id, DECORATIONS, decorations);
	_memory->WriteArray<int>(id, DECORATION_FLAGS, decoration_flags);
	_memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(colored_regions.size() / 4) });
	_memory->WriteArray<int>(id, COLORED_REGIONS, colored_regions, true);
	_memory->WritePanelData<float>(id, OUTER_BACKGROUND, outer_background_color);
	_memory->WritePanelData<int>(id, OUTER_BACKGROUND_MODE, { outer_background_mode });
	_memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
	_memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, background_region_color);
	_memory->WritePanelData<float>(id, PATH_COLOR, path_color);
	_memory->WritePanelData<__int64>(id, DECORATION_COLORS, { decorationsColorsPointer });
}
