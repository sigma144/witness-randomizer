#include "APRandomizer.h"

APRandomizer::APRandomizer() {
	_memory = std::make_shared<Memory>("witness64_d3d11.exe");
}

bool APRandomizer::Connect(HWND& messageBoxHandle, std::string& server, std::string& user, std::string& password) {
	std::string uri = buildUri(server);
	
	ap = new APClient("uuid", "The Witness", uri);

	bool connected = false;
	bool hasConnectionResult = false;

	ap->set_room_info_handler([&]() {
		const int item_handling_flags_all = 7;

		ap->ConnectSlot(user, password, item_handling_flags_all);
	});

	ap->set_location_checked_handler([&](const std::list<int64_t>& locations) {
		for (const auto& locationId : locations) {
			async->MarkLocationChecked(locationId);
		}
	});

	ap->set_slot_connected_handler([&](const nlohmann::json& slotData) {
		Seed = slotData["seed"];
		Hard = slotData["hard_mode"] == true;
		UnlockSymbols = slotData["unlock_symbols"] == true;
		DisableNonRandomizedPuzzles = slotData["disable_non_randomized_puzzles"] == true;

		for (auto& [key, val] : slotData["panelhex_to_id"].items())
		{
			int panelId = std::stoul(key, nullptr, 16);
			int locationId = val;

			panelIdToLocationId.insert({ panelId, locationId });
		}

		async = new APWatchdog(ap, panelIdToLocationId);
		async->start();

		connected = true;
		hasConnectionResult = true;
	});

	ap->set_slot_refused_handler([&](const std::list<std::string>& errors) {
		auto errorString = std::accumulate(errors.begin(), errors.end(), std::string(),
			[](const std::string& a, const std::string& b) -> std::string {
				return a + (a.length() > 0 ? "," : "") + b;
			});

		std::wstring wError = std::wstring(errorString.begin(), errorString.end());

		connected = false;
		hasConnectionResult = true;

		WCHAR errorMessage[100] = L"Connection Failed: ";
		wcscat_s(errorMessage, 100, wError.data());

		MessageBox(messageBoxHandle, errorMessage, NULL, MB_OK);
	});

	ap->set_items_received_handler([&](const std::list<APClient::NetworkItem>& items) {
		bool updatePanels = false;

		for (const auto& item : items) {
			switch (item.item)
			{
				//Puzzle Symbols
				case ITEM_DOTS:							unlockedDots = true;							break;
				case ITEM_COLORED_DOTS:					unlockedColoredDots = true;				break;
				case ITEM_SOUND_DOTS:					unlockedSoundDots = true;					break;
				case ITEM_SYMMETRY:						unlockedSymmetry = true;					break;
				case ITEM_TRIANGLES:						unlockedTriangles = true;					break;
				case ITEM_ERASOR:							unlockedErasers = true;						break;
				case ITEM_TETRIS:							unlockedTetris = true;						break;
				case ITEM_TETRIS_ROTATED:				unlockedTetrisRotated = true;				break;
				case ITEM_TETRIS_NEGATIVE:				unlockedTetrisNegative = true;			break;
				case ITEM_TETRIS_NEGATIVE_ROTATED:	unlockedTetrisNegative = true	;			break;
				case ITEM_STARS:							unlockedStars = true;						break;
				case ITEM_STARS_WITH_OTHER_SYMBOL:	unlockedStarsWithOtherSimbol = true;	break;
				case ITEM_SQUARES:						unlockedStones = true;						break;
				case ITEM_COLORED_SQUARES:				unlockedColoredStones = true;				break;
				//Powerups
				case ITEM_TEMP_SPEED_BOOST:			async->ApplyTemporarySpeedBoost();		break;
				case ITEM_TEMP_SPEED_REDUCTION:		async->ApplyTemporarySlow();				break;
				//Traps
				case ITEM_POWER_SURGE:					async->TriggerPowerSurge();				break;

				default:
					break;
			}

			if (item.item < ITEM_TEMP_SPEED_BOOST)
				updatePanels = true;
		}

		if (updatePanels)
			enablePuzzles();
	});

	(new APServerPoller(ap))->start();

	auto start = std::chrono::system_clock::now();

	while (!hasConnectionResult) {
		if (DateTime::since(start).count() > 5000) { //5 seconnd timeout on waiting for response from server
			connected = false;
			hasConnectionResult = true;

			std::wstring wideServer = Converty::Utf8ToWide(uri);

			WCHAR errorMessage[100] = L"Timeout while connecting to server: ";
			wcscat_s(errorMessage, 100, wideServer.data());

			MessageBox(messageBoxHandle, errorMessage, NULL, MB_OK);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return connected;
}

std::string APRandomizer::buildUri(std::string& server)
{
	std::string uri = server;

	if (uri.rfind("ws://", 0) == std::string::npos)
		uri = "ws://" + uri;
	if (uri.find(":") == std::string::npos)
		uri = uri + ":38281";
	else if (uri.rfind(":") == uri.length())
		uri = uri + "38281";

	return uri;
}

void APRandomizer::Initialize(HWND loadingHandle) {
	PreventSnipes(); //Prevents Snipes to preserve progression randomizer experience

	if (UnlockSymbols)
		disablePuzzles(loadingHandle);

	if (DisableNonRandomizedPuzzles)
		; //TODO: handle
}

void APRandomizer::disablePuzzles(HWND loadingHandle) {
	for (int i = 0; i < sizeof(AllPuzzles) / sizeof(AllPuzzles[0]); i++)
	{
		std::wstring text = L"Locking puzzles: " + std::to_wstring(i) + L"/" + std::to_wstring(sizeof(AllPuzzles));
		SetWindowText(loadingHandle, text.c_str());

		if (!_memory->ReadPanelData<int>(AllPuzzles[i], SOLVED))
			disablePuzzle(AllPuzzles[i]);
	}
}

void APRandomizer::enablePuzzles() {
	std::vector<PuzzleData*> puzzlesToEnable;

	for (auto const& [id, puzzle] : disabledPuzzles)
	{
		if (
				(!unlockedStones && puzzle->hasStones)
				|| (!unlockedColoredStones && puzzle->hasColoredStones)
				|| (!unlockedStars && puzzle->hasStars)
				|| (!unlockedTetris && puzzle->hasTetris)
				|| (!unlockedTetrisRotated && puzzle->hasTetrisRotated)
				|| (!unlockedTetrisNegative && puzzle->hasTetrisNegative)
				|| (!unlockedTetrisNegativeRotated && puzzle->hasTetrisNegativeRotated)
				|| (!unlockedErasers && puzzle->hasErasers)
				|| (!unlockedTriangles && puzzle->hasTriangles)
				|| (!unlockedDots && puzzle->hasDots)
				|| (!unlockedColoredDots && puzzle->hasColoredDots)
				|| (!unlockedInvisibleDots && puzzle->hasInvisibleDots)
				|| (!unlockedArrows && puzzle->hasArrows)
				|| (!unlockedSymmetry && puzzle->hasSymmetry)
			)
			continue;

		puzzlesToEnable.push_back(puzzle);
	}

	for (auto& puzzle : puzzlesToEnable) {
		puzzle->Restore(_memory);
		disabledPuzzles.erase(puzzle->id);
		delete puzzle;
	}
}

void APRandomizer::GenerateNormal() {
}

void APRandomizer::GenerateHard() {
}

void APRandomizer::PreventSnipes()
{
	// Distance-gate shadows laser to prevent sniping through the bars
	_memory->WritePanelData<float>(0x19650, MAX_BROADCAST_DISTANCE, { 2.5 });
}

void APRandomizer::disablePuzzle(int id) {
	if (disabledPuzzles.count(id) == 1)
		return;

	PuzzleData* puzzle = new PuzzleData(id);
	puzzle->Read(_memory);

	disabledPuzzles.insert({ id, puzzle });

	const int width = 4;
	const int height = 2;

	std::vector<float> intersections;
	std::vector<int> intersectionFlags;
	std::vector<int> connectionsA;
	std::vector<int> connectionsB;
	std::vector<int> decorations;
	std::vector<int> decorationsFlags;
	std::vector<int> polygons;

	addMissingSimbolsDisplay(intersections, intersectionFlags, connectionsA, connectionsB);

	Special::createText(id, "missing", intersections, connectionsA, connectionsB, 0.1f, 0.9f, 0.1f, 0.4f);

	addPuzzleSimbols(puzzle, decorations, decorationsFlags, intersections, intersectionFlags, polygons);

	_memory->WritePanelData<int>(id, GRID_SIZE_X, { width + 1 });
	_memory->WritePanelData<int>(id, GRID_SIZE_Y, { height + 1 });
	_memory->WritePanelData<float>(id, PATH_WIDTH_SCALE, { 0.6f });
	_memory->WritePanelData<int>(id, NUM_DOTS, { static_cast<int>(intersectionFlags.size()) }); //amount of intersections
	_memory->WriteArray<float>(id, DOT_POSITIONS, intersections); //position of each point as array of x,y,x,y,x,y so this vector is twice the suze if sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_FLAGS, intersectionFlags); //flags for each point such as entrance or exit
	_memory->WritePanelData<int>(id, NUM_CONNECTIONS, { static_cast<int>(connectionsA.size()) }); //amount of connected points, for each connection we specify start in sourceConnectionsA and end in sourceConnectionsB
	_memory->WriteArray<int>(id, DOT_CONNECTION_A, connectionsA); //start of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WriteArray<int>(id, DOT_CONNECTION_B, connectionsB); //end of a connection between points, contains position of point in sourceIntersectionFlags
	_memory->WritePanelData<int>(id, NUM_DECORATIONS, { static_cast<int>(decorations.size()) });
	_memory->WriteArray<int>(id, DECORATIONS, decorations);
	_memory->WriteArray<int>(id, DECORATION_FLAGS, decorationsFlags);
	if (polygons.size() > 0) { //TODO maybe just always write ?
		_memory->WritePanelData<int>(id, NUM_COLORED_REGIONS, { static_cast<int>(polygons.size()) / 4 }); //why devide by 4 tho?
		_memory->WriteArray<int>(id, COLORED_REGIONS, polygons);
	}
	_memory->WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void APRandomizer::addMissingSimbolsDisplay(std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& connectionsA, std::vector<int>& connectionsB) {
	//does not respect width/height but its hardcoded at a grid of 4/2

	//panel coordinates go from 0,0 in bottom left to 1,1 in top right
	//format vector of x,y,x,y,x,y,x,y,x,y
	std::vector<float> gridIntersections = {
		0.1f, 0.1f, 0.3f, 0.1f, 0.5f, 0.1f, 0.7f, 0.1f, 0.9f, 0.1f, //bottom row
		0.1f, 0.3f, 0.3f, 0.3f, 0.5f, 0.3f, 0.7f, 0.3f, 0.9f, 0.3f, //middle row
		0.1f, 0.5f, 0.3f, 0.5f, 0.5f, 0.5f, 0.7f, 0.5f, 0.9f, 0.5f,  //top row
		0.0f, 0.0f, 1.0f, 1.0f //start, exit needed to prevent crash on re-randomization
	};

	//flag for each point
	//must be half the size of gridIntersections as its one per intersections
	std::vector<int> gridIntersectionFlags = {
		0, 0, 0, 0, 0, //bottom row
		0, 0, 0, 0, 0, //middle row
		0, 0, 0, 0, 0, //top row
		IntersectionFlags::STARTPOINT, IntersectionFlags::ENDPOINT //start, exit
	};

	//starts of a connection, the value refers to the position of an intersection point the intersections array
	//works together with gridConnectionsB
	std::vector<int> gridConnectionsA = {
		0, 1, 2, 3, //horizontal bottom row
		5, 6, 7, 8, //horizontal middle row
		10,11,12,13,//horizontal top row
		0, 5, //vertical left column
		1, 6, //vertical 2 column
		2, 7, //vertical 3 column
		3, 8, //vertical 4 column
		4, 9  //vertical right column
	};

	//end of a connection, the value refers to the position of an intersection point the intersections array
	//works together with gridConnectionsA
	std::vector<int> gridConnectionsB = {
		1, 2, 3, 4, //horizontal bottom row
		6, 7, 8, 9, //horizontal middle row
		11,12,13,14,//horizontal top row
		5, 10, //vertical left column
		6, 11, //vertical 2 column
		7, 12, //vertical 3 column
		8, 13, //vertical 4 column
		9, 14  //vertical right column
	};

	intersections.insert(intersections.begin(), gridIntersections.begin(), gridIntersections.end());
	intersectionFlags.insert(intersectionFlags.begin(), gridIntersectionFlags.begin(), gridIntersectionFlags.end());
	connectionsA.insert(connectionsA.begin(), gridConnectionsA.begin(), gridConnectionsA.end());
	connectionsB.insert(connectionsB.begin(), gridConnectionsB.begin(), gridConnectionsB.end());
}

void APRandomizer::addPuzzleSimbols(PuzzleData* puzzle, std::vector<int>& decorations, std::vector<int>& decorationsFlags,
	std::vector<float>& intersections, std::vector<int>& intersectionFlags, std::vector<int>& polygons) {

	//does not respect width/height but its hardcoded at a grid of 4/2
	//stored the puzzle simnbols per grid section, from bottom to top from left to right so buttom left is decoration 0
	std::vector<int> gridDecorations(8, 0);
	std::vector<int> gridDecorationsFlags(8, 0);

	int column = 4;
	int secondRowOffset = -column;

	if (puzzle->hasStones)
	{
		if (puzzle->hasColoredStones)
		{
			gridDecorations[column] = Decoration::Stone | Decoration::Color::Orange;
			gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::Blue;
			column++;
		}
		else
		{
			gridDecorations[column] = Decoration::Stone | Decoration::Color::Black;
			gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::White;
			column++;
		}
	}
	if (puzzle->hasStars)
	{
		if (puzzle->hasStarsWithOtherSymbol) {
			gridDecorations[column] = Decoration::Star | Decoration::Color::Green;
			gridDecorations[column + secondRowOffset] = Decoration::Stone | Decoration::Color::Green;
			column++;
		}
		else {
			gridDecorations[column] = Decoration::Star | Decoration::Color::Magenta;
			column++;
		}
	}
	if (puzzle->hasTetris) //no idea how
	{
		if (puzzle->hasTetrisRotated) {
			//normal rotated
		}
		else if (puzzle->hasTetrisNegative) {
			if (puzzle->hasTetrisNegativeRotated) {
				//rotated negative
			}
			else {
				//non rotated negative
			}
		}
		else {
			gridDecorations[column] = Decoration::Poly | Decoration::Color::Yellow; 
			column++;
		}
	}
	if (puzzle->hasErasers)
	{
		gridDecorations[column] = Decoration::Eraser;
		column++;
	}
	if (puzzle->hasTriangles)
	{
		gridDecorations[column] = Decoration::Triangle2;
		column++;
	}
	if (puzzle->hasDots) {
		//add black dot on line
	}
	if (puzzle->hasColoredDots) {
		// add a blue and orange dot on line
	}
	if (puzzle->hasInvisibleDots) {
		// add an invisiable dot ??? wtf ???
	}
	if (puzzle->hasSoundDots) {
		// add a small and latge red dot
	}
	if (puzzle->hasSymmetry) { 
		// decide how to represent
	}
	if (puzzle->hasArrows)
	{
		Panel panel;
		panel.render_arrow(column, 1, 2, 0, intersections, intersectionFlags, polygons); //BUG: should either pass decorations or gridDecorations along
		column++;
	}

	decorations.insert(decorations.begin(), gridDecorations.begin(), gridDecorations.end());
	decorationsFlags.insert(decorationsFlags.begin(), gridDecorationsFlags.begin(), gridDecorationsFlags.end());
}