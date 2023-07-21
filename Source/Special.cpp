// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Special.h"

#include "../App/Version.h"
#include "Archipelago/SkipSpecialCases.h"
#include "MultiGenerate.h"
#include "Quaternion.h"
#include "Watchdog.h"

#include <thread>

void Special::generateSpecialSymMaze(std::shared_ptr<Generate> gen, int id) {
	do {
		gen->setFlagOnce(Generate::Config::DisableWrite);
		generator->setSymbol(Decoration::Start, 0, 16);
		generator->setSymbol(Decoration::Start, 22, 16);
		generator->setSymbol(Decoration::Exit, 8, 0);
		generator->setSymbol(Decoration::Exit, 14, 0);
		generator->setFlagOnce(Generate::Config::ShortPath);
		generator->generateMaze(0x0005C);
	} while (generator->_path.count(Point(12, 16)));
	std::shared_ptr<Panel> puzzle = gen->_panel;
	for (int x = 0; x < puzzle->_width / 2; x++) {
		for (int y = 0; y < puzzle->_height; y++) {
			Point sp = puzzle->get_sym_point(x, y, Panel::Symmetry::Vertical);
			if (puzzle->_grid[sp.first][sp.second] & Decoration::Gap) {
				puzzle->_grid[x][y] = puzzle->_grid[sp.first][sp.second];
				puzzle->_grid[sp.first][sp.second] = 0;
			}
		}
	}
	gen->write(id);
}

void Special::generateReflectionDotPuzzle(std::shared_ptr<Generate> gen, int id1, int id2, std::vector<std::pair<int, int>> symbols, Panel::Symmetry symmetry, bool split)
{
	Memory* memory = Memory::get();

	gen->setFlagOnce(Generate::Config::DisableWrite);
	gen->generate(id1, symbols);
	std::shared_ptr<Panel> puzzle = gen->_panel;
	std::shared_ptr<Panel> flippedPuzzle = std::make_shared<Panel>(id2);
	std::vector<Point> dots;
	for (int x = 0; x < puzzle->_width; x++) {
		for (int y = 0; y < puzzle->_height; y++) {
			Point sp = puzzle->get_sym_point(x, y, symmetry);
			if (puzzle->_grid[x][y] & Decoration::Dot) {
				int symbol = (sp.first & 1) == 1 ? Decoration::Dot_Row : (sp.second & 1) == 1 ? Decoration::Dot_Column : Decoration::Dot_Intersection;
				flippedPuzzle->_grid[sp.first][sp.second] = symbol | IntersectionFlags::DOT_IS_INVISIBLE;
				dots.push_back({ x, y });
			}
			else if (puzzle->_grid[x][y] & Decoration::Gap) {
				int symbol = (sp.first & 1) == 1 ? Decoration::Gap_Row : Decoration::Gap_Column;
				flippedPuzzle->_grid[sp.first][sp.second] = symbol;
			}
			else flippedPuzzle->_grid[sp.first][sp.second] = puzzle->_grid[x][y];
		}
	}
	if (split) {
		int size = static_cast<int>(dots.size());
		while (dots.size() > size / 2 + Random::rand() % 2) {
			Point dot = pop_random(dots);
			Point sp = puzzle->get_sym_point(dot.first, dot.second, symmetry);
			puzzle->_grid[dot.first][dot.second] |= IntersectionFlags::DOT_IS_INVISIBLE;
			flippedPuzzle->_grid[sp.first][sp.second] &= ~IntersectionFlags::DOT_IS_INVISIBLE;
		}
		if (Random::rand() % 2) {
			Point dot = pop_random(dots);
			Point sp = puzzle->get_sym_point(dot.first, dot.second, symmetry);
			flippedPuzzle->_grid[sp.first][sp.second] &= ~IntersectionFlags::DOT_IS_INVISIBLE;
		}
		Color color = memory->ReadPanelData<Color>(id2, SUCCESS_COLOR_A);
		memory->WritePanelData<Color>(id2, PATTERN_POINT_COLOR, { color });
	}
	flippedPuzzle->_startpoints.clear();
	for (Point p : puzzle->_startpoints) {
		flippedPuzzle->_startpoints.push_back(puzzle->get_sym_point(p, symmetry));
	}
	flippedPuzzle->_endpoints.clear();
	for (Endpoint p : puzzle->_endpoints) {
		Point sp = puzzle->get_sym_point(p.GetX(), p.GetY(), symmetry);
		flippedPuzzle->_endpoints.push_back(Endpoint(sp.first, sp.second, gen->_panel->get_sym_dir(p.GetDir(), symmetry),
			IntersectionFlags::ENDPOINT | (p.GetDir() == Endpoint::Direction::UP || p.GetDir() == Endpoint::Direction::DOWN ? IntersectionFlags::COLUMN : IntersectionFlags::ROW)));
	}
	if (id1 == 0x00A5B && split) {
		if (checkDotSolvability(puzzle, flippedPuzzle, symmetry)) {
			generateReflectionDotPuzzle(gen, id1, id2, symbols, symmetry, split);
			return;
		}
	}
	gen->write(id1);
	flippedPuzzle->Write(id2);
}

void Special::generateAntiPuzzle(int id)
{
	while (true) {
		generator->setFlagOnce(Generate::Config::DisableWrite);
		generator->generate(id, Decoration::Poly | Decoration::Can_Rotate, 2);
		std::set<Point> open = generator->_gridpos;
		std::vector<int> symbols;
		for (int x = 1; x < generator->_panel->_width; x += 2) {
			for (int y = 1; y < generator->_panel->_height; y += 2) {
				if (generator->get_symbol_type(generator->get(x, y)) == Decoration::Poly) {
					symbols.push_back(generator->get(x, y));
					generator->set(x, y, 0);
					for (Point p : generator->get_region(Point(x, y))) {
						open.erase(p);
					}
				}
			}
		}
		if (open.size() == 0) continue;
		std::set<Point> region = generator->get_region(*open.begin());
		if (region.size() != open.size() || open.size() < symbols.size() + 1) continue;
		for (int s : symbols) {
			Point p = generator->pick_random(open);
			generator->set(p, s | Decoration::Negative);
			open.erase(p);
		}
		generator->set(generator->pick_random(open), Decoration::Poly | 0xffff0000); //Full block
		generator->write(id);
		generator->resetConfig();
		return;
	}
}

void Special::generateColorFilterPuzzle(int id, Point size, const std::vector<std::pair<int, int>>& symbols, const Color& filter, bool colorblind)
{
	generator->setFlagOnce(Generate::Config::DisableWrite);
	generator->setGridSize(size.first, size.second);
	generator->generate(id, symbols);
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
	for (int y = generator->_panel->_height - 2; y>0; y -= 2) {
		for (int x = 1; x < generator->_panel->_width - 1; x += 2) {
			if (generator->get(x, y) == 0) symbolColors.push_back({ 0, 0, 0, 0 });
			else symbolColors.push_back(availableColors[(generator->get(x, y) & 0xf) - 1]);
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
	generator->setFlagOnce(Generate::Config::WriteColors);
	generator->write(id);
	Memory::get()->WriteArray(id, DECORATION_COLORS, symbolColors);
	generator->resetConfig();
}

void Special::generateSoundDotPuzzle(int id1, int id2, std::vector<int> dotSequence, bool writeSequence) {
	generator->setFlag(Generate::Config::DisableReset);
	generateSoundDotPuzzle(id1, { 5, 5 }, dotSequence, writeSequence);
	generator->write(id2);
	Memory::get()->WritePanelData(id2, PATTERN_POINT_COLOR, Memory::get()->ReadPanelData<Color>(id2, SUCCESS_COLOR_A));
	generator->resetConfig();
}

void Special::generateSoundDotPuzzle(int id, Point size, std::vector<int> dotSequence, bool writeSequence) {
	generator->resetConfig();
	generator->setFlagOnce(Generate::Config::DisableWrite);
	generator->setFlagOnce(Generate::Config::LongPath);
	generator->setGridSize(size.first, size.second);
	if (id == 0x014B2) { //Have to force there to only be one correct sequence
		generator->generate(id, Decoration::Dot_Intersection, static_cast<int>(dotSequence.size() - 1));
		while (generator->get(6, 0) == Decoration::Dot_Intersection || generator->get(8, 2) == Decoration::Dot_Intersection)
			generator->generate(id, Decoration::Dot_Intersection, static_cast<int>(dotSequence.size() - 1));
		if (generator->get(7, 0) == PATH) generator->set(7, 0, Decoration::Dot_Row);
		else generator->set(8, 1, Decoration::Dot_Column);
	}
	else generator->generate(id, Decoration::Dot_Intersection, static_cast<int>(dotSequence.size()));
	Point p = *generator->_starts.begin();
	std::set<Point> path = generator->_path;
	int seqPos = 0;
	while (!generator->_exits.count(p)) {
		path.erase(p);
		int sym = generator->get(p);
		if (sym & Decoration::Dot) {
			generator->set(p, sym | dotSequence[seqPos++]);
		}
		for (Point dir : Generate::_DIRECTIONS1) {
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

		Memory* memory = Memory::get();
		memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN, { static_cast<int>(dotSequence.size()) });
		memory->WriteArray(id, DOT_SEQUENCE, dotSequence, true);
	}
	generator->write(id);
}

void Special::generateSoundDotReflectionPuzzle(int id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored, bool writeSequence)
{
	Memory* memory = Memory::get();

	generator->resetConfig();
	generator->setFlagOnce(Generate::Config::DisableWrite);
	generator->setFlagOnce(Generate::Config::LongPath);
	generator->setSymmetry(Panel::Symmetry::Rotational);
	generator->setGridSize(size.first, size.second);
	if (id != 0x00AFB) {
		memory->WritePanelData(id, SUCCESS_COLOR_B, memory->ReadPanelData<Color>(id, SUCCESS_COLOR_A));
		generator->setSymbol(Decoration::Start, 0, generator->_height - 1); generator->setSymbol(Decoration::Start, generator->_width - 1, 0);
		generator->setSymbol(Decoration::Exit, 0, 0); generator->setSymbol(Decoration::Exit, generator->_width - 1, generator->_height - 1);
	}
	if (id == 0x00C41 || id == 0x014B2) { //Generate with one less dot than the pattern (for set start/exit)
		generator->generate(id, Decoration::Dot_Intersection | Decoration::Color::Blue, static_cast<int>(dotSequence1.size() - 1), Decoration::Dot_Intersection | Decoration::Color::Yellow, static_cast<int>(dotSequence2.size() - 1));
	}
	else if (id == 0x00AFB && writeSequence) { //Shipwreck Expert
		while (!generateSoundDotReflectionSpecial(id, size, dotSequence1, dotSequence2, numColored));
		return;
	}
	else generator->generate(id, Decoration::Dot_Intersection | Decoration::Color::Blue, static_cast<int>(dotSequence1.size()), Decoration::Dot_Intersection | Decoration::Color::Yellow, static_cast<int>(dotSequence2.size()));
	std::set<Point> path1 = generator->_path1, path2 = generator->_path2;
	Point p1, p2;
	std::set<Point> dots1, dots2;
	for (Point p : generator->_starts) {
		if (generator->_path1.count(p)) p1 = p;
		if (generator->_path2.count(p)) p2 = p;
	}
	if (id == 0x00C41) {
		generator->set(p1, Decoration::Dot_Intersection);
		generator->set(p2, Decoration::Dot_Intersection);
	}
	int seqPos = 0;
	while (!generator->_exits.count(p1)) {
		path1.erase(p1);
		int sym = generator->get(p1);
		if (sym & Decoration::Dot) {
			generator->set(p1, sym | dotSequence1[seqPos++]);
			dots1.insert(p1);
		}
		for (Point dir : Generate::_DIRECTIONS1) {
			Point newp = p1 + dir;
			if (path1.count(newp)) {
				p1 = newp;
				break;
			}
		}
	}
	seqPos = 0;
	while (!generator->_exits.count(p2)) {
		path2.erase(p2);
		int sym = generator->get(p2);
		if (sym & Decoration::Dot) {
			generator->set(p2, sym | dotSequence2[seqPos++]);
			dots2.insert(p2);
		}
		for (Point dir : Generate::_DIRECTIONS1) {
			Point newp = p2 + dir;
			if (path2.count(newp)) {
				p2 = newp;
				break;
			}
		}
	}
	if (id == 0x014B2) {
		generator->set(p1, dotSequence1[seqPos] | Decoration::Dot_Intersection);
		generator->set(p2, dotSequence2[seqPos] | Decoration::Dot_Intersection);
		numColored += 2;
	}
	for (int i = static_cast<int>(dotSequence1.size() + dotSequence2.size()); i > numColored; i--) {
		if (i % 2 == 0) { //Want to evenly distribute colors between blue/orange (approximately)
			Point p = pop_random(dots1);
			generator->set(p, generator->get(p) & ~DOT_IS_BLUE); //Remove color
		}
		else {
			Point p = pop_random(dots2);
			generator->set(p, generator->get(p) & ~DOT_IS_ORANGE); //Remove color
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
	generator->write(id);
	memory->WritePanelData(id, POWER_OFF_ON_FAIL, 0);
	generator->setSymmetry(Panel::Symmetry::None);
	if (dotSequence1 != dotSequence2 && id != 0x00AFB) (new JungleWatchdog(id, dotSequence1, dotSequence2))->start();
}

bool Special::generateSoundDotReflectionSpecial(int id, Point size, std::vector<int> dotSequence1, std::vector<int> dotSequence2, int numColored) {
	Memory* memory = Memory::get(); 
	
	generator->resetConfig();
	generator->resetVars();
	generator->setFlagOnce(Generate::Config::DisableWrite);
	generator->setFlagOnce(Generate::Config::LongPath);
	generator->setSymmetry(Panel::Symmetry::RotateLeft);
	generator->setGridSize(size.first, size.second);
	std::vector<Point> starts = { { 0, 0 },{ generator->_width - 1, 0 },{ 0, generator->_height - 1 },{ generator->_width - 1, generator->_height - 1 } };
	Point start = pick_random<Point>(starts);
	generator->setSymbol(Decoration::Start, start.first, start.second);
	generator->setSymbol(Decoration::Exit, 6, 0); generator->setSymbol(Decoration::Exit, generator->_width - 1, 6);
	generator->setSymbol(Decoration::Exit, 0, generator->_height - 7); generator->setSymbol(Decoration::Exit, generator->_width - 7, generator->_height - 1);
	generator->generate(id, Decoration::Dot_Intersection | Decoration::Color::Blue, static_cast<int>(dotSequence1.size() - 1), Decoration::Dot_Intersection | Decoration::Color::Yellow, static_cast<int>(dotSequence2.size() - 1));
	std::set<Point> path1 = generator->_path1, path2 = generator->_path2;
	std::set<Point> intersect;
	for (Point p : path1) {
		if (p.first % 2 != 0 || p.second % 2 != 0)
			continue;
		if (path2.count(p)) {
			if (generator->get(p) & Decoration::Dot)
				return false;
			intersect.insert(p);
		}
	}
	Point pshared = pick_random(intersect);
	generator->set(pshared, Decoration::Dot_Intersection);
	Point p1 = start, p2 = generator->get_sym_point(start);
	std::set<Point> dots1, dots2;

	int seqPos = 0;
	while (seqPos < dotSequence1.size()) {
		path1.erase(p1);
		int sym = generator->get(p1);
		if (sym & Decoration::Dot) {
			generator->set(p1, sym | dotSequence1[seqPos++]);
			dots1.insert(p1);
		}
		for (Point dir : Generate::_DIRECTIONS1) {
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
		int sym = generator->get(p2);
		if (sym & Decoration::Dot) {
			if ((sym & 0xf000) && ((sym & 0xf000) == IntersectionFlags::DOT_MEDIUM))
				return false;
			if ((sym & 0xf000) && (sym & 0xf000) != dotSequence2[seqPos])
				return false;
			generator->set(p2, sym | dotSequence2[seqPos++]);
			if (!generator->_starts.count(p2)) dots2.insert(p2);
		}
		for (Point dir : Generate::_DIRECTIONS1) {
			Point newp = p2 + dir;
			if (path2.count(newp)) {
				p2 = newp;
				break;
			}
		}
	}

	dots1.erase(pshared);
	dots2.erase(pshared);
	generator->set(pshared, generator->get(pshared) & (~DOT_IS_BLUE | DOT_IS_ORANGE));
	for (int i = static_cast<int>(dotSequence1.size() + dotSequence2.size()) - 2; i > numColored; i--) {
		if (i % 2 == 0) { //Want to evenly distribute colors between blue/orange (approximately)
			Point p = pop_random(dots1);
			generator->set(p, generator->get(p) & ~DOT_IS_BLUE); //Remove color
		}
		else {
			Point p = pop_random(dots2);
			generator->set(p, generator->get(p) & ~DOT_IS_ORANGE); //Remove color
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
	generator->_panel->_startpoints = { { 0, 0}, { generator->_width - 1, 0 }, { 0, generator->_height - 1 }, { generator->_width - 1, generator->_height - 1 } };
	generator->write(id);
	generator->setSymmetry(Panel::Symmetry::None);
	return true;
}

void Special::generateRGBStonePuzzleN(int id)
{
	while (true) {
		generator->setFlagOnce(Generate::Config::DisableWrite);
		generator->generate(id);
		int amount = 16;
		std::set<Decoration::Color> used;
		std::vector<Decoration::Color> colors = { Decoration::Black, Decoration::White, Decoration::Red, Decoration::Green, Decoration::Blue, Decoration::Cyan, Decoration::Yellow, Decoration::Magenta };
		while (amount > 0) {
			Decoration::Color c = generator->pick_random(colors);
			if (generator->place_stones(c, 1)) {
				amount--;
				used.insert(c);
			}
		}
		if (used.size() < 5) continue;
		generator->setFlagOnce(Generate::Config::WriteColors);
		generator->write(id);
		return;
	}
}

void Special::generateRGBStarPuzzleN(int id)
{
	while (true) {
		generator->setFlagOnce(Generate::Config::DisableWrite);
		generator->generate(id);
		int amount = 16;
		std::set<Decoration::Color> used;
		std::set<Decoration::Color> colors = { Decoration::Black, Decoration::White, Decoration::Red, Decoration::Green, Decoration::Blue, Decoration::Cyan, Decoration::Yellow, Decoration::Magenta };
		while (amount > 0) {
			Decoration::Color c = generator->pick_random(colors);
			if (generator->place_stars(c, 2)) {
				amount -= 2;
				used.insert(c);
				if (used.size() == 4) colors = used;
			}
		}
		if (used.size() < 4)
			continue;
		generator->setFlagOnce(Generate::Config::WriteColors);
		generator->write(id);
		return;
	}
}

void Special::generateRGBStonePuzzleH(int id) {
	while (true) {
		generator->setFlagOnce(Generate::Config::DisableWrite);
		generator->generate(id);
		int amount = 10;
		std::set<Decoration::Color> used;
		std::vector<Decoration::Color> colors = { Decoration::Black, Decoration::Red, Decoration::Green, Decoration::Blue, Decoration::Magenta, Decoration::Yellow };
		while (amount > 0) {
			Decoration::Color c = generator->pick_random(colors);
			if (generator->place_stones(c, 1)) {
				amount--;
				used.insert(c);
			}
		}
		if (used.size() < 5) continue;
		if (!generator->place_erasers({ Decoration::Color::White }, { Decoration::Stone | pick_random(colors) })) {
			continue;
		}
		Point eraserPos;
		for (int x = 1; x < generator->_panel->_width; x += 2) {
			for (int y = 1; y < generator->_panel->_height; y += 2) {
				if (generator->get_symbol_type(generator->get(x, y)) == Decoration::Eraser) {
					eraserPos = { x, y };
					break;
				}
			}
		}
		int count = 0;
		for (Point dir : generator->_DIRECTIONS2) {
			if (!generator->off_edge(eraserPos + dir) && generator->get(eraserPos + dir) == 0) count++;
		}
		if (count < 2) continue;
		generator->setFlagOnce(Generate::Config::WriteColors);
		generator->write(id);
		return;
	}
}

void Special::generateRGBDotPuzzleH(int id) {
	Memory* memory = Memory::get();

	memory->WritePanelData<float>(id, PATTERN_POINT_COLOR, {1, 0, 0, 1});
	memory->WritePanelData<float>(id, PATTERN_POINT_COLOR_A, { 0, 1, 1, 1 });
	memory->WritePanelData<float>(id, PATTERN_POINT_COLOR_B, { 1, 1, 0, 1 });
	memory->WritePanelData<float>(id, ACTIVE_COLOR, { 0, 1, 1, 1 });
	memory->WritePanelData<float>(id, REFLECTION_PATH_COLOR, { 1, 1, 0, 1 });
	generator->setGridSize(7, 7);
	generator->setSymmetry(Panel::Symmetry::Rotational);
	generator->setSymbol(Decoration::Exit, 0, 14); generator->setSymbol(Decoration::Exit, 14, 0);
	generator->setSymbol(Decoration::Exit, 0, 0); generator->setSymbol(Decoration::Exit, 14, 14);
	generator->generate(id, Decoration::Dot_Intersection | Decoration::Color::Cyan, 2, Decoration::Dot_Intersection | Decoration::Color::Yellow, 4, Decoration::Dot_Intersection, 6,
		Decoration::Triangle | Decoration::Color::Orange, 4, Decoration::Start, 4);
	generator->resetConfig();
}

void Special::generateJungleVault(int id)
{
	Memory* memory = Memory::get();

	//This panel won't render symbols off the grid, so all I can do is move the dots around

	//a: 4 9 16 23, b: 12, c: 1, ab: 3 5 6 10 11 15 17 18 20 21 22, ac: 14, bc: 2 19, all: 7 8 13
	std::vector<std::vector<int>> sols = {
		{ 0, 5, 10, 15, 20, 21, 16, 11, 6, 7, 8, 3, 4, 9, 14, 13, 18, 17, 22, 23, 24, 25 },
		{ 0, 5, 10, 15, 20, 21, 22, 17, 12, 11, 6, 7, 2, 3, 8, 13, 18, 19, 24, 25 },
		{ 0, 1, 2, 7, 8, 13, 14, 19, 24, 25 } };
	std::vector<std::vector<int>> dotPoints1 = { { 4, 9, 16, 23 }, { 2, 19 }, { 2, 19 } };
	std::vector<std::vector<int>> dotPoints2 = { { 7, 8, 13 }, { 3, 5, 6, 10, 11, 15, 17, 18, 20, 21, 22 }, { 14, 1 } };
	generator->initPanel(id);
	generator->clear();
	int sol = Random::rand() % sols.size();
	auto[x1, y1] = generator->_panel->loc_to_xy(generator->pick_random(dotPoints1[sol]));
	auto[x2, y2] = generator->_panel->loc_to_xy(generator->pick_random(dotPoints2[sol]));
	generator->set(x1, y1, Decoration::Dot_Intersection);
	generator->set(x2, y2, Decoration::Dot_Intersection);
	memory->WritePanelData(id, SEQUENCE_LEN, static_cast<int>(sols[sol].size()));
	memory->WriteArray(id, SEQUENCE, sols[sol], true);
	generator->write(id);
}

void Special::generateApplePuzzle(int id, bool changeExit, bool flip)
{
	Memory* memory = Memory::get();

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
		int newExit = pop_random(exits);
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

void Special::generateKeepLaserPuzzle(int id, const std::set<Point>& path1, const std::set<Point>& path2, const std::set<Point>& path3, const std::set<Point>& path4, std::vector<std::pair<int, int>> symbols)
{
	PuzzleSymbols psymbols(symbols);
	PuzzleSymbols psymbolsBackup = psymbols;
	generator->resetConfig();
	generator->pathWidth = 0.4f;
	generator->setGridSize(10, 11);
	generator->_starts = { { 0, 2 },{ 20, 10 },{ 8, 14 },{ 0, 22 },{ 20, 22 } };
	generator->_exits = { { 0, 18 },{ 0, 0 },{ 20, 0 },{ 0, 12 } };
	if (psymbols.getNum(Decoration::Triangle) > 0) generator->_exits.insert({ 0, 22 });
	generator->initPanel(id);
	generator->clear();
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
	for (Point p : pathPoints) generator->set_path(p);
	for (Point p : gaps) generator->set(p, p.first % 2 == 0 ? Decoration::Gap_Column : Decoration::Gap_Row);
	for (Point p : path1) generator->set_path(Point(p.first + 12, p.second + 14));
	for (Point p : path2) generator->set_path(Point(p.first + 12, p.second + 2));
	for (Point p : path3) generator->set_path(Point(8 - p.first, 8 - p.second + 2));
	if (psymbols.getNum(Decoration::Triangle) == 0) { //Normal mode
		for (Point p : path4) generator->set_path(Point(p.first, p.second + 14));
		if (path4.count(Point({ 8, 4 }))) for (Point p : pathPoints2) generator->set_path(p);
	}
	else { //Hard mode
		for (Point p : path4) generator->set_path(Point(8 - p.first, 8 - p.second + 14));
		for (Point p : pathPoints3) generator->set_path(p);
	}

	std::vector<std::string> solution; //For debugging only
	for (int y = 0; y < generator->_panel->_height; y++) {
		std::string row;
		for (int x = 0; x < generator->_panel->_width; x++) {
			if (generator->get(x, y) == PATH) {
				row += "xx";
			}
			else row += "    ";
		}
		solution.push_back(row);
	}

	while (!generator->place_all_symbols(psymbols)) {
		for (int x = 0; x < generator->_panel->_width; x++)
			for (int y = 0; y < generator->_panel->_height; y++)
				if (generator->get(x, y) != PATH && (generator->get(x, y) & 0x1fffff) != Decoration::Gap)
					generator->set(x, y, 0);
		generator->_openpos = generator->_gridpos;
		for (int i = 0; i < psymbols.symbols[Decoration::Poly].size(); i++) {
			psymbols.symbols[Decoration::Poly][i].second = psymbolsBackup.symbols[Decoration::Poly][i].second + Random::rand() % 3 - Random::rand() % 3;
			if (psymbols.symbols[Decoration::Poly][i].second < 1) psymbols.symbols[Decoration::Poly][i].second = 1;
		}
	}

	for (int x = 0; x < generator->_panel->_width; x++)
		for (int y = 0; y < generator->_panel->_height; y++)
			if ((generator->get(x, y) & 0x1fffff) == Decoration::Gap)
				generator->set(x, y, IntersectionFlags::OPEN);
	generator->set(10, 10, IntersectionFlags::GAP);
	generator->set(12, 12, IntersectionFlags::NO_POINT);
	generator->set(10, 12, IntersectionFlags::NO_POINT);
	generator->set(10, 14, IntersectionFlags::NO_POINT);
	generator->set(10, 16, IntersectionFlags::NO_POINT);
	generator->set(10, 18, IntersectionFlags::ENDPOINT);
	generator->set(10, 20, IntersectionFlags::NO_POINT);
	generator->set(10, 22, IntersectionFlags::NO_POINT);
	generator->setFlagOnce(Generate::Config::EnableFlash);
	generator->write(id);
	if (psymbols.getNum(Decoration::Triangle) > 0) (new KeepWatchdog())->start();
}

void Special::generateMountaintop(int id, const std::vector<std::pair<int, int>>& symbolVec)
{
	std::vector<std::vector<Point>> perspectiveU = {
	{ { 0, 3 },{ 1, 2 },{ 1, 4 },{ 2, 1 },{ 2, 3 },{ 3, 2 },{4, 7}, { 6, 1 },{6, 7}, { 7, 0 },{ 7, 2 },{ 7, 4 },{7, 6}, { 8, 1 },{ 8, 3 },{ 8, 5 },{ 9, 2 },{ 9, 4 },{ 9, 6 } },
	{ { 0, 3 },{ 0, 5 },{ 1, 2 },{ 1, 4 },{ 2, 1 },{ 2, 3 },{ 3, 2 },{ 3, 4 },{ 4, 3 },{ 4, 5 },{ 5, 4 },{ 6, 3 },{ 6, 5 },{ 7, 4 },{ 8, 5 },{8, 9},{9, 2},{ 9, 6 } }, 
	{ { 0, 3 },{ 0, 5 },{ 0, 7 },{ 1, 2 },{ 1, 4 },{ 1, 6 },{ 1, 8 },{ 2, 3 },{ 2, 5 },{ 2, 7 },{ 3, 4 },{ 3, 6 },{ 4, 5 },{ 4, 7 },{ 5, 6 },{ 6, 5 },{ 6, 7 },{ 7, 4 },{ 7, 6 },{ 8, 7 },{8, 9},{10, 5} }, 
	{ { 0, 7 },{ 1, 6 },{ 1, 8 },{ 2, 5 },{ 3, 4 },{4, 7}, {6, 7}, { 7, 4 },{ 7, 6 },{ 7, 8 },{ 8, 5 },{ 8, 7 },{ 8, 9 },{ 9, 6 },{ 9, 8 },{ 10, 7 } }, 
	{ {1, 2},{2, 1}, { 2, 5 },{ 4, 3 },{ 4, 5 },{ 6, 3 },{ 6, 5 },{6, 9}, { 7, 4 },{7, 8}, { 8, 3 },{ 8, 5 },{ 9, 2 },{ 9, 4 },{ 9, 6 },{ 10, 3 },{ 10, 5 },{ 10, 7 } },
	{ {0, 7}, { 1, 4 },{2, 1}, { 2, 3 },{ 2, 5 },{ 3, 4 },{3, 6}, { 4, 3 },{ 5, 2 },{ 6, 1 },{ 6, 3 },{ 7, 0 },{ 7, 2 },{ 7, 4 },{ 8, 1 },{ 8, 3 },{ 8, 5 },{ 9, 4 },{ 9, 6 },{ 10, 3 },{ 10, 5 },{ 10, 7 } },
	};
	if (generator->get_symbol_type(symbolVec[0].first) == Decoration::Triangle) { //Hard mode
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
		g->setSymbol(Decoration::Gap, 5, 0);
		g->setSymbol(Decoration::Gap, 5, 10);
		g->setFlag(Generate::Config::PreserveStructure);
		g->setFlag(Generate::ShortPath);
		g->setFlag(Generate::DecorationsOnly);
	}
	gens[0]->_starts = { { 2, 0 } }; gens[1]->_starts = { { 2, 10 } }; gens[2]->_starts = { { 10, 4 },{ 10, 6 } };
	gens[0]->_exits = { { 8, 10 } }; gens[1]->_exits = { { 8, 0 } }; gens[2]->_exits = { { 0, 4 },{ 0, 6 } };
	gens[0]->setObstructions(perspectiveU); gens[1]->setObstructions(perspectiveL); gens[2]->setObstructions(perspectiveC);
	generator->generateMulti(id, gens, symbolVec);
}

void Special::generateMultiPuzzle(std::vector<int> ids, const std::vector<std::vector<std::pair<int, int>>>& symbolVec, bool flip) {
	Memory* memory = Memory::get();

	generator->resetConfig();
	generator->setFlagOnce(Generate::Config::DisableWrite);
	generator->generate(ids[0]);
	std::vector<PuzzleSymbols> symbols;
	for (auto sym : symbolVec) symbols.emplace_back(PuzzleSymbols(sym));
	std::vector<Generate> gens;
	for (int i = 0; i < ids.size(); i++) gens.emplace_back(Generate());
	for (int i = 0; i < ids.size(); i++) {
		gens[i].setFlag(Generate::Config::DisableWrite);
		gens[i].setFlag(Generate::WriteColors);
		if (symbols[i].getNum(Decoration::Poly)  - symbols[i].getNum(Decoration::Eraser) > 1) gens[i].setFlag(Generate::RequireCombineShapes);
	}
	while (!generateMultiPuzzle(ids, gens, symbols, generator->_path)) {
		generator->generate(ids[0]);
	}
	for (int i = 0; i < ids.size(); i++) {
		gens[i].write(ids[i]);
		generator->incrementProgress();
		if (symbolVec[0][0].first == (Decoration::Triangle | Decoration::Color::Orange)) { //Hard mode
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
	generator->resetConfig();
	generator->resetVars();
}

bool Special::generateMultiPuzzle(std::vector<int> ids, std::vector<Generate>& gens, const std::vector<PuzzleSymbols>& symbols, const std::set<Point>& path) {
	for (int i = 0; i < ids.size(); i++) {
		gens[i]._custom_grid.clear();
		gens[i].setPath(path);
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

void Special::generate2Bridge(int id1, int id2)
{
	std::vector<std::shared_ptr<Generate>> gens;
	for (int i = 0; i < 3; i++) gens.push_back(std::make_shared<Generate>());
	for (std::shared_ptr<Generate> g : gens) {
		g->setFlag(Generate::Config::DisableWrite);
		g->setFlag(Generate::Config::DisableReset);
		g->setFlag(Generate::Config::DecorationsOnly);
		g->setFlag(Generate::Config::ShortPath);
		g->setFlag(Generate::Config::WriteColors);
	}
	while (!generate2Bridge(id1, id2, gens));
	gens[1]->write(id1);
	gens[1]->write(id2);
	generator->incrementProgress();
}

bool Special::generate2Bridge(int id1, int id2, std::vector<std::shared_ptr<Generate>> gens)
{
	for (int i = 0; i < gens.size(); i++) {
		gens[i]->_custom_grid.clear();
		gens[i]->setPath(std::set<Point>());
		std::vector<Point> walls = { { 12, 1 },{ 12, 3 },{ 3, 8 },{ 9, 8 } };
		for (Point p : walls) gens[i]->setSymbol(Decoration::Gap, p.first, p.second);
		if (i % 2 == 0) {
			gens[i]->setObstructions({ { 5, 8 },{ 6, 7 },{ 7, 8 } });
		}
		else {
			gens[i]->setObstructions({ { 5, 0 },{ 6, 1 },{ 7, 0 } });
		}
	}

	int steps = 2;

	gens[0]->_exits = { { 12, 8 } };
	gens[1]->_exits = { { 0, 0 } };

	PuzzleSymbols symbols({ {Decoration::Poly | Decoration::Can_Rotate | Decoration::Color::Yellow, 1}, {Decoration::Star | Decoration::Color::Yellow, 1} });
	int fails = 0;
	while (!gens[0]->generate(id1, symbols)) {
		if (fails++ > 20)
			return false;
	}

	gens[1]->setPath(gens[0]->_path);
	gens[1]->customPath.clear();
	gens[1]->_custom_grid = gens[0]->_panel->_grid;
	symbols = PuzzleSymbols({ { Decoration::Poly | Decoration::Can_Rotate | Decoration::Color::Yellow, 1 },{ Decoration::Star | Decoration::Color::Yellow, 3 } });
	fails = 0;
	while (!gens[1]->generate(id2, symbols)) {
		if (fails++ > 20) {
			return false;
		}
	}

	if (gens[0]->get(11, 1) != 0 || gens[1]->get(11, 1) != 0 || gens[1]->get(11, 3) != 0 || gens[1]->get(11, 3) != 0)
		return false;
	
	//Make sure both shapes weren't blocked off by the same path
	int shapeCount = 0;
	for (int x = 1; x < gens[1]->_panel->_width; x += 2) {
		for (int y = 1; y < gens[1]->_panel->_height; y += 2) {
			if (gens[1]->get_symbol_type(gens[1]->get(x, y)) == Decoration::Poly && gens[1]->get_region(Point(x, y)).size() == gens[0]->get_region(Point(x, y)).size())
				if (++shapeCount == 2) return false;
		}
	}

	for (int x = 1; x < gens[1]->_panel->_width; x += 2) {
		for (int y = 1; y < gens[1]->_panel->_height; y += 2) {
			if (gens[1]->get_symbol_type(gens[1]->get(x, y)) == Decoration::Star) {
				if (!gens[1]->get_region(Point(x, y)).count({ 9, 7 }))
					continue;
				gens[1]->set(x, y, Decoration::Eraser | Decoration::Color::White);
				return true;
			}
		}
	}

	return false;
}

void Special::generate2BridgeH(int id1, int id2)
{
	Memory* memory = Memory::get();

	std::vector<std::shared_ptr<Generate>> gens;
	for (int i = 0; i < 3; i++) gens.push_back(std::make_shared<Generate>());
	for (std::shared_ptr<Generate> g : gens) {
		g->setFlag(Generate::Config::DisableWrite);
		g->setFlag(Generate::Config::DisableReset);
		g->setFlag(Generate::Config::DecorationsOnly);
		g->setFlag(Generate::Config::ShortPath);
		g->setFlag(Generate::Config::WriteColors);
	}
	while (!generate2BridgeH(id1, id2, gens));
	
	
	gens[0]->write(id1);
	gens[0]->write(id2);
	generator->incrementProgress();

	int numIntersections = memory->ReadPanelData<int>(id1, NUM_DOTS);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id1, DOT_FLAGS, numIntersections);
	std::vector<int> intersectionFlags2 = memory->ReadArray<int>(id2, DOT_FLAGS, numIntersections);
	for (int x = 0; x < gens[0]->_panel->_width; x += 2) {
		for (int y = 0; y < gens[0]->_panel->_height; y += 2) {
			if (gens[0]->get(x, y) == Decoration::Dot_Intersection) {
				intersectionFlags[gens[0]->_panel->xy_to_loc(x, y)] = Decoration::Dot_Intersection;
				intersectionFlags2[gens[0]->_panel->xy_to_loc(x, y)] = Decoration::Dot_Intersection;
			}
			else {
				intersectionFlags[gens[0]->_panel->xy_to_loc(x, y)] &= ~Decoration::Dot;
				intersectionFlags2[gens[0]->_panel->xy_to_loc(x, y)] &= ~Decoration::Dot;
			}
		}
	}
	memory->WriteArray(id1, DOT_FLAGS, intersectionFlags);
	memory->WriteArray(id2, DOT_FLAGS, intersectionFlags2);
	memory->WritePanelData(id1, NEEDS_REDRAW, 1);
	memory->WritePanelData(id2, NEEDS_REDRAW, 1);
}

bool Special::generate2BridgeH(int id1, int id2, std::vector<std::shared_ptr<Generate>> gens)
{
	for (int i = 0; i < gens.size(); i++) {
		gens[i]->_custom_grid.clear();
		gens[i]->setPath(std::set<Point>());
		std::vector<Point> walls = { { 12, 1 },{ 12, 3 },{ 3, 8 },{ 9, 8 } };
		for (Point p : walls) gens[i]->setSymbol(Decoration::Gap, p.first, p.second);
		if (i == 0) {
			gens[i]->setObstructions({ { 3, 0 },{ 3, 2 },{ 3, 4 },{4, 5}, {5, 6}, {6, 7}, {8, 7}, {9, 0}, {9, 2}, {9, 4}, {10, 5}, {12, 5} });
		}
		else {
			gens[i]->setObstructions({ {1, 4}, { 1, 6 },{ 1, 8 }, {4, 1}, {6, 7}, {7, 8} });
		}
	}

	gens[0]->_exits = { { 12, 8 } };
	gens[1]->_exits = { { 0, 0 } };

	gens[0]->generate(id1);
	gens[1]->setPath(gens[0]->_path);
	gens[1]->customPath.clear();
	gens[1]->_custom_grid = gens[0]->_panel->_grid;
	gens[1]->generate(id2);
	std::vector<Point> points = { {12, 2}, { 11, 2 }, { 10, 2 }, { 10, 3 }, { 10, 4 }, { 9, 4 }, { 8, 4 }, { 7, 4 }, { 6, 4 } };
	for (int i = 0; i < points.size(); i++) {
		if (gens[1]->get(points[i]) == PATH) break;
		gens[1]->set_path(points[i]);
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
				gens[1]->set_path({ i, 2 });
			}
		}
		else if (state == 2) {
			if (gens[1]->get(i, 2) == PATH && gens[0]->get(i, 2) != PATH) break;
			if (gens[1]->get(i, 2) == PATH && gens[0]->get(i, 2) == PATH) return false;
			gens[1]->set_path({ i, 2 });
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
				gens[1]->set_path({ i, 6 });
				gens[0]->set(i + 1, 6, Decoration::Dot_Intersection);
			}
		}
		else if (state == 2) {
			if (gens[1]->get(i, 6) == PATH && gens[0]->get(i, 6) != PATH) break;
			if (gens[1]->get(i, 6) == PATH && gens[0]->get(i, 6) == PATH) return false;
			gens[1]->set_path({ i, 6 });
		}
	}

	int count = 0;
	std::set<Point> open = gens[0]->_gridpos;
	while (open.size() > 0) {
		Point pos = *(open.begin());
		std::set<Point> region = gens[1]->get_region(pos);
		if (region.size() == 1 || region.size() > 6) return false;
		int symbol = gens[0]->make_shape_symbol(region, false, false);
		if (!symbol) return false;
		gens[0]->set(pick_random(region), symbol | Decoration::Color::Yellow);
		for (Point p : region) open.erase(p);
		count++;
	}
	return count == 6;
}

bool checkShape(const std::set<Point>& shape, int direction) {
	//Make sure it is not off the grid
	for (Point p : shape) if (p.first < 0 || p.first > 7 || p.second < 0 || p.second > 7)
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
	std::vector<int> ids = { 0x09EFF, 0x09F01, 0x09FC1, 0x09F8E };
	int idfloor = 0x09FDA;
	//{ 0x09EFF, 0x09F01, 0x09FC1, 0x09F8E }, 0x09FDA
	generator->resetConfig();
	std::vector<Point> floorPos = { { 3, 3 },{ 7, 3 },{ 3, 7 },{ 7, 7 } };
	generator->openPos = std::set<Point>(floorPos.begin(), floorPos.end());
	generator->setFlag(Generate::Config::DisableWrite);
	//Make sure no duplicated symbols
	std::set<int> sym;
	do {
		generator->generate(idfloor, Decoration::Poly, 4);
		sym.clear();
		for (Point p : floorPos) sym.insert(generator->get(p));
	} while (sym.size() < 4);

	int rotateIndex = Random::rand() % 3;
	for (int i = 0; i < 4; i++) {
		int symbol = generator->get(floorPos[i]);

		correctShapesById[ids[i]] = symbol;

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
			symbol = generator->make_shape_symbol(newShape, true, false);
			if (symbol == 0) {
				symbol = generator->get(floorPos[i]);
				rotateIndex++;
			}
		}

		Generate gen;
		for (Point p : newShape) {
			for (Point dir : Generate::_DIRECTIONS2) {
				if (!newShape.count(p + dir)) {
					gen.setSymbol(PATH, p.first + dir.first / 2, p.second + dir.second / 2);
				}
			}
		}
		gen.setPath({ {0, 0} }); //Just to stop it from trying to make a path
		gen.setFlag(Generate::Config::DecorationsOnly);
		gen.setFlag(Generate::Config::DisableWrite);
		if (i == rotateIndex) gen.generate(ids[i], { });
		else
		{
			gen.generate(ids[i], Decoration::Poly, 1, Decoration::Eraser | Decoration::Color::Green, 1);
			std::set<Point> covered;
			int decoyShape;
			for (int x = 1; x <= 7; x += 2)
				for (int y = 1; y <= 7; y += 2)
					if (gen.get(x, y) != 0) {
						covered.emplace(Point(x, y));
						if (gen.get_symbol_type(gen.get(x, y)) == Decoration::Poly) decoyShape = gen.get(x, y);
					}
			for (Point p : covered) newShape.erase(p);
			if (newShape.size() == 0 || decoyShape == symbol) {
				i--;
				continue;
			}
		}
		Point pos = pick_random(newShape);
		gen.setVal(symbol, pos.first, pos.second);
		gen.write(ids[i]);
	}
	generator->incrementProgress();
	generator->resetVars();
	generator->resetConfig();
}

void Special::generateMountainFloorH()
{
	const std::vector<int> ids = { 0x09EFF, 0x09F01, 0x09FC1, 0x09F8E };
	const int idfloor = 0x09FDA;
	generator->resetConfig();
	std::vector<Point> floorPos = { { 3, 3 },{ 7, 3 },{ 3, 7 },{ 7, 7 } };
	generator->openPos = std::set<Point>(floorPos.begin(), floorPos.end());
	generator->setFlag(Generate::Config::DisableWrite);
	generator->setFlag(Generate::Config::MountainFloorH);
	generator->setSymmetry(Panel::Symmetry::Rotational);
	generator->setSymbol(Decoration::Start, 0, 10); generator->setSymbol(Decoration::Start, 10, 0);
	generator->setSymbol(Decoration::Exit, 0, 0); generator->setSymbol(Decoration::Exit, 10, 10);
	//Make sure no duplicated symbols
	std::set<int> sym;
	do {
		generator->generate(idfloor, Decoration::Poly, 6);
		sym.clear();
		for (Point p : floorPos) sym.insert(generator->get(p));
	} while (sym.size() < 4);

	int combine = 0;
	for (int i = 0; i < 4; i++) {
		int symbol = generator->get(floorPos[i]);

		correctShapesById[ids[i]] = symbol;
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
			for (Point dir : Generate::_DIRECTIONS2) {
				if (!newShape.count(p + dir)) {
					gen.setSymbol(PATH, p.first + dir.first / 2, p.second + dir.second / 2);
				}
			}
		}
		gen.setPath({ { 0, 0 } }); //Just to stop it from trying to make a path
		gen.setFlag(Generate::Config::DecorationsOnly);
		gen.setFlag(Generate::Config::DisableWrite);
		gen.setFlag(Generate::Config::MountainFloorH);
		gen.setFlag(Generate::Config::DisableCancelShapes);
		PuzzleSymbols symbols({ { Decoration::Poly, 2 },{ Decoration::Eraser | Decoration::Color::Green, 1 } });
		if (newShape.size() > 5) {
			if (combine == 0) symbols = PuzzleSymbols({ { Decoration::Poly, 3 },{ Decoration::Eraser | Decoration::Color::Green, 1 } });
			if (combine == 1) symbols = PuzzleSymbols({ { Decoration::Poly, 3 },{ Decoration::Poly | Decoration::Negative | Decoration::Color::Cyan, 1 } });
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
			if (gen.get_symbol_type(gen.get(p)) == Decoration::Poly) count++;
			if (gen.get_symbol_type(gen.get(p)) == Decoration::Eraser) count--;
		}
		if (count != (newShape.size() > 5 ? combine == 2 ? 4 : 2 : 1)) {
			i--;
			if (newShape.size() > 5) combine--;
			continue;
		}
		//Check that the symbols aren't the same
		std::set<int> symbolSet;
		for (Point p : gen._gridpos) {
			if (gen.get_symbol_type(gen.get(p)) == Decoration::Poly) symbolSet.insert(gen.get(p));
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
	for (Point p : floorPos) generator->set(p, Decoration::Poly);
	generator->write(idfloor);
	generator->resetConfig();
}

void Special::generatePivotPanel(int id, Point gridSize, const std::vector<std::pair<int, int>>& symbolVec, bool colorblind) {
	int width = gridSize.first * 2 + 1, height = gridSize.second * 2 + 1;
	std::vector<std::shared_ptr<Generate>> gens;
	for (int i = 0; i < 3; i++) gens.push_back(std::make_shared<Generate>());
	for (std::shared_ptr<Generate> gen : gens) {
		gen->seed(Random::rand());
		gen->colorblind = colorblind;
		gen->setSymbol(Decoration::Start, width / 2, height - 1);
		gen->setGridSize(gridSize.first, gridSize.second);
		gen->setFlag(Generate::Config::FixBackground);
		gen->setFlag(Generate::Config::DisableWrite);
	}
	gens[0]->setSymbol(Decoration::Exit, 0, height / 2);
	gens[1]->setSymbol(Decoration::Exit, width - 1, height / 2);
	gens[2]->setSymbol(Decoration::Exit, width / 2, 0);
	generator->generateMulti(id, gens, symbolVec);
	gens[0]->_panel->_endpoints.clear();
	gens[0]->_panel->SetGridSymbol(0, height / 2, Decoration::Exit, Decoration::Color::None);
	gens[0]->_panel->SetGridSymbol(width - 1, height / 2, Decoration::Exit, Decoration::Color::None);
	gens[0]->_panel->SetGridSymbol(width / 2, 0, Decoration::Exit, Decoration::Color::None);
	gens[0]->setFlag(Generate::Config::TreehouseColors);
	gens[0]->write(id);
	Memory* memory = Memory::get();
	int style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	memory->WritePanelData<int>(id, STYLE_FLAGS, { style | Panel::Style::IS_PIVOTABLE });
}

void Special::modifyGate(int id)
{
	Memory* memory = Memory::get();

	int numIntersections = memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<float> intersections = memory->ReadArray<float>(id, DOT_POSITIONS, numIntersections * 2);
	std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	if (intersectionFlags[24] == 0) return;
	int numConnections = memory->ReadPanelData<int>(id, NUM_CONNECTIONS);
	std::vector<int> connections_a = memory->ReadArray<int>(id, DOT_CONNECTION_A, numConnections);
	std::vector<int> connections_b = memory->ReadArray<int>(id, DOT_CONNECTION_B, numConnections);
	int style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	intersectionFlags[6] |= Decoration::Start;
	intersectionFlags[18] |= Decoration::Start;
	intersectionFlags[11] = Decoration::Dot_Intersection;
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
	memory->WritePanelData<float>(id, PATTERN_POINT_COLOR, { 0.1f, 0.1f, 0.1f, 1 });
	memory->WritePanelData(id, STYLE_FLAGS, style | Panel::Style::HAS_DOTS);
	memory->WritePanelData(id, NEEDS_REDRAW, 1);
}

void Special::addDecoyExits(std::shared_ptr<Generate> gen, int amount) {
	while (amount > 0) {
		Point pos;
		switch (Random::rand() % 4) {
		case 0: pos = Point(0, Random::rand() % gen->_height); break;
		case 1: pos = Point(gen->_width - 1, Random::rand() % gen->_height); break;
		case 2: pos = Point(Random::rand() % gen->_width, 0); break;
		case 3: pos = Point(Random::rand() % gen->_width, gen->_height - 1); break;
		}
		if (pos.first % 2) pos.first--;
		if (pos.second % 2) pos.second--;
		if (gen->_exits.count(pos) || gen->_exits.count(gen->get_sym_point(pos)))
			continue;
		gen->_panel->SetGridSymbol(pos.first, pos.second, Decoration::Exit, Decoration::Color::None);
		gen->_exits.insert(pos);
		amount--;
	}
}

void Special::initSSGrid(std::shared_ptr<Generate> gen) {
	gen->setSymbol(Decoration::Start, 0, 0);  gen->setSymbol(Decoration::Start, 6, 0), gen->setSymbol(Decoration::Start, 8, 0), gen->setSymbol(Decoration::Start, 14, 0);
	gen->setSymbol(Decoration::Start, 0, 6);  gen->setSymbol(Decoration::Start, 6, 6), gen->setSymbol(Decoration::Start, 8, 6), gen->setSymbol(Decoration::Start, 14, 6);
	gen->setSymbol(Decoration::Start, 0, 8);  gen->setSymbol(Decoration::Start, 6, 8), gen->setSymbol(Decoration::Start, 8, 8), gen->setSymbol(Decoration::Start, 14, 8);
	gen->setSymbol(Decoration::Start, 0, 14);  gen->setSymbol(Decoration::Start, 6, 14), gen->setSymbol(Decoration::Start, 8, 14), gen->setSymbol(Decoration::Start, 14, 14);
	gen->setSymbol(Decoration::Exit, 2, 0);  gen->setSymbol(Decoration::Exit, 4, 0), gen->setSymbol(Decoration::Exit, 10, 0), gen->setSymbol(Decoration::Exit, 12, 0);
	gen->setSymbol(Decoration::Exit, 2, 14);  gen->setSymbol(Decoration::Exit, 4, 14), gen->setSymbol(Decoration::Exit, 10, 14), gen->setSymbol(Decoration::Exit, 12, 14);
	gen->setSymbol(Decoration::Exit, 0, 2);  gen->setSymbol(Decoration::Exit, 0, 4), gen->setSymbol(Decoration::Exit, 0, 10), gen->setSymbol(Decoration::Exit, 0, 12);
	gen->setSymbol(Decoration::Exit, 14, 2);  gen->setSymbol(Decoration::Exit, 14, 4), gen->setSymbol(Decoration::Exit, 14, 10), gen->setSymbol(Decoration::Exit, 14, 12);
}

void Special::initRotateGrid(std::shared_ptr<Generate> gen)
{
	gen->setSymbol(Decoration::Start, 4, 0);  gen->setSymbol(Decoration::Start, 10, 4), gen->setSymbol(Decoration::Start, 6, 10), gen->setSymbol(Decoration::Start, 0, 6);
	gen->setSymbol(Decoration::Exit, 6, 0);  gen->setSymbol(Decoration::Exit, 10, 6), gen->setSymbol(Decoration::Exit, 4, 10), gen->setSymbol(Decoration::Exit, 0, 4);
}

void Special::initPillarSymmetry(std::shared_ptr<Generate> gen, int id, Panel::Symmetry symmetry)
{
	gen->setSymmetry(symmetry);
	switch (symmetry) {
	case Panel::Symmetry::PillarParallel:
		gen->setSymbol(Decoration::Start, 0, gen->_height - 1);  gen->setSymbol(Decoration::Start, 6, gen->_height - 1), gen->setSymbol(Decoration::Exit, 0, 0);  gen->setSymbol(Decoration::Exit, 6, 0); break;
	case Panel::Symmetry::PillarVertical:
		gen->setSymbol(Decoration::Start, 2, gen->_height - 1);  gen->setSymbol(Decoration::Start, 4, gen->_height - 1), gen->setSymbol(Decoration::Exit, 2, 0);  gen->setSymbol(Decoration::Exit, 4, 0); break;
	case Panel::Symmetry::PillarHorizontal:
		gen->setSymbol(Decoration::Start, 0, gen->_height - 1);  gen->setSymbol(Decoration::Exit, 6, gen->_height - 1), gen->setSymbol(Decoration::Exit, 0, 0);  gen->setSymbol(Decoration::Start, 6, 0); break;
	case Panel::Symmetry::PillarRotational:
		gen->setSymbol(Decoration::Start, 0, gen->_height - 1);  gen->setSymbol(Decoration::Exit, 6, gen->_height - 1), gen->setSymbol(Decoration::Exit, 0, 0);  gen->setSymbol(Decoration::Start, 6, 0); break;
	}

	Memory* memory = Memory::get();
	memory->WritePanelData<Color>(id, SUCCESS_COLOR_B, { memory->ReadPanelData<Color>(id, SUCCESS_COLOR_A) });
}

void Special::generateSymmetryGate(int id)
{
	Memory* memory = Memory::get();

	generator->resetConfig();
	generator->setFlag(Generate::Config::DisableWrite);
	generator->setFlag(Generate::Config::WriteInvisible);
	generator->setSymmetry(Panel::Symmetry::RotateRight);
	generator->setSymbol(Decoration::Start, 0, 0);
	generator->setSymbol(Decoration::Start, 0, 8);
	generator->setSymbol(Decoration::Start, 8, 0);
	generator->setSymbol(Decoration::Start, 8, 8);
	generator->setSymbol(Decoration::Exit, 4, 0);
	generator->setSymbol(Decoration::Exit, 4, 8);
	generator->setSymbol(Decoration::Exit, 0, 4);
	generator->setSymbol(Decoration::Exit, 8, 4);
	generator->generate(id, Decoration::Triangle | Decoration::Color::Yellow, 4);
	std::vector<Point> breakPos = { {0, 3}, {2, 3}, {4, 3}, {6, 3}, {8, 3}, {0, 5}, {2, 5}, {4, 5}, {6, 5}, {8, 5} };
	for (Point p : breakPos) generator->set(p, IntersectionFlags::COLUMN | 0x40000);
	breakPos = { { 3, 0 },{ 3, 2 },{ 3, 4 },{ 3, 6 },{ 3, 8 },{ 5, 0 },{ 5, 2 },{ 5, 4 },{ 5, 6 },{ 5, 8 } };
	for (Point p : breakPos) generator->set(p, IntersectionFlags::ROW | 0x40000);
	//Expand the grid a little to prevent the collision detection from glitching out
	generator->_panel->minx = 0.08f;
	generator->_panel->maxx = 0.92f;
	generator->_panel->miny = 0.08f;
	generator->_panel->maxy = 0.92f;
	generator->write(id);
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

bool Special::checkDotSolvability(std::shared_ptr<Panel> panel1, std::shared_ptr<Panel> panel2, Panel::Symmetry correctSym) {
	std::vector<Panel::Symmetry> sym = { Panel::Symmetry::FlipXY, Panel::Symmetry::FlipNegXY, Panel::Symmetry::RotateLeft, Panel::Symmetry::RotateRight };
	for (Panel::Symmetry s : sym) {
		if (s == correctSym) continue;
		bool found = false;
		//Check for three dots at a point
		for (int x = 0; x < panel1->_width; x += 2) {
			for (int y = 0; y < panel1->_height; y += 2) {
				int count = 0;
				for (Point dir : Generate::_DIRECTIONS1) {
					Point p = { x + dir.first, y + dir.second };
					Point sp = panel1->get_sym_point(p, s);
					if (p.first < 0 || p.second < 0 || p.first >= panel1->_width || p.second >= panel1->_height) continue;
					if (((panel1->_grid[p.first][p.second] & Decoration::Dot) && !(panel1->_grid[p.first][p.second] & IntersectionFlags::DOT_IS_INVISIBLE)) ||
						((panel2->_grid[sp.first][sp.second] & Decoration::Dot) && !(panel2->_grid[sp.first][sp.second] & IntersectionFlags::DOT_IS_INVISIBLE))) {
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
		for (int x = 1; x < panel1->_width; x += 2) {
			for (int y = 1; y < panel1->_height; y += 2) {
				int count = 0;
				for (Point dir : Generate::_DIRECTIONS1) {
					Point p = { x + dir.first, y + dir.second };
					Point sp = panel1->get_sym_point(p, s);
					if (((panel1->_grid[p.first][p.second] & Decoration::Dot) && !(panel1->_grid[p.first][p.second] & IntersectionFlags::DOT_IS_INVISIBLE)) ||
						((panel2->_grid[sp.first][sp.second] & Decoration::Dot) && !(panel2->_grid[sp.first][sp.second] & IntersectionFlags::DOT_IS_INVISIBLE))) {
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

void Special::createArrowPuzzle(int id, int x, int y, int dir, int ticks, const std::vector<Point>& gaps)
{
	generator->initPanel(id);
	generator->clear();
	generator->set(x, y, Decoration::Arrow | (ticks << 12) | (dir << 16));
	for (Point p : gaps) {
		generator->set(p, p.first % 2 ? Decoration::Gap_Row : Decoration::Gap_Column);
	}
	generator->write(id);
}

void Special::createArrowSecretDoor(int id)
{
	generator->backgroundColor = { 0, 0, 0, 1 };
	generator->arrowColor = { 1, 0.6f, 0, 1 };
	generator->successColor = { 1, 0.6f, 0, 1 };
	generator->initPanel(id);
	generator->clear();
	generator->set(1, 1, Decoration::Arrow | (3 << 12) | (4 << 16));
	generator->set(1, 5, Decoration::Arrow | (3 << 12) | (2 << 16));
	generator->set(1, 9, Decoration::Arrow | (3 << 12) | (5 << 16));
	generator->set(9, 1, Decoration::Arrow | (3 << 12) | (7 << 16));
	generator->set(9, 5, Decoration::Arrow | (3 << 12) | (3 << 16));
	generator->set(9, 9, Decoration::Arrow | (3 << 12) | (6 << 16));
	generator->write(id);
}

void Special::generateCenterPerspective(int id, const std::vector<std::pair<int, int>>& symbolVec, int symbolType)
{
	std::vector<std::vector<Point>> obstructions = { { { 5, 0 },{ 5, 2 },{ 5, 4 } },{ { 5, 6 },{ 5, 8 },{ 5, 10 } },{ { 0, 5 },{ 2, 5 },{ 4, 5 } },{ { 6, 5 },{ 8, 5 },{ 10, 5 } } };
	generator->setObstructions(obstructions);
	do {
		generator->setFlagOnce(Generate::Config::DisableWrite);
		generator->generate(id, symbolVec);
	} while ((generator->get(5, 5) & 0xFF00) != symbolType);
	generator->write(id);
}

void Special::createText(int id, std::string text, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB,
		float left, float right, float top, float bottom) {
	//012
	//345
	//678
	std::map<char, std::vector<int>> coords = {
		{ 'a',{ 6,0,2,8,5,3 } },
		{ 'b',{ 0,2,8,6,0,3,5 } },
		{ 'c',{ 2,0,6,8 } },
		{ 'd',{ 0,1,5,7,6,0 } },
		{ 'e',{ 2,0,3,5,3,6,8 } },
		{ 'f',{ 6,0,2,0,3,5 } },
		{ 'g',{ 2,0,6,8,5,4 } },
		{ 'h',{ 6,0,3,5,2,8 } },
		{ 'i',{ 0,2,1,7,6,8 } },
		{ 'j',{ 3,6,8,2 } },
		{ 'k',{ 0,6,3,2,3,8 } },
		{ 'l',{ 0,6,8 } },
		{ 'm',{ 6,0,4,2,8 } },
		{ 'n',{ 6,0,8,2 } },
		{ 'o',{ 0,2,8,6,0 } },
		{ 'p',{ 6,0,2,5,3 } },
		{ 'q',{ 0,2,8,4,8,6,0 } },
		{ 'r',{ 6,0,2,5,3,8 } },
		{ 's',{ 2,0,3,5,8,6 } },
		{ 't',{ 0,2,1,7 } },
		{ 'u',{ 0,6,8,2 } },
		{ 'v',{ 0,7,2 } },
		{ 'w',{ 0,6,4,8,2 } },
		{ 'x',{ 0,8,4,2,6 } },
		{ 'y',{ 0,4,7,4,2 } },
		{ 'z',{ 0,2,6,8 } },
		{ '0',{ 0,2,8,6,0 } },
		{ '1',{ 0,1,7,6,8 } },
		{ '2',{ 0,2,5,3,6,8 } },
		{ '3',{ 0,2,5,3,5,8,6 } },
		{ '4',{ 0,3,5,2,8 } },
		{ '5',{ 2,0,3,5,8,6 } },
		{ '6',{ 2,0,6,8,5,3 } },
		{ '7',{ 0,2,7 } },
		{ '8',{ 0,2,8,6,0,3,5 } },
		{ '9',{ 6,8,2,0,3,5 } },
		{ '!',{ 1,4,7 } },
		{ ' ',{ } },
		{ '.',{ 7 } },
		{ '-',{ 3,5 } },
		{ ':',{ 4,7 } },
		{ '/', {6,1} },
	};

	float spacingX = (right - left) / (text.size() * 3 - 1);
	float spacingY = (top - bottom) / 2;

	for (int i = 0; i < text.size(); i++) {
		char c = std::tolower(text[i]);
		for (int j = 0; j < coords[c].size(); j++) {
			int n = coords[c][j];
			intersections.emplace_back((n % 3 + i * 3) * spacingX + left);
			intersections.emplace_back(1 - ((2 - n / 3) * spacingY + bottom));
			if (j > 0 && !(c == '!' && j == 2) && !(c == ':')) {
				connectionsA.emplace_back(static_cast<int>(intersections.size()) / 2 - 2);
				connectionsB.emplace_back(static_cast<int>(intersections.size()) / 2 - 1);
			}
		}
	}
}

void Special::drawText(int id, std::vector<float>& intersections, std::vector<int>& connectionsA, std::vector<int>& connectionsB, const std::vector<float>& finalLine) {

	std::vector<int> intersectionFlags;
	for (int i = 0; i < intersections.size() / 2; i++) {
		intersectionFlags.emplace_back(0);
	}
	intersections.emplace_back(finalLine[0]);
	intersectionFlags.emplace_back(Decoration::Start);

	for (int i = 1; i < finalLine.size(); i++) {
		intersections.emplace_back(finalLine[i]);
		if (i % 2 == 0) {
			intersectionFlags.emplace_back(i == finalLine.size() - 2 ? Decoration::Exit : 0);
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

void Special::drawSeedAndDifficulty(int id, int seed, bool hard, bool setSeed, bool options)
{
	std::vector<float> intersections;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;

	createText(id, hard ? "expert" : "normal", intersections, connectionsA, connectionsB, 0.1f, 0.9f, 0.25f, 0.4f);
	std::string seedStr = std::to_string(seed);
	createText(id, seedStr, intersections, connectionsA, connectionsB, 0.5f - seedStr.size()*0.06f, 0.5f + seedStr.size()*0.06f, setSeed ? 0.6f : 0.65f, setSeed ? 0.75f : 0.8f);
	if (setSeed) createText(id, "Archipelago", intersections, connectionsA, connectionsB, 0.05f, 0.95f, 0.86f, 0.96f);
	std::string version = VERSION_STR;
	createText(id, version, intersections, connectionsA, connectionsB, 0.98f - version.size()*0.06f, 0.98f, 0.02f, 0.10f);
	if (options) createText(id, "option", intersections, connectionsA, connectionsB, 0.02f, 0.5f, 0.02f, 0.10f);

	drawText(id, intersections, connectionsA, connectionsB, { 0.1f, 0.5f, 0.9f, 0.5f });

	Memory* memory = Memory::get();
	memory->WritePanelData(id, PATH_WIDTH_SCALE, 0.5f);
	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}


void Special::drawGoodLuckPanel(int id)
{
	std::vector<float> intersections;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;
	createText(id, "good", intersections, connectionsA, connectionsB, 0.2f, 0.8f, 0.07f, 0.23f);
	createText(id, "luck!", intersections, connectionsA, connectionsB, 0.2f, 0.8f, 0.77f, 0.93f);
	drawText(id, intersections, connectionsA, connectionsB, { 0.66f, 0.62f, 0.66f, 0.69f, 0.32f, 0.69f, 0.51f, 0.51f, 0.32f, 0.32f, 0.66f, 0.32f, 0.66f, 0.39f });
}

void Special::SetRequiredLasers(int mountain, int challenge) {
	int id = 0x09F7F;

	std::vector<float> intersections;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;
	std::vector<float> finalLine = { 0.5f, 0.04f, 0.5f, 0.14f, 0.5f, 0.23f, 0.5f, 0.32f, 0.5f, 0.41f, 0.5f, 0.50f, 0.5f, 0.59f, 0.5f, 0.68f , 0.5f, 0.77f , 0.5f, 0.86f , 0.5f, 0.95f };

	int jM = (mountain - 1);
	float yMountain = finalLine[jM * 2 + 1];

	int jC = (challenge - 1);
	float yChallenge = finalLine[jC * 2 + 1];

	finalLine.insert(finalLine.begin() + 4, 0.43);
	finalLine.insert(finalLine.begin() + 5, yMountain);

	finalLine.insert(finalLine.begin() + 8, 0.57);
	finalLine.insert(finalLine.begin() + 9, yChallenge);

	if (jM > 1) {
		if (jM > 2) {
			jM++;
		}
		jM++;
	}
	if (jC > 1) {
		if (jC > 2) {
			jC++;
		}
		jC++;
	}

	std::vector<int> intersectionFlags;
	for (int i = 0; i < intersections.size() / 2; i++) {
		intersectionFlags.emplace_back(0);
	}
	intersections.emplace_back(finalLine[0]);
	intersectionFlags.emplace_back(Decoration::Start);

	for (int i = 1; i < finalLine.size(); i++) {
		intersections.emplace_back(finalLine[i]);
		if (i % 2 == 0) {
			intersectionFlags.emplace_back(i == 4 || i == 8 ? Decoration::Exit : 0);

			if (i == 4 || i == 8) {
				connectionsA.emplace_back(static_cast<int>(i == 4 ? jM : jC)); connectionsB.emplace_back(static_cast<int>(intersectionFlags.size() - 1));
			}
			else if (i == 6 || i == 10) {
				connectionsA.emplace_back(static_cast<int>(intersectionFlags.size() - 3)); connectionsB.emplace_back(static_cast<int>(intersectionFlags.size() - 1));
			}
			else {
				connectionsA.emplace_back(static_cast<int>(intersectionFlags.size() - 2)); connectionsB.emplace_back(static_cast<int>(intersectionFlags.size() - 1));
			}
		}
	}


	int seqLen = 0;
	std::vector<int> seq = { };

	Memory* memory = Memory::get();

	std::vector<int> decorations(memory->ReadPanelData<int>(id, NUM_DECORATIONS), 0);
	std::vector<int> decorationsFlags(memory->ReadPanelData<int>(id, NUM_DECORATIONS), 0);

	memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
	memory->WriteArray<int>(id, DECORATIONS, decorations);
	memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);

	memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { 0 });
	memory->WriteArray<int>(id, COLORED_REGIONS, { });

	memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.6f });
	memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) });
	memory->WriteArray<float>(id, DOT_POSITIONS, intersections);
	memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags);
	memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) });
	memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA);
	memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB);
	memory->WritePanelData<int>(id, SEQUENCE_LEN, { 0 });
	memory->WritePanelData<INT64>(id, SEQUENCE, { 0 });
	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void Special::SkipPanel(int id, std::string text, bool kickOut) {
	Memory* memory = Memory::get();

	// Special Skipping Animations

	if (id == 0x03612) { // Quarry laser panels
		if (!memory->ReadPanelData<int>(0x288E9, 0x1E4))
		{
			memory->OpenDoor(0x288E9);

			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		if (!memory->ReadPanelData<int>(0x28AD4, 0x1E4))
		{
			memory->OpenDoor(0x28AD4);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		memory->PowerGauge(0x1553, 0x1549, 2);
		memory->PowerGauge(0x1553, 0x368A, 1);
	}
	else if (id == 0x1C349) { // Symmetry island upper latches
		if (!memory->ReadPanelData<int>(0x28AE8, 0x1E4))
		{
			memory->OpenDoor(0x28AE8);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		if (!memory->ReadPanelData<int>(0x28AED, 0x1E4))
		{
			memory->OpenDoor(0x28AED);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
	else if (id == 0x03629) { // Tutorial Gate Latches
		if (!memory->ReadPanelData<int>(0x288E8, 0x1E4))
		{
			memory->OpenDoor(0x288E8);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		if (!memory->ReadPanelData<int>(0x288F3, 0x1E4))
		{
			memory->OpenDoor(0x288F3);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		if (!memory->ReadPanelData<int>(0x28942, 0x1E4))
		{
			memory->OpenDoor(0x28942);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		memory->PowerGauge(0x003C4, 0x3F, 3);
		memory->PowerGauge(0x003C4, 0x3C, 2);
		memory->PowerGauge(0x003C4, 0x51, 1);
	}

	memory->UpdatePanelJunctions(id);

	// Special Skipping Animations Done

	if (skip_specialCase.count(id) == 0) {
		DrawSimplePanel(id, text, kickOut);
		return;
	}

	//Symmetry Blue/Yellow panels before laser

	if (id == 0x00A52 || id == 0x00A61) {
		DrawSimplePanel(0x00A52, text, kickOut);
		DrawSimplePanel(0x00A61, text, kickOut);
		return;
	}

	if (id == 0x00A57 || id == 0x00A64) {
		DrawSimplePanel(0x00A57, text, kickOut);
		DrawSimplePanel(0x00A64, text, kickOut);
		return;
	}

	if (id == 0x00A5B || id == 0x00A68) {
		DrawSimplePanel(0x00A5B, text, kickOut);
		DrawSimplePanel(0x00A68, text, kickOut);
		return;
	}

	//Keep Pressure Plates

	if (id == 0x0A3A8) {
		DrawSimplePanel(0x033EA, text, kickOut);
		return;
	}
	if (id == 0x0A3B9) {
		DrawSimplePanel(0x01BE9, text, kickOut);
		return;
	}
	if (id == 0x0A3BB) {
		DrawSimplePanel(0x01CD3, text, kickOut);
		return;
	}
	if (id == 0x0A3AD) {
		DrawSimplePanel(0x01D3F, text, kickOut);
		return;
	}

	if (id == 0x09E86 || id == 0x09ED8) {
		DrawSimplePanel(0x09E86, text, kickOut);
		DrawSimplePanel(0x09ED8, text, kickOut);
		return;
	}

	if (id == 0x09FC1 || id == 0x09F8E || id == 0x09F01 || id == 0x09EFF) { 
		ColorPanel(id, text);
		SkipMetapuzzle(id, text, kickOut);
		return;
	}

	if (id == 0x181F5) {
		ColorPanel(id, text);
		int num_dec = memory->ReadPanelData<int>(id, NUM_DECORATIONS);
		std::vector<int> dec = memory->ReadArray<int>(id, DECORATIONS, num_dec);

		for (int i = 0; i < dec.size(); i++) {
			int decoration = dec[i];

			if((decoration & 0x700) == Decoration::Shape::Triangle || (decoration & 0x700) == Decoration::Shape::Stone || (decoration & 0xF00) == Decoration::Shape::Star) dec[i] = 0;
		}

		memory->WriteArray(id, DECORATIONS, dec);
		memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });

		return;
	}

	if (id == 0x334D8) {
		int num_dec = memory->ReadPanelData<int>(id, NUM_DECORATIONS);
		std::vector<int> dec = memory->ReadArray<int>(id, DECORATIONS, num_dec);

		for (int i = 0; i < dec.size(); i++) {
			int decoration = dec[i];

			if ((decoration & 0x700) == Decoration::Shape::Triangle) dec[i] = 0;
		}

		memory->WriteArray(id, DECORATIONS, dec);
		memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
		return;
	}
}

void Special::SetVanillaMetapuzzleShapes() {
	correctShapesById[0x09f01] = 0x00310400;
	correctShapesById[0x09f8E] = 0x02230400;
	correctShapesById[0x09fc1] = 0x00330400;
	correctShapesById[0x09EFF] = 0x00130400;
}

void Special::SkipMetapuzzle(int id, std::string text, bool kickOut) {
	Memory* memory = Memory::get();

	int num_dec = memory->ReadPanelData<int>(id, NUM_DECORATIONS);
	std::vector<int> dec = memory->ReadArray<int>(id, DECORATIONS, num_dec);

	if (!correctShapesById.count(id)) return;

	int correctShape = correctShapesById[id];

	int likelyShape = -1;
	int definitelyShape = -1;

	bool hardMode = memory->ReadPanelData<int>(0x00182, BACKGROUND_REGION_COLOR + 12) == 1;

	for (int i = 0; i < dec.size(); i++) {
		int decoration = dec[i];

		if ((decoration & ~0x1000) == correctShape) definitelyShape = i;
		if (decoration & 0x1000) definitelyShape = i;

		dec[i] = 0;
	}

	if (definitelyShape != -1 && !hardMode) {
		dec[definitelyShape] = correctShape;
	}
	else {
		Shape shape;
		for (int j = 0; j < 16; j++) {
			if (correctShape & (1 << (j + 16))) {
				shape.emplace(Point((j % 4) * 2 + 1, 8 - ((j / 4) * 2 + 1)));
			}
		}

		//normalize Shape

		int leftBound = 100;
		int upBound = 100;
		int rightBound = -1;
		int downBound = -1;

		for(Point p : shape){
			leftBound = std::min(p.first, leftBound);
			upBound = std::min(p.second, upBound);
			rightBound = std::max(p.first, rightBound);
			downBound = std::max(p.second, downBound);
		}

		Shape normalizedShape;

		for (Point p : shape) {
			p.first -= leftBound;
			p.first /= 2;
			p.second -= upBound;
			p.second /= 2;

			normalizedShape.emplace(p);
		}

		rightBound -= leftBound;
		rightBound /= 2;
		downBound -= upBound;
		downBound /= 2;

		auto first = normalizedShape.begin();
		Point chosenPoint = *first;

		int y = downBound - chosenPoint.second;
		
		int x = chosenPoint.first;

		if (id == 0x9f01 || id == 0x9f8e) {
			x = 3 - rightBound + chosenPoint.first;

			if (rightBound == 2 && downBound == 3) {
				x -= 1;
			}
		}
		else if (rightBound == 2 && downBound == 3) {
			x += 1;
		}

		if (rightBound == 3 && downBound == 2) {
			y += 1;
		}

		int position = x + y * 4;

		dec[position] = correctShape;
	}
	
	memory->WriteArray(id, DECORATIONS, dec);

	memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { 0 });
	memory->WriteArray<int>(id, COLORED_REGIONS, { });


	if (text == "Collected") {
		memory->WritePanelData<float>(id, OUTER_BACKGROUND, { 0.07f, 0.07f, 0.07f, 1.0f });
		memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, { 0.07f, 0.07f, 0.07f, 1.0f });
	}
	else if (text == "Skipped") {
		memory->WritePanelData<float>(id, OUTER_BACKGROUND, { 0.18f, 0.07f, 0.18f, 1.0f });
		memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, { 0.18f, 0.07f, 0.18f, 1.0f });
	}

	memory->WritePanelData<float>(id, PATH_COLOR, { 0.25f, 0.25f, 0.25f, 1.0f });
	memory->WritePanelData<float>(id, ACTIVE_COLOR, { 0.5f, 0.5f, 0.5f, 1.0f });
	memory->WritePanelData<int>(id, OUTER_BACKGROUND_MODE, { 1 });

	int style = memory->ReadPanelData<int>(id, STYLE_FLAGS);
	style &= ~Panel::Style::HAS_ERASERS;
	memory->WritePanelData<int>(id, STYLE_FLAGS, { style });

	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });


	//Panel panel = Panel(0x09FDA);
	//panel.Write(); // I have no idea why this works. But for some reason, drawing the simple panel isn't enough on its own. Reading and writing it, for some reason, makes it work??? Sigma explain

	//DrawSimplePanel(0x09FDA, text, kickOut);
}

void Special::ColorPanel(int id, std::string text) {
	if (text != "Collected" && text != "Disabled" && text != "Excluded" && skip_completelyExclude.count(id)) return;

	Memory* memory = Memory::get();

	bool arrows = false;

	std::vector<int> decorations = memory->ReadArray<int>(id, DECORATIONS, memory->ReadPanelData<int>(id, NUM_DECORATIONS));
	for (int decoration : decorations) {
		if ((decoration & 0x700) == Decoration::Shape::Arrow) {
			arrows = true;
			break;
		}
	}

	if (text == "Collected" && id != 0x28998) {
		memory->WritePanelData<float>(id, OUTER_BACKGROUND, { 0.07f, 0.07f, 0.07f, 1.0f });
		if(!arrows) memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, { 0.07f, 0.07f, 0.07f, 1.0f });
	}
	else if (text == "Skipped" && id != 0x28998) {
		memory->WritePanelData<float>(id, OUTER_BACKGROUND, { 0.18f, 0.07f, 0.18f, 1.0f });
		if (!arrows) memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, { 0.18f, 0.07f, 0.18f, 1.0f });
	}
	else if ((text == "Disabled" || text == "Excluded") && id != 0x28998) {
		memory->WritePanelData<float>(id, OUTER_BACKGROUND, { 0.9f, 0.9f, 0.9f, 1.0f });
		if (!arrows) memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, { 0.9f, 0.9f, 0.9f, 1.0f });
	}

	if (cutoutPanels.count(id)) {
		if (text == "Collected") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.07f, 0.07f, 0.07f, 1.0f });
			memory->WritePanelData<float>(id, ACTIVE_COLOR, { 0.25f, 0.25f, 0.25f, 1.0f });
			memory->WritePanelData<float>(id, SUCCESS_COLOR_A, { 0.25f, 0.25f, 0.25f, 1.0f });
		}
		else if (text == "Skipped") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.18f, 0.07f, 0.18f, 1.0f });
			memory->WritePanelData<float>(id, ACTIVE_COLOR, { 0.25f, 0.25f, 0.25f, 1.0f });
			memory->WritePanelData<float>(id, SUCCESS_COLOR_A, { 0.25f, 0.25f, 0.25f, 1.0f });
		}
		else if (text == "Disabled") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.9f, 0.9f, 0.9f, 1.0f });
			memory->WritePanelData<float>(id, ACTIVE_COLOR, { 0.8f, 0.2f, 0.2f, 1.0f });
			memory->WritePanelData<float>(id, SUCCESS_COLOR_A, { 0.8f, 0.2f, 0.2f, 1.0f });
		}
		else if (text == "Excluded") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.9f, 0.9f, 0.9f, 1.0f });
			memory->WritePanelData<float>(id, ACTIVE_COLOR, { 0.8f, 0.4f, 0.2f, 1.0f });
			memory->WritePanelData<float>(id, SUCCESS_COLOR_A, { 0.8f, 0.4f, 0.2f, 1.0f });
		}
	}
	else
	{
		if (text == "Collected") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.25f, 0.25f, 0.25f, 1.0f });
		}
		else if (text == "Skipped") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.25f, 0.25f, 0.25f, 1.0f });
		}
		else if (text == "Disabled") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.8f, 0.2f, 0.2f, 1.0f });
		}
		else if (text == "Excluded") {
			memory->WritePanelData<float>(id, PATH_COLOR, { 0.8f, 0.4f, 0.2f, 1.0f });
		}
		memory->WritePanelData<float>(id, ACTIVE_COLOR, { 0.5f, 0.5f, 0.5f, 1.0f });
	}
	
	if (id != 0x28998) memory->WritePanelData<int>(id, OUTER_BACKGROUND_MODE, { 1 });

	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void Special::DrawSimplePanel(int id, std::string text, bool kickOut)
{
	ColorPanel(id, text == "Disabled Completely" ? "Disabled" : text);

	Memory* memory = Memory::get();

	if (skip_completelyExclude.count(id)) {
		if (text == "Disabled Completely") {
			std::vector<float> intersections = { -0.3f, -0.3f, 1.3f, 1.3f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
			std::vector<int> intersectionFlags = { IntersectionFlags::STARTPOINT, IntersectionFlags::ENDPOINT };
			std::vector<int> connectionsA = { 2, 4 };
			std::vector<int> connectionsB = { 3, 5 };

			float path_width_scale = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);

			memory->WritePanelData<float>(id, PATTERN_SCALE, { 0.5f / path_width_scale });
			memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
			memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
			memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
			memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
			memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
			memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
			memory->WritePanelData<int>(id, TRACED_EDGES, { 0 }); //removed the traced line
			memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
		}
		
		return;
	}

	int style = memory->ReadPanelData<int>(id, STYLE_FLAGS);

	if (id != 0x01D3F) {
		style &= ~Panel::Style::SYMMETRICAL;
		memory->WritePanelData<INT64>(id, REFLECTION_DATA, { 0 });
	}

	style &= ~Panel::Style::HAS_TRIANGLES;
	style &= ~Panel::Style::HAS_DOTS;
	style &= ~Panel::Style::HAS_SHAPERS;

	if (id == 0x79df) {
		style = 0;
		memory->WritePanelData<int>(id, TARGET, { 0x34F1 });
	}

	memory->WritePanelData<int>(id, STYLE_FLAGS, { style });

	float width = 1.0f;

	if (text == "Disabled Completely") {
		if (false) { // Make this true for an Easter Egg :)
			std::vector<float> intersections = { -0.3f, -0.3f, 1.3f, 1.3f,
				0.25f, 0.9f, 0.25f, 0.6f,
				0.65f, 0.9f, 0.65f, 0.6f,     0.85f, 0.8f, 0.85f, 0.6f,
				0.15f, 0.4f, 0.15f, 0.1f,    0.35f, 0.4f, 0.35f, 0.1f,
				0.65f, 0.4f, 0.65f, 0.1f,    0.7f, 0.2f, 0.95f, 0.2f,

				0.0f, 0.5f, 1.0f, 0.5f,     0.5f, 0.0f, 0.5f, 1.0f,

			};
			std::vector<int> intersectionFlags = { IntersectionFlags::STARTPOINT, IntersectionFlags::ENDPOINT };
			std::vector<int> connectionsA = { 2, 4, 6, 8, 10, 12, 14, 16, 18 };
			std::vector<int> connectionsB = { 3, 5, 7, 9, 11, 13, 15, 17, 19 };

			int currentIntersections = intersections.size();

			float path_width_scale = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);

			memory->WritePanelData<float>(id, PATTERN_SCALE, { 0.5f / path_width_scale });
			memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
			memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
			memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
			memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
			memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
			memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
			memory->WritePanelData<int>(id, TRACED_EDGES, { 0 }); //removed the traced line
			memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
		}
		else {

			std::vector<float> intersections = { -0.3f, -0.3f, 1.3f, 1.3f };
			std::vector<int> intersectionFlags = { IntersectionFlags::STARTPOINT, IntersectionFlags::ENDPOINT };
			std::vector<int> connectionsA;
			std::vector<int> connectionsB;

			int currentIntersections = intersections.size();

			createText(id, "disabled", intersections, connectionsA, connectionsB, 0.1f, 0.9f, 0.35f, 0.65f);

			int newIntersections = intersections.size();

			for (int i = 0; i < (newIntersections - currentIntersections) / 2; i++)
				intersectionFlags.emplace_back(0);

			float path_width_scale = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);

			memory->WritePanelData<float>(id, PATTERN_SCALE, { 0.5f / path_width_scale });
			memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
			memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
			memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
			memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
			memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
			memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
			memory->WritePanelData<int>(id, TRACED_EDGES, { 0 }); //removed the traced line
			memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
		}
	}

	else if (!skip_noLine.count(id)) {
		std::vector<float> intersections;
		std::vector<int> connectionsA;
		std::vector<int> connectionsB;

		std::vector<float> finalLine = { 0.13f, 0.3f, 0.88f, 0.3f };

		if (skip_specialLine.count(id)) {
			finalLine = skip_specialLine.at(id);
		}
		else {
			width = 0.5f;
			createText(id, text, intersections, connectionsA, connectionsB, 0.5f - text.size() * 0.049f, 0.5f + text.size() * 0.049f, 0.2f, 0.4f);
		}

		std::vector<int> intersectionFlags;
		for (int i = 0; i < intersections.size() / 2; i++) {
			intersectionFlags.emplace_back(0);
		}
		intersections.emplace_back(finalLine[0]);
		intersectionFlags.emplace_back(Decoration::Start);

		for (int i = 1; i < finalLine.size(); i++) {
			intersections.emplace_back(finalLine[i]);
			if (i % 2 == 0) {
				intersectionFlags.emplace_back(i == finalLine.size() - 2 ? Decoration::Exit : 0);
				connectionsA.emplace_back(static_cast<int>(intersectionFlags.size()) - 2); connectionsB.emplace_back(static_cast<int>(intersectionFlags.size()) - 1);
			}
		}

		int seqLen = 0;
		std::vector<int> seq = { };

		float path_width_scale = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);

		memory->WritePanelData<float>(id, PATTERN_SCALE, { width / path_width_scale });
		memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) });
		memory->WriteArray<float>(id, DOT_POSITIONS, intersections);
		memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags);
		memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) });
		memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA);
		memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB);
		memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN_REFLECTION, { 0 });
		memory->WritePanelData<INT64>(id, DOT_SEQUENCE_REFLECTION, { 0 });
		memory->WritePanelData<int>(id, DOT_SEQUENCE_LEN, { 0 });
		memory->WritePanelData<INT64>(id, DOT_SEQUENCE, { 0 });

		memory->WritePanelData<int>(id, TRACED_EDGES, { 0 });
	}

	else

	{
		//Get rid of Dots.

		int num_dots = memory->ReadPanelData<int>(id, NUM_DOTS);
		std::vector<int> intersectionFlags = memory->ReadArray<int>(id, DOT_FLAGS, num_dots);
		std::vector<int> newIntFlags = {};
		for (int value : intersectionFlags) {
			int noDotColor = value & 0xFFFF00FF;

			if (noDotColor == Decoration::Dot_Intersection || noDotColor == Decoration::Dot_Row || noDotColor == Decoration::Dot_Column) {
				value &= ~IntersectionFlags::DOT;
				newIntFlags.emplace_back(value);
			}
			else newIntFlags.emplace_back(value);
		}

		memory->WriteArray<int>(id, DOT_FLAGS, newIntFlags);
	}


	std::vector<int> decorations(memory->ReadPanelData<int>(id, NUM_DECORATIONS), 0);
	std::vector<int> decorationsFlags(memory->ReadPanelData<int>(id, NUM_DECORATIONS), 0);



	memory->WritePanelData<int>(id, NUM_DECORATIONS, {static_cast<int>(decorations.size())});
	memory->WriteArray<int>(id, DECORATIONS, decorations);
	memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);

	memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { 0 });
	memory->WriteArray<int>(id, COLORED_REGIONS, { });

	memory->WritePanelData<int>(id, SEQUENCE_LEN, { 0 });
	memory->WritePanelData<INT64>(id, SEQUENCE, { 0 });

	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });

	if (kickOut) {
		float panelDistance = memory->ReadPanelData<float>(id, MAX_BROADCAST_DISTANCE);

		memory->WritePanelData<float>(id, MAX_BROADCAST_DISTANCE, { 0.0001f });


		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		memory->WritePanelData<float>(id, MAX_BROADCAST_DISTANCE, { panelDistance });
		memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
	}

	//Figure out a way to have the randomizer not touch a skipped panel when rerandomizing?
	//Not really necessary, actually, as you should never re-randomize while the game is already running with randomized panels. You should only re-randomize after the game was closed, and the vanilla panels were re-loaded.
}

void Special::writeGoalCondition(int id, std::string goal1, std::string goal2, int mountain_lasers, int challenge_lasers)
{
	std::vector<float> intersections;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;

	if (goal1 != ""){
		createText(id, goal1, intersections, connectionsA, connectionsB, 0.5f - goal1.size() * 0.04f, 0.5f + goal1.size() * 0.04f, 0.05f, 0.20f);
	}

	createText(id, goal2, intersections, connectionsA, connectionsB, 0.5f - goal2.size() * 0.04f, 0.5f + goal2.size() * 0.04f, 0.25f, 0.4f);

	std::string lasersString1 = " Lasers:";
	std::string lasersString2 = std::to_string(mountain_lasers) + " - " + std::to_string(challenge_lasers);
	createText(id, lasersString1, intersections, connectionsA, connectionsB, 0.5f - lasersString1.size() * 0.04f, 0.5f + lasersString1.size() * 0.04f, 0.6f, 0.75f);
	createText(id, lasersString2, intersections, connectionsA, connectionsB, 0.5f - lasersString2.size() * 0.04f, 0.5f + lasersString2.size() * 0.04f, 0.8f, 0.95f);

	drawText(id, intersections, connectionsA, connectionsB, { 0.1f, 0.5f, 0.9f, 0.5f });

	Memory* memory = Memory::get();
	memory->WritePanelData(id, PATH_WIDTH_SCALE, 0.4f);
	memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

//For testing/debugging purposes only
void Special::test() {

}


std::string Special::readStringFromPanels(std::vector<int> panelIDs) {
	std::string output;
	for (int panelIndex = 0; panelIndex < panelIDs.size(); panelIndex++) {
		int panelID = panelIDs[panelIndex];

		// First, check to see if this panel has been overridden at all.
		Memory* memory = Memory::get();
		if (memory->ReadPanelData<float>(panelID, VIDEO_STATUS_COLOR) == 1.0f) {
			// This panel hasn't been overridden, which means there is no more string data to read.
			break;
		}

		for (int propertyOffset = 0; propertyOffset < 16; propertyOffset += 4) {
			// Retrieve the data and append it to the array.
			const int intData = memory->ReadPanelData<int>(panelID, VIDEO_STATUS_COLOR + propertyOffset);
			output.append(reinterpret_cast<const char*>(&intData), 4);

			// Check to see if the string length matches what we would expect it to be if we had just written 4 bytes
			//   to it. This will not be the case if the data we just read contains a null terminator.
			int targetCharCount = panelIndex * 16 + propertyOffset + 4;
			if (output.length() < targetCharCount) {
				break;
			}
		}
	}

	return output;
}

void Special::writeStringToPanels(std::string string, std::vector<int> panelIDs) {
	for (int panelIndex = 0; panelIndex < panelIDs.size(); panelIndex++) {
		int panelID = panelIDs[panelIndex];

		// When determining whether a panel has been overridden, we check VIDEO_STATUS_COLOR + 12 to see if it has its
		//   default value of 1.0f. Clear this value so that we will always know that this panel has been overridden,
		//   even if we don't actually write enough data to require overriding VIDEO_STATUS_COLOR + 12.
		Memory* memory = Memory::get();
		memory->WritePanelData(panelID, VIDEO_STATUS_COLOR + 12, 0);

		// Iterate through the string, writing 4 characters at a time.
		for (int propertyOffset = 0; propertyOffset < 16; propertyOffset += 4) {
			int charIndex = propertyOffset + panelIndex * 16;
			if (charIndex > string.length()) {
				// We've run past the end of the string.
				return;
			}

			// Cast four bytes of the array to an integer and write it to the panel. We don't actually care if we read
			//   past the end of the character array here because we'll still be capturing the null terminator, which
			//   is what actually matters for reading.
			int intData = *reinterpret_cast<const int*>(string.c_str() + charIndex);
			memory->WritePanelData(panelID, VIDEO_STATUS_COLOR + propertyOffset, intData);
		}
	}
}

std::map<int, int> Special::correctShapesById = {};