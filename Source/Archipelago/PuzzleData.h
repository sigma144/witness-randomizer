#pragma once
#include "..\Panel.h"

struct PuzzleData
{
	public:
		PuzzleData(std::shared_ptr<Memory> _memory, int id);
		void Restore(std::shared_ptr<Memory> _memory);

		int id;

		bool hasStones;
		bool hasColoredStones;
		bool hasStars;
		bool hasTetris;
		bool hasErasers;
		bool hasTriangles;
		bool hasDots;
		bool hasColoredDots;
		bool hasInvisibleDots;
		bool hasArrows;

		int grid_size_x;
		int grid_size_y;
		float path_width_scale;
		std::vector<float> dot_positions;
		std::vector<int> dot_flags;
		std::vector<int> dot_connections_a;
		std::vector<int> dot_connections_b;
		std::vector<int> decorations;
		std::vector<int> decoration_flags;
		int num_colored_regions;
		std::vector<int> colored_regions;
};

