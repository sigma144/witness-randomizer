// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Special.h"
#include "MultiGenerate.h"
#include "Quaternion.h"
#include "../App/Version.h"
#include "Watchdog.h"

Special::Special(Generate* generator) {
	this->gen = generator;
	memory = Memory::get();
}

void Special::generateSpecialSymMaze(PanelID id) {
	do {
		gen->setConfigOnce(DisableWrite);
		gen->setSymbol(Start, 0, 16);
		gen->setSymbol(Start, 22, 16);
		gen->setSymbol(Exit, 8, 0);
		gen->setSymbol(Exit, 14, 0);
		gen->setConfigOnce(ShortPath);
		gen->generateMaze(SYM_MAZE_V5);
	} while (gen->path.count(Point(12, 16)));
	Panel* puzzle = &(gen->panel);
	for (int x = 0; x < puzzle->width / 2; x++) {
		for (int y = 0; y < puzzle->height; y++) {
			Point sp = puzzle->getSymPoint(x, y, gen->symmetry);
			if (puzzle->get(sp) & Gap) {
				puzzle->set(x, y, puzzle->get(sp));
				puzzle->set(sp, 0);
			}
		}
	}
	gen->write(id);
}

void Special::generateReflectionDotPuzzle(PanelID id1, PanelID id2, std::vector<std::pair<int, int>> symbols, Symmetry symmetry, bool split)
{
	gen->setConfigOnce(DisableWrite);
	gen->generate(id1, symbols);
	Panel* puzzle = &(gen->panel);
	Panel flippedPuzzle = Panel(id2);
	std::vector<Point> dots;
	for (int x = 0; x < puzzle->width; x++) {
		for (int y = 0; y < puzzle->height; y++) {
			Point sp = puzzle->getSymPoint(x, y, symmetry);
			if (puzzle->get(x, y) & Dot) {
				int symbol = (sp.x & 1) == 1 ? Dot_Row : (sp.y & 1) == 1 ? Dot_Column : Dot_Intersection;
				flippedPuzzle.set(sp, symbol | DOT_IS_INVISIBLE);
				dots.push_back({ x, y });
			}
			else if (puzzle->get(x, y) & Gap) {
				int symbol = (sp.x & 1) == 1 ? Gap_Row : Gap_Column;
				flippedPuzzle.set(sp, symbol);
			}
			else flippedPuzzle.set(sp, puzzle->get(x, y));
		}
	}
	if (split) {
		int size = static_cast<int>(dots.size());
		while (dots.size() > size / 2 + Random::rand() % 2) {
			Point dot = popRandom(dots);
			Point sp = puzzle->getSymPoint(dot.x, dot.y, symmetry);
			puzzle->setFlag(dot.x, dot.y, DOT_IS_INVISIBLE);
			flippedPuzzle.clearFlag(sp.x, sp.y, DOT_IS_INVISIBLE);
		}
		if (Random::rand() % 2) {
			Point dot = popRandom(dots);
			Point sp = puzzle->getSymPoint(dot.x, dot.y, symmetry);
			flippedPuzzle.clearFlag(sp.x, sp.y, DOT_IS_INVISIBLE);
		}
	}
	flippedPuzzle.startpoints.clear();
	for (Point p : puzzle->startpoints) {
		flippedPuzzle.startpoints.push_back(puzzle->getSymPoint(p.x, p.y, symmetry));
	}
	flippedPuzzle.endpoints.clear();
	for (Endpoint p : puzzle->endpoints) {
		Point sp = puzzle->getSymPoint(p.x, p.y, symmetry);
		flippedPuzzle.endpoints.push_back(Endpoint(sp.x, sp.y, gen->panel.getSymDir(p.dir, symmetry),
			ENDPOINT | (p.dir == Endpoint::Direction::UP || p.dir == Endpoint::Direction::DOWN ? COLUMN : ROW)));
	}
	//TODO: Do something about this hardcoding?
	if (id1 == SYM_YELLOW_3 && split) {
		if (checkDotSolvability(puzzle, &flippedPuzzle, symmetry)) {
			generateReflectionDotPuzzle(id1, id2, symbols, symmetry, split);
			return;
		}
	}
	gen->write(id1);
	flippedPuzzle.write(id2);
}

void Special::generateAntiPuzzle(PanelID id)
{
	while (true) {
		gen->setConfigOnce(DisableWrite);
		gen->generate(id, Poly | Rotate, 2);
		std::set<Point> open = gen->gridpos;
		std::vector<int> symbols;
		for (int x = 1; x < gen->panel.width; x += 2) {
			for (int y = 1; y < gen->panel.height; y += 2) {
				if (gen->getSymbolShape(gen->get(x, y)) == Poly) {
					symbols.push_back(gen->get(x, y));
					gen->set(x, y, 0);
					for (Point p : gen->getRegion(Point(x, y))) {
						open.erase(p);
					}
				}
			}
		}
		if (open.size() == 0) continue;
		std::set<Point> region = gen->getRegion(*open.begin());
		if (region.size() != open.size() || open.size() < symbols.size() + 1) continue;
		for (int s : symbols) {
			Point p = gen->pickRandom(open);
			gen->set(p, s | Negative);
			open.erase(p);
		}
		gen->set(gen->pickRandom(open), Poly | 0xffff0000); //Full block
		gen->write(id);
		gen->resetConfig();
		return;
	}
}


void Special::generateColorFilterPuzzle(PanelID id, Point size, const std::vector<std::pair<int, int>>& symbols, const Color& filter, bool colorblind)
{
	gen->setConfigOnce(DisableWrite);
	gen->setGridSize(size.x, size.y);
	gen->generate(id, symbols);
	std::vector<Color> availableColors = { {0, 0, 0, 1} };
	if (filter.r == 1) {
		for (int i = 0; i < availableColors.size(); i++) {
			Color c = availableColors[i];
			if (c.r == 0) availableColors.push_back({ 1, c.g, c.b, 1 });
		}
	}
	if (filter.g == 1) {
		for (int i = 0; i < availableColors.size(); i++) {
			Color c = availableColors[i];
			if (colorblind && c.r == 0) continue;
			if (c.g == 0) availableColors.push_back({ c.r, 1, c.b, 1 });
		}
	}
	if (filter.b == 1) {
		for (int i = 0; i < availableColors.size(); i++) {
			Color c = availableColors[i];
			if (c.b == 0) availableColors.push_back({ c.r, c.g, 1, 1 });
		}
	}
	for (int i = 0; i < availableColors.size(); i++) { //Shuffle
		std::swap(availableColors[i], availableColors[Random::rand() % availableColors.size()]);
	}
	std::vector<Color> symbolColors;
	for (int y = gen->panel.height - 2; y>0; y -= 2) {
		for (int x = 1; x < gen->panel.width - 1; x += 2) {
			if (gen->get(x, y) == 0) symbolColors.push_back({ 0, 0, 0, 0 });
			else symbolColors.push_back(availableColors[(gen->get(x, y) & 0xf) - 1]);
		}
	}
	bool pass = false;
	while (!pass) {
		//Add random variation in remaining color channel(s)
		for (Color &c : symbolColors) {
			if (c.a == 0) continue;
			if (filter.r == 0) c.r = static_cast<float>(Random::rand() % 2);
			if (filter.g == 0) c.g = static_cast<float>(Random::rand() % 2);
			if (filter.b == 0) c.b = static_cast<float>(Random::rand() % 2);
		}
		//Check for solvability
		std::map<Color, int> colorCounts;
		for (Color &c : symbolColors) {
			colorCounts[c]++;
		}
		for (const auto& pair : colorCounts) {
			if (pair.second % 2) {
				pass = true;
				break;
			}
		}
	}
	gen->setConfigOnce(WriteColors);
	gen->write(id);
	memory->WriteArray(id, DECORATION_COLORS, symbolColors);
	gen->resetConfig();
}

void Special::generateSoundDotPuzzle(PanelID id1, PanelID id2, std::vector<int> dotSequence, bool writeSequence) {
	//generator->setConfig(DisableReset);
	generateSoundDotPuzzle(id1, { 5, 5 }, dotSequence, writeSequence);
	gen->write(id2);
	gen->resetConfig();
}

void Special::generateSoundDotPuzzle(PanelID id, Point size, std::vector<int> dotSequence, bool writeSequence) {
	gen->resetConfig();
	gen->setConfigOnce(DisableWrite);
	gen->setConfigOnce(LongPath);
	gen->setGridSize(size.x, size.y);
	if (id == JUNGLE_DOT_6) { //Have to force there to only be one correct sequence
		gen->generate(id, Dot_Intersection, static_cast<int>(dotSequence.size() - 1));
		while (gen->get(6, 0) == Dot_Intersection || gen->get(8, 2) == Dot_Intersection)
			gen->generate(id, Dot_Intersection, static_cast<int>(dotSequence.size() - 1));
		if (gen->get(7, 0) == PATH) gen->set(7, 0, Dot_Row);
		else gen->set(8, 1, Dot_Column);
	}
	else gen->generate(id, Dot_Intersection, static_cast<int>(dotSequence.size()));
	Point p = *gen->starts.begin();
	std::set<Point> path = gen->path;
	int seqPos = 0;
	while (!gen->exits.count(p)) {
		path.erase(p);
		int sym = gen->get(p);
		if (sym & Dot) {
			gen->set(p, sym | dotSequence[seqPos++]);
		}
		for (Point dir : Panel::DIRECTIONS) {
			Point newp = p + dir;
			if (path.count(newp)) {
				p = newp;
				break;
			}
		}
	}
	if (writeSequence) {
		for (int i = 0; i < dotSequence.size(); i++) {
			if (dotSequence[i] == DOT_SMALL) dotSequence[i] = 1;
			if (dotSequence[i] == DOT_MEDIUM) dotSequence[i] = 2;
			if (dotSequence[i] == DOT_LARGE) dotSequence[i] = 3;
		}
		memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN, { static_cast<int>(dotSequence.size()) });
		memory->WriteArray(id, DOT_SEQUENCE, dotSequence, true);
	}
	gen->write(id);
}

void Special::generateSoundDotReflectionPuzzle(PanelID id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored, bool writeSequence)
{
	//TODO: Get rid of hardcoding?
	gen->resetConfig();
	gen->setConfigOnce(DisableWrite);
	gen->setConfigOnce(LongPath);
	gen->setSymmetry(Rotational);
	gen->setGridSize(size.x, size.y);
	if (id != SHIPWRECK_VAULT) {
		gen->setSymbol(Start, 0, gen->height - 1); gen->setSymbol(Start, gen->width - 1, 0);
		gen->setSymbol(Exit, 0, 0); gen->setSymbol(Exit, gen->width - 1, gen->height - 1);
	}
	if (id == JUNGLE_DOT_5 || id == JUNGLE_DOT_6) { //Generate with one less dot than the pattern (for set start/exit)
		gen->generate(id, Dot_Intersection | Blue, static_cast<int>(dotSequence1.size() - 1), Dot_Intersection | Yellow, static_cast<int>(dotSequence2.size() - 1));
	}
	else if (id == SHIPWRECK_VAULT && writeSequence) { //Shipwreck Expert
		while (!generateSoundDotReflectionSpecial(id, size, dotSequence1, dotSequence2, numColored));
		return;
	}
	else gen->generate(id, Dot_Intersection | Blue, static_cast<int>(dotSequence1.size()), Dot_Intersection | Yellow, static_cast<int>(dotSequence2.size()));
	std::set<Point> path1 = gen->path1, path2 = gen->path2;
	Point p1, p2;
	std::set<Point> dots1, dots2;
	for (Point p : gen->starts) {
		if (gen->path1.count(p)) p1 = p;
		if (gen->path2.count(p)) p2 = p;
	}
	if (id == JUNGLE_DOT_5) {
		gen->set(p1, Dot_Intersection);
		gen->set(p2, Dot_Intersection);
	}
	int seqPos = 0;
	while (!gen->exits.count(p1)) {
		path1.erase(p1);
		int sym = gen->get(p1);
		if (sym & Dot) {
			gen->set(p1, sym | dotSequence1[seqPos++]);
			dots1.insert(p1);
		}
		for (Point dir : Panel::DIRECTIONS) {
			Point newp = p1 + dir;
			if (path1.count(newp)) {
				p1 = newp;
				break;
			}
		}
	}
	seqPos = 0;
	while (!gen->exits.count(p2)) {
		path2.erase(p2);
		int sym = gen->get(p2);
		if (sym & Dot) {
			gen->set(p2, sym | dotSequence2[seqPos++]);
			dots2.insert(p2);
		}
		for (Point dir : Panel::DIRECTIONS) {
			Point newp = p2 + dir;
			if (path2.count(newp)) {
				p2 = newp;
				break;
			}
		}
	}
	if (id == JUNGLE_DOT_6) {
		gen->set(p1, dotSequence1[seqPos] | Dot_Intersection);
		gen->set(p2, dotSequence2[seqPos] | Dot_Intersection);
		numColored += 2;
	}
	for (int i = static_cast<int>(dotSequence1.size() + dotSequence2.size()); i > numColored; i--) {
		if (i % 2 == 0) { //Want to evenly distribute colors between blue/orange (approximately)
			Point p = popRandom(dots1);
			gen->set(p, gen->get(p) & ~DOT_IS_BLUE); //Remove color
		}
		else {
			Point p = popRandom(dots2);
			gen->set(p, gen->get(p) & ~DOT_IS_ORANGE); //Remove color
		}
	}
	if (writeSequence) {
		for (int i = 0; i < dotSequence1.size(); i++) {
			if (dotSequence1[i] == DOT_SMALL) dotSequence1[i] = 1;
			if (dotSequence1[i] == DOT_MEDIUM) dotSequence1[i] = 2;
			if (dotSequence1[i] == DOT_LARGE) dotSequence1[i] = 3;
		}
		memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN, { static_cast<int>(dotSequence1.size()) });
		memory->WriteArray(id, DOT_SEQUENCE, dotSequence1, true);
		for (int i = 0; i < dotSequence2.size(); i++) {
			if (dotSequence2[i] == DOT_SMALL) dotSequence2[i] = 1;
			if (dotSequence2[i] == DOT_MEDIUM) dotSequence2[i] = 2;
			if (dotSequence2[i] == DOT_LARGE) dotSequence2[i] = 3;
		}
		memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN_REFLECTION, { static_cast<int>(dotSequence2.size()) });
		memory->WriteArray(id, DOT_SEQUENCE_REFLECTION, dotSequence2, true);
	}
	if (id == SHIPWRECK_VAULT) gen->setConfig(DisableFlash);
	gen->write(id);
	memory->WritePanelData(id, POWER_OFF_ON_FAIL, 0);
	gen->resetConfig();
	if (dotSequence1 != dotSequence2 && id != SHIPWRECK_VAULT) (new JungleWatchdog(id, dotSequence1, dotSequence2))->start();
}

bool Special::generateSoundDotReflectionSpecial(PanelID id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored) {
	gen->resetConfig();
	gen->resetVars();
	gen->setConfigOnce(DisableWrite);
	gen->setConfigOnce(LongPath);
	gen->setSymmetry(RotateLeft);
	gen->setGridSize(size.x, size.y);
	std::vector<Point> starts = { { 0, 0 },{ gen->width - 1, 0 },{ 0, gen->height - 1 },{ gen->width - 1, gen->height - 1 } };
	Point start = pickRandom<Point>(starts);
	gen->setSymbol(Start, start.x, start.y);
	gen->setSymbol(Exit, 6, 0); gen->setSymbol(Exit, gen->width - 1, 6);
	gen->setSymbol(Exit, 0, gen->height - 7); gen->setSymbol(Exit, gen->width - 7, gen->height - 1);
	gen->generate(id, Dot_Intersection | Blue, static_cast<int>(dotSequence1.size() - 1), Dot_Intersection | Yellow, static_cast<int>(dotSequence2.size() - 1));
	std::set<Point> path1 = gen->path1, path2 = gen->path2;
	std::set<Point> intersect;
	for (Point p : path1) {
		if (p.x % 2 != 0 || p.y % 2 != 0)
			continue;
		if (path2.count(p)) {
			if (gen->get(p) & Dot)
				return false;
			intersect.insert(p);
		}
	}
	Point pshared = pickRandom(intersect);
	gen->set(pshared, Dot_Intersection);
	Point p1 = start, p2 = gen->getSymPoint(start);
	std::set<Point> dots1, dots2;

	int seqPos = 0;
	while (seqPos < dotSequence1.size()) {
		path1.erase(p1);
		int sym = gen->get(p1);
		if (sym & Dot) {
			gen->set(p1, sym | dotSequence1[seqPos++]);
			dots1.insert(p1);
		}
		for (Point dir : Panel::DIRECTIONS) {
			Point newp = p1 + dir;
			if (path1.count(newp)) {
				p1 = newp;
				break;
			}
		}
	}
	seqPos = 0;
	while (seqPos < dotSequence2.size()) {
		path2.erase(p2);
		int sym = gen->get(p2);
		if (sym & Dot) {
			if ((sym & 0xf000) && ((sym & 0xf000) == DOT_MEDIUM))
				return false;
			if ((sym & 0xf000) && (sym & 0xf000) != dotSequence2[seqPos])
				return false;
			gen->set(p2, sym | dotSequence2[seqPos++]);
			if (!gen->starts.count(p2)) dots2.insert(p2);
		}
		for (Point dir : Panel::DIRECTIONS) {
			Point newp = p2 + dir;
			if (path2.count(newp)) {
				p2 = newp;
				break;
			}
		}
	}

	dots1.erase(pshared);
	dots2.erase(pshared);
	gen->set(pshared, gen->get(pshared) & (~DOT_IS_BLUE | DOT_IS_ORANGE));
	for (int i = static_cast<int>(dotSequence1.size() + dotSequence2.size()) - 2; i > numColored; i--) {
		if (i % 2 == 0) { //Want to evenly distribute colors between blue/orange (approximately)
			Point p = popRandom(dots1);
			gen->set(p, gen->get(p) & ~DOT_IS_BLUE); //Remove color
		}
		else {
			Point p = popRandom(dots2);
			gen->set(p, gen->get(p) & ~DOT_IS_ORANGE); //Remove color
		}
	}
	for (int i = 0; i < dotSequence1.size(); i++) {
		if (dotSequence1[i] == DOT_SMALL) dotSequence1[i] = 1;
		if (dotSequence1[i] == DOT_MEDIUM) dotSequence1[i] = 2;
		if (dotSequence1[i] == DOT_LARGE) dotSequence1[i] = 3;
	}
	memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN, { static_cast<int>(dotSequence1.size()) });
	memory->WriteArray(id, DOT_SEQUENCE, dotSequence1, true);
	for (int i = 0; i < dotSequence2.size(); i++) {
		if (dotSequence2[i] == DOT_SMALL) dotSequence2[i] = 1;
		if (dotSequence2[i] == DOT_MEDIUM) dotSequence2[i] = 2;
		if (dotSequence2[i] == DOT_LARGE) dotSequence2[i] = 3;
	}
	memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN_REFLECTION, { static_cast<int>(dotSequence2.size()) });
	memory->WriteArray(id, DOT_SEQUENCE_REFLECTION, dotSequence2, true);
	gen->panel.startpoints = { { 0, 0}, { gen->width - 1, 0 }, { 0, gen->height - 1 }, { gen->width - 1, gen->height - 1 } };
	if (id == SHIPWRECK_VAULT) gen->setConfig(DisableFlash);
	gen->write(id);
	gen->resetConfig();
	return true;
}

void Special::generateRGBStonePuzzleN(PanelID id)
{
	while (true) {
		gen->setConfigOnce(DisableWrite);
		gen->generate(id);
		int amount = 16;
		std::set<SymbolColor> used;
		std::vector<SymbolColor> colors = { Black, White, Red, Green, Blue, Cyan, Yellow, Magenta };
		while (amount > 0) {
			SymbolColor c = gen->pickRandom(colors);
			if (gen->placeStones(c, 1)) {
				amount--;
				used.insert(c);
			}
		}
		if (used.size() < 5) continue;
		gen->setConfigOnce(WriteColors);
		gen->setConfigOnce(DisableFlash);
		gen->write(id);
		return;
	}
}

void Special::generateRGBStarPuzzleN(PanelID id)
{
	while (true) {
		gen->setConfigOnce(DisableWrite);
		gen->generate(id);
		int amount = 16;
		std::set<SymbolColor> used;
		std::set<SymbolColor> colors = { Black, White, Red, Green, Blue, Cyan, Yellow, Magenta };
		while (amount > 0) {
			SymbolColor c = gen->pickRandom(colors);
			if (gen->placeStars(c, 2)) {
				amount -= 2;
				used.insert(c);
				if (used.size() == 4) colors = used;
			}
		}
		if (used.size() < 4)
			continue;
		gen->setConfigOnce(WriteColors);
		gen->setConfigOnce(DisableFlash);
		gen->write(id);
		return;
	}
}

void Special::generateRGBStonePuzzleH(PanelID id) {
	while (true) {
		gen->setConfigOnce(DisableWrite);
		gen->generate(id);
		int amount = 10;
		std::set<SymbolColor> used;
		std::vector<SymbolColor> colors = { Black, Red, Green, Blue, Magenta, Yellow };
		while (amount > 0) {
			SymbolColor c = gen->pickRandom(colors);
			if (gen->placeStones(c, 1)) {
				amount--;
				used.insert(c);
			}
		}
		if (used.size() < 5) continue;
		if (!gen->placeErasers({ White }, { Stone | pickRandom(colors) })) {
			continue;
		}
		Point eraserPos;
		for (int x = 1; x < gen->panel.width; x += 2) {
			for (int y = 1; y < gen->panel.height; y += 2) {
				if (gen->getSymbolShape(gen->get(x, y)) == Eraser) {
					eraserPos = { x, y };
					break;
				}
			}
		}
		int count = 0;
		for (Point dir : Panel::DIRECTIONS_2) {
			if (gen->get(eraserPos + dir) == 0) count++;
		}
		if (count < 2) continue;
		gen->setConfigOnce(WriteColors);
		gen->setConfigOnce(DisableFlash);
		gen->write(id);
		return;
	}
}

void Special::generateRGBDotPuzzleH(PanelID id) {
	//TODO: Doing these writes here will mess up double mode.
	// Do the colors look correct using this config mode instead?
	//memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR, {1, 0, 0, 1});
	//memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR_A, { 0, 1, 1, 1 });
	//memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR_B, { 1, 1, 0, 1 });
	//memory->WritePanelData<Color>(id, ACTIVE_COLOR, { 0, 1, 1, 1 });
	//memory->WritePanelData<Color>(id, REFLECTION_PATH_COLOR, { 1, 1, 0, 1 });
	gen->setConfig(CyanAndYellowLines);
	gen->setGridSize(7, 7);
	gen->setSymmetry(Rotational);
	gen->setSymbol(Exit, 0, 14); gen->setSymbol(Exit, 14, 0);
	gen->setSymbol(Exit, 0, 0); gen->setSymbol(Exit, 14, 14);
	gen->setConfigOnce(DisableFlash);
	gen->generate(id, Dot_Intersection | Cyan, 2, Dot_Intersection | Yellow, 4, Dot_Intersection, 6,
		Triangle | Orange, 4, Start, 4);
	gen->resetConfig();
}

void Special::generateJungleVault(PanelID id)
{
	//This panel won't render symbols off the grid, so all I can do is move the dots around

	//a: 4 9 16 23, b: 12, c: 1, ab: 3 5 6 10 11 15 17 18 20 21 22, ac: 14, bc: 2 19, all: 7 8 13
	std::vector<std::vector<int>> sols = {
		{ 0, 5, 10, 15, 20, 21, 16, 11, 6, 7, 8, 3, 4, 9, 14, 13, 18, 17, 22, 23, 24, 25 },
		{ 0, 5, 10, 15, 20, 21, 22, 17, 12, 11, 6, 7, 2, 3, 8, 13, 18, 19, 24, 25 },
		{ 0, 1, 2, 7, 8, 13, 14, 19, 24, 25 } };
	std::vector<std::vector<int>> dotPoints1 = { { 4, 9, 16, 23 }, { 2, 19 }, { 2, 19 } };
	std::vector<std::vector<int>> dotPoints2 = { { 7, 8, 13 }, { 3, 5, 6, 10, 11, 15, 17, 18, 20, 21, 22 }, { 14, 1 } };
	gen->initPanel(id);
	gen->clear();
	int sol = Random::rand() % sols.size();
	auto[x1, y1] = gen->panel.indexToPoint(gen->pickRandom(dotPoints1[sol]));
	auto[x2, y2] = gen->panel.indexToPoint(gen->pickRandom(dotPoints2[sol]));
	gen->set(x1, y1, Dot_Intersection);
	gen->set(x2, y2, Dot_Intersection);
	gen->write(id);
	memory->WritePanelData(id, SEQUENCE_LEN, static_cast<int>(sols[sol].size()));
	memory->WriteArray(id, SEQUENCE, sols[sol], true);
}

void Special::generateApplePuzzle(PanelID id, bool changeExit, bool flip)
{
	//Is there a way to move the apples? Might be impossible without changing the game files.
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<int> sequence = memory->ReadArray<int>(id, SEQUENCE, 6);
	int exit = sequence[5];
	std::vector<int> exits;
	if (changeExit) {
		for (int i = 15; i < 31; i++) {
			if (intersectionFlags[i] == 9) exits.push_back(i);
		}
		int newExit = popRandom(exits);
		intersectionFlags[newExit] = 1;
		intersectionFlags[exit] = 9; //Gets rid of old exit
		for (int i = 5; i > 1; i--) {
			sequence[i] = newExit;
			newExit = (newExit - 1) / 2;
		}
		int numConnections = memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
		std::vector<int> connections_a = memory->ReadArray<int>(id, DOT_CONNECTION_A, numConnections);
		std::vector<int> connections_b = memory->ReadArray<int>(id, DOT_CONNECTION_B, numConnections);
		for (int i = 0; i < numConnections; i++) {
			if (connections_b[i] == exit) {
				connections_a[i] = sequence[4];
				connections_b[i] = sequence[5];
			}
		}
		memory->WriteArray(id, DOT_FLAGS, intersectionFlags);
		memory->WriteArray(id, SEQUENCE, sequence, true);
		memory->WriteArray(id, DOT_CONNECTION_A, connections_a);
		memory->WriteArray(id, DOT_CONNECTION_B, connections_b);
		memory->WritePanelData(id, NEEDS_REDRAW, 1);
	}
	if (flip) {
		std::vector<float> intersections = memory->ReadArray<float>(id, DOT_POSITIONS, numIntersections * 2);
		for (int i = 0; i < intersections.size(); i += 2) {
			intersections[i] = 1 - intersections[i];
		}
		memory->WriteArray(id, DOT_POSITIONS, intersections);
		memory->WritePanelData(id, NEEDS_REDRAW, 1);
	}
}

void Special::generateKeepLaserPuzzle(PanelID id, const std::set<Point>& path1, const std::set<Point>& path2, const std::set<Point>& path3, const std::set<Point>& path4, std::vector<std::pair<int, int>> symbols)
{
	PuzzleSymbols psymbols(symbols);
	PuzzleSymbols psymbolsBackup = psymbols;
	gen->resetConfig();
	gen->lineThickness = 0.4f;
	gen->setGridSize(10, 11);
	gen->starts = { { 0, 2 },{ 20, 10 },{ 8, 14 },{ 0, 22 },{ 20, 22 } };
	gen->exits = { { 0, 18 },{ 0, 0 },{ 20, 0 },{ 0, 12 } };
	if (psymbols.getNum(Triangle) > 0) gen->exits.insert({ 0, 22 });
	gen->initPanel(id);
	gen->clear();
	std::vector<Point> gaps = { { 17, 20 },{ 16, 19 },{ 20, 17 },{ 15, 14 },{ 15, 16 },{ 18, 13 },{ 20, 13 }, //Yellow Internal Puzzle Walls
	{ 11, 12 }, { 11, 14 }, { 11, 16 }, { 11, 18 }, { 11, 20 }, { 11, 22 }, { 10, 13 }, { 10, 15 }, { 10, 17 }, { 10, 19 }, { 10, 21 }, { 12, 13 }, { 13, 12 }, { 16, 13 }, //Yellow External Puzzle Walls
	{ 10, 11 },{ 12, 11 },{ 14, 11 },{ 16, 11 },{ 18, 11 },{ 11, 0 },{ 11, 2 },{ 11, 4 },{ 11, 8 },{ 11, 10 },{ 14, 1 },{ 16, 1 },{ 18, 1 },{ 20, 1 }, //Pink Puzzle Walls
	{ 2, 1 },{ 4, 1 },{ 6, 1 },{ 8, 1 },{ 0, 11 },{ 2, 11 },{ 4, 11 },{ 6, 11 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 9, 8 },{ 9, 10 },{ 9, 12 }, //Green Puzzle Walls
	{ 0, 13 },{ 2, 13 },{ 4, 13 },{ 6, 13 },{ 9, 14 },{ 9, 16 },{ 9, 20 },{ 9, 22 }, //Blue Puzzle Walls
	};
	std::vector<Point> pathPoints = { { 14, 13 },{ 14, 12 },{ 15, 12 },{ 16, 12 },{ 17, 12 },{ 18, 12 },{ 19, 12 },{ 20, 12 },{ 20, 11 },
	{ 11, 6 },{ 10, 6 },{ 10, 5 },{ 10, 4 },{ 10, 3 },{ 10, 2 },{ 10, 1 },{ 10, 0 },{ 9, 0 },{ 8, 0 },{ 7, 0 },{ 6, 0 },{ 5, 0 },{ 4, 0 },{ 3, 0 },
	{ 2, 0 },{ 1, 0 },{ 0, 0 },{ 0, 1 },{ 8, 11 },{ 8, 12 },{ 8, 13 } };
	std::vector<Point> pathPoints2 = { { 9, 18 },{ 10, 18 } }; //For exiting out the right side of the last puzzle
	std::vector<Point> pathPoints3 = { { 8, 14 },{ 7, 14 },{ 6, 14 },{ 5, 14 },{ 4, 14 },{ 3, 14 },{ 2, 14 },{ 1, 14 },{ 0, 14 },{ 0, 15 },{ 0, 16 },{ 0, 17 } }; //For hard mode
	for (Point p : pathPoints) gen->setPath(p);
	for (Point p : gaps) gen->set(p, p.x % 2 == 0 ? Gap_Column : Gap_Row);
	for (Point p : path1) gen->setPath(Point(p.x + 12, p.y + 14));
	for (Point p : path2) gen->setPath(Point(p.x + 12, p.y + 2));
	for (Point p : path3) gen->setPath(Point(8 - p.x, 8 - p.y + 2));
	if (psymbols.getNum(Triangle) == 0) { //Normal mode
		for (Point p : path4) gen->setPath(Point(p.x, p.y + 14));
		if (path4.count(Point({ 8, 4 }))) for (Point p : pathPoints2) gen->setPath(p);
	}
	else { //Hard mode
		for (Point p : path4) gen->setPath(Point(8 - p.x, 8 - p.y + 14));
		for (Point p : pathPoints3) gen->setPath(p);
	}

	std::vector<std::string> solution; //For debugging only
	for (int y = 0; y < gen->panel.height; y++) {
		std::string row;
		for (int x = 0; x < gen->panel.width; x++) {
			if (gen->get(x, y) == PATH) {
				row += "xx";
			}
			else row += "    ";
		}
		solution.push_back(row);
	}

	while (!gen->placeSymbols(psymbols)) {
		for (int x = 0; x < gen->panel.width; x++)
			for (int y = 0; y < gen->panel.height; y++)
				if (gen->get(x, y) != PATH && (gen->get(x, y) & 0x1fffff) != Gap)
					gen->set(x, y, 0);
		gen->openpos = gen->gridpos;
		for (int i = 0; i < psymbols.symbols[Poly].size(); i++) {
			psymbols.symbols[Poly][i].second = psymbolsBackup.symbols[Poly][i].second + Random::rand() % 3 - Random::rand() % 3;
			if (psymbols.symbols[Poly][i].second < 1) psymbols.symbols[Poly][i].second = 1;
		}
	}

	for (int x = 0; x < gen->panel.width; x++)
		for (int y = 0; y < gen->panel.height; y++)
			if ((gen->get(x, y) & 0x1fffff) == Gap)
				gen->set(x, y, OPEN);
	gen->set(10, 10, GAP);
	gen->set(12, 12, NO_POINT);
	gen->set(10, 12, NO_POINT);
	gen->set(10, 14, NO_POINT);
	gen->set(10, 16, NO_POINT);
	gen->set(10, 18, ENDPOINT);
	gen->set(10, 20, NO_POINT);
	gen->set(10, 22, NO_POINT);
	gen->write(id);
	if (psymbols.getNum(Triangle) > 0) (new KeepWatchdog())->start();
}

void Special::generateMountaintop(PanelID id, const std::vector<std::pair<int, int>>& symbolVec)
{
	std::vector<std::vector<Point>> perspectiveU = {
	{ { 0, 3 },{ 1, 2 },{ 1, 4 },{ 2, 1 },{ 2, 3 },{ 3, 2 },{4, 7}, { 6, 1 },{6, 7}, { 7, 0 },{ 7, 2 },{ 7, 4 },{7, 6}, { 8, 1 },{ 8, 3 },{ 8, 5 },{ 9, 2 },{ 9, 4 },{ 9, 6 } },
	{ { 0, 3 },{ 0, 5 },{ 1, 2 },{ 1, 4 },{ 2, 1 },{ 2, 3 },{ 3, 2 },{ 3, 4 },{ 4, 3 },{ 4, 5 },{ 5, 4 },{ 6, 3 },{ 6, 5 },{ 7, 4 },{ 8, 5 },{8, 9},{9, 2},{ 9, 6 } }, 
	{ { 0, 3 },{ 0, 5 },{ 0, 7 },{ 1, 2 },{ 1, 4 },{ 1, 6 },{ 1, 8 },{ 2, 3 },{ 2, 5 },{ 2, 7 },{ 3, 4 },{ 3, 6 },{ 4, 5 },{ 4, 7 },{ 5, 6 },{ 6, 5 },{ 6, 7 },{ 7, 4 },{ 7, 6 },{ 8, 7 },{8, 9},{10, 5} }, 
	{ { 0, 7 },{ 1, 6 },{ 1, 8 },{ 2, 5 },{ 3, 4 },{4, 7}, {6, 7}, { 7, 4 },{ 7, 6 },{ 7, 8 },{ 8, 5 },{ 8, 7 },{ 8, 9 },{ 9, 6 },{ 9, 8 },{ 10, 7 } }, 
	{ {1, 2},{2, 1}, { 2, 5 },{ 4, 3 },{ 4, 5 },{ 6, 3 },{ 6, 5 },{6, 9}, { 7, 4 },{7, 8}, { 8, 3 },{ 8, 5 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 3 },{ 10, 5 },{ 10, 7 } },
	{ {0, 7}, { 1, 4 },{2, 1}, { 2, 3 },{ 2, 5 },{ 3, 4 },{3, 6}, { 4, 3 },{ 5, 2 },{ 6, 1 },{ 6, 3 },{ 7, 0 },{ 7, 2 },{ 7, 4 },{ 8, 1 },{ 8, 3 },{ 8, 5 },{ 9, 4 },{ 9, 6 },{ 10, 3 },{ 10, 5 },{ 10, 7 } },
	};
	if (gen->getSymbolShape(symbolVec[0].first) == Triangle) { //Hard mode
		perspectiveU = { { { 1, 2 },{ 1, 4 },{ 3, 0 },{ 3, 2 },{ 3, 4 },{ 3, 6 },{ 5, 8 },{ 6, 7 },{ 7, 0 },{ 7, 2 },{ 7, 4 },{ 8, 1 },{ 8, 3 },{ 8, 5 },{ 8, 9 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 3 },{ 10, 5 },{ 10, 7 } } };
	}
	std::vector<std::vector<Point>> perspectiveL = {
	{ { 0, 3 },{ 0, 5 },{ 1, 2 },{ 1, 4 },{ 2, 1 },{ 2, 3 },{2, 9}, { 3, 2 },{ 5, 2 },{ 6, 1 },{ 6, 3 },{ 7, 0 },{ 7, 2 },{ 7, 4 },{ 8, 3 },{ 8, 5 },{ 9, 4 },{ 9, 6 } }, 
	{ { 0, 3 },{ 0, 5 },{ 1, 2 },{ 1, 4 },{ 2, 1 },{ 2, 3 },{ 3, 0 },{ 3, 2 },{ 4, 1 },{ 4, 3 },{ 5, 2 },{ 5, 4 },{ 6, 3 },{ 6, 5 },{ 7, 4 },{8, 1}, { 8, 5 },{ 9, 6 }, {10, 3} }, 
	{ { 0, 3 },{ 0, 5 },{ 0, 7 },{ 1, 2 },{ 1, 4 },{ 1, 6 },{ 1, 8 },{ 2, 3 },{ 2, 5 },{ 2, 7 },{ 3, 4 },{ 3, 6 },{ 4, 5 },{ 4, 7 },{ 5, 6 },{ 6, 5 },{ 6, 7 },{ 7, 4 },{ 7, 6 },{8, 1}, { 8, 7 }, {10, 5} }, 
	{ { 0, 7 },{ 1, 6 },{ 2, 5 },{2, 9}, { 3, 4 },{4, 7},{5, 8}, { 6, 9 },{ 7, 4 },{ 7, 6 },{ 7, 8 },{ 7, 10 },{ 8, 7 },{ 8, 9 },{ 9, 8 } }, 
	{ {0, 3},{0, 7},{1, 2},{1, 8}, { 2, 5 },{2, 9}, { 4, 3 },{ 4, 5 },{ 6, 3 },{ 6, 5 },{ 7, 4 },{ 8, 3 },{ 8, 5 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 3 },{ 10, 5 },{ 10, 7 } }, 
	{ { 1, 2 },{ 1, 4 },{ 2, 3 },{ 3, 0 },{ 3, 2 },{ 4, 1 },{ 4, 3 },{4, 5}, { 5, 2 },{5, 6}, {6, 7}, { 7, 4 },{ 8, 3 },{ 8, 5 },{8, 7}, { 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 3 },{ 10, 5 },{ 10, 7 } }
	};
	std::vector<std::vector<Point>> perspectiveC = {
	{ { 0, 3 },{0, 7}, { 1, 2 },{ 1, 4 },{1, 8}, { 2, 1 },{ 2, 3 },{ 3, 2 },{ 6, 1 },{ 7, 0 },{ 7, 2 },{ 7, 4 },{ 8, 1 },{ 8, 3 },{ 8, 5 },{ 9, 2 },{ 9, 4 },{ 9, 6 } },
	{ { 0, 7 },{ 1, 6 },{ 2, 5 },{ 3, 4 },{4, 7},{5, 2}, {5, 4}, {5, 6}, { 6, 9 },{ 7, 4 },{ 7, 6 },{ 7, 8 },{ 7, 10 },{ 8, 7 },{ 8, 9 },{ 9, 8 } }
	};

	std::vector<std::shared_ptr<Generate>> gens;
	for (int i = 0; i < 3; i++) gens.push_back(std::make_shared<Generate>());
	for (std::shared_ptr<Generate> g : gens) {
		g->setGridSize(5, 5);
		g->setSymbol(Gap, 5, 0);
		g->setSymbol(Gap, 5, 10);
		g->setConfig(PreserveStructure);
		g->setConfig(ShortPath);
		g->setConfig(DecorationsOnly);
	}
	gens[0]->starts = { { 2, 0 } }; gens[1]->starts = { { 2, 10 } }; gens[2]->starts = { { 10, 4 },{ 10, 6 } };
	gens[0]->exits = { { 8, 10 } }; gens[1]->exits = { { 8, 0 } }; gens[2]->exits = { { 0, 4 },{ 0, 6 } };
	gens[0]->setObstructions(perspectiveU); gens[1]->setObstructions(perspectiveL); gens[2]->setObstructions(perspectiveC);
	gen->generateMulti(id, gens, symbolVec);
}

void Special::generateMultiPuzzle(std::vector<PanelID> ids, const std::vector<std::vector<std::pair<int, int>>>& symbolVec, bool flip) {
	gen->resetConfig();
	gen->setConfigOnce(DisableWrite);
	gen->generate(ids[0]);
	std::vector<PuzzleSymbols> symbols;
	for (auto sym : symbolVec) symbols.emplace_back(PuzzleSymbols(sym));
	std::vector<Generate> gens;
	for (int i = 0; i < ids.size(); i++) gens.emplace_back(Generate());
	for (int i = 0; i < ids.size(); i++) {
		gens[i].setConfig(DisableWrite);
		gens[i].setConfig(WriteColors);
		if (symbols[i].getNum(Poly)  - symbols[i].getNum(Eraser) > 1) gens[i].setConfig(RequireCombineShapes);
	}
	while (!generateMultiPuzzle(ids, gens, symbols, gen->path)) {
		gen->generate(ids[0]);
	}
	for (int i = 0; i < ids.size(); i++) {
		gens[i].write(ids[i]);
		gen->incrementProgress();
		if (symbolVec[0][0].first == (Triangle | Orange)) { //Hard mode
			int numIntersections = memory->ReadPanelData<int>(ids[i], NUM_DOTS);
			std::vector<float> intersections = memory->ReadArray<float>(ids[i], DOT_POSITIONS, numIntersections * 2);
			for (int j = 0; j < intersections.size(); j += 2) {
				float x = intersections[j], y = intersections[j + 1];
				if (i == 1) { intersections[j] = y; intersections[j + 1] = x; }
				if (i == 2) { intersections[j + 1] = 1 - y; }
				if (i == 3) { intersections[j] = 1 - y; intersections[j + 1] = x; }
				if (i == 4) { intersections[j] = 1 - x; }
				if (i == 5) { intersections[j] = 1 - x; intersections[j + 1] = 1 - y; }
			}
			memory->WriteArray(ids[i], DOT_POSITIONS, intersections);
		}
	}
	gen->resetConfig();
	gen->resetVars();
}

bool Special::generateMultiPuzzle(std::vector<PanelID> ids, std::vector<Generate>& gens, const std::vector<PuzzleSymbols>& symbols, const std::set<Point>& path) {
	for (int i = 0; i < ids.size(); i++) {
		gens[i].customGrid.clear();
		gens[i].setCustomPath(path);
		int fails = 0;
		while (!gens[i].generate(ids[i], symbols[i])) {
			if (fails++ > 20)
				return false;
		}
	}
	std::vector<std::string> solution; //For debugging only
	for (int y = 0; y < 11; y++) {
		std::string row;
		for (int x = 0; x < 11; x++) {
			if (path.count(Point(x, y))) {
				row += "xx";
			}
			else row += "    ";
		}
		solution.push_back(row);
	}
	return true;
}

void Special::generate2Bridge(PanelID id1, PanelID id2)
{
	std::vector<std::shared_ptr<Generate>> gens;
	for (int i = 0; i < 3; i++) gens.push_back(std::make_shared<Generate>());
	for (std::shared_ptr<Generate> g : gens) {
		g->setConfig(DisableWrite);
		//g->setConfig(DisableReset);
		g->setConfig(DecorationsOnly);
		g->setConfig(ShortPath);
		g->setConfig(WriteColors);
	}
	while (!generate2Bridge(id1, id2, gens));
	gens[1]->write(id1);
	gens[1]->write(id2);
	gen->incrementProgress();
}

bool Special::generate2Bridge(PanelID id1, PanelID id2, std::vector<std::shared_ptr<Generate>> gens)
{
	for (int i = 0; i < gens.size(); i++) {
		gens[i]->customGrid.clear();
		gens[i]->setCustomPath(std::set<Point>());
		std::vector<Point> walls = { { 12, 1 },{ 12, 3 },{ 3, 8 },{ 9, 8 } };
		for (Point p : walls) gens[i]->setSymbol(Gap, p.x, p.y);
		if (i % 2 == 0) {
			gens[i]->setObstructions({ { 5, 8 },{ 6, 7 },{ 7, 8 } });
		}
		else {
			gens[i]->setObstructions({ { 5, 0 },{ 6, 1 },{ 7, 0 } });
		}
	}

	int steps = 2;

	gens[0]->exits = { { 12, 8 } };
	gens[1]->exits = { { 0, 0 } };

	if (!gens[0]->tryGenerate(id1, { {Poly | Rotate | Yellow, 1}, {Star | Yellow, 1} }))
		return false;

	gens[1]->setCustomPath(gens[0]->path);
	gens[1]->customPath.clear();
	gens[1]->customGrid = gens[0]->panel.getGrid();

	if (!gens[1]->tryGenerate(id2, { { Poly | Rotate | Yellow, 1 },{ Star | Yellow, 3 } }))
		return false;

	if (gens[0]->get(11, 1) != 0 || gens[1]->get(11, 1) != 0 || gens[1]->get(11, 3) != 0 || gens[1]->get(11, 3) != 0)
		return false;
	
	//Make sure both shapes weren't blocked off by the same path
	int shapeCount = 0;
	for (int x = 1; x < gens[1]->panel.width; x += 2) {
		for (int y = 1; y < gens[1]->panel.height; y += 2) {
			if (gens[1]->getSymbolShape(gens[1]->get(x, y)) == Poly && gens[1]->getRegion(Point(x, y)).size() == gens[0]->getRegion(Point(x, y)).size())
				if (++shapeCount == 2) return false;
		}
	}

	for (int x = 1; x < gens[1]->panel.width; x += 2) {
		for (int y = 1; y < gens[1]->panel.height; y += 2) {
			if (gens[1]->getSymbolShape(gens[1]->get(x, y)) == Star) {
				if (!gens[1]->getRegion(Point(x, y)).count({ 9, 7 }))
					continue;
				gens[1]->set(x, y, Eraser | White);
				return true;
			}
		}
	}

	return false;
}

void Special::generate2BridgeH(PanelID id1, PanelID id2)
{
	std::vector<std::shared_ptr<Generate>> gens;
	for (int i = 0; i < 3; i++) gens.push_back(std::make_shared<Generate>());
	for (std::shared_ptr<Generate> g : gens) {
		g->setConfig(DisableWrite);
		//g->setConfig(DisableReset);
		g->setConfig(DecorationsOnly);
		g->setConfig(ShortPath);
		g->setConfig(WriteColors);
	}
	while (!generate2BridgeH(id1, id2, gens));
	
	
	gens[0]->write(id1);
	gens[0]->write(id2);
	gen->incrementProgress();

	int numIntersections = memory->ReadPanelData<int>(id1, NUM_DOTS);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id1, DOT_FLAGS, numIntersections);
	std::vector<int> intersectionFlags2 = memory->ReadArray<int>(id2, DOT_FLAGS, numIntersections);
	for (int x = 0; x < gens[0]->panel.width; x += 2) {
		for (int y = 0; y < gens[0]->panel.height; y += 2) {
			if (gens[0]->get(x, y) == Dot_Intersection) {
				intersectionFlags[gens[0]->panel.pointToIndex(x, y)] = Dot_Intersection;
				intersectionFlags2[gens[0]->panel.pointToIndex(x, y)] = Dot_Intersection;
			}
			else {
				intersectionFlags[gens[0]->panel.pointToIndex(x, y)] &= ~Dot;
				intersectionFlags2[gens[0]->panel.pointToIndex(x, y)] &= ~Dot;
			}
		}
	}
	memory->WriteArray(id1, DOT_FLAGS, intersectionFlags);
	memory->WriteArray(id2, DOT_FLAGS, intersectionFlags2);
	memory->WritePanelData(id1, NEEDS_REDRAW, 1);
	memory->WritePanelData(id2, NEEDS_REDRAW, 1);
	(new BridgeWatchdog())->start();
}

bool Special::generate2BridgeH(PanelID id1, PanelID id2, std::vector<std::shared_ptr<Generate>> gens)
{
	for (int i = 0; i < gens.size(); i++) {
		gens[i]->customGrid.clear();
		gens[i]->setCustomPath(std::set<Point>());
		std::vector<Point> walls = { { 12, 1 },{ 12, 3 },{ 3, 8 },{ 9, 8 } };
		for (Point p : walls) gens[i]->setSymbol(Gap, p.x, p.y);
		if (i == 0) {
			gens[i]->setObstructions({ { 3, 0 },{ 3, 2 },{ 3, 4 },{4, 5}, {5, 6}, {6, 7}, {8, 7}, {9, 0}, {9, 2}, {9, 4}, {10, 5}, {12, 5} });
		}
		else {
			gens[i]->setObstructions({ {1, 4}, { 1, 6 },{ 1, 8 }, {4, 1}, {6, 7}, {7, 8} });
		}
	}

	gens[0]->exits = { { 12, 8 } };
	gens[1]->exits = { { 0, 0 } };

	gens[0]->generate(id1);
	gens[1]->setCustomPath(gens[0]->path);
	gens[1]->customPath.clear();
	gens[1]->customGrid = gens[0]->panel.getGrid();
	gens[1]->generate(id2);
	std::vector<Point> points = { {12, 2}, { 11, 2 }, { 10, 2 }, { 10, 3 }, { 10, 4 }, { 9, 4 }, { 8, 4 }, { 7, 4 }, { 6, 4 } };
	for (int i = 0; i < points.size(); i++) {
		if (gens[1]->get(points[i]) == PATH) break;
		gens[1]->setPath(points[i]);
		if (i == points.size() - 1) return false;
	}
	int state = 0;
	for (int i = 8; i >= 0; i--) {
		if (state == 0) {
			if (gens[1]->get(i, 2) == PATH) state++;
		}
		else if (state == 1) {
			if (gens[1]->get(i, 2) != PATH) {
				state++;
				gens[1]->setPath({ i, 2 });
			}
		}
		else if (state == 2) {
			if (gens[1]->get(i, 2) == PATH && gens[0]->get(i, 2) != PATH) break;
			if (gens[1]->get(i, 2) == PATH && gens[0]->get(i, 2) == PATH) return false;
			gens[1]->setPath({ i, 2 });
		}
	}
	state = 0;
	for (int i = 12; i >= 0; i--) {
		if (state == 0) {
			if (gens[1]->get(i, 6) == PATH) state++;
		}
		else if (state == 1) {
			if (gens[1]->get(i, 6) != PATH) {
				state++;
				gens[1]->setPath({ i, 6 });
				gens[0]->set(i + 1, 6, Dot_Intersection);
			}
		}
		else if (state == 2) {
			if (gens[1]->get(i, 6) == PATH && gens[0]->get(i, 6) != PATH) break;
			if (gens[1]->get(i, 6) == PATH && gens[0]->get(i, 6) == PATH) return false;
			gens[1]->setPath({ i, 6 });
		}
	}

	int count = 0;
	std::set<Point> open = gens[0]->gridpos;
	while (open.size() > 0) {
		Point pos = *(open.begin());
		std::set<Point> region = gens[1]->getRegion(pos);
		if (region.size() == 1 || region.size() > 6) return false;
		int symbol = gens[0]->makeShapeSymbol(region, false, false);
		if (!symbol) return false;
		gens[0]->set(pickRandom(region), symbol | Yellow);
		for (Point p : region) open.erase(p);
		count++;
	}
	return count == 6;
}

bool checkShape(const std::set<Point>& shape, int direction) {
	//Make sure it is not off the grid
	for (Point p : shape) if (p.x < 0 || p.x > 7 || p.y < 0 || p.y > 7)
		return false;
	//Make sure it is drawable
	std::vector<Point> points1, points2;
	if (direction == 0) {
		points1 = { { 1, 7 },{ 3, 7 },{ 5, 7 },{ 7, 7 },{ 7, 5 },{ 7, 3 },{ 7, 1 } };
		points2 = { { 1, 7 },{ 1, 5 },{ 1, 3 },{ 1, 1 },{ 3, 1 },{ 5, 1 },{ 7, 1 } };
	}
	else {
		points1 = { { 7, 7 },{ 5, 7 },{ 3, 7 },{ 1, 7 },{ 1, 5 },{ 1, 3 },{ 1, 1 } };
		points2 = { { 7, 7 },{ 7, 5 },{ 7, 3 },{ 7, 1 },{ 5, 1 },{ 3, 1 },{ 1, 1 } };
	}
	int count = 0;
	bool consecutive = false;
	for (Point p : points1) {
		if (shape.count(p)) {
			if (!consecutive) {
				count++;
				consecutive = true;
			}
		}
		else consecutive = false;
	}
	if (count == 1)
		return true;
	count = 0;
	consecutive = false;
	for (Point p : points2) {
		if (shape.count(p)) {
			if (!consecutive) {
				count++;
				consecutive = true;
			}
		}
		else consecutive = false;
	}
	if (count == 1)
		return true;
	return false;
}

void Special::generateMountainFloor()
{
	std::vector<PanelID> ids = { MOUNTAIN_META_UL, MOUNTAIN_META_UR, MOUNTAIN_META_DL, MOUNTAIN_META_DR };
	PanelID idfloor = MOUNTAIN_META_FLOOR;
	gen->resetConfig();
	std::vector<Point> floorPos = { { 3, 3 },{ 7, 3 },{ 3, 7 },{ 7, 7 } };
	gen->openPos = std::set<Point>(floorPos.begin(), floorPos.end());
	gen->setConfig(DisableWrite);
	gen->setSymmetry(Vertical);
	//Make sure no duplicated symbols
	std::set<int> sym;
	do {
		gen->generate(idfloor, Poly, 4);
		sym.clear();
		for (Point p : floorPos) sym.insert(gen->get(p));
	} while (sym.size() < 4);

	int rotateIndex = Random::rand() % 3;
	for (int i = 0; i < 4; i++) {
		int symbol = gen->get(floorPos[i]);
		//Convert to shape
		Shape shape;
		for (int j = 0; j < 16; j++) {
			if (symbol & (1 << (j + 16))) {
				shape.emplace(Point((j % 4) * 2 + 1, 8 - ((j / 4) * 2 + 1)));
			}
		}
		//Translate randomly
		Shape newShape;
		do {
			Point shift = Point((Random::rand() % 4) * 2, -(Random::rand() % 4) * 2);
			newShape.clear();
			for (Point p : shape) newShape.insert(p + shift);
		} while (!checkShape(newShape, i % 2));
		if (i == rotateIndex) {
			symbol = gen->makeShapeSymbol(newShape, true, false);
			if (symbol == 0) {
				symbol = gen->get(floorPos[i]);
				rotateIndex++;
			}
		}

		Generate g;
		for (Point p : newShape) {
			for (Point dir : Panel::DIRECTIONS_2) {
				if (!newShape.count(p + dir)) {
					g.setSymbol(PATH, p.x + dir.x / 2, p.y + dir.y / 2);
				}
			}
		}
		g.setCustomPath({ {0, 0} }); //Just to stop it from trying to make a path
		g.setConfig(DecorationsOnly);
		g.setConfig(DisableWrite);
		if (i == rotateIndex) g.generate(ids[i], { });
		else
		{
			g.generate(ids[i], Poly, 1, Eraser | Green, 1);
			std::set<Point> covered;
			int decoyShape;
			for (int x = 1; x <= 7; x += 2)
				for (int y = 1; y <= 7; y += 2)
					if (g.get(x, y) != 0) {
						covered.emplace(Point(x, y));
						if (g.getSymbolShape(g.get(x, y)) == Poly) decoyShape = g.get(x, y);
					}
			for (Point p : covered) newShape.erase(p);
			if (newShape.size() == 0 || decoyShape == symbol) {
				i--;
				continue;
			}
		}
		Point pos = pickRandom(newShape);
		g.setVal(symbol, pos.x, pos.y);
		g.write(ids[i]);
	}
	gen->incrementProgress();
	gen->resetVars();
	gen->resetConfig();
}

void Special::generateMountainFloorH()
{
	std::vector<PanelID> ids = { MOUNTAIN_META_UL, MOUNTAIN_META_UR, MOUNTAIN_META_DL, MOUNTAIN_META_DR };
	PanelID idfloor = MOUNTAIN_META_FLOOR;
	gen->resetConfig();
	std::vector<Point> floorPos = { { 3, 3 },{ 7, 3 },{ 3, 7 },{ 7, 7 } };
	gen->openPos = std::set<Point>(floorPos.begin(), floorPos.end());
	gen->setConfig(DisableWrite);
	gen->setConfig(MountainFloorH);
	gen->setSymmetry(Rotational);
	gen->setSymbol(Start, 0, 10); gen->setSymbol(Start, 10, 0);
	gen->setSymbol(Exit, 0, 0); gen->setSymbol(Exit, 10, 10);
	//Make sure no duplicated symbols
	std::set<int> sym;
	do {
		gen->generate(idfloor, Poly, 6);
		sym.clear();
		for (Point p : floorPos) sym.insert(gen->get(p));
	} while (sym.size() < 4);

	int combine = 0;
	for (int i = 0; i < 4; i++) {
		int symbol = gen->get(floorPos[i]);
		//Convert to shape
		Shape shape;
		for (int j = 0; j < 16; j++) {
			if (symbol & (1 << (j + 16))) {
				shape.emplace(Point((j % 4) * 2 + 1, 8 - ((j / 4) * 2 + 1)));
			}
		}
		//Translate randomly
		Shape newShape;
		int fails = 0;
		do {
			if (fails++ == 50) {
				generateMountainFloorH();
				return;
			}
			Point shift = Point((Random::rand() % 4) * 2, -(Random::rand() % 4) * 2);
			newShape.clear();
			for (Point p : shape) newShape.insert(p + shift);
		} while (!checkShape(newShape, i % 2));

		Generate gen;
		for (Point p : newShape) {
			for (Point dir : Panel::DIRECTIONS_2) {
				if (!newShape.count(p + dir)) {
					gen.setSymbol(PATH, p.x + dir.x / 2, p.y + dir.y / 2);
				}
			}
		}
		gen.setCustomPath({ { 0, 0 } }); //Just to stop it from trying to make a path
		gen.setConfig(DecorationsOnly);
		gen.setConfig(DisableWrite);
		gen.setConfig(MountainFloorH);
		gen.setConfig(DisableCancelShapes);
		PuzzleSymbols symbols({ { Poly, 2 },{ Eraser | Green, 1 } });
		if (newShape.size() > 5) {
			if (combine == 0) symbols = PuzzleSymbols({ { Poly, 3 },{ Eraser | Green, 1 } });
			if (combine == 1) symbols = PuzzleSymbols({ { Poly, 3 },{ Poly | Negative | Cyan, 1 } });
			combine++;
		}
		fails = 0;
		while (!gen.generate(ids[i], symbols)) {
			if (fails++ > 50) {
				generateMountainFloorH();
				return;
			}
		}
		//Check that the symbols made it into the shape
		int count = 0;
		for (Point p : newShape) {
			if (gen.getSymbolShape(gen.get(p)) == Poly) count++;
			if (gen.getSymbolShape(gen.get(p)) == Eraser) count--;
		}
		if (count != (newShape.size() > 5 ? combine == 2 ? 4 : 2 : 1)) {
			i--;
			if (newShape.size() > 5) combine--;
			continue;
		}
		//Check that the symbols aren't the same
		std::set<int> symbolSet;
		for (Point p : gen.gridpos) {
			if (gen.getSymbolShape(gen.get(p)) == Poly) symbolSet.insert(gen.get(p));
		}
		if (symbolSet.size() <= 1) {
			i--;
			if (newShape.size() > 5) combine--;
			continue;
		}
		gen.write(ids[i]);
	}
	if (combine != 2) {
		generateMountainFloorH();
		return;
	}
	for (Point p : floorPos) gen->set(p, Poly);
	gen->write(idfloor);
	gen->resetConfig();
}

void Special::generatePivotPanel(PanelID id, Point gridSize, const std::vector<std::pair<int, int>>& symbolVec, bool colorblind) {
	int width = gridSize.x * 2 + 1, height = gridSize.y * 2 + 1;
	std::vector<std::shared_ptr<Generate>> gens;
	for (int i = 0; i < 3; i++) gens.push_back(std::make_shared<Generate>());
	for (std::shared_ptr<Generate> gen : gens) {
		gen->seed(Random::rand());
		gen->colorblind = colorblind;
		gen->setSymbol(Start, width / 2, height - 1);
		gen->setGridSize(gridSize.x, gridSize.y);
		gen->setConfig(FixBackground);
		gen->setConfig(DisableWrite);
	}
	gens[0]->setSymbol(Exit, 0, height / 2);
	gens[1]->setSymbol(Exit, width - 1, height / 2);
	gens[2]->setSymbol(Exit, width / 2, 0);
	gen->generateMulti(id, gens, symbolVec);
	gens[0]->panel.endpoints.clear();
	gens[0]->panel.setGridSymbol(0, height / 2, Exit, NoColor);
	gens[0]->panel.setGridSymbol(width - 1, height / 2, Exit, NoColor);
	gens[0]->panel.setGridSymbol(width / 2, 0, Exit, NoColor);
	gens[0]->setConfig(WriteColors);
	gens[0]->write(id);
	int style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	memory->WritePanelData<int>(id, STYLE_FLAGS, { style | IS_PIVOTABLE });
}

void Special::modifyGate(PanelID id)
{
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<float> intersections = memory->ReadArray<float>(id, DOT_POSITIONS, numIntersections * 2);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	if (intersectionFlags[24] == 0) return;
	int numConnections = memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	std::vector<int> connections_a = memory->ReadArray<int>(id, DOT_CONNECTION_A, numConnections);
	std::vector<int> connections_b = memory->ReadArray<int>(id, DOT_CONNECTION_B, numConnections);
	int style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	intersectionFlags[6] |= Start;
	intersectionFlags[18] |= Start;
	intersectionFlags[11] = Dot_Intersection;
	intersectionFlags[24] = 0;
	intersectionFlags.push_back(0x400001);
	intersections.push_back(0.5f);
	intersections.push_back(1 - intersections[51]);
	connections_a.push_back(24);
	connections_b.push_back(numIntersections);
	std::vector<int> symData;
	for (int i = 0; i < numIntersections + 1; i++) {
		bool pushed = false;
		for (int j = 0; j < numIntersections + 1; j++) {
			if (std::round(intersections[i * 2] * 30) == std::round(30 - intersections[j * 2] * 30) &&
				std::round(intersections[i * 2 + 1] * 30) == std::round(30 - intersections[j * 2 + 1] * 30)) {
				symData.push_back(j);
				pushed = true;
				break;
			}
		}
		if (!pushed) symData.push_back(0);
	}
	memory->WriteArray(id, DOT_FLAGS, intersectionFlags);
	memory->WriteArray(id, DOT_POSITIONS, intersections);
	memory->WriteArray(id, DOT_CONNECTION_A, connections_a);
	memory->WriteArray(id, DOT_CONNECTION_B, connections_b);
	memory->WritePanelData(id, NUM_DOTS, numIntersections + 1);
	memory->WritePanelData(id, NUM_CONNECTIONS, numConnections + 1);
	memory->WriteArray(id, REFLECTION_DATA, symData);
	Color successColor = memory->ReadPanelData<Color>(id, SUCCESS_COLOR_A);
	memory->WritePanelData(id, SUCCESS_COLOR_B, successColor);
	memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR, { 0.1f, 0.1f, 0.1f, 1 });
	memory->WritePanelData(id, STYLE_FLAGS, style | HAS_DOTS);
	memory->WritePanelData(id, NEEDS_REDRAW, 1);
}

void Special::addDecoyExits(int amount) {
	while (amount > 0) {
		Point pos;
		switch (Random::rand() % 4) {
		case 0: pos = Point(0, Random::rand() % gen->height); break;
		case 1: pos = Point(gen->width - 1, Random::rand() % gen->height); break;
		case 2: pos = Point(Random::rand() % gen->width, 0); break;
		case 3: pos = Point(Random::rand() % gen->width, gen->height - 1); break;
		}
		if (pos.x % 2) pos.x--;
		if (pos.y % 2) pos.y--;
		if (gen->exits.count(pos) || gen->exits.count(gen->getSymPoint(pos)))
			continue;
		gen->panel.setGridSymbol(pos.x, pos.y, Exit, NoColor);
		gen->exits.insert(pos);
		amount--;
	}
}

void Special::initSecretSymmetryGrid() {
	gen->setSymbol(Start, 0, 0);  gen->setSymbol(Start, 6, 0), gen->setSymbol(Start, 8, 0), gen->setSymbol(Start, 14, 0);
	gen->setSymbol(Start, 0, 6);  gen->setSymbol(Start, 6, 6), gen->setSymbol(Start, 8, 6), gen->setSymbol(Start, 14, 6);
	gen->setSymbol(Start, 0, 8);  gen->setSymbol(Start, 6, 8), gen->setSymbol(Start, 8, 8), gen->setSymbol(Start, 14, 8);
	gen->setSymbol(Start, 0, 14);  gen->setSymbol(Start, 6, 14), gen->setSymbol(Start, 8, 14), gen->setSymbol(Start, 14, 14);
	gen->setSymbol(Exit, 2, 0);  gen->setSymbol(Exit, 4, 0), gen->setSymbol(Exit, 10, 0), gen->setSymbol(Exit, 12, 0);
	gen->setSymbol(Exit, 2, 14);  gen->setSymbol(Exit, 4, 14), gen->setSymbol(Exit, 10, 14), gen->setSymbol(Exit, 12, 14);
	gen->setSymbol(Exit, 0, 2);  gen->setSymbol(Exit, 0, 4), gen->setSymbol(Exit, 0, 10), gen->setSymbol(Exit, 0, 12);
	gen->setSymbol(Exit, 14, 2);  gen->setSymbol(Exit, 14, 4), gen->setSymbol(Exit, 14, 10), gen->setSymbol(Exit, 14, 12);
}

void Special::initRotateGrid()
{
	gen->setSymbol(Start, 4, 0);  gen->setSymbol(Start, 10, 4), gen->setSymbol(Start, 6, 10), gen->setSymbol(Start, 0, 6);
	gen->setSymbol(Exit, 6, 0);  gen->setSymbol(Exit, 10, 6), gen->setSymbol(Exit, 4, 10), gen->setSymbol(Exit, 0, 4);
}

void Special::initPillarSymmetry(PanelID id, Symmetry symmetry)
{
	gen->setSymmetry(symmetry);
	switch (symmetry) {
	case PillarParallel:
		gen->setSymbol(Start, 0, gen->height - 1);  gen->setSymbol(Start, 6, gen->height - 1), gen->setSymbol(Exit, 0, 0);  gen->setSymbol(Exit, 6, 0); break;
	case PillarVertical:
		gen->setSymbol(Start, 2, gen->height - 1);  gen->setSymbol(Start, 4, gen->height - 1), gen->setSymbol(Exit, 2, 0);  gen->setSymbol(Exit, 4, 0); break;
	case PillarHorizontal:
		gen->setSymbol(Start, 0, gen->height - 1);  gen->setSymbol(Exit, 6, gen->height - 1), gen->setSymbol(Exit, 0, 0);  gen->setSymbol(Start, 6, 0); break;
	case PillarRotational:
		gen->setSymbol(Start, 0, gen->height - 1);  gen->setSymbol(Exit, 6, gen->height - 1), gen->setSymbol(Exit, 0, 0);  gen->setSymbol(Start, 6, 0); break;
	}
}

void Special::generateSymmetryGate(PanelID id)
{
	gen->resetConfig();
	gen->setConfig(DisableWrite);
	gen->setConfig(InvisibleSymmetryLine);
	gen->setSymmetry(RotateRight);
	gen->setSymbol(Start, 0, 0);
	gen->setSymbol(Start, 0, 8);
	gen->setSymbol(Start, 8, 0);
	gen->setSymbol(Start, 8, 8);
	gen->setSymbol(Exit, 4, 0);
	gen->setSymbol(Exit, 4, 8);
	gen->setSymbol(Exit, 0, 4);
	gen->setSymbol(Exit, 8, 4);
	gen->generate(id, Triangle | Yellow, 4);
	std::vector<Point> breakPos = { {0, 3}, {2, 3}, {4, 3}, {6, 3}, {8, 3}, {0, 5}, {2, 5}, {4, 5}, {6, 5}, {8, 5} };
	for (Point p : breakPos) gen->set(p, COLUMN | 0x40000);
	breakPos = { { 3, 0 },{ 3, 2 },{ 3, 4 },{ 3, 6 },{ 3, 8 },{ 5, 0 },{ 5, 2 },{ 5, 4 },{ 5, 6 },{ 5, 8 } };
	for (Point p : breakPos) gen->set(p, ROW | 0x40000);
	//Expand the grid a little to prevent the collision detection from glitching out
	gen->panel.minx = 0.08f;
	gen->panel.maxx = 0.92f;
	gen->panel.miny = 0.08f;
	gen->panel.maxy = 0.92f;
	gen->write(id);
	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<float> intersections = memory->ReadArray<float>(id, DOT_POSITIONS, numIntersections * 2);
	std::vector<int> symData;
	for (int i = 0; i < numIntersections; i++) {
		bool pushed = false;
		for (int j = 0; j < numIntersections; j++) {
			int precision = 30;
			if (std::round(intersections[i * 2] * precision) == std::round(precision - intersections[j * 2 + 1] * precision) &&
				std::round(intersections[i * 2 + 1] * precision) == std::round(intersections[j * 2] * precision)) {
				symData.push_back(j);
				pushed = true;
				break;
			}
		}
		if (!pushed)
			symData.push_back(0);
	}
	memory->WriteArray(id, REFLECTION_DATA, symData);
}

bool Special::checkDotSolvability(Panel* panel1, Panel* panel2, Symmetry correctSym) {
	std::vector<Symmetry> sym = { FlipXY, FlipNegXY, RotateLeft, RotateRight };
	for (Symmetry s : sym) {
		if (s == correctSym) continue;
		bool found = false;
		//Check for three dots at a point
		for (int x = 0; x < panel1->width; x += 2) {
			for (int y = 0; y < panel1->height; y += 2) {
				int count = 0;
				for (Point dir : Panel::DIRECTIONS) {
					Point p = { x + dir.x, y + dir.y };
					Point sp = panel1->getSymPoint(p.x, p.y, s);
					if (p.x < 0 || p.y < 0 || p.x >= panel1->width || p.y >= panel1->height) continue;
					if (panel1->getFlag(p.x, p.y, Dot) && !panel1->getFlag(p.x, p.y, DOT_IS_INVISIBLE) ||
						panel2->getFlag(sp.x, sp.y, Dot) && !panel2->getFlag(sp.x, sp.y, DOT_IS_INVISIBLE)) {
						if (++count == 3) {
							found = true;
							break;
						}
					}
				}
				if (found) break;
			}
			if (found) break;
		}
		//Check for four dots in a circle
		for (int x = 1; x < panel1->width; x += 2) {
			for (int y = 1; y < panel1->height; y += 2) {
				int count = 0;
				for (Point dir : Panel::DIRECTIONS) {
					Point p = { x + dir.x, y + dir.y };
					Point sp = panel1->getSymPoint(p.x, p.y, s);
					if (panel1->getFlag(p.x, p.y, Dot) && !panel1->getFlag(p.x, p.y, DOT_IS_INVISIBLE) ||
						panel2->getFlag(sp.x, sp.y, Dot) && !panel2->getFlag(sp.x, sp.y, DOT_IS_INVISIBLE)) {
						if (++count == 4) {
							found = true;
							break;
						}
					}
				}
				if (found) break;
			}
			if (found) break;
		}
		if (!found) return true;
	}
	return false;
}

void Special::createArrowPuzzle(PanelID id, int x, int y, SymbolId symbolId, const std::vector<Point>& gaps)
{
	gen->initPanel(id);
	gen->clear();
	gen->set(x, y, GetWitnessDecorationId(symbolId) | Purple);
	for (Point p : gaps) {
		gen->set(p, p.x % 2 ? Gap_Row : Gap_Column);
	}
	gen->write(id);
}

void Special::createArrowSecretDoor(PanelID id)
{
	//TODO: Replace this with setSymbol.
	gen->setConfigOnce(DisableFlash);
	gen->backgroundColor = { 0, 0, 0, 1 };
	gen->foregroundColor = { 0, 0, 0, 1 };
	gen->successColor = { 1, 0.6f, 0, 1 };
	gen->initPanel(id);
	gen->clear();
	gen->set(1, 1, GetWitnessDecorationId(SymbolId::Arrow3NE) | Orange);
	gen->set(1, 5, GetWitnessDecorationId(SymbolId::Arrow3E) | Orange);
	gen->set(1, 9, GetWitnessDecorationId(SymbolId::Arrow3SE) | Orange);
	gen->set(9, 1, GetWitnessDecorationId(SymbolId::Arrow3NW) | Orange);
	gen->set(9, 5, GetWitnessDecorationId(SymbolId::Arrow3W) | Orange);
	gen->set(9, 9, GetWitnessDecorationId(SymbolId::Arrow3SW) | Orange);
	gen->write(id);
}

void Special::generateCenterPerspective(PanelID id, const std::vector<std::pair<int, int>>& symbolVec, Symbol symbolType)
{
	//TODO: Use setSymbol rather than a while loop
	std::vector<std::vector<Point>> obstructions = { { { 5, 0 },{ 5, 2 },{ 5, 4 } },{ { 5, 6 },{ 5, 8 },{ 5, 10 } },{ { 0, 5 },{ 2, 5 },{ 4, 5 } },{ { 6, 5 },{ 8, 5 },{ 10, 5 } } };
	gen->setObstructions(obstructions);
	do {
		gen->setConfigOnce(DisableWrite);
		gen->generate(id, symbolVec);
	} while ((gen->get(5, 5) & 0xFF00) != symbolType);
	gen->write(id);
}

void Special::createText(PanelID id, std::string text, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB,
		float left, float right, float top, float bottom) {
	//012
	//345
	//678
	std::map<char, std::vector<int>> coords = {
		{ 'a',{ 6,0,2,8,5,3 } },{ 'c',{ 2,0,6,8 } },{ 'd',{ 0,1,5,7,6,0 } },{ 'e',{ 2,0,3,5,3,6,8 } },{ 'g',{ 2,0,6,8,5,4 } },{ 'i',{ 0,2,1,7,6,8 } },{ 'k',{ 0,6,3,2,3,8 } },
		{ 'l',{ 0,6,8 } },{ 'm',{ 6,0,4,2,8 } },{ 'n',{ 6,0,8,2 } },{ 'o',{ 0,2,8,6,0 } },{ 'p',{ 6,0,2,5,3 } },{ 'r',{ 6,0,2,5,3,8 } },{ 's',{ 2,0,3,5,8,6 } },
		{ 't',{ 0,2,1,7 } },{ 'u',{ 0,6,8,2 } },{ 'x',{ 0,8,4,2,6 } },{ '0',{ 0,2,8,6,0 } },{ '1',{ 0,1,7,6,8 } },{ '2',{ 0,2,5,3,6,8 } },{ '3',{ 0,2,5,3,5,8,6 } },
		{ '4',{ 0,3,5,2,8 } },{ '5',{ 2,0,3,5,8,6 } },{ '6',{ 2,0,6,8,5,3 } },{ '7',{ 0,2,7 } },{ '8',{ 0,2,8,6,0,3,5 } },{ '9',{ 6,8,2,0,3,5 } },
		{ '!',{ 1,4,7 } },{ ' ',{ } },{ '.',{ 7 } },{ '-',{ 3,5 } }
	};

	float spacingX = (right - left) / (text.size() * 3 - 1);
	float spacingY = (top - bottom) / 2;

	for (int i = 0; i < text.size(); i++) {
		char c = text[i];
		for (int j = 0; j < coords[c].size(); j++) {
			int n = coords[c][j];
			intersections.emplace_back((n % 3 + i * 3) * spacingX + left);
			intersections.emplace_back(1 - ((2 - n / 3) * spacingY + bottom));
			if (j > 0 && !(c == '!' && j == 2)) {
				connectionsA.emplace_back(static_cast<int>(intersections.size()) / 2 - 2);
				connectionsB.emplace_back(static_cast<int>(intersections.size()) / 2 - 1);
			}
		}
	}
}

void Special::drawText(PanelID id, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB, const std::vector<float>& finalLine) {

	std::vector<int> intersectionFlags;
	for (int i = 0; i < intersections.size() / 2; i++) {
		intersectionFlags.emplace_back(0);
	}
	intersections.emplace_back(finalLine[0]);
	intersectionFlags.emplace_back(Start);

	for (int i = 1; i < finalLine.size(); i++) {
		intersections.emplace_back(finalLine[i]);
		if (i % 2 == 0) {
			intersectionFlags.emplace_back(i == finalLine.size() - 2 ? Exit : 0);
			connectionsA.emplace_back(static_cast<int>(intersectionFlags.size()) - 2); connectionsB.emplace_back(static_cast<int>(intersectionFlags.size()) - 1);
		}
	}

	Memory* memory = Memory::get();
	memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.6f });
	memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) });
	memory->WriteArray<float>(id, DOT_POSITIONS, intersections);
	memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags);
	memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) });
	memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA);
	memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB);
	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void Special::drawSeedAndDifficulty(PanelID id, int seed, bool hard, bool setSeed, bool options)
{
	std::vector<float> intersections;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;

	createText(id, hard ? "expert" : "normal", intersections, connectionsA, connectionsB, 0.1f, 0.9f, 0.25f, 0.4f);
	std::string seedStr = std::to_string(seed);
	createText(id, seedStr, intersections, connectionsA, connectionsB, 0.5f - seedStr.size()*0.06f, 0.5f + seedStr.size()*0.06f, setSeed ? 0.6f : 0.65f, setSeed ? 0.75f : 0.8f);
	if (setSeed) createText(id, "set seed", intersections, connectionsA, connectionsB, 0.1f, 0.9f, 0.86f, 0.96f);
	std::string version = VERSION_STR;
	createText(id, version, intersections, connectionsA, connectionsB, 0.98f - version.size()*0.06f, 0.98f, 0.02f, 0.10f);
	if (options) createText(id, "option", intersections, connectionsA, connectionsB, 0.02f, 0.5f, 0.02f, 0.10f);

	drawText(id, intersections, connectionsA, connectionsB, { 0.1f, 0.5f, 0.9f, 0.5f });
}

void Special::drawGoodLuckPanel(PanelID id)
{
	std::vector<float> intersections;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;
	createText(id, "good", intersections, connectionsA, connectionsB, 0.2f, 0.8f, 0.07f, 0.23f);
	createText(id, "luck!", intersections, connectionsA, connectionsB, 0.2f, 0.8f, 0.77f, 0.93f);
	drawText(id, intersections, connectionsA, connectionsB, { 0.66f, 0.62f, 0.66f, 0.69f, 0.32f, 0.69f, 0.51f, 0.51f, 0.32f, 0.32f, 0.66f, 0.32f, 0.66f, 0.39f });
}

void Special::setTarget(PanelID puzzle, PanelID target) {
	Memory::get()->WritePanelData(puzzle, TARGET, target + 1);
}

void Special::clearTarget(PanelID puzzle) {
	Memory::get()->WritePanelData(puzzle, TARGET, 0);
}

void Special::copyTarget(PanelID puzzle, PanelID sourceTarget) {
	Memory::get()->WritePanelData(puzzle, TARGET, Memory::get()->ReadPanelData<int>(sourceTarget, TARGET));
}

void Special::setTargetAndDeactivate(PanelID puzzle, PanelID target) {
	if (!hasBeenRandomized()) //Only deactivate on a fresh save file (since power state is preserved)
		Memory::get()->WritePanelDataVector<float>(target, POWER, { 0.0, 0.0 });
	Memory::get()->WritePanelData(puzzle, TARGET, target + 1);
}

void Special::setPower(PanelID puzzle, bool power) {
	if (!power && hasBeenRandomized()) return; //Only deactivate on a fresh save file (since power state is preserved)
	if (power) Memory::get()->WritePanelDataVector<float>(puzzle, POWER, { 1.0, 1.0 });
	else Memory::get()->WritePanelDataVector<float>(puzzle, POWER, { 0.0, 0.0 });
}

bool Special::hasBeenPlayed() {
	return Memory::get()->ReadPanelData<float>(TUT_MAZE_2, POWER) > 0 || Memory::get()->ReadPanelData<int>(TUT_ENTER_1, TRACED_EDGES) > 0;
}

bool Special::hasBeenRandomized() {
	return Memory::get()->ReadPanelData<int>(TUT_ENTER_1, BACKGROUND_HIDDEN_VAR) > 0;
}

//For testing/debugging purposes only
void Special::test() {
	Random::seed(0);

	g.resetConfig();
	
}
