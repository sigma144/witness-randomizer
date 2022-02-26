#include "PuzzleData.h"

PuzzleData::PuzzleData(std::shared_ptr<Memory> _memory, int puzzleId) {
	id = puzzleId;

	grid_size_x = _memory->ReadPanelData<int>(id, GRID_SIZE_X);
	grid_size_y = _memory->ReadPanelData<int>(id, GRID_SIZE_Y);
	path_width_scale = _memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);
	
	int numberOfDots = _memory->ReadPanelData<int>(id, NUM_DOTS);
	dot_positions = _memory->ReadArray<float>(id, DOT_POSITIONS, numberOfDots * 2);
	dot_flags = _memory->ReadArray<int>(id, DOT_FLAGS, numberOfDots);

	int numberOfConnections = _memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	dot_connections_a = _memory->ReadArray<int>(id, DOT_CONNECTION_A, numberOfConnections);
	dot_connections_b = _memory->ReadArray<int>(id, DOT_CONNECTION_B, numberOfConnections);
	
	int numberOfDecorations = _memory->ReadPanelData<int>(id, NUM_DECORATIONS);
	decorations = _memory->ReadArray<int>(id, DECORATIONS, numberOfDecorations);
	decoration_flags = _memory->ReadArray<int>(id, DECORATION_FLAGS, numberOfDecorations);

	//num_colored_regions = _memory->ReadPanelData<int>(id, NUM_COLORED_REGIONS);
	//colored_regions = _memory->ReadArray<int>(id, COLORED_REGIONS, num_colored_regions);

	for (int i = 0; i < numberOfDecorations; i++)
	{
		if ((decoration_flags[i] & 0x700) == Decoration::Shape::Stone) {
			if (decoration_flags[i] & 0x00E)
				hasColoredStones = true;
			else
				hasStones = true;
		}
		else if ((decoration_flags[i] & 0x700) == Decoration::Shape::Star) {
			hasStars = true;
		}
		else if ((decoration_flags[i] & 0x700) == Decoration::Shape::Poly) {
			hasTetris = true;
		}
		else if ((decoration_flags[i] & 0x700) == Decoration::Shape::Eraser) {
			hasErasers = true;
		}
		else if ((decoration_flags[i] & 0x700) == Decoration::Shape::Triangle) {
			hasTriangles = true;
		}
		else if ((decoration_flags[i] & 0x700) == Decoration::Shape::Arrow) {
			hasArrows = true;
		}
	}

	for (int i = 0; i < numberOfDots; i++)
	{
		if (dot_flags[i] & DOT) {
			if (dot_flags[i] & DOT_IS_BLUE || dot_flags[i] & DOT_IS_ORANGE)
				hasColoredDots = true;
			else if (dot_flags[i] & DOT_IS_INVISIBLE)
				hasInvisibleDots = true;
			else
				hasDots = true;
		}
	}
}

void PuzzleData::Restore(std::shared_ptr<Memory> _memory) {
	_memory->WritePanelData<int>(id, GRID_SIZE_X, { grid_size_x });
	_memory->WritePanelData<int>(id, GRID_SIZE_Y, { grid_size_y });
	_memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { path_width_scale });
	_memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(dot_flags.size()) }); //amount of intersections
	_memory->WriteArray<float>(id, DOT_POSITIONS, dot_positions); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_FLAGS, dot_flags); //flags for each point such as entrance or exit
	_memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(dot_connections_a.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
	_memory->WriteArray<int>(id, DOT_CONNECTION_A, dot_connections_a); //start of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_CONNECTION_B, dot_connections_b); //end of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
	_memory->WriteArray<int>(id, DECORATIONS, decorations);
	_memory->WriteArray<int>(id, DECORATION_FLAGS, decoration_flags);
	//_memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { num_colored_regions }); //why devide by 4 tho?
	//_memory->WriteArray<int>(id, COLORED_REGIONS, colored_regions);
	_memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}
