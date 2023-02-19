#include "../HUDManager.h"
#include "APRandomizer.h"
#include "APGameData.h"
#include "../Panels.h"

bool APRandomizer::Connect(HWND& messageBoxHandle, std::string& server, std::string& user, std::string& password) {
	std::string uri = buildUri(server);

	ap = new APClient("uuid", "The Witness", uri);

	try {	ap->set_data_package_from_file(DATAPACKAGE_CACHE);	}
	catch (std::exception) { /* ignore */ }

	bool connected = false;
	bool hasConnectionResult = false;

	ap->set_room_info_handler([&]() {
		const int item_handling_flags_all = 7;

		ap->ConnectSlot(user, password, item_handling_flags_all, {}, {0, 3, 8});
	});

	ap->set_location_checked_handler([&](const std::list<int64_t>& locations) {
		while (!randomizationFinished) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		for (const auto& locationId : locations)
			async->MarkLocationChecked(locationId, this->Collect);
	});

	ap->set_slot_disconnected_handler([&]() {
		async->getHudManager()->queueBannerMessage("Randomiser has been disconnected.");
		std::string message = "The randomizer seems to have unexpectedly disconnected. Please reload both the game and the randomizer.";

		MessageBoxA(GetActiveWindow(), message.c_str(), NULL, MB_OK);
	});

	ap->set_socket_disconnected_handler([&]() {
		async->getHudManager()->queueBannerMessage("Randomiser has been disconnected.");
		std::string message = "The randomizer seems to have unexpectedly disconnected. Please reload both the game and the randomizer.";

		MessageBoxA(GetActiveWindow(), message.c_str(), NULL, MB_OK);
	});

	ap->set_items_received_handler([&](const std::list<APClient::NetworkItem>& items) {
		while (!randomizationFinished) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		_memory->WritePanelData<int>(0x0064, VIDEO_STATUS_COLOR + 12, { mostRecentItemId });
		
		for (const auto& item : items) {
			int realitem = item.item;
			int advancement = item.flags;

			if (progressiveItems.count(realitem)) {
				if (progressiveItems[realitem].size() == 0) {
					continue;
				}

				realitem = progressiveItems[realitem][0];
				progressiveItems[item.item].erase(progressiveItems[item.item].begin());
			}

			bool unlockLater = false;

			if (item.item != ITEM_TEMP_SPEED_BOOST && item.item != ITEM_TEMP_SPEED_REDUCTION && item.item != ITEM_POWER_SURGE) {
				unlockItem(realitem);
				panelLocker->UpdatePuzzleLocks(state, realitem);
			}
			else {
				unlockLater = true;
			}

			if (itemIdToDoorSet.count(realitem)) {
				for (int doorHex : itemIdToDoorSet[realitem]) {
					async->UnlockDoor(doorHex);
				}
			}




			if (mostRecentItemId >= item.index + 1) continue;

			if (unlockLater) {
				unlockItem(realitem);
			}

			mostRecentItemId = item.index + 1;

			_memory->WritePanelData<int>(0x0064, VIDEO_STATUS_COLOR + 12, { mostRecentItemId });

			while (async->processingItemMessages) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			async->QueueReceivedItem({ item.item, advancement, realitem });
		}
	});

	ap->set_slot_connected_handler([&](const nlohmann::json& slotData) {
		Seed = slotData["seed"];
		FinalPanel = slotData["victory_location"];

		PuzzleRandomization = slotData.contains("puzzle_randomization") ? (int) slotData["puzzle_randomization"] : 0;
		UnlockSymbols = slotData.contains("shuffle_symbols") ? slotData["shuffle_symbols"] == true : true;
		EarlyUTM = slotData.contains("early_secret_area") ? slotData["early_secret_area"] == true : false;
		if (slotData.contains("mountain_lasers")) MountainLasers = slotData["mountain_lasers"];
		if (slotData.contains("challenge_lasers")) ChallengeLasers = slotData["challenge_lasers"];
		DisableNonRandomizedPuzzles = slotData.contains("disable_non_randomized_puzzles") ? slotData["disable_non_randomized_puzzles"] == true : false;
		EPShuffle = slotData.contains("shuffle_EPs") ? slotData["shuffle_EPs"] != 0 : false;

		if (!UnlockSymbols) {
			state.unlockedArrows = true;
			state.unlockedColoredDots = true;
			state.unlockedColoredStones = true;
			state.unlockedDots = true;
			state.unlockedErasers = true;
			state.unlockedFullDots = true;
			state.unlockedInvisibleDots = true;
			state.unlockedSoundDots = true;
			state.unlockedStars = true;
			state.unlockedStarsWithOtherSimbol = true;
			state.unlockedStones = true;
			state.unlockedSymmetry = true;
			state.unlockedTetris = true;
			state.unlockedTetrisNegative = true;
			state.unlockedTetrisRotated = true;
			state.unlockedTriangles = true;
		}

		state.requiredChallengeLasers = ChallengeLasers;
		state.requiredMountainLasers = MountainLasers;

		if (slotData.contains("progressive_item_lists")) {
			for (auto& [key, val] : slotData["progressive_item_lists"].items()) {
				int itemId = std::stoul(key, nullptr, 10);
				std::vector<int> v = val;

				progressiveItems.insert({ itemId, v });
			}
		}

		for (auto& [key, val] : slotData["panelhex_to_id"].items()) {
			int panelId = std::stoul(key, nullptr, 16);
			int locationId = val;

			panelIdToLocationId.insert({ panelId, locationId });
			panelIdToLocationIdReverse.insert({ locationId, panelId });
		}

		if (slotData.contains("obelisk_side_id_to_EPs")) {
			for (auto& [key, val] : slotData["obelisk_side_id_to_EPs"].items()) {
				int sideId = std::stoul(key, nullptr, 10);
				std::set<int> v = val;

				obeliskSideIDsToEPHexes.insert({ sideId, v });
			}
		}

		if (slotData.contains("ep_to_name")) {
			for (auto& [key, val] : slotData["ep_to_name"].items()) {
				int sideId = std::stoul(key, nullptr, 16);

				epToName.insert({ sideId, val });
			}
		}

		if (slotData.contains("precompleted_puzzles")) {
			for (int key : slotData["precompleted_puzzles"]) {
				precompletedLocations.insert(key);
			}
		}

		if (slotData.contains("disabled_panels")) {
			for (std::string keys : slotData["disabled_panels"]) {
				int key = std::stoul(keys, nullptr, 16);
				if (!actuallyEveryPanel.count(key)) continue;
				disabledPanels.insert(key);
			}
		}

		if (slotData.contains("item_id_to_door_hexes")) {
			for (auto& [key, val] : slotData["item_id_to_door_hexes"].items()) {
				int itemId = std::stoul(key, nullptr, 10);
				std::set<int> v = val;

				itemIdToDoorSet.insert({ itemId, v });
			}
		}

		if (slotData.contains("door_hexes_in_the_pool")) {
			for (int key : slotData["door_hexes_in_the_pool"]) {
				doorsActuallyInTheItemPool.insert(key);
			}
		}
		else
		{
			for (auto& [key, val] : itemIdToDoorSet) {
				for(int door : val){
					doorsActuallyInTheItemPool.insert(door);
				}
			}
		}

		if (slotData.contains("log_ids_to_hints")) {
			for (auto& [key, val] : slotData["log_ids_to_hints"].items()) {
				int logId = std::stoul(key, nullptr, 10);
				std::string message;
				int64_t location_id_in_this_world = -1;
				for (int i = 0; i < val.size() - 1; i++) {
					std::string line = val[i];
					if (!line.empty()) {
						message.append(line);
						message.append(" ");
					}
				}

				location_id_in_this_world = val[val.size() - 1];
				
				audioLogMessages.insert({ logId, std::pair(message, location_id_in_this_world) });
			}
		}

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

		WCHAR errorMessage[200] = L"Connection Failed: ";
		wcscat_s(errorMessage, 200, wError.data());

		MessageBox(messageBoxHandle, errorMessage, NULL, MB_OK);
	});

	ap->set_retrieved_handler([&](const std::map <std::string, nlohmann::json> response) {
		for (auto [key, value] : response) {
			if(key.find("WitnessLaser") != std::string::npos) async->HandleLaserResponse(key, value, Collect);
			if(key.find("WitnessEP") != std::string::npos) async->HandleEPResponse(key, value, Collect);
		}
	});

	ap->set_set_reply_handler([&](const std::string key, const nlohmann::json value, nlohmann::json original_value) {
		if (value != original_value) {
			if(key.find("WitnessLaser") != std::string::npos) async->HandleLaserResponse(key, value, Collect);
			if(key.find("WitnessEP") != std::string::npos) async->HandleEPResponse(key, value, Collect);
		}
	});

	ap->set_print_json_handler([&](const APClient::PrintJSONArgs jsonArgs) {
		if (!jsonArgs.receiving || !jsonArgs.item || jsonArgs.item->player != ap->get_player_number())
			return;

		const APClient::NetworkItem item = *jsonArgs.item;
		const int receiver = *jsonArgs.receiving;
		const auto msg = jsonArgs.data;

		while (!randomizationFinished) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		int counter = 10; //Try for 10 seconds to see if something else than "Unknown" shows up
		while (ap->get_item_name(item.item) == "Unknown" && counter > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			counter--;
		}

		auto findResult = std::find_if(std::begin(panelIdToLocationId), std::end(panelIdToLocationId), [&](const std::pair<int, int>& pair) {
			return pair.second == item.location;
		});

		if (findResult == std::end(panelIdToLocationId)) return;

		bool receiving = receiver == ap->get_player_number();

		std::string player = ap->get_player_alias(receiver);
		std::string itemName = ap->get_item_name(item.item);
		std::string locationName = ap->get_location_name(item.location);

		bool hint = jsonArgs.type == "Hint";
		bool found = (jsonArgs.found) ? *jsonArgs.found : false;

		if (hint) {
			if (async->seenAudioMessages.count(item.location)) return;

			std::string isFor = "";
			if (!receiving) isFor = " for " + player;

			if(!found) async->getHudManager()->queueBannerMessage("Hint: " + itemName + isFor + " is on " + locationName + ".");
		}
		else {
			int location = item.location;

			bool panelSolved = false;
			int panelId = panelIdToLocationIdReverse[location];

			if (panelIdToLocationIdReverse.count(location) && async->CheckPanelHasBeenSolved(panelId)) {
				async->getHudManager()->queueBannerMessage("(Collect) Sent " + itemName + " to " + player + ".", getColorForItem(item));
			}
			else
			{
				async->SetItemReward(findResult->first, item);
				if(!receiving) async->getHudManager()->queueBannerMessage("Sent " + itemName + " to " + player + ".", getColorForItem(item));
			}
		}
	});

	ap->set_data_package_changed_handler([&](const nlohmann::json& data) {
		ap->save_data_package(DATAPACKAGE_CACHE);
	});

	(new APServerPoller(ap))->start();

	auto start = std::chrono::system_clock::now();

	while (!hasConnectionResult) {
		if (DateTime::since(start).count() > 5000) { //5 seconnd timeout on waiting for response from server
			connected = false;
			hasConnectionResult = true;

			std::wstring wideServer = Converty::Utf8ToWide(uri);

			WCHAR errorMessage[200] = L"Timeout while connecting to server: ";
			wcscat_s(errorMessage, 200, wideServer.data());

			MessageBox(messageBoxHandle, errorMessage, NULL, MB_OK);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return connected;
}

void APRandomizer::unlockItem(int item) {
	switch (item) {
		case ITEM_DOTS:									state.unlockedDots = true;							break;
		case ITEM_COLORED_DOTS:							state.unlockedColoredDots = true;				break;
		case ITEM_FULL_DOTS:									state.unlockedFullDots = true;							break;
		case ITEM_SOUND_DOTS:							state.unlockedSoundDots = true;					break;
		case ITEM_SYMMETRY:								state.unlockedSymmetry = true;					break;
		case ITEM_TRIANGLES:								state.unlockedTriangles = true;					break;
		case ITEM_ERASOR:									state.unlockedErasers = true;						break;
		case ITEM_TETRIS:									state.unlockedTetris = true;						break;
		case ITEM_TETRIS_ROTATED:						state.unlockedTetrisRotated = true;				break;
		case ITEM_TETRIS_NEGATIVE:						state.unlockedTetrisNegative = true;			break;
		case ITEM_STARS:									state.unlockedStars = true;						break;
		case ITEM_STARS_WITH_OTHER_SYMBOL:			state.unlockedStarsWithOtherSimbol = true;	break;
		case ITEM_B_W_SQUARES:							state.unlockedStones = true;						break;
		case ITEM_COLORED_SQUARES:						state.unlockedColoredStones = true;				break;
		case ITEM_SQUARES: state.unlockedStones = state.unlockedColoredStones = true;				break;
		case ITEM_ARROWS: state.unlockedArrows = true; break;

		//Powerups
		case ITEM_TEMP_SPEED_BOOST:					async->ApplyTemporarySpeedBoost();				break;
		case ITEM_PUZZLE_SKIP:							async->AddPuzzleSkip(); break;

		//Traps
		case ITEM_POWER_SURGE:							async->TriggerPowerSurge();						break;
		case ITEM_TEMP_SPEED_REDUCTION:				async->ApplyTemporarySlow();						break;
	}
}

std::string APRandomizer::buildUri(std::string& server)
{
	std::string uri = server;

	if (uri.find(":") == std::string::npos)
		uri = uri + ":38281";
	else if (uri.back() == ':')
		uri = uri + "38281";

	return uri;
}

void APRandomizer::PostGeneration(HWND loadingHandle) {
	for (int eID : precompletedLocations) {
		if (allEPs.count(eID)) {
			_memory->SolveEP(eID);
			if (precompletableEpToName.count(eID) && precompletableEpToPatternPointBytes.count(eID)) {
				_memory->MakeEPGlow(precompletableEpToName.at(eID), precompletableEpToPatternPointBytes.at(eID));
			}
		}
	}

	// EP-related slowing down of certain bridges etc.

	if (EPShuffle) {
		_memory->WritePanelData<float>(0x005A2, OPEN_RATE, { 0.02f }); // Swamp Rotating Bridge, 2x (Instead of 4x)
		
		_memory->WritePanelData<float>(0x09E26, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x (Instead of 2x)
		_memory->WritePanelData<float>(0x09E98, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		_memory->WritePanelData<float>(0x09ED4, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		_memory->WritePanelData<float>(0x09EE3, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		_memory->WritePanelData<float>(0x09F10, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		_memory->WritePanelData<float>(0x09F11, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
	}

	// Bunker door colors

	int num_dec = _memory->ReadPanelData<int>(0x17C2E, NUM_DECORATIONS);

	if(num_dec != 1){
		std::vector<int> decorations = _memory->ReadArray<int>(0x17C2E, DECORATIONS, num_dec);

		decorations[3] = 264;
		decorations[12] = 264;

		_memory->WriteArray<int>(0x17C2E, DECORATIONS, decorations);
	}

	// Challenge Timer Colors

	_memory->WritePanelData<float>(0x0A332, PATTERN_POINT_COLOR_A, { 0.0f, 1.0f, 1.0f, 1.0f });
	_memory->WritePanelData<float>(0x0A332, PATTERN_POINT_COLOR_B, { 1.0f, 1.0f, 0.0f, 1.0f });

	PreventSnipes(); //Prevents Snipes to preserve progression randomizer experience

	if(MountainLasers != 7 || ChallengeLasers != 11) Special::SetRequiredLasers(MountainLasers, ChallengeLasers);

	async->SkipPreviouslySkippedPuzzles();

	for (int panel : desertPanels) {
		_memory->UpdatePanelJunctions(panel);
	}

	if (FinalPanel == 0x09F7F) {
		Special::writeGoalCondition(0x0042D, " Goal:", "Box Short", MountainLasers, ChallengeLasers);
	}
	else if (FinalPanel == 0xFFF00) {
		Special::writeGoalCondition(0x0042D, " Goal:", "Box Long", MountainLasers, ChallengeLasers);
	}
	else if (FinalPanel == 0x3D9A9) {
		Special::writeGoalCondition(0x0042D, " Goal:", "Elevator", MountainLasers, ChallengeLasers);
	}
	else if (FinalPanel == 0x0356B) {
		Special::writeGoalCondition(0x0042D, " Goal:", "Challenge", MountainLasers, ChallengeLasers);
	}

	_memory->DisplaySubtitles("", "", "");
	
	setPuzzleLocks(loadingHandle);

	async->ResetPowerSurge();

	_memory->applyDestructivePatches();

	randomizationFinished = true;
	_memory->showMsg = false;

	async->getHudManager()->queueBannerMessage("Randomized!");

	async->start();
}

void APRandomizer::setPuzzleLocks(HWND loadingHandle) {
	for (int i = 0; i < sizeof(AllPuzzles) / sizeof(AllPuzzles[0]); i++)	{
		std::wstring text = L"Locking puzzles: " + std::to_wstring(i) + L"/" + std::to_wstring(sizeof(AllPuzzles));
		SetWindowText(loadingHandle, text.c_str());

		if (!_memory->ReadPanelData<int>(AllPuzzles[i], SOLVED));
			panelLocker->UpdatePuzzleLock(state, AllPuzzles[i]);
	}

	SetWindowText(loadingHandle, L"Done!");
}

void APRandomizer::Init() {
	mostRecentItemId = _memory->ReadPanelData<int>(0x0064, VIDEO_STATUS_COLOR + 12);

	for (int panel : AllPuzzles) {
		_memory->InitPanel(panel);
	}

	PanelRestore::RestoreOriginalPanelData(_memory);
}

void APRandomizer::GenerateNormal(HWND skipButton, HWND availableSkips) {
	Special::SetVanillaMetapuzzleShapes();

	async = new APWatchdog(ap, panelIdToLocationId, FinalPanel, panelLocker, skipButton, availableSkips, epToName, audioLogMessages, obeliskSideIDsToEPHexes, EPShuffle, PuzzleRandomization, &state, solveModeSpeedFactor);
	SeverDoors();

	if (DisableNonRandomizedPuzzles)
		panelLocker->DisableNonRandomizedPuzzles(disabledPanels, doorsActuallyInTheItemPool);
}

void APRandomizer::GenerateHard(HWND skipButton, HWND availableSkips) {
	async = new APWatchdog(ap, panelIdToLocationId, FinalPanel, panelLocker, skipButton, availableSkips, epToName, audioLogMessages, obeliskSideIDsToEPHexes, EPShuffle, PuzzleRandomization, &state, solveModeSpeedFactor);
	SeverDoors();

	//Mess with Town targets
	Special::copyTarget(0x03C08, 0x28A0D); Special::copyTarget(0x28A0D, 0x28998);
	
	Special::setTargetAndDeactivate(0x03C0C, 0x03C08);

	if (doorsActuallyInTheItemPool.count(0x28A0D)) {
		Special::setPower(0x28A0D, false);
	}
	else
	{
		Special::setTargetAndDeactivate(0x28998, 0x28A0D);
	}

	_memory->PowerNext(0x03629, 0x36);

	if (DisableNonRandomizedPuzzles)
		panelLocker->DisableNonRandomizedPuzzles(disabledPanels, doorsActuallyInTheItemPool);
}

void APRandomizer::PreventSnipes()
{
	// Distance-gate shadows laser to prevent sniping through the bars
	_memory->WritePanelData<float>(0x19650, MAX_BROADCAST_DISTANCE, { 2.7 });
}

void APRandomizer::SkipPuzzle() {
	async->SkipPuzzle();
}

void APRandomizer::SeverDoors() {
	for (int id : doorsActuallyInTheItemPool) {
		async->SeverDoor(id);
	}
}

bool APRandomizer::InfiniteChallenge(bool enable) {
	if(randomizationFinished) async->InfiniteChallenge(enable);
	return randomizationFinished;
}

