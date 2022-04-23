#pragma once
#include "..\Panel.h"

struct PuzzleData
{
	public:
		PuzzleData(int panelId) {
			id = panelId;
		};

		void Read(std::shared_ptr<Memory> _memory);
		void Restore(std::shared_ptr<Memory> _memory);

		int id;

		bool hasStones = false;
		bool hasColoredStones = false;
		bool hasStars = false;
		bool hasStarsWithOtherSymbol = false;
		bool hasTetris = false;
		bool hasTetrisRotated = false;
		bool hasTetrisNegative = false;
		bool hasErasers = false;
		bool hasTriangles = false;
		bool hasDots = false;
		bool hasColoredDots = false;
		bool hasInvisibleDots = false;
		bool hasSoundDots = false;
		bool hasArrows = false;
		bool hasSymmetry = false;

		int grid_size_x;
		int grid_size_y;
		float path_width_scale;
		std::vector<float> dot_positions;
		std::vector<int> dot_flags;
		std::vector<int> dot_connections_a;
		std::vector<int> dot_connections_b;
		std::vector<int> decorations;
		std::vector<int> decoration_flags;
		std::vector<int> colored_regions;
};

