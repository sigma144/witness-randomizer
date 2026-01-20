#pragma once
#include "Generate.h"
#include "Random.h"

//Class for generating puzzles with multiple solutions.
//TODO: Make this more generalized. Probably try to remove splitStones hardcoding.
class MultiGenerate
{
public:

	MultiGenerate() { splitStones = false; }
	~MultiGenerate() { }

	std::vector<std::shared_ptr<Generate>> generators;

	void generate(PanelID id, const std::vector<std::shared_ptr<Generate>>& gens, const std::vector<std::pair<int, int>>& symbolVec);

	bool splitStones;

private:

	bool generate(PanelID id, PuzzleSymbols symbols);
	bool placeAllSymbols(PuzzleSymbols symbols);
	bool canPlaceGap(Point pos);
	bool placeStones(int color, int amount);
	bool placeStars(int color, int amount);
	bool canPlaceTriangle(Point pos);
	bool placeTriangles(int color, int amount);

	friend class Special;
};

