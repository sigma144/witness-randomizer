// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MultiGenerate.h"

inline Point operator+(const Point& l, const Point& r) { return { l.x + r.x, l.y + r.y }; }

void MultiGenerate::generate(PanelID id, const std::vector<std::shared_ptr<Generate>>& gens, const std::vector<std::pair<int, int>>& symbolVec)
{
	generators = gens;
	PuzzleSymbols symbols(symbolVec);
	while (!generate(id, symbols));
}

bool MultiGenerate::generate(PanelID id, PuzzleSymbols symbols)
{
	for (std::shared_ptr<Generate> g : generators) {
		g->initPanel(id);
		int fails = 0;
		while (!g->generatePath(symbols)) {
			if (fails++ > 20)
				return false;
		}
	}
	if (!placeAllSymbols(symbols))
		return false;
	if (!generators[0]->hasConfig(DisableWrite)) generators[0]->write(id);
	return true;
}

bool MultiGenerate::placeAllSymbols(PuzzleSymbols symbols)
{
	for (std::pair<int, int> s : symbols[Stone]) if (!placeStones(s.first & 0xf, s.second))
		return false;
	for (std::pair<int, int> s : symbols[Triangle]) if (!placeTriangles(s.first & 0xf, s.second))
		return false;
	for (std::pair<int, int> s : symbols[Star]) if (!placeStars(s.first & 0xf, s.second))
		return false;
	return true;
}

bool MultiGenerate::canPlaceGap(Point pos)
{
	for (std::shared_ptr<Generate> g : generators) {
		if (g->get(pos) != 0)
			return false;
	}
	return true;
}

bool MultiGenerate::placeStones(int color, int amount)
{
	std::set<Point> open = generators[0]->openpos;
	while (amount > 0) {
		if (open.size() < amount)
			return false;
		Point pos = Random::pickRandom(open);
		bool valid = true;
		for (std::shared_ptr<Generate> g : generators) {
			std::set<Point> region = g->getRegion(pos);
			if (!g->canPlaceStone(region, color)) {
				for (Point p : region) open.erase(p);
				valid = false;
			}
			else if (splitStones) {
				for (Point p : region) open.erase(p);
			}
		}
		if (!valid) continue;
		for (std::shared_ptr<Generate> g : generators) {
			g->set(pos, Stone | color);
			g->openpos.erase(pos);
			open.erase(pos);
		}
		amount--;
	}
	return true;
}

bool MultiGenerate::placeStars(int color, int amount)
{
	std::set<Point> open = generators[0]->openpos;
	int halfPoint = amount / 2;
	std::set<std::shared_ptr<Generate>> allowNonMatch;
	if (amount % 2) {
		for (std::shared_ptr<Generate> g : generators)
			allowNonMatch.insert(g);
	}
	while (amount > 0) {
		if (open.size() < amount)
			return false;
		Point pos = Random::pickRandom(open);
		std::vector<std::set<Point>> regions;
		std::vector<std::shared_ptr<Generate>> nonMatch;
		for (std::shared_ptr<Generate> g : generators) {
			std::set<Point> region = g->getRegion(pos);
			if (region.size() == 1) {
				for (Point p : region) open.erase(p);
				continue;
			}
			int count = g->panel.countColor(region, color);
			if (count == 0) {
				if (amount <= halfPoint || amount == halfPoint + 1 && allowNonMatch.count(g))
					for (Point p : region) open.erase(p);
				else regions.push_back(region);
			}
			else if (count == 1) {
				if (amount <= halfPoint && !g->hasStar(region, color) || amount > halfPoint && (!allowNonMatch.count(g) || g->hasStar(region, color)))
					for (Point p : region) open.erase(p);
				else {
					regions.push_back(region);
					if (amount > halfPoint) nonMatch.push_back(g);
				}
			}
			else {
				for (Point p : region) open.erase(p);
			}
		}
		if (regions.size() < generators.size()) continue;
		for (std::shared_ptr<Generate> g : nonMatch) allowNonMatch.erase(g);
		for (std::set<Point>& region : regions) for (Point p : region) open.erase(p);
		for (std::shared_ptr<Generate> g : generators) {
			g->set(pos, Star | color);
			g->openpos.erase(pos);
		}
		amount--;
		if (amount == halfPoint) open = generators[0]->openpos;
	}
	return true;
}

bool MultiGenerate::canPlaceTriangle(Point pos)
{
	int count = generators[0]->panel.countSides(pos);
	if (count == 0) return false;
	for (int i = 1; i < generators.size(); i++) {
		if (generators[i]->panel.countSides(pos) != count) return false;
	}
	return true;
}

bool MultiGenerate::placeTriangles(int color, int amount)
{
	std::set<Point> open;
	for (Point p : generators[0]->openpos) {
		if (canPlaceTriangle(p)) open.insert(p);
	}
	if (generators[0]->get(1, 1) == 0xA05) { //Mountain Hatch Perspective puzzle
		open.erase({ 1, 5 }); open.erase({ 9, 5 });
	}
	while (amount > 0) {
		if (open.size() < amount)
			return false;
		Point pos = Random::pickRandom(open);
		open.erase(pos);
		int count = generators[0]->panel.countSides(pos);
		for (std::shared_ptr<Generate> g : generators) {
			g->set(pos, Triangle | (count << 16) | color);
			g->openpos.erase(pos);
		}
		amount--;
	}
	return true;
}
