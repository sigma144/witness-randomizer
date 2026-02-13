// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Generate.h"
#include "Randomizer.h"
#include "MultiGenerate.h"
#include "Special.h"

Generate::Generate() {
	randomizer = Randomizer::get();
	handle = NULL;
	areaTotal = genTotal = totalPuzzles = areaPuzzles = stoneTypes = 0;
	bisect = false;
	parity = -1;
	randomSeed = rand();
	resetConfig();
}

void Generate::generate(PanelID id, const std::vector<std::pair<int, int>>& symbolVec) {
	PuzzleSymbols symbols(symbolVec);
	while (!generate(id, symbols));
}

bool Generate::tryGenerate(PanelID id, const std::vector<std::pair<int, int>>& symbolVec, int attempts) {
	int fails = 0;
	PuzzleSymbols symbols(symbolVec);
	while (!generate(id, symbols)) {
		if (fails++ > attempts)
			return false;
	}
	return true;
}

//Generate puzzle with multiple solutions. id - id of the puzzle. gens - the generators that will be used to make solutions. symbolVec - pairs of symbols and amounts to use
void Generate::generateMulti(PanelID id, std::vector<std::shared_ptr<Generate>> gens, std::vector<std::pair<int, int>> symbolVec) {
	MultiGenerate gen;
	gen.splitStones = (id == MOUNTAIN_TOP); //Mountaintop
	gen.generate(id, gens, symbolVec);
	incrementProgress();
}

//Generate puzzle with multiple solutions. id - id of the puzzle. numSolutions - the number of possible solutions. symbolVec - pairs of symbols and amounts to use
void Generate::generateMulti(PanelID id, int numSolutions, std::vector<std::pair<int, int>> symbolVec) {
	MultiGenerate gen;
	gen.splitStones = (id == MOUNTAIN_TOP); //Mountaintop
	std::vector<std::shared_ptr<Generate>> gens;
	for (; numSolutions > 0; numSolutions--)
		gens.push_back(std::make_shared<Generate>());
	gen.generate(id, gens, symbolVec);
	incrementProgress();
}

//Make a maze puzzle. The maze will have one solution. id - id of the puzzle
void Generate::generateMaze(PanelID id) {
	while (!generateMazeI(id, 0, 0));
}

//Make a maze puzzle. The maze will have one solution. id - id of the puzzle. numStarts - how many starts to add (only one will be valid). numExits - how many exits to add. All will work
//Setting numStarts or numExits to 0 will keep the starts/exits where they originally were, otherwise the starts/exits originally there will be removed and new ones randomly placed.
void Generate::generateMaze(PanelID id, int numStarts, int numExits) {
	while (!generateMazeI(id, numStarts, numExits));
}

//Read in default panel data, such as dimensions, symmetry, starts/exits, etc. id - id of the puzzle
void Generate::initPanel(PanelID id) {
	panel = Panel(id);
	if (width > 0 && height > 0 && !(width == panel.width && height == panel.height)) {
		panel.resize(panel.isCylinder ? width - 1 : width, height);
	}
	if (hasConfig(TreehouseLayout)) {
		initTreehouseLayout();
	}
	//TODO: Get rid of this and replace with setSymbol only.
	if (customGrid.size() > 0) { //If we want to start with a certain default grid when generating
		if (customGrid.size() < panel.width) {
			customGrid.resize(panel.width, std::vector<int>());
		}
		if (customGrid[customGrid.size() - 1].size() < panel.height) {
			for (auto& row : customGrid) {
				row.resize(panel.height, 0);
			}
		}
		if (hasConfig(PreserveStructure)) {
			for (int x = 0; x < panel.width; x++)
				for (int y = 0; y < panel.height; y++)
					if (get(x, y) == OPEN || panel.getFlag(x, y, 0x60000f) == NO_POINT || panel.getFlag(x, y, Empty) == Empty)
						customGrid[x][y] = get(x, y);
		}
		panel.setGrid(customGrid);
	}
	//Sync up start/exit points between panel and generator. If both are different, the generator's start/exit point list will be used
	if (starts.size() == 0)
		starts = std::set<Point>(panel.startpoints.begin(), panel.startpoints.end());
	else
		panel.startpoints = std::vector<Point>(starts.begin(), starts.end());
	if (exits.size() == 0) {
		for (Endpoint e : panel.endpoints) {
			exits.emplace(Point(e.x, e.y));
		}
	}
	else {
		panel.endpoints.clear();
		for (Point e : exits) {
			panel.setGridSymbol(e.x, e.y, Exit, NoColor);
		}
	}
	//Fill gridpos with every available grid block
	gridpos.clear();
	for (int x = 1; x < panel.width; x += 2) {
		for (int y = 1; y < panel.height; y += 2) {
			if (!(hasConfig(PreserveStructure) && panel.getFlag(x, y, Empty) == Empty)) //TODO: When do these empty points get erased?
				gridpos.emplace(Point(x, y));
		}
	}
	//Init the open positions available for symbols. Defaults to every grid block unless a custom openpos has been specified
	//TODO: Replace all this stuff with setSymbol.
	if (openPos.size() > 0) openpos = openPos;
	else openpos = gridpos;
	for (Point p : blockPos) openpos.erase(p); //Remove the points which the user has defined to not place symbols on
	for (Point p : splitPoints) openpos.erase(p); //The split points will have erasers and cannot have any other symbols placed on them
	panel.symmetry = symmetry;
	if (lineThickness != 1) //Init path scale. "1" is considered the default, and therefore means no change.
		panel.lineThickness = lineThickness;
}

//Place a specific symbol into the puzzle at the specified location. The generator will add other symbols, but will leave the set ones where they are.
//symbol - the symbol to place. //x, y - the coordinates to put it at. (0, 0) is at top left. Lines are at even coordinates and grid blocks at odd coordinates
void Generate::setSymbol(int symbol, int x, int y) {
	//TODO: Probably write into map instead of custom grid.
	if (customGrid.size() < x + 1) {
		customGrid.resize(x + 1, std::vector<int>());
		for (auto& row : customGrid) {
			row.resize(customGrid[0].size(), 0);
		}
	}
	for (auto& row : customGrid) {
		if (row.size() < y + 1) {
			row.resize(y + 1, 0);
		}
	}
	if (symbol == Start) starts.emplace(Point(x, y));
	else if (symbol == Exit) exits.emplace(Point(x, y));
	else customGrid[x][y] = symbol; //Starts and exits are not set into the grid
}

//Set the dimensions of the puzzles. This setting will persist between puzzle generation calls. (0, 0) will have the generator use the same dimensions as the orignal puzzle.
//width, height - the dimensions to use, measured in grid blocks.
void Generate::setGridSize(int width, int height) {
	if (width <= 0 || height <= 0) {
		this->width = 0; this->height = 0;
	}
	else {
		this->width = width * 2 + 1; this->height = height * 2 + 1;
	}
}

//Set the type of symmetry to use. This setting will persist between puzzle generation calls.
void Generate::setSymmetry(Symmetry symmetry) {
	this->symmetry = symmetry;
	if (symmetry == ParallelV || symmetry == ParallelVFlip) {
		std::vector<Point> points;
		for (int y = 0; y < height; y += 2) points.emplace_back(Point(width / 2, y));
		setObstructions(points); //This prevents the generator from invalidly passing through the center line
		//TODO: What if there are existing obstructions?
	}
	else if (symmetry == ParallelH || symmetry == ParallelHFlip) {
		std::vector<Point> points;
		for (int x = 0; x < width; x += 2) points.emplace_back(Point(x, height / 2));
		setObstructions(points); //This prevents the generator from invalidly passing through the center line
	}
	panel.symmetry = symmetry;
}

//Write out panel data to the puzzle with the given id
void Generate::write(PanelID id) {
	Memory* memory = Memory::get();
	
	incrementProgress();

	bool hasBeenRandomized = memory->ReadPanelData<int>(TUT_ENTER_1, BACKGROUND_HIDDEN_VAR) > 0;

	if (hasConfig(ResetColors)) {
		panel.colorMode = Panel::Reset;
	}
	else if (hasConfig(AlternateColors)) {
		panel.colorMode = Panel::Alternate;
	}
	else if (hasConfig(WriteColors)) {
		panel.colorMode = Panel::WriteColors;
	}
	if (!hasBeenRandomized) {
		if (hasConfig(CyanAndYellowLines)) {
			memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR_A, { 0.35f, 1, 0.92f, 1 });
			memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR_B, { 1, 1, 0, 1 });
			memory->WritePanelData<Color>(id, ACTIVE_COLOR, { 0.35f, 1, 0.92f, 1 });
			memory->WritePanelData<Color>(id, REFLECTION_PATH_COLOR, { 1, 1, 0, 1 });
		}
		if (hasConfig(InvisibleSymmetryLine)) {
			memory->WritePanelData<Color>(id, REFLECTION_PATH_COLOR, { 0, 0, 0, 0 });
		}
		if (hasConfig(MatchDotColor)) {
			Color color = memory->ReadPanelData<Color>(id, SUCCESS_COLOR_A);
			memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR, color);
		}
		else if (hasConfig(FixDotColor)) {
			memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR, { 0.1f, 0.1f, 0.1f, 1 });
		}
		else if (memory->ReadPanelData<Color>(id, PATTERN_POINT_COLOR).b > 0.2f) {
			memory->WritePanelData<Color>(id, PATTERN_POINT_COLOR, { 0, 0, 0, 1 });
		}
		if (foregroundColor.a > 0) {
			memory->WritePanelData<Color>(id, BACKGROUND_REGION_COLOR, foregroundColor);
		}
		if (gridColor.a > 0) {
			memory->WritePanelData<Color>(id, PATH_COLOR, gridColor);
		}
		if (activeColor.a > 0) {
			memory->WritePanelData<Color>(id, ACTIVE_COLOR, activeColor);
		}
	}
	if (backgroundColor.a > 0) {
		memory->WritePanelData<Color>(id, OUTER_BACKGROUND, backgroundColor);
	}
	if (successColor.a > 0) {
		memory->WritePanelData<Color>(id, SUCCESS_COLOR_A, successColor);
		memory->WritePanelData<Color>(id, SUCCESS_COLOR_B, successColor);
	}
	else if (panel.symmetry && !(panel.style & IS_2COLOR)) {
		memory->WritePanelData<Color>(id, SUCCESS_COLOR_B, memory->ReadPanelData<Color>(id, SUCCESS_COLOR_A));
	}
	if (failColor.a > 0) {
		memory->WritePanelData<Color>(id, ERROR_COLOR, failColor);
	}
	if (hasConfig(TreehouseLayout)) {
		memory->WritePanelData(id, SPECULAR_ADD, 0.001f); //Get rid of annoying glare on treehouse panels
	}

	memory->WritePanelData<int>(id, POWER_OFF_ON_FAIL, hasConfig(PowerOffOnFail));

	panel.decorationsOnly = hasConfig(DecorationsOnly);
	panel.disableFlash = hasConfig(DisableFlash);
	if (id == KEEP_PRESSURE_1 || id == KEEP_PRESSURE_2 || id == KEEP_PRESSURE_3 || id == KEEP_PRESSURE_4)
		panel.disableFlash = true;
	if (hasConfig(PreserveStructure) || hasConfig(DecorationsOnly))
		panel.fixBackground = false;

	panel.write(id);
	
	resetVars(); //Reset generator data such as openpos, custom grids, etc. that doesn't persist across puzzles

	//Undo any one-time config changes
	for (Config c : oneTimeAdd) {
		config.erase(c);
	}
	oneTimeAdd.clear();
	for (Config c : oneTimeRemove) {
		config.insert(c);
	}
	oneTimeRemove.clear();
	//Manually advance seed by 1 each generation to prevent seeds "funneling" from repeated fails
	Random::seed(randomSeed);
	randomSeed = rand();
}

//Reset all config flags and persistent settings, including width/height and symmetry.
void Generate::resetConfig() {
	setGridSize(0, 0);
	symmetry = NoSymmetry;
	lineThickness = 1;
	config.clear();
	oneTimeAdd.clear();
	oneTimeRemove.clear();
	backgroundColor = foregroundColor = gridColor = activeColor = successColor = failColor = { 0, 0, 0, 0 };
}

//Increment the counter on the progress indicator. This is called each time a puzzle is written, but may be called manually in other situations
void Generate::incrementProgress() {
	areaTotal++;
	genTotal++;
	if (handle) {
		int total = (totalPuzzles == 0 ? areaPuzzles : totalPuzzles);
		if (total == 0) return;
		std::wstring text = areaName + L": " + std::to_wstring(areaTotal) + L"/" + std::to_wstring(areaPuzzles) + L" (" + std::to_wstring(genTotal * 100 / total) + L"%)";
		SetWindowText(handle, text.c_str());
	}
}

//----------------------Private--------------------------

//Add the point (pos) to the intended solution path, using symmetry if applicable.
void Generate::setPath(Point pos) {
	set(pos.x, pos.y, PATH);
	path.insert(pos);
	if (panel.symmetry) {
		Point sp = getSymPoint(pos.x, pos.y);
		set(sp.x, sp.y, PATH);
		path.insert(sp);
		path1.insert(pos);
		path2.insert(sp);
	}
}

//Remove the path and all symbols from the grid. This does not affect starts/exits. If PreserveStructure is active, open gaps will be kept. If a custom grid is set, this will reset it back to the custom grid state.
void Generate::clear() {
	if (customGrid.size() > 0) {
		panel.setGrid(customGrid); //TOOD: Why don't we clear here?
	}
	else for (int x = 0; x < panel.width; x++) {
		for (int y = 0; y < panel.height; y++) {
			if (hasConfig(PreserveStructure) && (get(x, y) == OPEN || panel.getFlag(x, y, 0x60000f) == NO_POINT || panel.getFlag(x, y, Empty) == Empty))
				continue;
			set(x, y, 0);
		}
	}
	path.clear(); path1.clear(); path2.clear();
}

//Reset generator variables and lists used when generating puzzles. (not config settings)
void Generate::resetVars() {
	starts.clear();
	exits.clear();
	customGrid.clear();
	hitPoints.clear();
	obstructions.clear();
	openPos.clear();
	blockPos.clear();
	splitPoints.clear();
	customPath.clear();
}

//Place start and exits in central positions like in the treehouse
void Generate::initTreehouseLayout() {
	bool pivot = panel.endpoints.size() > 2;
	setSymbol(Start, panel.width / 2, panel.height - 1);
	setSymbol(Exit, panel.width / 2, 0);
	if (pivot) {
		setSymbol(Exit, panel.width - 1, panel.height / 2 );
		setSymbol(Exit, 0, panel.height / 2);
	}
}

//Private version of generateMaze. Should be called again if false is returned.
//The algorithm works by generating a correct path, then extending lines off of it until the maze is filled.
bool Generate::generateMazeI(PanelID id, int numStarts, int numExits) {
	initPanel(id);
	bool fullGaps = hasConfig(FullGaps);
	if (numStarts > 0) placeStart(numStarts);
	if (numExits > 0) placeExit(numExits);
	//Prevent start and exit from overlapping, except in one one particular puzzle.
	if (id == SYM_MAZE_R3 && randomizer->difficulty == Expert) {
		clear();
		panel.endpoints.clear();
		exits.clear();
		Point start = pickRandom(starts);
		panel.setGridSymbol(start.x, start.y, Exit, NoColor);
		Point sp = getSymPoint(start.x, start.y);
		panel.setGridSymbol(sp.x, sp.y, Exit, NoColor);
		setPath(start); setPath(sp);
	}
	else {
		for (Point p : starts)
			if (exits.count(p))
				return false;

		clear();
		if (hasConfig(ShortPath)) {
			while (!generatePathLength((panel.width + panel.height),
				min((panel.width + panel.height) * 2, (panel.width / 2 + 1) * (panel.height / 2 + 1) * 1 / 2))) clear();
		}
		while (!generatePathLength((panel.width + panel.height),
			min((panel.width + panel.height) * 2, (panel.width / 2 + 1) * (panel.height / 2 + 1) * 4 / 5))) clear();
	}
	
	std::set<Point> backupPath = path; //Backup

	//Extra false starts are tracked in a separate list so that the generator can make sure to extend each of them by a higher amount than usual.
	std::set<Point> extraStarts;
	for (Point pos : starts) {
		if (!path.count(pos)) {
			extraStarts.insert(pos);
		}
		setPath(pos);
	}
	//Check to see if the correct path runs over any of the false start points. If so, start over
	if (extraStarts.size() != (panel.symmetry ? starts.size() / 2 - 1 : starts.size() - 1))
		return false;

	std::set<Point> check;
	std::vector<Point> deadEndH, deadEndV;
	for (Point p : path) {
		if (p.x % 2 == 0 && p.y % 2 == 0)
			check.insert(p); //Only extend off of the points at grid intersections.
	}
	while (check.size() > 0) {
		//Pick a random extendable point and extend it for some randomly chosen amount of units.
		Point randomPos = (extraStarts.size() > 0 ? pickRandom(extraStarts) : pickRandom(check));
		Point pos = randomPos;
		for (int i = (extraStarts.size() > 0 ? 7 : 1); i >= 0; i--) { //False starts are extended by up to 7 units. Other points are extended 1 unit at a time
			std::vector<Point> validDir;
			for (Point dir : Panel::DIRECTIONS_2) {
				if (get(pos + dir) == 0) {
					validDir.push_back(dir);
				}
			}
			if (validDir.size() < 2) check.erase(pos); //If there are 0 or 1 open directions, the point cannot be extended again.
			if (validDir.size() == 0) {
				if (extraStarts.size() > 0) {
					return false; //Not all the starts were extended successfully.
				}
				//If full gaps mode is enabled, detect dead ends, so that square tips can be put on them
				if (fullGaps && !exits.count(pos) && !starts.count(pos)) {
					int countOpenRow = 0, countOpenColumn = 0;
					for (Point dir2 : Panel::DIRECTIONS) {
						if (get(pos + dir2) == PATH) {
							if (dir2.x == 0) countOpenColumn++;
							else countOpenRow++;
						}
					}
					if (countOpenRow + countOpenColumn == 1) {
						if (countOpenRow) deadEndH.push_back(pos);
						else deadEndV.push_back(pos);
					}
				}
				break; //A dead end has been reached, extend a different point
			}
			Point dir = pickRandom(validDir);
			Point newPos = pos + dir;
			setPath(pos + dir / 2);
			setPath(newPos);
			check.insert(newPos);
			pos = newPos;
		}
		if (extraStarts.size() > 0) extraStarts.erase(randomPos);
	}
	//Put openings or gaps in any unused row or column segment
	for (int y = 0; y < panel.height; y++) {
		for (int x = (y + 1) % 2; x < panel.width; x += 2) {
			if (get(x, y) != PATH) {
				set(x, y, fullGaps ? OPEN : x % 2 == 0 ? Gap_Column : Gap_Row);
				if (panel.symmetry) {
					Point sp = getSymPoint(x, y);
					if (sp.x == x && sp.y == y || sp.x == x && x % 2 == 0 && abs(sp.y - y) <= 2 ||
						sp.y == y && y % 2 == 0 && abs(sp.x - x) <= 2 || abs(sp.x - x) == 1) {
						set(x, y, PATH);
					}
					else if (rand(2) == 0) {
						set(sp.x, sp.y, PATH);
					}
					else {
						set(x, y, PATH);
						set(sp.x, sp.y, fullGaps ? OPEN : x % 2 == 0 ? Gap_Column : Gap_Row);
					}
				}
			}
		}
	}
	//Put square ends on any dead ends
	for (Point p : deadEndH) {
		set(p, Gap_Row);
	}
	for (Point p : deadEndV) {
		set(p, Gap_Column);
	}
	path = backupPath; //Restore backup of the correct solution for testing purposes
	std::vector<std::string> solution; //For debugging only
	for (int y = 0; y < panel.height; y++) {
		std::string row;
		for (int x = 0; x < panel.width; x++) {
			if (path.count(Point(x, y))) {
				row += "xx";
			}
			else row += "    ";
		}
		solution.push_back(row);
	}
	if (!hasConfig(DisableWrite)) write(id);
	return true;
}

//The primary generation function. id - id of the puzzle. symbols - a structure representing the amount and types of each symbol to add to the puzzle
//The algorithm works by making a random path and then adding the chosen symbols to the grid in such a way that they will be satisfied by the path.
//if at some point the generator fails to add a symbol while still making the solution correct, the function returns false and must be called again.
bool Generate::generate(PanelID id, PuzzleSymbols symbols) {
	initPanel(id);

	//Multiple erasers are forced to be separate by default. This is because combining them causes unpredictable and inconsistent behavior. 
	if (symbols.getNum(Eraser) > 1 && !hasConfig(CombineErasers)) {
		setSymbol(Gap_Row, 1, 0);
		setSymbol(Gap_Row, panel.width - 2, panel.height - 1);
		splitPoints = { Point(1, 1), Point(panel.width - 2, panel.height - 2) };
		initPanel(id); //Re-initiaiize to account for the newly added information
	}

	//Init parity for full dot puzzles
	if (symbols.getNum(Dot) >= panel.getNumGridPoints() - 2)
		parity = (panel.getParity() + (
			!symbols.any(Start) ? getParity(pickRandom(starts)) :
			!symbols.any(Exit) ? getParity(pickRandom(exits)) : rand(2))) % 2;
	else parity = -1; //-1 indicates a non-full dot puzzle

	if (symbols.any(Start)) placeStart(symbols.getNum(Start));
	if (symbols.any(Exit)) placeExit(symbols.getNum(Exit));

	//Make a random path unless a fixed one has been defined
	if (customPath.size() == 0) {
		int fails = 0;
		while (!generatePath(symbols)) {
			if (fails++ > 20) return false; //It gets several chances to make a path so that the whole init process doesn't have to be repeated so many times
		}
	}
	else path = customPath;

	std::vector<std::string> solution; //For debugging only
	for (int y = 0; y < panel.height; y++) {
		std::string row;
		for (int x = 0; x < panel.width; x++) {
			if (get(x, y) == PATH) {
				row += "xx";
			}
			else row += "    ";
		}
		solution.push_back(row);
	}

	//Attempt to add the symbols
	if (!placeSymbols(symbols))
		return false;

	if (!hasConfig(DisableWrite)) write(id);
	return true;
}

//Place the provided symbols onto the puzzle. symbols - a structure describing types and amounts of symbols to add.
bool Generate::placeSymbols(PuzzleSymbols & symbols) {
	std::vector<int> eraseSymbols;
	std::vector<int> eraserColors;
	std::array<int, Invisible> starDiff; //For symbols that have region color constraints so we place stars earlier
	starDiff.fill(0);
	//If erasers are present, choose symbols to be erased and remove them pre-emptively
	for (std::pair<int, int> s : symbols[Eraser]) {
		for (int i = 0; i < s.second; i++) {
			eraserColors.push_back(s.first & 0xf);
			eraseSymbols.push_back(hasConfig(FalseParity) ? Dot_Intersection : symbols.popRandomSymbol());
		}
	}

	//Symbols are placed in stages according to their type
	//In each of these loops, s.first is the symbol and s.second is the amount of it to add
	//TODO: Separate negative rotated shapes

	SHAPEDIRECTIONS = (hasConfig(DisconnectShapes) ? DISCONNECT : Panel::DIRECTIONS_2);
	int numShapes = 0, numRotate = 0, numNegative = 0;
	std::vector<int> colors, negativeColors;
	for (std::pair<int, int> s : symbols[Poly]) {
		for (int i = 0; i < s.second; i++) {
			if (s.first & Rotate) numRotate++;
			if (s.first & Negative) {
				numNegative++;
				negativeColors.push_back(s.first & 0xf);
			}
			else {
				numShapes++;
				colors.push_back(s.first & 0xf);
			}
		}
	}
	if (numShapes > 0 && !placeShapes(colors, negativeColors, numShapes, numRotate, numNegative) || numShapes == 0 && numNegative > 0)
		return false;

	stoneTypes = static_cast<int>(symbols[Stone].size());
	bisect = true; //This flag helps the generator prevent making two adjacent regions of stones the same color
	for (const std::pair<int, int>& s : symbols[Stone]) {
		if (!placeStones(s.first & 0xf, s.second)) return false;
	}
	for (const std::pair<int, int>& s : symbols[Triangle]) {
		if (!placeTriangles(s.first & 0xf, s.second, s.first >> 16)) return false;
	}
	for (const std::pair<int, int>& s : symbols[Arrow]) {
		if (!placeArrows(s.first & 0xf, s.second, s.first >> 20)) return false;
	}
	for (const std::pair<int, int>& s : symbols[AntiTriangle]) {
		if (!placeAntiTriangles(s.first & 0xf, s.second, s.first >> 20)) return false;
	}
	for (const std::pair<int, int>& s : symbols[Cave]) {
		if (!placeCaveClues(s.first & 0xf, s.second, s.first >> 20)) return false;
	}
	for (const std::pair<int, int>& s : symbols[Minesweeper0]) {
		if (!placeMinesweeperClues(s.first & 0xf, s.second, s.first >> 20)) return false;
	}
	for (const std::pair<int, int>& s : symbols[Flower]) {
		int tempStarDiff = 0;
		if (symbols.style == HAS_STARS) {	
			for (const std::pair<int, int>& t : symbols[Star]) {
				if ((s.first & 0xf) == (t.first & 0xf)) { //Get how many stars of the same color are planned to be placed
					tempStarDiff = rand(0, t.second); //Take off some stars to be placed as pairs with the flowers
				}
			}
		}
		if (!placeFlowers(s.first & 0xf, s.second - tempStarDiff)) return false;
		if (!placeFlowerStarPairs(s.first & 0xf, tempStarDiff)) return false;
		starDiff[s.first & 0xf] = tempStarDiff;
	}
	for (const std::pair<int, int>& s : symbols[Star]) {
		if (!placeStars(s.first & 0xf, s.second - starDiff[s.first & 0xf])) return false; 
	}
	if (symbols.style == HAS_STARS && hasConfig(TreehouseLayout) && !checkStarZigzag(panel)) {
		return false;
	}
	if (eraserColors.size() > 0 && !placeErasers(eraserColors, eraseSymbols)) {
		return false;
	}
	for (const std::pair<int, int>& s : symbols[Dot]) {
		if (!placeDots(s.second, (s.first & 0xf), (s.first & ~0xf) == Dot_Intersection)) return false;
	}
	for (const std::pair<int, int>& s : symbols[Gap]) {
		if (!placeGaps(s.second)) return false;
	}
	return true;
}

//Generate a random path for a puzzle with the provided symbols.
//The path starts at a random start and will not cross through walls or symbols.
//Puzzle symbols are provided because they can influence how long the path should be.
bool Generate::generatePath(PuzzleSymbols& symbols) {
	clear();
	if (obstructions.size() > 0) {
		//TODO: Change this code to not be duplicated.
		std::vector<Point> walls = pickRandom(obstructions);
		for (Point p : walls) if (get(p) == 0) set(p, p.x % 2 == 0 ? Gap_Column : Gap_Row);
		bool result = (parity != -1 || hasConfig(LongestPath) ? generateLongestPath() :
			hitPoints.size() > 0 ? generateSpecialPath() : 
			hasConfig(ShortPath) ? generatePathLength(1) :
			generatePathLength(panel.getNumGridPoints() * 3 / 4));
		for (Point p : walls) if (get(p) & Gap) set(p, 0);
		return result;
	}
	if (parity != -1 || hasConfig(LongestPath)) {
		return generateLongestPath();
	}
	if (hitPoints.size() > 0) {
		return generateSpecialPath();
	}
	if (hasConfig(ShortPath))
		return generatePathLength(1);
	//The diagonal symmetry puzzles have a lot of points that can't be hit, so I have to reduce the path length
	if (panel.symmetry == FlipXY || panel.symmetry == FlipNegXY) {
		return generatePathLength(panel.getNumGridPoints() * 3 / 4 - panel.width / 2);
	}
	//Dot puzzles have a longer path by default. Vertical/horizontal symmetry puzzles are also longer because they tend to be too simple otherwise
	if (hasConfig(LongPath) || symbols.style == HAS_DOTS && !hasConfig(PreserveStructure) &&
		!(panel.symmetry == Vertical && (panel.width / 2) % 2 == 0 ||
			panel.symmetry == Horizontal && (panel.height / 2) % 2 == 0)) {
		return generatePathLength(panel.getNumGridPoints() * 7 / 8);
	}
	//For stone puzzles, the path must have a certain number of regions
	if (symbols.style == HAS_STONES && splitPoints.size() == 0)
		return generatePathRegions(min(symbols.getNum(Stone), (panel.width / 2 + panel.height / 2) / 2 + 1));
	if (symbols.style == HAS_SHAPERS) {
		if (hasConfig(SplitShapes)) {
			return generatePathRegions(symbols.getNum(Poly) + 1);
		}
		return generatePathLength(panel.getNumGridPoints() / 2);
	}
	return generatePathLength(panel.getNumGridPoints() * 3 / 4);
}

//Generate a random path with the provided minimum length.
bool Generate::generatePathLength(int minLength, int maxLength) {
	int fails = 0;
	Point pos = adjustPoint(pickRandom(starts));
	Point exit = adjustPoint(pickRandom(exits));
	if (offEdge(pos) || offEdge(exit)) return false;
	setPath(pos);
	while (pos != exit) {
		if (fails++ > 20)
			return false;
		Point dir = pickRandom(Panel::DIRECTIONS_2);
		Point newPos = pos + dir;
		if (get(newPos) != 0 || get(pos + dir / 2) != 0
			|| newPos == exit && path.size() / 2 + 2 < minLength) continue;
		if (panel.symmetry && (offEdge(getSymPoint(newPos)) || newPos == getSymPoint(newPos)))
			continue;
		setPath(pos + dir / 2);
		setPath(newPos);
		pos = newPos;
		fails = 0;
	}
	return path.size() / 2 + 1 >= minLength && path.size() / 2 + 1 <= maxLength;
}

//Generate a path with the provided number of regions.
bool Generate::generatePathRegions(int minRegions) {
	int fails = 0;
	int regions = 1;
	Point pos = adjustPoint(pickRandom(starts));
	Point exit = adjustPoint(pickRandom(exits));
	if (offEdge(pos) || offEdge(exit)) return false;
	setPath(pos);
	while (pos != exit) {
		if (fails++ > 20)
			return false;
		Point dir = pickRandom(Panel::DIRECTIONS_2);
		Point newPos = pos + dir;
		if (get(newPos) != 0 || get(pos + dir / 2) != 0
			|| newPos == exit && regions < minRegions)
			continue;
		if (panel.symmetry && (offEdge(getSymPoint(newPos)) || newPos == getSymPoint(newPos))) continue;
		setPath(pos + dir / 2);
		setPath(newPos);
		if (!onEdge(newPos) && onEdge(pos)) {
			regions++;
			if (panel.symmetry) regions++;
		}
		pos = newPos;
		fails = 0;
	}
	return regions >= minRegions;
}

//Generate a path that covers the maximum number of points.
bool Generate::generateLongestPath() {
	Point pos = adjustPoint(pickRandom(starts));
	Point exit = adjustPoint(pickRandom(exits));
	if (offEdge(pos) || offEdge(exit)) return false;
	Point block(-10, -10);
	if (hasConfig(FalseParity)) { //If false parity, one dot must be left uncovered
		if (getParity(pos + exit) == panel.getParity())
			return false;
		block = Point(rand(panel.width / 2 + 1) * 2, rand(panel.height / 2 + 1) * 2);
		while (pos == block || exit == block) {
			block = Point(rand(panel.width / 2 + 1) * 2, rand(panel.height / 2 + 1) * 2);
		}
		setPath(block);
	}
	else if (getParity(pos + exit) != panel.getParity())
		return false;
	int fails = 0;
	int reqLength = panel.getNumGridPoints() + static_cast<int>(path.size()) / 2;
	bool centerFlag = !onEdge(pos);
	setPath(pos);
	while (pos != exit && !(panel.symmetry && getSymPoint(pos) == exit)) {
		if (fails++ > 20)
			return false;
		Point dir = pickRandom(Panel::DIRECTIONS_2);
		for (Point checkDir : Panel::DIRECTIONS_2) {
			Point check = pos + checkDir;
			if (get(check) != 0)
				continue;
			if (check == exit) continue;
			int open = 0;
			for (Point checkDir2 : Panel::DIRECTIONS_2) {
				if (get(check + checkDir2) == 0) {
					if (++open >= 2) break;
				}
			}
			if (open < 2) {
				dir = checkDir;
				break;
			}
		}
		Point newPos = pos + dir;
		//Various checks to see if going this direction will lead to any issues 
		if (get(newPos) != 0 || get(pos + dir / 2) != 0
			|| newPos == exit && path.size() / 2 + 3 < reqLength ||
			panel.symmetry && getSymPoint(newPos) == exit && path.size() / 2 + 3 < reqLength) continue;
		if (panel.symmetry && (offEdge(getSymPoint(newPos)) || newPos == getSymPoint(newPos))) continue;
		if (onEdge(newPos) && !panel.isCylinder && panel.symmetry != Horizontal && newPos + dir != block && get(newPos + dir) != 0) {
			if (centerFlag && offEdge(newPos + dir)) {
				centerFlag = false;
			}
			else {
				int open = 0;
				for (Point checkDir : Panel::DIRECTIONS_2) {
					if (get(newPos + checkDir) == 0) {
						if (++open >= 2) break;
					}
				}
				if (open >= 2) continue;
			}
		}
		setPath(pos + dir / 2);
		setPath(newPos);
		pos = newPos;
		fails = 0;
	}
	if (!offEdge(block)) //Uncover the one dot for false parity
		set(block, 0);
	return path.size() / 2 + 1 == reqLength;
}

//Generate path that passes through all of the hitPoints in order
bool Generate::generateSpecialPath() {
	Point pos = adjustPoint(pickRandom(starts));
	Point exit = adjustPoint(pickRandom(exits));
	if (offEdge(pos) || offEdge(exit))
		return false;
	setPath(pos);
	for (Point p : hitPoints) {
		set(p, PATH);
	}
	int hitIndex = 0;
	int minLength = panel.getNumGridPoints() * 3 / 4;
	while (pos != exit) {
		std::vector<Point> validDir;
		for (Point dir : Panel::DIRECTIONS_2) {
			Point newPos = pos + dir;
			if (offEdge(newPos)) continue;
			Point connectPos = pos + dir / 2;
			//Go through the hit point if passing next to it
			if (get(connectPos) == PATH && hitIndex < hitPoints.size() && connectPos == hitPoints[hitIndex]) {
				validDir = { dir };
				hitIndex++;
				break;
			}
			if (get(newPos) != 0 || get(connectPos) != 0 || newPos == exit && (hitIndex != hitPoints.size() || path.size() / 2 + 2 < minLength))
				continue;
			if (panel.symmetry && newPos == getSymPoint(newPos)) continue;
			bool fail = false;
			for (Point dir : Panel::DIRECTIONS) {
				if (get(newPos + dir) == PATH && newPos + dir != hitPoints[hitIndex]) {
					fail = true;
					break;
				}
			}
			if (fail) continue;
			validDir.push_back(dir);
		}
		if (validDir.size() == 0)
			return false;
		Point dir = pickRandom(validDir);
		setPath(pos + dir / 2);
		setPath(pos + dir);
		pos = pos + dir;
	}
	return hitIndex == hitPoints.size() && path.size() >= minLength;
}

void Generate::erasePath()
{
	for (int y = 0; y < panel.height; y++) {
		for (int x = 0; x < panel.width; x++) {
			if (get(x, y) == PATH) {
				set(x, y, 0);
			}
		}
	}
}

//If a point is on an edge, bump it randomly to an adjacent vertex. Otherwise, the point is untouched
Point Generate::adjustPoint(Point pos) {
	if (pos.x % 2 != 0) {
		if (get(pos) != 0) return { -10, -10 };
		setPath(pos);
		return Point(pos.x - 1 + rand(2) * 2, pos.y);
	}
	if (pos.y % 2 != 0) {
		if (get(pos) != 0) return { -10, -10 };
		setPath(pos);
		return Point(pos.x, pos.y - 1 + rand(2) * 2);
	}
	if (panel.symmetry && exits.count(pos) && !exits.count(getSymPoint(pos)))
		return { -10, -10 };
	return pos;
}

//Place a start point in a random location
bool Generate::placeStart(int amount) {
	starts.clear();
	panel.startpoints.clear();
	while (amount > 0) {
		Point pos = Point(rand(panel.width / 2 + 1) * 2, rand(panel.height / 2 + 1) * 2);
		if (hasConfig(StartEdgeOnly))
			switch (rand(4)) {
				case 0: pos.x = 0; break;
				case 1: pos.y = 0; break;
				case 2: pos.x = panel.width - 1; break;
				case 3: pos.y = panel.height - 1; break;
			}
		if (parity != -1 && getParity(pos) != (amount == 1 ? parity : 1 - parity)) continue;
		if (starts.count(pos) || exits.count(pos)) continue;
		if (panel.symmetry && pos == getSymPoint(pos)) continue;
		//Highly discourage putting start points adjacent
		bool adjacent = false;
		for (Point dir : Panel::DIRECTIONS_2) {
			if (get(pos + dir) == Start) {
				adjacent = true;
				break;
			}
		}
		if (adjacent && rand(10) > 0) continue;
		starts.insert(pos);
		panel.setGridSymbol(pos.x, pos.y, Start, NoColor);
		amount--;
		if (panel.symmetry) {
			Point sp = getSymPoint(pos);
			starts.insert(sp);
			panel.setGridSymbol(sp.x, sp.y,Start, NoColor);
		}
	}
	return true;
}

//Place an exit point in a random location on the edge of the grid
//TODO: Reuse the code from placeStart?
bool Generate::placeExit(int amount) {
	exits.clear();
	panel.endpoints.clear();
	while (amount > 0) {
		Point pos = Point(rand(panel.width / 2 + 1) * 2, rand(panel.height / 2 + 1) * 2);
		switch (rand(4)) {
		case 0: pos.x = 0; break;
		case 1: pos.y = 0; break;
		case 2: pos.x = panel.width - 1; break;
		case 3: pos.y = panel.height - 1; break;
		}
		if (parity != -1 && (getParity(pos) + panel.getParity()) % 2 != (amount == 1 ? parity : 1 - parity)) continue;
		if (starts.count(pos) || exits.count(pos)) continue;
		if (panel.symmetry && pos == getSymPoint(pos)) continue;
		if (panel.symmetry && getSymPoint(pos).x != 0 && getSymPoint(pos).y != 0) continue;
		//Prevent putting exit points adjacent
		bool adjacent = false;
		for (const Point& p : exits) {
			for (Point dir : Panel::DIRECTIONS_2) {
				if (pos + dir == p) {
					adjacent = true;
					break;
				}
			}
		}
		if (adjacent) continue;
		exits.insert(pos);
		panel.setGridSymbol(pos.x, pos.y, Exit, NoColor);
		amount--;
		if (panel.symmetry) {
			Point sp = getSymPoint(pos);
			exits.insert(sp);
			panel.setGridSymbol(sp.x, sp.y, Exit, NoColor);
		}
	}
	return true;
}

//Check if a gap can be placed at pos.
bool Generate::canPlaceGap(Point pos) {
	//Prevent putting open gaps at edges of the puzzle
	if (onEdge(pos)) {
		if (hasConfig(FullGaps)) return false;
	}
	else if (rand(2) == 0) return false; //Encourages gaps on outside border
	//Prevent putting a gap on top of a start/end point
	if (starts.count(pos) || exits.count(pos))
		return false;
	//For symmetry puzzles, prevent putting gaps symmetrically opposite or in unreachable spots
	if (panel.symmetry && getSymPoint(pos) == pos || (get(getSymPoint(pos)) & Gap)) return false;
	if ((panel.symmetry == ParallelH || panel.symmetry == ParallelHFlip) && pos.y == panel.height / 2) return false;
	if ((panel.symmetry == ParallelV || panel.symmetry == ParallelVFlip) && pos.x == panel.width / 2) return false;
	if (panel.symmetry == FlipNegXY && (pos.x + pos.y == width - 1 || pos.x + pos.y == width + 1)) return false;
	if (panel.symmetry == FlipXY && (pos.x - pos.y == 1 || pos.x - pos.y == -1)) return false;
	if (hasConfig(FullGaps)) { //Prevent forming dead ends with open gaps
		std::vector<Point> checkPoints = (pos.x % 2 == 0 ? std::vector<Point>({ Point(pos.x, pos.y - 1), Point(pos.x, pos.y + 1) })
			: std::vector<Point>({ Point(pos.x - 1, pos.y), Point(pos.x + 1, pos.y) }));
		for (Point check : checkPoints) {
			int valid = 4;
			for (Point dir : Panel::DIRECTIONS) {
				Point p = check + dir;
				if (offEdge(p) || get(p) & GAP || get(p) == OPEN) {
					if (--valid <= 2) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

//Place the given amount of gaps randomly around the puzzle
bool Generate::placeGaps(int amount) {
	bool fullGaps = hasConfig(FullGaps);
	std::set<Point> open;
	for (int y = 0; y < panel.height; y++) {
		for (int x = (y + 1) % 2; x < panel.width; x += 2) {
			if (get(x, y) == 0) {
				open.emplace(Point(x, y));
			}
		}
	}
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		if (canPlaceGap(pos)) {
			set(pos, fullGaps ? OPEN : pos.x % 2 == 0 ? Gap_Column : Gap_Row);
			amount--;
		}
		open.erase(pos);
	}
	return true;
}

//Check if a dot can be placed at pos.
bool Generate::canPlaceDot(Point pos, bool intersectionOnly) {
	if (get(pos) & DOT)
		return false;
	if (panel.symmetry) {
		//For symmetry puzzles, make sure the current pos and symmetric pos are both valid
		Point symPos = getSymPoint(pos);
		if (symPos == pos) return false;
		Symmetry backupSym = panel.symmetry;
		panel.symmetry = NoSymmetry; //To prevent endless recursion
		if (!canPlaceDot(symPos, intersectionOnly)) {
			panel.symmetry = backupSym;
			return false;
		}
		panel.symmetry = backupSym;
	}
	if (panel.symmetry == RotateLeft && path1.count(pos) && path2.count(pos))
		return false; //Prevent sharing of dots between symmetry lines
	if (hasConfig(DisableDotIntersection)) return true;
	for (Point dir : Panel::DIRECTIONS8) {
		Point p = pos + dir;
		if (get(p) & DOT) {
			//Don't allow adjacent dots
			if (dir.x == 0 || dir.y == 0)
				return false;
			//Allow diagonally adjacent placement some of the time
			if (rand(2) > 0)
				return false;
		}
	}
	//Allow 2-space horizontal/vertical placement some of the time
	if (rand(intersectionOnly ? 10 : 5) > 0) {
		for (Point dir : Panel::DIRECTIONS_2) {
			Point p = pos + dir;
			if (get(p) & DOT) {
				return false;
			}
		}
	}
	return true;
}

//Place the given amount of dots at random points on the path
bool Generate::placeDots(int amount, int color, bool intersectionOnly) {
	if (parity != -1) { //For full dot puzzles, don't put dots on the starts and exits unless there are multiple
		for (int x = 0; x < panel.width; x += 2) {
			for (int y = 0; y < panel.height; y += 2) {
				if (starts.size() == 1 && starts.count(Point(x, y))) continue;
				if (exits.size() == 1 && exits.count(Point(x, y))) continue;
				if (get(x, y) == 0) continue;
				set(x, y, Dot_Intersection);
			}
		}
		amount -= panel.getNumGridPoints();
		if (amount <= 0) return true;
		intersectionOnly = false;
		setConfigOnce(DisableDotIntersection);
	}
	if (color == Blue || color == Cyan)
		color = IntersectionFlags::DOT_IS_BLUE;
	else if (color == Yellow || color == Orange)
		color = IntersectionFlags::DOT_IS_ORANGE;
	else color = 0;

	std::set<Point> open = (color == 0 ? path : color == IntersectionFlags::DOT_IS_BLUE ? path1 : path2);
	for (Point p : starts) open.erase(p);
	for (Point p : exits) open.erase(p);
	for (Point p : blockPos) open.erase(p);
	if (intersectionOnly) {
		std::set<Point> intersections;
		for (Point p : open) {
			if (p.x % 2 == 0 && p.y % 2 == 0)
				intersections.insert(p);
		}
		open = intersections;
	}
	if (hasConfig(DisableDotIntersection)) {
		std::set<Point> intersections;
		for (Point p : open) {
			if (p.x % 2 != 0 || p.y % 2 != 0)
				intersections.insert(p);
		}
		open = intersections;
	}
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		open.erase(pos);
		if (!canPlaceDot(pos, intersectionOnly)) continue;
		int symbol = (pos.x & 1) == 1 ? Dot_Row : (pos.y & 1) == 1 ? Dot_Column : Dot_Intersection;
		set(pos, symbol | color);
		for (Point dir : Panel::DIRECTIONS) {
			open.erase(pos + dir);
		} //If symmetry, set a flag to break the point symmetric to the dot
		if (panel.symmetry) {
			Point sp = getSymPoint(pos);
			symbol = (sp.x & 1) == 1 ? Dot_Row : (sp.y & 1) == 1 ? Dot_Column : Dot_Intersection;
			if (symbol != Dot_Intersection) set(sp, symbol & ~Dot);
			open.erase(sp);
			for (Point dir : Panel::DIRECTIONS) {
				open.erase(sp + dir);
			}
		}
		amount--;
	}
	return true;
}

//Check if a stone can be placed at pos.
bool Generate::canPlaceStone(const std::set<Point>& region, int color) {
	for (Point p : region) {
		int sym = get(p);
		if (getType(sym) == Stone) return getColor(sym) == color;
	}
	return true;
}

//Place the given amount of stones with the given color
bool Generate::placeStones(int color, int amount) {
	std::set<Point> open = openpos;
	std::set<Point> open2; //Used to store open points removed from the first pass, to make sure a stone is put in every non-adjacent region
	int passCount = 0;
	int originalAmount = amount;
	while (amount > 0) {
		if (open.size() == 0) {
			//Make sure there is room for the remaining stones and enough partitions have been made (based on the grid size)
			if (open2.size() < amount || bisect && passCount < min(originalAmount, (panel.width / 2 + panel.height / 2 + 2) / 4))
				return false;
			//Put remaining stones wherever they will fit
			Point pos = pickRandom(open2);
			set(pos, Stone | color);
			openpos.erase(pos);
			open2.erase(pos);
			amount--;
			continue;
		}
		Point pos = pickRandom(open);
		std::set<Point> region = panel.getRegion(pos);
		if (!canPlaceStone(region, color)) {
			for (Point p : region) {
				open.erase(p);
			}
			continue;
		}
		if (stoneTypes > 2) { //If more than two colors, group stones together, otherwise it takes too long to generate.
			open.clear();
			for (Point p : region) {
				if (openpos.count(p))
					open.insert(p);
			}
		}
		open.erase(pos);
		if (panel.symmetry) {
			open.erase(getSymPoint(pos));
		}
		if (stoneTypes == 2) {
			for (Point p : region) {
				if (open.erase(p)) open2.insert(p);
			} //Remove adjacent regions from the open list
			for (Point p : region) {
				for (Point dir : Panel::DIRECTIONS8_2) {
					Point pos2 = p + dir;
					if (open.count(pos2) && !region.count(pos2)) {
						for (Point P : panel.getRegion(pos2)) {
							open.erase(P);
						}
					}
				}
			}
		}
		set(pos, Stone | color);
		openpos.erase(pos);
		amount--;
		passCount++;
	}
	bisect = false; //After placing one color, adjacent regions are allowed
	stoneTypes--;
	return true;
}

//Generate a random shape. region - the region of points to choose from; points chosen will be removed.
//bufferRegion - points that may be chosen twice due to overlapping shapes; points will be removed from here before points in region.
//maxSize - the maximum size of the generated shape. Whether the points can be contiguous or not is determined by local variable SHAPEDIRECTIONS
Shape Generate::generateShape(std::set<Point>& region, std::set<Point>& bufferRegion, Point pos, int maxSize) {
	Shape shape;
	shape.insert(pos);
	if (!bufferRegion.erase(pos))
		region.erase(pos);
	while (shape.size() < maxSize && region.size() > 0) {
		pos = pickRandom(shape);
		int i = 0;
		for (; i < 10; i++) {
			Point dir = pickRandom(SHAPEDIRECTIONS);
			Point p = pos + dir;
			if (region.count(p) && !shape.count(p)) {
				shape.insert(p);
				if (!bufferRegion.erase(p))
					region.erase(p);
				break;
			}
		}
		if (i == 10)
			return shape;
	}
	return shape;
}

//Get the integer representing the shape, accounting for whether it is rotated or negative. -1 rotation means a random rotation, depth is for controlling recursion and should be set to 0
int Generate::makeShapeSymbol(Shape shape, bool rotated, bool negative, int rotation, int depth) {
	int symbol = static_cast<int>(Poly);
	if (rotated) {
		if (rotation == -1) {
			if (makeShapeSymbol(shape, rotated, negative, 0, depth + 1) == makeShapeSymbol(shape, rotated, negative, 1, depth + 1))
				return 0; //Check to make sure the shape is not the same when rotated
			rotation = rand(4);
		}
		symbol |= Rotate;
		Shape newShape; //Rotate shape points according to rotation
		for (Point p : shape) {
			switch (rotation) {
				case 0: newShape.insert(p); break;
				case 1: newShape.emplace(Point(p.y, -p.x)); break;
				case 2: newShape.emplace(Point(-p.y, p.x)); break;
				case 3: newShape.emplace(Point(-p.x, -p.y)); break;
			}
		}
		shape = newShape;
	}
	if (negative) symbol |= Negative;
	int xmin = INT_MAX, xmax = INT_MIN, ymin = INT_MAX, ymax = INT_MIN;
	for (Point p : shape) {
		if (p.x < xmin) xmin = p.x;
		if (p.x > xmax) xmax = p.x;
		if (p.y < ymin) ymin = p.y;
		if (p.y > ymax) ymax = p.y;
	}
	if (xmax - xmin > 6 || ymax - ymin > 6) { //Shapes cannot be more than 4 in width and height
		if (!panel.isCylinder) return 0; 
		if (ymax - ymin > 6 || depth > panel.width / 2) return 0;
		Shape newShape;
		for (Point p : shape) newShape.insert({ (p.x - xmax + panel.width) % panel.width, p.y });
		return makeShapeSymbol(newShape, rotated, negative, rotation, depth + 1);
	}
	//Translate to the corner and set bit flags (16 bits, 1 where a shape block is present)
	for (Point p : shape) {
		symbol |= (1 << ((p.x - xmin) / 2 + (ymax  - p.y) * 2)) << 16;
	}
	if (rand(4) > 0) { //The generator makes a certain type of symbol way too often (2x2 square with another square attached), this makes it much less frequent
		int type = symbol >> 16;
		if (type == 0x0331 || type == 0x0332 || type == 0x0037 || type == 0x0067 || type == 0x0133 || type == 0x0233 || type == 0x0073 || type == 0x0076)
			return 0;
	}
	return symbol;
}

std::vector<Point> Generate::DISCONNECT = { Point(0, 2), Point(0, -2), Point(2, 0), Point(-2, 0), Point(2, 2), Point(2, -2), Point(-2, -2), Point(-2, 2),
	Point(0, 2), Point(0, -2), Point(2, 0), Point(-2, 0), Point(2, 2), Point(2, -2), Point(-2, -2), Point(-2, 2),
	Point(0, 4), Point(0, -4), Point(4, 0), Point(-4, 0), //Used to make the discontiguous shapes
};
std::vector<Point> Generate::SHAPEDIRECTIONS = { }; //This will eventually be set to one of the above lists

//Place the given amount of shapes with random colors selected from the color vectors.
//colors - colors for regular shapes, negativeColors - colors for negative shapes, amount - how many normal shapes
//numRotated - how many rotated shapes, numNegative - how many negative shapes
bool Generate::placeShapes(const std::vector<int>& colors, const std::vector<int>& negativeColors, int amount, int numRotated, int numNegative) {
	std::set<Point> open = openpos;
	int shapeSize = hasConfig(SmallShapes) ? 2 : hasConfig(BigShapes) ? amount == 1 ? 8 : 6 : 4;
	int targetArea = amount * shapeSize * 7 / 8; //Average size must be at least 7/8 of the target size
	if (amount * shapeSize > panel.getNumGridBlocks()) targetArea = panel.getNumGridBlocks();
	int originalAmount = amount;
	if (hasConfig(MountainFloorH) && panel.width == 9) { //The 4 small puzzles shape size may vary depending on the path
		targetArea = 0;
		removeConfig(MountainFloorH);
	}
	int totalArea = 0;
	int minx = panel.width, miny = panel.height, maxx = 0, maxy = 0;
	int colorIndex = rand(colors.size());
	int colorIndexN = rand(negativeColors.size() + 1);
	bool shapesCanceled = false, shapesCombined = false, flatShapes = true;
	if (amount == 1) shapesCombined = true;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		std::set<Point> region = panel.getRegion(pos);
		std::set<Point> bufferRegion;
		std::set<Point> open2; //Open points for just that region
		for (Point p : region) {
			if (open.erase(p)) open2.insert(p);
		}
		if (region.size() + totalArea == panel.getNumGridBlocks() &&
			targetArea != panel.getNumGridBlocks()) continue; //To prevent shapes from filling every grid point
		std::vector<Shape> shapes;
		std::vector<Shape> shapesN;
		int numShapesN = min(rand(numNegative + 1), static_cast<int>(region.size()) / 3); //Negative blocks may be at max 1/3 of the regular blocks
		if (amount == 1) numShapesN = numNegative;
		if (numShapesN) {
			std::set<Point> regionN = gridpos;
			int maxSize = static_cast<int>(region.size()) - numShapesN * 3; //Max size of negative shapes
			if (maxSize == 0) maxSize = 1;
			for (int i = 0; i < numShapesN; i++) {
				pos = pickRandom(region);
				//Try to pick a random point adjacent to a shape
				for (int i = 0; i < 10; i++) {
					Point p = pos + pickRandom(SHAPEDIRECTIONS);
					if (regionN.count(p) && !region.count(p)) {
						pos = p;
						break;
					}
				}
				if (!regionN.count(pos)) return false;
				Shape shape = generateShape(regionN, pos, min(rand(1, 3), maxSize));
				shapesN.push_back(shape);
				for (Point p : shape) {
					if (region.count(p)) bufferRegion.insert(p); //Buffer region stores overlap between shapes
					else region.insert(p);
				}
			}
		}
		int numShapes = static_cast<int>(region.size() + bufferRegion.size()) / (shapeSize + 1) + 1; //Pick a number of shapes to make. I tried different ones until I found something that made a good variety of shapes
		if (numShapes == 1 && bufferRegion.size() > 0) numShapes++; //If there is any overlap, we need at least two shapes
		if (numShapes < amount && region.size() > shapeSize && rand(2) == 1) numShapes++; //Adds more variation to the shape sizes
		if (region.size() <= shapeSize + 1 && bufferRegion.size() == 0 && rand(2) == 1) numShapes = 1; //For more variation, sometimes make a bigger shape than the target if the size is close
		if (hasConfig(MountainFloorH)) {
			if (region.size() < 19) continue;
			numShapes = 6; //The big mountain floor puzzle on hard mode needs additional shapes since some combine
			targetArea = 19;
		}
		if (hasConfig(SplitShapes) && numShapes != 1) continue;
		if (hasConfig(RequireCombineShapes) && numShapes == 1) continue;
		bool cancel = false;
		//Check if the region is too big for the number of shapes chosen
		//TODO: Make OnlyCancelShapes put canceling shapes only in large regions?
		if (numShapes > amount || hasConfig(OnlyCancelShapes)) {
			if (numNegative < 2 || hasConfig(DisableCancelShapes)) continue;
			//Make canceling shapes. Positive and negative will be switched so that code can be reused
			cancel = true;
			std::set<Point> regionN = gridpos;
			numShapes = max(2, rand(1, numNegative));	//Actually the negative shapes
			numShapesN = min(amount, 1);				//Actually the positive shapes
			if (numShapesN >= numShapes * 3 || numShapesN * 5 <= numShapes) continue;
			shapes.clear();
			shapesN.clear();
			region.clear();
			bufferRegion.clear();
			for (int i = 0; i < numShapesN; i++) {
				Shape shape = generateShape(regionN, pickRandom(regionN), min(shapeSize + 1, numShapes * 2 / numShapesN + rand(-1, 1)));
				shapesN.push_back(shape);
				for (Point p : shape) {
					region.insert(p);
				}
			}
			shapesCanceled = true;
			//Let the rest of the algorithm create the cancelling shapes
		}
		if (panel.symmetry && numShapes == originalAmount && numShapes >= 3 && !panel.isCylinder && !region.count(Point((panel.width / 4) * 2 + 1, (panel.height / 4) * 2 + 1)))
			continue; //Prevent it from shoving all shapes to one side of symmetry
		if ((panel.symmetry == ParallelH || panel.symmetry == ParallelV ||
			panel.symmetry == ParallelHFlip || panel.symmetry == ParallelVFlip)
			&& region.count(Point((panel.width / 4) * 2 + 1, (panel.height / 4) * 2 + 1)))
			continue; //Prevent parallel symmetry from making regions through the center line (this tends to make the puzzles way too hard)
		if (!cancel && numShapesN && (numShapesN > 1 && numRotated > 0 || numShapesN > 2 || numShapes + numShapesN > 6))
			continue; //Trying to prevent the game's shape calculator from lagging too much
		if (!(hasConfig(MountainFloorH) && panel.width == 11) && open2.size() < numShapes + numShapesN) continue; //Not enough space to put the symbols
		if (numShapes == 1) {
			shapes.push_back(region);
			region.clear();
		}
		else for (; numShapes > 0; numShapes--) {
			if (region.size() == 0) break;
			Shape shape = generateShape(region, bufferRegion, pickRandom(region), cancel ? rand(1, 3) : shapeSize);
			if (!cancel && numShapesN) for (Shape s : shapesN) if (std::equal(shape.begin(), shape.end(), s.begin(), s.end())) return false; //Prevent unintentional in-group canceling
			shapes.push_back(shape);
		}
		//Take remaining area and try to stick it to existing shapes
		multibreak:
		while (region.size() > 0) {
			pos = pickRandom(region);
			for (Shape& shape : shapes) {
				if (shape.size() > shapeSize || shape.count(pos) > 0) continue;
				for (Point p : shape) {
					for (Point dir : Panel::DIRECTIONS_2) {
						if (pos + dir == p) {
							shape.insert(pos);
							if (!bufferRegion.erase(pos))
								region.erase(pos);
							goto multibreak;
						}
					}
				}
			}
			//Failed to cover entire region, need to pick a different region
			break;
		}
		if (region.size() > 0) continue;
		if (cancel) { //Undo swap for canceling shapes
			std::swap(shapes, shapesN);
		}
		numShapes = static_cast<int>(shapes.size());
		for (Shape& shape : shapesN) {
			shapes.push_back(shape);
		}
		if (hasConfig(DisconnectShapes)) {
			//Make sure at least one shape is disconnected
			bool disconnect = false;
			for (Shape& shape : shapes) {
				if (shape.size() == 1) continue;
				disconnect = true;
				for (Point p : shape) {
					for (Point dir : Panel::DIRECTIONS_2) {
						if (shape.count(p + dir)) {
							disconnect = false;
							break;
						}
					}
					if (!disconnect) break;
				}
				if (disconnect) break;
			}
			if (!disconnect) continue;
		}
		if (numShapes > 1) shapesCombined = true;
		numNegative -= static_cast<int>(shapesN.size());
		if (hasConfig(MountainFloorH) && amount == 6) { //For mountain floor, combine some of the shapes together
			if (!combineShapes(shapes) || !combineShapes(shapes)) //Must call this twice b/c there are two combined areas
				return false;
			amount -= 2;
		}
		for (Shape& shape : shapes) {
			int symbol = makeShapeSymbol(shape, (numRotated-- > 0), (numShapes-- <= 0));
			if (symbol == 0)
				return false;
			if (!((symbol >> 16) == 0x000F || (symbol >> 16) == 0x1111))
				flatShapes = false;
			//Attempt not to put shape symbols adjacent
			Point pos;
			for (int i = 0; i < 10; i++) {
				if (open2.size() == 0) return false;
				pos = pickRandom(open2);
				bool pass = true;
				for (Point dir : Panel::DIRECTIONS8_2) {
					Point p = pos + dir;
					if (get(p) & Poly) {
						pass = false;
						break;
					}
				}
				if (pass) break;
			}
			if (symbol & Negative) set(pos, symbol | negativeColors[(colorIndexN++) % negativeColors.size()]);
			else {
				set(pos, symbol | colors[(colorIndex++) % colors.size()]);
				totalArea += static_cast<int>(shape.size());
				amount--;
			}
			open2.erase(pos);
			openpos.erase(pos);
			if (panel.symmetry && !panel.isCylinder && originalAmount >= 3) {
				for (const Point& p : shape) {
					if (p.x < minx) minx = p.x;
					if (p.y < miny) miny = p.y;
					if (p.x > maxx) maxx = p.x;
					if (p.y > maxy) maxy = p.y;
				}
			}
		}
	} //Do some final checks - make sure targetArea has been reached, all shapes have been placed, and that config requirements have been met
	if (totalArea < targetArea || numNegative > 0 ||
		hasConfig(RequireCancelShapes) && !shapesCanceled ||
		hasConfig(RequireCombineShapes) && !shapesCombined ||
		originalAmount > 1 && flatShapes)
		return false;
	//If symmetry, make sure it didn't shove all the shapes to one side
	if (panel.symmetry && !panel.isCylinder && originalAmount >= 3 &&
		(minx >= panel.width / 2 || maxx <= panel.width / 2 || miny >= panel.height / 2 || maxy <= panel.height / 2))
		return false;
	return true;
}

//Place the given amount of stars with the given color
bool Generate::placeStars(int color, int amount) {
	std::set<Point> open = openpos;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		std::set<Point> region = panel.getRegion(pos);
		std::set<Point> open2; //All of the open points in that region
		for (Point p : region) {
			if (open.erase(p)) open2.insert(p);
		}
		int count = panel.countColor(region, color);
		if (count >= 2) continue; //Too many of that color
		if (open2.size() + count < 2) continue; //Not enough space to get 2 of that color
		if (count == 0 && amount == 1) continue; //If one star is left, it needs a pair
		set(pos, Star | color);
		openpos.erase(pos);
		amount--;
		if (count == 0) { //Add a second star of the same color
			open2.erase(pos);
			if (open2.size() == 0)
				return false;
			pos = pickRandom(open2);
			set(pos, Star | color);
			openpos.erase(pos);
			amount--;
		}
	}
	return true;
}

//Check if there is a star in the given region
bool Generate::hasStar(const std::set<Point>& region, int color) {
	for (Point p : region) {
		if (get(p) == (Star | color)) return true;
	}
	return false;
}

//Check that it didn't create an obvious solution where you just zigzag across the rows
bool Generate::checkStarZigzag(Panel& panel) {
	if (panel.width <= 5 || panel.height <= 5) return true;
	for (int y = 1; y < panel.height; y += 2) {
		std::map<int, int> colorCount;
		for (int x = 1; x < panel.width; x += 2) {
			int color = get(x, y);
			if (color == 0) continue;
			if (!colorCount.count(color)) colorCount[color] = 0;
			colorCount[color] += 1;
		}
		for (std::pair<int, int> count : colorCount)
			if (count.second % 2 != 0)
				return true;
	}
	return false;
}

//Place the given amount of triangles with the given color. targetCount is how many triangles are in the symbol, or 0 for random
bool Generate::placeTriangles(int color, int amount, int targetCount)
{
	//TODO: Replace with setSymbol.
	if (panel.id == KEEP_PRESSURE_1 && randomizer->difficulty == Expert) {
		int count = panel.countSides({ 1, 3 });
		set({ 1, 3 }, Triangle | color | (count << 16));
		openpos.erase({ 1, 3 });
	}
	std::set<Point> open = openpos;
	//If the block is adjacent to a start or exit, don't allow triangles to be placed there
	for (Point dir : Panel::DIRECTIONS) {
		for (Point start : starts) {
			if (open.count(start + dir))
				open.erase(start + dir);
		}
		for (Point exit : exits) {
			if (open.count(exit + dir))
				open.erase(exit + dir);
		}
	}
	int count1 = 0, count2 = 0, count3 = 0;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		int count = panel.countSides(pos);
		open.erase(pos);
		if (panel.symmetry) {
			open.erase(getSymPoint(pos));
		}
		if (count == 0 || targetCount && count != targetCount) continue;
		//Attempt to balance the amount of 1, 2, and 3 triangles
		if (count == 1) {
			if (!targetCount && count1 * 2 > count2 + count3 && rand(2) == 0) continue;
			count1++;
		}
		if (count == 2) {
			if (!targetCount && count2 * 2 > count1 + count3 && rand(2) == 0) continue;
			count2++;
		}
		if (count == 3) {
			if (!targetCount && count3 * 2 > count1 + count2 && rand(2) == 0) continue;
			count3++;
		}
		set(pos, Triangle | color | (count << 16));
		openpos.erase(pos);
		amount--;
	}
	return true;
}

//Place the given amount of erasers with the given colors. eraseSymbols are the symbols that were erased
bool Generate::placeErasers(const std::vector<int>& colors, const std::vector<int>& eraseSymbols) {
	std::set<Point> open = openpos;
	//TODO: Replace with setSymbol.
	if (panel.id == CAVES_PERSPECTIVE_2 && randomizer->difficulty == Expert) open.erase({ 5, 5 });
	int amount = static_cast<int>(colors.size());
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		int toErase = eraseSymbols[amount - 1];
		int color = colors[amount - 1];
		Point pos = pickRandom(open);
		std::set<Point> region = panel.getRegion(pos);
		std::set<Point> open2;
		for (Point p : region) {
			if (open.erase(p)) open2.insert(p);
		}
		if (splitPoints.size() > 0) { //Make sure this is one of the split point regions
			bool found = false;
			for (Point p : splitPoints) {
				if (region.count(p)) {
					found = true;
					break;
				}
			}
			if (!found) continue;
		}
		//TODO: Don't hardcode this
		if (panel.id == CAVES_PERSPECTIVE_2 && randomizer->difficulty == Expert && !region.count({ 5, 5 })) continue; //For the puzzle in the cave with a pillar in middle
		if (hasConfig(MakeStonesUnsolvable)) {
			std::set<Point> valid;
			for (Point p : open2) {
				//Try to make a checkerboard pattern with the stones
				if (get(p + Point(2, 2)) == toErase && get(p + Point(0, 2)) != 0 && get(p + Point(0, 2)) != toErase && get(p + Point(2, 0)) != 0 && get(p + Point(2, 0)) != toErase ||
					get(p + Point(-2, 2)) == toErase && get(p + Point(0, 2)) != 0 && get(p + Point(0, 2)) != toErase && get(p + Point(-2, 0)) != 0 && get(p + Point(-2, 0)) != toErase ||
					get(p + Point(2, -2)) == toErase && get(p + Point(0, -2)) != 0 && get(p + Point(0, -2)) != toErase && get(p + Point(2, 0)) != 0 && get(p + Point(2, 0)) != toErase ||
					get(p + Point(-2, -2)) == toErase && get(p + Point(0, -2)) != 0 && get(p + Point(0, -2)) != toErase && get(p + Point(-2, 0)) != 0 && get(p + Point(-2, 0)) != toErase)
					valid.insert(p);
			}
			open2 = valid;
		}
		if ((open2.size() == 0 || splitPoints.size() == 0 && open2.size() == 1) && !(toErase & Dot)) continue;
		bool canPlace = false;
		if (getType(toErase) == Stone) {
			canPlace = !canPlaceStone(region, toErase & 0xf);
		}
		else if (getType(toErase) == Star) {
			canPlace = (panel.countColor(region, toErase & 0xf) + (color == (toErase & 0xf) ? 1 : 0) != 1);
		}
		else canPlace = true;
		if (!canPlace) continue;

		if (getType(toErase) == Stone || getType(toErase) == Star) {
			set(pos, toErase);
		}
		else if (toErase & Dot) { //Find an open edge to put the dot on
			std::set<Point> openEdge;
			for (Point p : region) {
				for (Point dir : Panel::DIRECTIONS8) {
					if (toErase == Dot_Intersection && (dir.x == 0 || dir.y == 0)) continue;
					Point p2 = p + dir;
					if (get(p2) == 0 && (hasConfig(FalseParity) || canPlaceDot(p2, false))) {
						openEdge.insert(p2);
					}
				}
			}
			if (openEdge.size() == 0)
				continue;
			pos = pickRandom(openEdge);
			toErase &= ~INTERSECTION;
			if ((toErase & 0xf) == Blue || (toErase & 0xf) == Cyan) toErase |= DOT_IS_BLUE;
			if ((toErase & 0xf) == Yellow || (toErase & 0xf) == Orange) toErase |= DOT_IS_ORANGE;
			toErase &= ~0x4000f; //Take away extra flags from the symbol
			if ((pos.x & 1) == 0 && (pos.y & 1) == 0) toErase |= Dot_Intersection;
			else if ((pos.y & 1) == 0) toErase |= Dot_Row;
			set(pos, ((pos.x & 1) == 1 ? Dot_Row : (pos.y & 1) == 1 ? Dot_Column : Dot_Intersection) | (toErase & 0xffff));
		}
		else if (getType(toErase) == Poly) {
			int symbol = 0; //Make a random shape to cancel
			while (symbol == 0) {
				std::set<Point> area = gridpos;
				int shapeSize;
				if ((toErase & Negative) || hasConfig(SmallShapes)) shapeSize = rand(1, 3);
				else {
					shapeSize = rand(1, 5);
					if (shapeSize < 3)
						shapeSize += rand(3);
				}
				Shape shape = generateShape(area, pickRandom(area), shapeSize);
				if (shape.size() == region.size()) continue; //Don't allow the shape to match the region, to guarantee it will be wrong
				symbol = makeShapeSymbol(shape, toErase & Rotate, toErase & Negative);
			}
			set(pos, symbol | (toErase & 0xf));
		}
		else if (getType(toErase) == Triangle) {
			//If the block is adjacent to a start or exit, don't place a triangle there
			//TODO: Don't hardcode this
			if (hasConfig(TreehouseLayout) || panel.id == CAVES_PERSPECTIVE_3) {
				bool found = false;
				for (Point dir : Panel::DIRECTIONS) {
					if (starts.count(pos + dir) || exits.count(pos + dir)) {
						found = true;
						break;
					}
				}
				if (found) continue;
			}
			int count = panel.countSides(pos);
			if (count == 0) count = rand(1, 3);
			else count = (count + rand(2)) % 3 + 1;
			set(pos, toErase | (count << 16));
		}

		if (!(toErase & Dot)) {
			openpos.erase(pos);
			open2.erase(pos);
		}
		//Place the eraser at a random open point
		if (splitPoints.size() == 0) pos = pickRandom(open2);
		else for (Point p : splitPoints) if (region.count(p)) { pos = p; break; }
		//TODO: Don't hardcode this
		if (panel.id == CAVES_PERSPECTIVE_2 && randomizer->difficulty == Expert) {
			if (get(5, 5) != 0) return false;
			pos = { 5, 5 }; //For the puzzle in the cave with a pillar in middle
		}
		set(pos, Eraser | color);
		openpos.erase(pos);
		amount--;
	}
	return true;
}

//WIP
int Generate::makeCanceledSymbol(Point pos, int toErase, const std::set<Point>& region) {
	if (getType(toErase) == Poly) {
		int symbol = 0; //Make a random shape to cancel
		while (symbol == 0) {
			std::set<Point> area = gridpos;
			int shapeSize;
			if ((toErase & Negative) || hasConfig(SmallShapes)) shapeSize = rand(1, 3);
			else {
				shapeSize = rand(1, 5);
				if (shapeSize < 3)
					shapeSize += rand(3);
			}
			Shape shape = generateShape(area, pickRandom(area), shapeSize);
			symbol = makeShapeSymbol(shape, toErase & Rotate, toErase & Negative);
		}
		return symbol | getColor(toErase);
	}
	if (getType(toErase) == Triangle && (toErase & 0xF0000) == 0) {
		//If the block is adjacent to a start or exit, don't place a triangle there
		//TODO: Don't hardcode this
		if (hasConfig(TreehouseLayout) || panel.id == CAVES_PERSPECTIVE_3) {
			bool found = false;
			for (Point dir : Panel::DIRECTIONS) {
				if (starts.count(pos + dir) || exits.count(pos + dir)) {
					found = true;
					break;
				}
			}
			if (found) return 0;
		}
		int count = rand(3) + 1;
		return toErase | (count << 16);
	}

	return toErase;
}

//For the mountain floor puzzle on hard mode. Combine two tetris shapes into one
bool Generate::combineShapes(std::vector<Shape>& shapes) {
	for (int i = 0; i < shapes.size(); i++) {
		for (int j = 0; j < shapes.size(); j++) {
			if (i == j) continue;
			if (shapes[i].size() + shapes[j].size() <= 5) continue;
			if (shapes[i].size() > 5 || shapes[j].size() > 5) continue;
			//Look for adjacent points
			for (Point p1 : shapes[i]) {
				for (Point p2 : shapes[j]) {
					for (Point dir : Panel::DIRECTIONS_2) {
						if (p1 + dir == p2) {
							//Combine shapes
							for (Point p : shapes[i]) shapes[j].insert(p);
							//Make sure there are no holes
							std::set<Point> area = gridpos;
							for (Point p : shapes[j]) area.erase(p);
							while (area.size() > 0) {
								std::set<Point> region;
								std::vector<Point> check;
								check.push_back(*area.begin());
								region.insert(*area.begin());
								while (check.size() > 0) {
									Point p = check[check.size() - 1];
									check.pop_back();
									for (Point dir : Panel::DIRECTIONS) {
										Point p2 = p + dir * 2;
										if (area.count(p2) && region.insert(p2).second) {
											check.push_back(p2);
										}
									}
								}
								bool connected = false;
								for (Point p : region) {
									if (p.x == 1 || p.y == 1 || p.x == panel.width - 2 || p.y == panel.height - 2) {
										connected = true;
										break;
									}
								}
								if (!connected) return false;
								for (Point p : region) area.erase(p);
							}
							shapes.erase(shapes.begin() + i);
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

//Place the given amount of arrows with the given color. targetCount is how many ticks on the arrows, or 0 for random
bool Generate::placeArrows(int color, int amount, int targetCount) {
	std::set<Point> open = openpos;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		open.erase(pos);
		int fails = 0;
		while (fails++ < 20) { //Keep picking random directions until one works
			int choice = (parity == -1 ? rand(8) : rand(4));
			Point dir = Panel::DIRECTIONS8_2[choice];
			if (panel.isCylinder && dir.y == 0) continue; //Sideways arrows on a pillar would wrap forever
			int count = panel.countCrossings(pos, dir);
			if (count == 0 || count > 3 || targetCount && count != targetCount) continue;
			if (dir.x < 0 && count == (pos.x + 1) / 2 || dir.x > 0 && count == (panel.width - pos.x) / 2 ||
				dir.y < 0 && count == (pos.y + 1) / 2 || dir.y > 0 && count == (panel.height - pos.y) / 2 && rand(10) > 0)
				continue; //Make it so that there will be some possible edges that aren't passed, in the vast majority of cases
			set(pos, SymbolData::GetValFromSymbolID(ARROW1E + choice + (count - 1) * 8) | color);
			openpos.erase(pos);
			amount--;
			break;
		}
	}
	return true;
}

bool Generate::placeAntiTriangles(int color, int amount, int targetCount) {
	std::set<Point> open = openpos;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		open.erase(pos);
		int count = panel.countTurns(pos);
		if (count == 0 || count > 4 || targetCount && count != targetCount) continue;
		set(pos, SymbolData::GetValFromSymbolID(ANTITRIANGLE1 + count - 1) | color);
		openpos.erase(pos);
		amount--;
	}
	return true;
}

bool Generate::placeCaveClues(int color, int amount, int targetCount) {
	std::set<Point> open = openpos;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		open.erase(pos);
		int count = 1;
		for (Point dir : Panel::DIRECTIONS) {
			Point temp = pos;
			while (get(temp + dir + dir) != OFF_GRID && get(temp + dir) != PATH && (get(temp + dir + dir)&Empty) != Empty) {
				count++;
				temp = temp + dir + dir;
			}
		}
		if (count > 9 || (targetCount && count != targetCount && targetCount > 0)) continue; 
		set(pos, SymbolData::GetValFromSymbolID(CAVE1 + count - 1) | color);
		openpos.erase(pos);
		amount--;
	}
	return true;
}

bool Generate::placeMinesweeperClues(int color, int amount, int targetCount) {
	std::set<Point> open = openpos;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		open.erase(pos);
		int count = 0;
		std::set<Point> region = panel.getRegion(pos);
		for (Point dir : Panel::DIRECTIONS8_2) {
			if (get(pos + dir) != OFF_GRID && !(region.count(pos + dir)))
				count++;
		}
		if (targetCount && count != targetCount && targetCount != 9) continue;
		set(pos, SymbolData::GetValFromSymbolID(MINESWEEPER0 + count) | color);
		openpos.erase(pos);
		amount--;
	}
	return true;
}

bool Generate::placeFlowers(int color, int amount) {
	std::set<Point> open = openpos;
	for (Point p : gridpos) {
		if (get(p) && (get(p) & 0xf) == color) {
			for (Point q : getRegion(p)) {
				if (!(q.x == p.x || q.y == p.y)) {
					open.erase(q);
				}
			}
		}
	}
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos = pickRandom(open);
		std::set<Point> region = panel.getRegion(pos);
		std::set<Point> col, row;
		for (Point p : region) {
			if (p.x == pos.x)
				row.insert(p);
			if (p.y == pos.y)
				col.insert(p);
		}
		int colCount = panel.countColor(col, color);
		int rowCount = panel.countColor(row, color);
		if ((colCount > 0) && (rowCount > 0)) {
			open.erase(pos); // Impossible to place a flower in this location
			continue; 
		}
		if ((colCount > 0) != (rowCount > 0)) {
			set(pos, SymbolData::GetValFromSymbolID(FLOWER) | color);
			for (Point q : getRegion(pos)) {
				if (!(q.x == pos.x || q.y == pos.y)) {
					open.erase(q);
				}
			}
			amount--;
			if (colCount > 0) {
				for (Point p : row) {
					open.erase(p);
				}
			}
			else {
				for (Point p : col) {
					open.erase(p);
				}
			}
			openpos.erase(pos);
		}
		if (colCount == 0 && rowCount == 0) {
			if (amount <= 1) {
				open.erase(pos);
				continue;
			}
			set(pos, SymbolData::GetValFromSymbolID(FLOWER) | color);
			amount--;
			open.erase(pos);
			openpos.erase(pos);
			std::set<Point> crossSub = row;
			crossSub.insert(col.begin(), col.end());
			std::set<Point> openSub;
			for (Point p : crossSub) {
				if (open.erase(p)) openSub.insert(p);
			}
			while (openSub.size() > 0) {
				Point pos2 = pickRandom(openSub);
				openSub.erase(pos2);
				if (col.find(pos2) != col.end()) {
					std::set<Point> subRow;
					for (Point p : region) {
						if (p.x == pos2.x)
							subRow.insert(p);
					}
					if (panel.countColor(subRow, color) > 0) {
						open.erase(pos2);
						openSub.erase(pos2);
					}
					else {
						set(pos2, SymbolData::GetValFromSymbolID(FLOWER) | color);
						for (Point q : getRegion(pos2)) {
							if (!(q.x == pos2.x || q.y == pos2.y)) {
								open.erase(q);
							}
						}
						amount--;
						for (Point p : subRow) {
							open.erase(p);
						}
						openpos.erase(pos2);
						if (amount != 1)
							break;
					}
				}
				else {
					std::set<Point> subCol;
					for (Point p : region) {
						if (p.y == pos2.y)
							subCol.insert(p);
					}
					if (panel.countColor(subCol, color) > 0) {
						open.erase(pos2);
						openSub.erase(pos2);
					}
					else {
						set(pos2, SymbolData::GetValFromSymbolID(FLOWER) | color);
						for (Point q : getRegion(pos2)) {
							if (!(q.x == pos2.x || q.y == pos2.y)) {
								open.erase(q);
							}
						}
						amount--;
						for (Point p : subCol) {
							open.erase(p);
						}
						openpos.erase(pos2);
						if (amount != 1)
							break;
					}
				}
			}
			if (openSub.size() == 0)
				return false;
		}
	}
	return true;
}

bool Generate::placeFlowerStarPairs(int color, int amount) {
	std::set<Point> open = openpos;
	while (amount > 0) {
		if (open.size() == 0)
			return false;
		Point pos1 = pickRandom(open);
		open.erase(pos1);
		std::set<Point> region = panel.getRegion(pos1);
		if (panel.countColor(region, color) > 0) 
			continue;
		std::set<Point> openSub;
		for (Point p : region) {
			if (open.erase(p) && (p.x == pos1.x || p.y == pos1.y)) openSub.insert(p);
		}
		if (openSub.size() == 0)
			continue;
		Point pos2 = pickRandom(openSub); 
		open.erase(pos2);
		openpos.erase(pos1);
		openpos.erase(pos2);
		int symbolOrder = rand(0, 1);
		if (symbolOrder) {
			set(pos1, Star | color);
			set(pos2, SymbolData::GetValFromSymbolID(FLOWER) | color);
		}
		else {
			set(pos2, Star | color);
			set(pos1, SymbolData::GetValFromSymbolID(FLOWER) | color);
		}
		amount--;
	}
	return true;
}