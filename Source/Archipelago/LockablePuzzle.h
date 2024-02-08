#pragma once

#include <vector>
#include <string>

class APState;

struct LockablePuzzle
{
public:
	LockablePuzzle(int panelId) {
		id = panelId;
	};

	virtual void Read();
	virtual void UpdateLock(APState state);
	virtual void Restore();

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
	bool hasSoundDots = false;
	bool hasArrows = false;
	bool hasSymmetry = false;
	bool hasFullDots = false;
	bool needsChallengeLasers = false;
	bool needsMountainLasers = false;
};

struct LockablePanel : LockablePuzzle {
	LockablePanel(int panelId) : LockablePuzzle(panelId) {}

	void Read();
	void UpdateLock(APState state);
	void Restore();

private:
	int grid_size_x;
	int grid_size_y;
	float path_width_scale;
	float pattern_scale;
	std::vector<float> dot_positions;
	std::vector<int> dot_flags;
	std::vector<int> dot_connections_a;
	std::vector<int> dot_connections_b;
	std::vector<int> decorations;
	std::vector<int> decoration_flags;
	std::vector<int> colored_regions;
	__int64 decorationsColorsPointer;

	std::vector<float> outer_background_color;
	std::vector<float> path_color;
	std::vector<float> background_region_color;
	int outer_background_mode;

	void addMissingSymbolsDisplay(std::vector<float>& newIntersections, std::vector<int>& newIntersectionFlags, std::vector<int>& newConnectionsA, std::vector<int>& newConnectionsB, int id);
	void createText(std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, float left, float right, float top, float bottom);
	void createCenteredText(std::string text, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, float top, float bottom);
	int addPuzzleSymbols(const APState& state, std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB, std::vector<int>& decorations, std::vector<int>& decorationsFlags, std::vector<int>& polygons, int id);
};

struct LockableEP : LockablePuzzle {
	LockableEP(int panelId) : LockablePuzzle(panelId) {}

	void Read();
	void UpdateLock(APState state);
	void Restore();
	void DisableEP(bool recolor);
	void LockEP(bool disable, bool recolor);
};

struct LockableObelisk : LockablePuzzle {
	LockableObelisk(int panelId) : LockablePuzzle(panelId) {}

	std::vector<float> original_orientation;

	void Read();
	void UpdateLock(APState state);
	void Restore();
};