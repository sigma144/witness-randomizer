#pragma once
#include "Panel.h"
#include "Randomizer.h"
#include "PuzzleSymbols.h"
#include <stdlib.h>
#include <string>
#include <time.h>
#include <set>
#include <algorithm>
#include "Random.h"

typedef std::set<Point> Shape;

//The main class for generating puzzles.
class Generate
{
public:
	Generate();
	
	void generate(PanelID id) { PuzzleSymbols symbols({ }); while (!generate(id, symbols)); }
	void generateHelper(PanelID id, std::vector<std::pair<int, int>>& symbolAmounts) { generate(id, symbolAmounts); };
	template <typename... Params>
	void generateHelper(PanelID id, std::vector<std::pair<int, int>>& symbolAmounts, int symbol, int amount, Params... params) {
		symbolAmounts.push_back({ symbol, amount });
		generateHelper(id, symbolAmounts, params...);
	};
	template <typename... Params>
	void generate(PanelID id, Params... params) {
		std::vector<std::pair<int, int>> symbolAmounts;
		generateHelper(id, symbolAmounts, params...);
	};
	void generate(PanelID id, const std::vector<std::pair<int, int>>& symbolVec);
	bool tryGenerate(PanelID id, const std::vector<std::pair<int, int>>& symbolVec, int attempts);
	bool tryGenerate(PanelID id, const std::vector<std::pair<int, int>>& symbolVec) { return tryGenerate(id, symbolVec, 20); }
	void generateMulti(PanelID id, std::vector<std::shared_ptr<Generate>> gens, std::vector<std::pair<int, int>> symbolVec);
	void generateMulti(PanelID id, int numSolutions, std::vector<std::pair<int, int>> symbolVec);
	void generateMaze(PanelID id);
	void generateMaze(PanelID id, int numStarts, int numExits);

	void initPanel(PanelID id);
	void setCustomPath(const std::set<Point>& path) { customPath = path; for (Point p : path) setSymbol(PATH, p.x, p.y); }
	void setObstructions(const std::vector<Point>& walls) { obstructions = { walls }; }
	void setObstructions(const std::vector<std::vector<Point>>& walls) { obstructions = walls; }
	void setSymbol(int symbol, int x, int y);
	void setVal(int val, int x, int y) { panel.set(x, y, val); }
	void setGridSize(int width, int height);
	void setSymmetry(Symmetry symmetry);
	void write(PanelID id);
	void setConfig(Config option) { config.insert(option); };
	void setConfigOnce(Config option) { config.insert(option); oneTimeAdd.insert(option); };
	bool hasConfig(Config option) { return config.count(option); };
	void removeConfig(Config option) { config.erase(option); };
	void removeConfigOnce(Config option) { config.erase(option); oneTimeRemove.insert(option); };
	void resetConfig();
	void seed(long seed) { Random::seed(seed); randomSeed = Random::rand(); }

	void setLoadingHandle(HWND handle) { this->handle = handle; }
	void setLoadingData(int totalPuzzles) { this->totalPuzzles = totalPuzzles; genTotal = 0; }
	void setLoadingData(const std::wstring& areaName, int numPuzzles) { this->areaName = areaName; areaPuzzles = numPuzzles; areaTotal = 0; }
	void incrementProgress();

	std::set<Point> getRegion(Point pos) { return panel.getRegion(pos); }

	float lineThickness;
	std::vector<Point> hitPoints; //The generated path will be forced to hit these points in order
	std::set<Point> openPos; //Custom set of points that can have symbols placed on
	std::set<Point> blockPos; //Point that must be left open
	Color backgroundColor, foregroundColor, gridColor, activeColor, successColor, failColor;

//Don't call these private methods in PuzzleList unless you're sure you need to!
private:
	int get(int x, int y) { return panel.get(x, y); }
	int get(Point p) { return panel.get(p); }
	void set(int x, int y, int val) { panel.set(x, y, val); }
	void set(Point p, int val) { panel.set(p, val); }
	int getSymbolShape(int flags) { return flags & 0xF00; }
	void setPath(Point pos);
	Point getSymPoint(int x, int y) { return panel.getSymPoint(x, y); }
	Point getSymPoint(Point p) { return panel.getSymPoint(p.x, p.y); }
	int getParity(Point pos) { return (pos.x / 2 + pos.y / 2) % 2; }
	void clear();
	void resetVars();
	void initTreehouseLayout();
	int rand() { return Random::rand(); }
	int rand(int mod) { return Random::rand(mod); }
	int rand(size_t mod) { return Random::rand(mod); }
	int rand(int min, int max) { return Random::rand(min, max); }
	template <class T> T pickRandom(const std::vector<T>& vec) { return Random::pickRandom(vec); }
	template <class T> T pickRandom(const std::set<T>& set) { return Random::pickRandom(set); }
	template <class T> T popRandom(const std::vector<T>& vec) { return Random::popRandom(vec); }
	template <class T> T popRandom(const std::set<T>& set) { return Random::popRandom(set); }
	bool onEdge(Point p) { return (!panel.isCylinder && (p.x == 0 || p.x + 1 == panel.width) || p.y == 0 || p.y + 1 == panel.height); }
	bool offEdge(Point p) { return get(p.x, p.y) == OFF_GRID; }
	static std::vector<Point> SHAPEDIRECTIONS, DISCONNECT;
	bool generateMazeI(PanelID id, int numStarts, int numExits);
	bool generate(PanelID id, PuzzleSymbols symbols); //************************************************************
	bool placeSymbols(PuzzleSymbols& symbols);
	bool generatePath(PuzzleSymbols& symbols);
	bool generatePathLength(int minLength, int maxLength);
	bool generatePathLength(int minLength) { return generatePathLength(minLength, 10000); };
	bool generatePathRegions(int minRegions);
	bool generateLongestPath();
	bool generateSpecialPath();
	void erasePath();
	Point adjustPoint(Point pos);

	bool placeStart(int amount);
	bool placeExit(int amount);
	bool canPlaceGap(Point pos);
	bool placeGaps(int amount);
	bool canPlaceDot(Point pos, bool intersectionOnly);
	bool placeDots(int amount, int color, bool intersectionOnly);
	bool canPlaceStone(const std::set<Point>& region, int color);
	bool placeStones(int color, int amount);
	Shape generateShape(std::set<Point>& region, std::set<Point>& bufferRegion, Point pos, int maxSize);
	Shape generateShape(std::set<Point>& region, Point pos, int maxSize) { std::set<Point> buffer; return generateShape(region, buffer, pos, maxSize); }
	int makeShapeSymbol(Shape shape, bool rotated, bool negative, int rotation, int depth);
	int makeShapeSymbol(const Shape& shape, bool rotated, bool negative) { return makeShapeSymbol(shape, rotated, negative, -1, 0); }
	bool placeShapes(const std::vector<int>& colors, const std::vector<int>& negativeColors, int amount, int numRotated, int numNegative);
	bool placeStars(int color, int amount);
	bool hasStar(const std::set<Point>& region, int color);
	bool checkStarZigzag(Panel& panel);
	bool placeTriangles(int color, int amount, int targetCount);
	bool placeErasers(const std::vector<int>& colors, const std::vector<int>& eraseSymbols);
	bool combineShapes(std::vector<Shape>& shapes);
	//Custom Symbols
	bool placeArrows(int color, int amount, int targetCount);
	bool placeAntiTriangles(int color, int amount, int targetCount);
	bool placeCaveClues(int color, int amount, int targetCount);
	bool placeMinesweeperClues(int color, int amount, int targetCount);

	

	Panel panel;
	std::vector<std::vector<int>> customGrid;
	int width, height;
	Symmetry symmetry;
	std::set<Point> starts, exits;
	std::set<Point> gridpos; //Coordinates of the cells of the grid.
	std::set<Point> openpos; //(Possibly not proper) subset of gridpos where decorations can be.
	std::set<Point> path, path1, path2; //Path points
	std::set<Point> customPath; //TODO: Add points in the correct order (for symbols where that matters)
	bool bisect; //Used for stone generation
	int stoneTypes; //Used fo stone generation
	std::set<Config> config, oneTimeAdd, oneTimeRemove;
	long randomSeed;
	std::vector<Point> splitPoints; //For double eraser generation. TODO: Replace this with setSymbol.
	int parity;
	std::vector<std::vector<Point>> obstructions;

	Randomizer* randomizer;
	HWND handle;
	int areaTotal, genTotal, areaPuzzles, totalPuzzles;
	std::wstring areaName;

	friend class PuzzleList;
	friend class Special;
	friend class MultiGenerate;
};

