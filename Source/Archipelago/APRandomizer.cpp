#include "APRandomizer.h"

#include "../ClientWindow.h"
#include "../HUDManager.h"
#include "../Panels.h"
#include "../Randomizer.h"
#include "../Special.h"
#include "APGameData.h"
#include "APWatchdog.h"
#include "Client/apclientpp/apclient.hpp"
#include "Hints.h"
#include "PanelLocker.h"
#include "../DateTime.h"
#include "PanelRestore.h"
#include "ASMPayloadManager.h"
#include "LockablePuzzle.h"
#include "../Utilities.h"
#include "SkipSpecialCases.h"

APRandomizer::APRandomizer() {
	panelLocker = new PanelLocker();
};

bool APRandomizer::Connect(std::string& server, std::string& user, std::string& password) {
	ClientWindow* clientWindow = ClientWindow::get();
	clientWindow->logLine("Connecting");
	std::string uri = buildUri(server);

	if (ap) ap->reset();
	ap = new APClient("uuid", "The Witness", uri);

	bool connected = false;
	bool hasConnectionResult = false;

	clientWindow->logLine("Connect: Setting handlers.");
	ap->set_room_info_handler([&]() {
		const int item_handling_flags_all = 7;

		ap->ConnectSlot(user, password, item_handling_flags_all, {}, {0, 4, 5});
	});

	ap->set_location_checked_handler([&](const std::list<int64_t>& locations) {
		while (!randomizationFinished) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		for (const auto& locationId : locations)
			async->MarkLocationChecked(locationId);
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

		for (auto item : items) {
			async->QueueItem(item);
		}
	});

	ap->set_slot_connected_handler([&](const nlohmann::json& slotData) {
		clientWindow->logLine("Connect: Successful connection response!");
		clientWindow->logLine("Connect: Getting basic slot data information.");
		Seed = slotData["seed"];
		FinalPanel = slotData["victory_location"];

		PuzzleRandomization = slotData.contains("puzzle_randomization") ? (int) slotData["puzzle_randomization"] : 0;
		UnlockSymbols = slotData.contains("shuffle_symbols") ? slotData["shuffle_symbols"] == true : true;
		EarlyUTM = slotData.contains("early_secret_area") ? slotData["early_secret_area"] == true : false;
		if (slotData.contains("mountain_lasers")) MountainLasers = slotData["mountain_lasers"];
		if (slotData.contains("challenge_lasers")) ChallengeLasers = slotData["challenge_lasers"];
		DisableNonRandomizedPuzzles = slotData.contains("disable_non_randomized_puzzles") ? slotData["disable_non_randomized_puzzles"] == true : false;
		EPShuffle = slotData.contains("shuffle_EPs") ? slotData["shuffle_EPs"] != 0 : false;
		DeathLink = slotData.contains("death_link") ? slotData["death_link"] == true : false;
		DeathLinkAmnesty = slotData.contains("death_link_amnesty") ? (int) slotData["death_link_amnesty"] : 0;
		if (!DeathLink) DeathLinkAmnesty = -1;

		ElevatorsComeToYou = slotData.contains("elevators_come_to_you") ? slotData["elevators_come_to_you"] == true : false;

		if (!UnlockSymbols) {
			state.unlockedArrows = true;
			state.unlockedColoredDots = true;
			state.unlockedColoredStones = true;
			state.unlockedDots = true;
			state.unlockedErasers = true;
			state.unlockedFullDots = true;
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

		clientWindow->logLine("Connect: Getting progressive items.");
		if (slotData.contains("progressive_item_lists")) {
			for (auto& [key, val] : slotData["progressive_item_lists"].items()) {
				int itemId = std::stoul(key, nullptr, 10);
				std::vector<int> v = val;

				progressiveItems[itemId] = v;
			}
		}

		clientWindow->logLine("Connect: Getting panelhex to id.");
		for (auto& [key, val] : slotData["panelhex_to_id"].items()) {
			int panelId = std::stoul(key, nullptr, 16);
			int locationId = val;

			panelIdToLocationId.insert({ panelId, locationId });
			panelIdToLocationIdReverse.insert({ locationId, panelId });
		}

		clientWindow->logLine("Connect: Getting Obelisk Side to EPs.");
		if (slotData.contains("obelisk_side_id_to_EPs")) {
			for (auto& [key, val] : slotData["obelisk_side_id_to_EPs"].items()) {
				int sideId = std::stoul(key, nullptr, 10);
				std::set<int> v = val;

				obeliskSideIDsToEPHexes.insert({ sideId, v });
			}
		}

		clientWindow->logLine("Connect: Getting EP to name.");
		if (slotData.contains("ep_to_name")) {
			for (auto& [key, val] : slotData["ep_to_name"].items()) {
				int sideId = std::stoul(key, nullptr, 16);

				entityToName.insert({ sideId, val });
			}
		}

		clientWindow->logLine("Connect: Getting Entity to Name.");
		if (slotData.contains("entity_to_name")) {
			for (auto& [key, val] : slotData["entity_to_name"].items()) {
				int sideId = std::stoul(key, nullptr, 16);

				entityToName.insert({ sideId, val });
			}
		}

		clientWindow->logLine("Connect: Getting Precompleted Puzzles.");
		if (slotData.contains("precompleted_puzzles")) {
			for (int key : slotData["precompleted_puzzles"]) {
				precompletedLocations.insert(key);

				//Back compat: Disabled EPs used to be "precompleted"
				if (allEPs.count(key)) disabledEntities.insert(key);
			}
		}

		clientWindow->logLine("Connect: Getting Disabled Puzzles.");
		if (slotData.contains("disabled_panels")) {
			for (std::string keys : slotData["disabled_panels"]) {
				int key = std::stoul(keys, nullptr, 16);
				disabledEntities.insert(key);
			}
		}

		if (slotData.contains("disabled_entities")) {
			for (int key : slotData["disabled_entities"]) {
				disabledEntities.insert(key);
			}
		}

		clientWindow->logLine("Connect: Getting item id to door hexes.");
		if (slotData.contains("item_id_to_door_hexes")) {
			for (auto& [key, val] : slotData["item_id_to_door_hexes"].items()) {
				int itemId = std::stoul(key, nullptr, 10);
				std::set<int> v = val;

				itemIdToDoorSet.insert({ itemId, v });
				
				for (int door : v) {
					doorToItemId.insert({ door, itemId });
				}
			}
		}

		clientWindow->logLine("Connect: Getting door hexes in the pool.");
		if (slotData.contains("door_hexes_in_the_pool")) {
			for (int key : slotData["door_hexes_in_the_pool"]) {
				doorsActuallyInTheItemPool.insert(key);
			}
		}

		clientWindow->logLine("Connect: Getting audio log hints.");
		if (slotData.contains("log_ids_to_hints")) {
			for (auto& [key, val] : slotData["log_ids_to_hints"].items()) {
				int logId = std::stoul(key, nullptr, 10);
				std::string message;
				int64_t location_id = -1;
				int32_t player_no = 0;
				bool extraInfoFound = false;
				std::string area = "";
				int32_t area_progression = -1;
				bool allowScout = true;

				for (int i = 0; i < val.size(); i++) {
					auto token = val[i];

					if (token.type() == nlohmann::json::value_t::number_integer || token.type() == nlohmann::json::value_t::number_unsigned || token.type() == nlohmann::json::value_t::null) {
						uint64_t integer = -1;
						if (token.type() != nlohmann::json::value_t::null) {
							integer = token;
						}
						if (!extraInfoFound) {
							location_id = integer;
							player_no = ap->get_player_number();
							extraInfoFound = true;
						}
						else if (area != "") {
							area_progression = integer;
						}
						else {
							player_no = integer;

							if (player_no == -1 && location_id != -1) {
								player_no = ap->get_player_number();
								allowScout = false;
							}
						}
					}
					else {
						std::string line = token;
						if (line.rfind("hinted_area:", 0) == 0) {
							area = line.substr(12);
							player_no = ap->get_player_number();
							extraInfoFound = true;
						}
						else if (line.rfind("containing_area:", 0) == 0) {
							area = line.substr(16);
							allowScout = false;
						}
						else if (!line.empty()) {
							if(message != "") message.append(" ");
							message.append(line);
						}
					}
				}
				
				inGameHints.insert({ logId, {message, location_id, player_no, area, area_progression, false, allowScout} });
			}
		}

		clientWindow->logLine("Connect: Getting lasers hints.");
		if (slotData.contains("laser_ids_to_hints")) {
			for (auto& [key, val] : slotData["laser_ids_to_hints"].items()) {
				int logId = std::stoul(key, nullptr, 10);
				std::string message;
				int64_t location_id = -1;
				int32_t player_no = 0;
				bool extraInfoFound = false;

				for (int i = 0; i < val.size(); i++) {
					auto token = val[i];

					if (token.type() == nlohmann::json::value_t::number_integer || token.type() == nlohmann::json::value_t::number_unsigned || token.type() == nlohmann::json::value_t::null) {
						uint64_t integer = -1;
						if (token.type() != nlohmann::json::value_t::null) {
							integer = token;
						}
						if (!extraInfoFound) {
							location_id = integer;
							player_no = ap->get_player_number();
							extraInfoFound = true;
						}
						else {
							player_no = integer;
						}
					}
					else {
						std::string line = token;
						if (!line.empty()) {
							if (message != "") message.append(" ");
							message.append(line);
						}
					}
				}

				inGameHints.insert({ logId, {message, location_id, player_no, "", -1, true, true} });
			}
		}

		if (DeathLink) {
			clientWindow->logLine("Connect: Setting DeathLink.");
			std::list<std::string> newTags = { "DeathLink" };

			ap->ConnectUpdate(false, 7, true, newTags);
		}

		connected = true;
		hasConnectionResult = true;
	});

	ap->set_slot_refused_handler([&](const std::list<std::string>& errors) {
		auto errorString = std::accumulate(errors.begin(), errors.end(), std::string(),
			[](const std::string& a, const std::string& b) -> std::string {
				return a + (a.length() > 0 ? "," : "") + b;
			});

		connected = false;
		hasConnectionResult = true;

		ClientWindow::get()->showMessageBox("Connection failed: " + errorString);
	});

	ap->set_bounced_handler([&](nlohmann::json packet) {
		std::list<std::string> tags = packet["tags"];

		bool deathlink = (std::find(tags.begin(), tags.end(), "DeathLink") != tags.end());

		if (deathlink) {
			auto data = packet["data"];
			std::string cause = "";
			if (data.contains("cause")) {
				cause = data["cause"];
			}
			std::string source = "";
			if (data.contains("source")) {
				source = data["source"];
			}

			double timestamp = data["time"];

			async->ProcessDeathLink(timestamp, cause, source);
		}
	});

	ap->set_location_info_handler([&](const std::list<APClient::NetworkItem> itemlist) {
		for (APClient::NetworkItem n : itemlist) {
			async->setLocationItemFlag(n.location, n.flags);
		}
	});

	ap->set_retrieved_handler([&](const std::map <std::string, nlohmann::json> response) {
		for (auto [key, value] : response) {
			if(key.find("WitnessLaser") != std::string::npos) async->HandleLaserResponse(key, value, SyncProgress);
			if(key.find("WitnessEP") != std::string::npos) async->HandleEPResponse(key, value, SyncProgress);
		}
	});

	ap->set_set_reply_handler([&](const std::string key, const nlohmann::json value, nlohmann::json original_value) {
		if (key.find("WitnessDeathLink") != std::string::npos) {
			async->SetValueFromServer(key, value);
			return;
		}

		if (key.find("WitnessLaserHint") != std::string::npos) async->HandleLaserHintResponse(key, value, SyncProgress); // Do not flip
		else if (key.find("WitnessLaser") != std::string::npos) async->HandleLaserResponse(key, value, SyncProgress); // Do not flip
		else if (key.find("WitnessEP") != std::string::npos) async->HandleEPResponse(key, value, SyncProgress);
		else if (key.find("WitnessAudioLog") != std::string::npos) async->HandleAudioLogResponse(key, value, SyncProgress);
		else if (key.find("WitnessSolvedPanels") != std::string::npos) async->HandleSolvedPanelsResponse(value, SyncProgress);
		else if (key.find("WitnessOpenedDoors") != std::string::npos) async->HandleOpenedDoorsResponse(value, SyncProgress);
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
			for (int seenAudioLog : async->seenAudioLogs) {
				if (inGameHints[seenAudioLog].locationID == item.location && inGameHints[seenAudioLog].playerNo == ap->get_player_number()) return;
			}
			for (int seenLaser : async->seenLasers) {
				if (inGameHints[seenLaser].locationID == item.location && inGameHints[seenLaser].playerNo == ap->get_player_number()) return;
			}

			std::string isFor = "";
			if (!receiving) isFor = " for " + player;

			if(!found) async->getHudManager()->queueNotification("Hint: " + itemName + isFor + " is on " + locationName + ".");
		}
		else {
			int location = item.location;

			bool panelSolved = false;
			int panelId = panelIdToLocationIdReverse[location];

			if (panelIdToLocationIdReverse.count(location) && async->CheckPanelHasBeenSolved(panelId)) {
				async->getHudManager()->queueNotification("(Collect) Sent " + itemName + " to " + player + ".", getColorByItemFlag(item.flags));
			}
			else
			{
				async->SetItemRewardColor(findResult->first, item.flags);
				if (!(item.item == ITEM_BONK_TRAP && receiving)) async->PlaySentJingle(findResult->first, item.flags);
				if(!receiving) async->getHudManager()->queueNotification("Sent " + itemName + " to " + player + ".", getColorByItemFlag(item.flags));
			}
		}
	});

	clientWindow->logLine("Connect: Starting poller.");
	(new APServerPoller(ap))->start();

	auto start = std::chrono::system_clock::now();

	clientWindow->logLine("Connect: Waiting for connection result.");
	while (!hasConnectionResult) {
		if (DateTime::since(start).count() > 5000) { //5 seconnd timeout on waiting for response from server
			connected = false;
			hasConnectionResult = true;

			std::string errorMessage = "Timeout while connecting to server: " + uri;
			ClientWindow::get()->showMessageBox(errorMessage);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return connected;
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

void APRandomizer::PreGeneration() {
	// Generate joke hints.
	std::vector<int> jokeHintOrder(GetJokeHints().size());
	std::iota(jokeHintOrder.begin(), jokeHintOrder.end(), 0);
	std::shuffle(jokeHintOrder.begin(), jokeHintOrder.end(), Random::gen);

	int jokeIndex = 0;
	std::vector<int> jokeHints;
	for (auto& [id, hint] : inGameHints) {
		if (hint.message.empty()) {
			jokeHints.push_back(id);
			hint.message = GetJokeHints().at(jokeHintOrder.at(jokeIndex));
			jokeIndex++;
		}
	}

	// Replace one junk hint with the credits hint.
	if (!jokeHints.empty()) {
		inGameHint& hint = inGameHints.at(jokeHints.at(Random::rand() % jokeHints.size()));
		hint.message = GetCreditsHint();
	}
}



void APRandomizer::PostGeneration() {
	ClientWindow* clientWindow = ClientWindow::get();
	Memory* memory = Memory::get();

	// Write gameplay relevant client options into datastorage

	clientWindow->logLine("Putting Settings in Datastorage.");
	ap->Set("WitnessSetting" + std::to_string(ap->get_player_number()) + "-Collect", NULL, false, {{"replace", CollectedPuzzlesBehavior}});
	ap->Set("WitnessSetting" + std::to_string(ap->get_player_number()) + "-Disabled", NULL, false, {{"replace", DisabledPuzzlesBehavior} });
	ap->Set("WitnessSetting" + std::to_string(ap->get_player_number()) + "-SyncProgress", NULL, false, { {"replace", SyncProgress} });

	ap->SetNotify({ "WitnessDeathLink" + std::to_string(ap->get_player_number()) });
	ap->Set("WitnessDeathLink" + std::to_string(ap->get_player_number()), NULL, true, { {"default", 0} });

	std::set<int64_t> allLocations;
	std::set<int64_t> missingLocations = ap->get_missing_locations();
	std::set<int64_t> checkedLocations = ap->get_checked_locations();
	allLocations.insert(missingLocations.begin(), missingLocations.end());
	allLocations.insert(checkedLocations.begin(), checkedLocations.end());
	std::list<int64_t> allLocationsList(allLocations.begin(), allLocations.end());

	// :)
	if (Utilities::isAprilFools()) {
		for (int panel : trivial_panels) {
			Special::swapStartAndEnd(panel);
		}
		for (int panel : door_timers) {
			Special::flipPanelHorizontally(panel);
		}
	}

	clientWindow->logLine("Making postgeneration game modifications.");
	// EP-related slowing down of certain bridges etc.
	if (EPShuffle) {
		clientWindow->setStatusMessage("Adjusting EP element speeds...");

		memory->WritePanelData<float>(0x005A2, OPEN_RATE, { 0.02f }); // Swamp Rotating Bridge, 2x (Instead of 4x)
		
		memory->WritePanelData<float>(0x09E26, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x (Instead of 2x)
		memory->WritePanelData<float>(0x09E98, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		memory->WritePanelData<float>(0x09ED4, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		memory->WritePanelData<float>(0x09EE3, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		memory->WritePanelData<float>(0x09F10, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x
		memory->WritePanelData<float>(0x09F11, OPEN_RATE, { 0.25f }); // Monastery Shutters, 1x

		memory->WritePanelData<float>(0x17E74, OPEN_RATE, { 0.03f }); // Swamp Flood gate (inner), 2x (Instead of 4x)
		memory->WritePanelData<float>(0x1802C, OPEN_RATE, { 0.03f }); // Swamp Flood gate (outer), 2x
	}

	// In Vanilla, Caves Invis Symmetry 3 turns on a power cable on Symmetry Island. This is never relevant in vanilla, but doors modes make it a legitimate issue.
	memory->WritePanelData<int>(0x00029, TARGET, { 0 }); 

	// Challenge Timer Colors
	memory->WritePanelData<float>(0x0A332, PATTERN_POINT_COLOR_A, { 0.0f, 1.0f, 1.0f, 1.0f });
	memory->WritePanelData<float>(0x0A332, PATTERN_POINT_COLOR_B, { 1.0f, 1.0f, 0.0f, 1.0f });

	clientWindow->setStatusMessage("Applying progression...");
	PreventSnipes(); //Prevents Snipes to preserve progression randomizer experience

	clientWindow->logLine("Changing mounatain laser box and goal condition.");
	if(MountainLasers != 7 || ChallengeLasers != 11) Special::SetRequiredLasers(MountainLasers, ChallengeLasers);
	if (MountainLasers > 7) {
		memory->WritePanelData<float>(0x09f7f, POSITION, { 153.95224, -53.066, 68.343, 0.97, -0.2462272793, 0.01424658205, -0.980078876, 0.06994858384 });
		ASMPayloadManager::get()->UpdateEntityPosition(0x09f7f);
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

	for (int panel : desertPanels) {
		memory->UpdatePanelJunctions(panel);
	}
	
	clientWindow->logLine("Setting Puzzle Locks.");
	setPuzzleLocks();

	clientWindow->logLine("Restoring previously taken actions.");
	async->SkipPreviouslySkippedPuzzles();

	async->ResetPowerSurge();

	clientWindow->logLine("Apply patches.");
	Memory::get()->applyDestructivePatches();

	randomizationFinished = true;
	memory->showMsg = false;

	if (disabledEntities.size() > 0) {
		clientWindow->setStatusMessage("Disabling Puzzles...");
		for (int checkID : disabledEntities) {
			async->DisablePuzzle(checkID);
		}
	}

	if (precompletedLocations.size() > 0) {
		clientWindow->setStatusMessage("Precompleting Puzzles...");
		for (int checkID : precompletedLocations) {
			if (allPanels.count(checkID)) {
				async->SkipPanel(checkID, "Excluded", false);
			}
		}
	}

	clientWindow->logLine("Randomization finished.");
	async->getHudManager()->queueBannerMessage("Randomized!");
	async->PlayFirstJingle();

	clientWindow->logLine("Starting APWatchdog.");
	async->start();

	clientWindow->logLine("Firing Locationscout.");
	ap->LocationScouts(allLocationsList);
}

void APRandomizer::HighContrastMode() {
	Memory* memory = Memory::get();

	std::vector<float> swampRedOuter = memory->ReadPanelData<float>(0x00982, OUTER_BACKGROUND, 4);
	std::vector<float> swampRedInner = memory->ReadPanelData<float>(0x00982, BACKGROUND_REGION_COLOR, 4);
	std::vector<float> pathColor = memory->ReadPanelData<float>(0x00982, PATH_COLOR, 4);
	std::vector<float> activeColor = memory->ReadPanelData<float>(0x00982, ACTIVE_COLOR, 4);

	for (int id : notThatBadSwampPanels) {
		memory->WritePanelData<float>(id, OUTER_BACKGROUND, swampRedOuter);
		memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, swampRedInner);
		memory->WritePanelData<float>(id, PATH_COLOR, pathColor);
		memory->WritePanelData<float>(id, ACTIVE_COLOR, activeColor);
	}

	for (int i = 0; i < 3; i++) {
		swampRedOuter[i] *= 0.75f;
		swampRedInner[i] *= 0.75f;
		pathColor[i] *= 0.75f;
	}

	for (int id : swampLowContrastPanels) {
		memory->WritePanelData<float>(id, OUTER_BACKGROUND, swampRedOuter);
		memory->WritePanelData<float>(id, BACKGROUND_REGION_COLOR, swampRedInner);
		memory->WritePanelData<float>(id, PATH_COLOR, pathColor);
		memory->WritePanelData<float>(id, ACTIVE_COLOR, activeColor);
	}
}

void APRandomizer::DisableColorCycle() {
	Memory* memory = Memory::get();

	for (int id : allPanels) {
		memory->WritePanelData<int>(id, COLOR_CYCLE_INDEX, -1);
	}

	for (int id : mountainOffset) {
		if (id == 0x09e79) {
			memory->WritePanelData<uint64_t>(id, PATTERN_NAME, memory->ReadPanelData<uint64_t>(0x00089, PATTERN_NAME));
		}
		else {
			memory->WritePanelData<uint64_t>(id, PATTERN_NAME, memory->ReadPanelData<uint64_t>(0x0008a, PATTERN_NAME));
		}
	}
}

void APRandomizer::setPuzzleLocks() {
	ClientWindow* clientWindow = ClientWindow::get();
	Memory* memory = Memory::get();

	const int puzzleCount = LockablePuzzles.size();
	for (int i = 0; i < puzzleCount; i++)	{
		clientWindow->setStatusMessage("Locking puzzles: " + std::to_string(i) + "/" + std::to_string(puzzleCount));
		if (!(allPanels.count(LockablePuzzles[i]) && memory->ReadPanelData<int>(LockablePuzzles[i], SOLVED)))
			panelLocker->UpdatePuzzleLock(state, LockablePuzzles[i]);
	}
}

void APRandomizer::InitPanels() {
	Memory* memory = Memory::get();

	for (int panel : LockablePuzzles) {
		if (allPanels.count(panel)) {
			memory->InitPanel(panel);
		}
	}

	Special::SetVanillaMetapuzzleShapes();
}

void APRandomizer::RestoreOriginals() {
	PanelRestore::RestoreOriginalPanelData();
}

void APRandomizer::GenerateNormal() {
	async = new APWatchdog(ap, panelIdToLocationId, FinalPanel, panelLocker, entityToName, inGameHints, obeliskSideIDsToEPHexes, EPShuffle, PuzzleRandomization, &state, solveModeSpeedFactor, ElevatorsComeToYou, CollectedPuzzlesBehavior, DisabledPuzzlesBehavior, disabledEntities, itemIdToDoorSet, progressiveItems, DeathLinkAmnesty, doorToItemId);
	SeverDoors();

	if (DisableNonRandomizedPuzzles)
		panelLocker->DisableNonRandomizedPuzzles(doorsActuallyInTheItemPool);
}

void APRandomizer::GenerateHard() {
	async = new APWatchdog(ap, panelIdToLocationId, FinalPanel, panelLocker, entityToName, inGameHints, obeliskSideIDsToEPHexes, EPShuffle, PuzzleRandomization, &state, solveModeSpeedFactor, ElevatorsComeToYou, CollectedPuzzlesBehavior, DisabledPuzzlesBehavior, disabledEntities, itemIdToDoorSet, progressiveItems, DeathLinkAmnesty, doorToItemId);
	SeverDoors();

	//Mess with Town targets
	Special::copyTarget(0x03C08, 0x28A0D); Special::copyTarget(0x28A0D, 0x28998);
	
	Special::setTargetAndDeactivate(0x03C0C, 0x03C08);

	if (doorsActuallyInTheItemPool.count(0x28A0D)) {
		Special::setTarget(0x28998, 0);
	}
	else {
		Special::setTargetAndDeactivate(0x28998, 0x28A0D);
	}

	Memory::get()->PowerNext(0x03629, 0x36);

	if (DisableNonRandomizedPuzzles)
		panelLocker->DisableNonRandomizedPuzzles(doorsActuallyInTheItemPool);
}

void APRandomizer::PreventSnipes()
{
	// Distance-gate shadows laser to prevent sniping through the bars
	if (doorsActuallyInTheItemPool.count(0x19665) && doorsActuallyInTheItemPool.count(0x194B2)) {
		Memory::get()->WritePanelData<float>(0x19650, MAX_BROADCAST_DISTANCE, { 2.7 });
	}
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

