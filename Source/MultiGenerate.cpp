
#include "MultiGenerate.h"

inline Point operator+(const Point& l, const Point& r) { return { l.first + r.first, l.second + r.second }; }

void MultiGenerate::generate(int id, const std::vector<std::shared_ptr<Generate>>& gens, const std::vector<std::pair<int, int>>& symbolVec)
{
	generators = gens;
	PuzzleSymbols symbols(symbolVec);
	while (!generate(id, symbols));
}

bool MultiGenerate::generate(int id, PuzzleSymbols symbols)
{
	for (std::shared_ptr<Generate> g : generators) {
		g->initPanel(id);
		int fails = 0;
		while (!g->generate_path(symbols)) {
			if (fails++ > 20)
				return false;
		}
	}

	std::vector<std::string> solution1; //For debugging only
	for (int y = 0; y < generators[0]->_panel->_height; y++) {
		std::string row;
		for (int x = 0; x < generators[0]->_panel->_width; x++) {
			if (generators[0]->get(x, y) == PATH) {
				row += "xx";
			}
			else row += "    ";
		}
		solution1.push_back(row);
	}
	std::vector<std::string> solution2; //For debugging only
	for (int y = 0; y < generators[1]->_panel->_height; y++) {
		std::string row;
		for (int x = 0; x < generators[1]->_panel->_width; x++) {
			if (generators[1]->get(x, y) == PATH) {
				row += "xx";
			}
			else row += "    ";
		}
		solution2.push_back(row);
	}
	std::vector<std::string> solution3; //For debugging only
	for (int y = 0; y < generators[2]->_panel->_height; y++) {
		std::string row;
		for (int x = 0; x < generators[2]->_panel->_width; x++) {
			if (generators[2]->get(x, y) == PATH) {
				row += "xx";
			}
			else row += "    ";
		}
		solution3.push_back(row);
	}

	if (!place_all_symbols(symbols))
		return false;

	if (!generators[0]->hasFlag(Generate::Config::DisableWrite)) generators[0]->write(id);
	return true;
}

bool MultiGenerate::place_all_symbols(PuzzleSymbols symbols)
{
	for (std::pair<int, int> s : symbols[Decoration::Stone]) if (!place_stones(s.first & 0xf, s.second))
		return false;
	for (std::pair<int, int> s : symbols[Decoration::Triangle]) if (!place_triangles(s.first & 0xf, s.second))
		return false;
	for (std::pair<int, int> s : symbols[Decoration::Star]) if (!place_stars(s.first & 0xf, s.second))
		return false;
	return true;
}

bool MultiGenerate::can_place_gap(Point pos)
{
	for (std::shared_ptr<Generate> g : generators) {
		if (g->get(pos) != 0)
			return false;
	}
	return true;
}

bool MultiGenerate::place_stones(int color, int amount)
{
	std::set<Point> open = generators[0]->_openpos;
	while (amount > 0) {
		if (open.size() < amount)
			return false;
		Point pos = pick_random(open);
		bool valid = true;
		for (std::shared_ptr<Generate> g : generators) {
			std::set<Point> region = g->get_region(pos);
			if (!g->can_place_stone(region, color)) {
				for (Point p : region) open.erase(p);
				valid = false;
			}
			else if (splitStones) {
				for (Point p : region) open.erase(p);
			}
		}
		if (!valid) continue;
		for (std::shared_ptr<Generate> g : generators) {
			g->set(pos, Decoration::Stone | color);
			g->_openpos.erase(pos);
			open.erase(pos);
		}
		amount--;
	}
	return true;
}

bool MultiGenerate::place_stars(int color, int amount)
{
	std::set<Point> open = generators[0]->_openpos;
	int halfPoint = amount / 2;
	for (std::shared_ptr<Generate> g : generators) g->_allowNonMatch = amount % 2;
	while (amount > 0) {
		if (open.size() < amount)
			return false;
		Point pos = pick_random(open);
		std::vector<std::set<Point>> regions;
		std::vector<std::shared_ptr<Generate>> nonMatch;
		for (std::shared_ptr<Generate> g : generators) {
			std::set<Point> region = g->get_region(pos);
			if (region.size() == 1) {
				for (Point p : region) open.erase(p);
				continue;
			}
			int count = g->count_color(region, color);
			if (count == 0) {
				if (amount <= halfPoint || amount == halfPoint + 1 && g->_allowNonMatch)
					for (Point p : region) open.erase(p);
				else regions.push_back(region);
			}
			else if (count == 1) {
				if (amount <= halfPoint && !g->has_star(region, color) || amount > halfPoint && (!g->_allowNonMatch || g->has_star(region, color)))
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
		for (std::shared_ptr<Generate> g : nonMatch) g->_allowNonMatch = false;
		for (std::set<Point>& region : regions) for (Point p : region) open.erase(p);
		for (std::shared_ptr<Generate> g : generators) {
			g->set(pos, Decoration::Star | color);
			g->_openpos.erase(pos);
		}
		amount--;
		if (amount == halfPoint) open = generators[0]->_openpos;
	}
	return true;
}

bool MultiGenerate::can_place_triangle(Point pos)
{
	int count = generators[0]->count_sides(pos);
	if (count == 0) return false;
	for (int i = 1; i < generators.size(); i++) {
		if (generators[i]->count_sides(pos) != count) return false;
	}
	return true;
}

bool MultiGenerate::place_triangles(int color, int amount)
{
	std::set<Point> open;
	for (Point p : generators[0]->_openpos) {
		if (can_place_triangle(p)) open.insert(p);
	}
	if (generators[0]->get(1, 1) == 0xA05) { //Mountain Hatch Perspective puzzle
		open.erase({ 1, 5 }); open.erase({ 9, 5 });
	}
	while (amount > 0) {
		if (open.size() < amount)
			return false;
		Point pos = pick_random(open);
		open.erase(pos);
		int count = generators[0]->count_sides(pos);
		for (std::shared_ptr<Generate> g : generators) {
			g->set(pos, Decoration::Triangle | (count << 16) | color);
			g->_openpos.erase(pos);
		}
		amount--;
	}
	return true;
}
