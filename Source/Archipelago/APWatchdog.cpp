#include "APWatchdog.h"

#include "../Input.h"
#include "../Panels.h"
#include "../Quaternion.h"
#include "SkipSpecialCases.h"

#include <thread>


#define CHEAT_KEYS_ENABLED 0


void APWatchdog::action() {
	auto currentFrameTime = std::chrono::system_clock::now();
	std::chrono::duration<float> frameDuration = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime;

	InteractionState interactionState = InputWatchdog::get()->getInteractionState();

	if (interactionState == InteractionState::Walking) HandleMovementSpeed(frameDuration.count());

	HandleKeyTaps();

	halfSecondCountdown -= frameDuration.count();
	if (halfSecondCountdown <= 0) {
		halfSecondCountdown += 0.5f;

		QueueItemMessages();

		CheckSolvedPanels();
		CheckEPSkips();

		HandlePowerSurge();
		CheckIfCanSkipPuzzle();
		DisableCollisions();
		RefreshDoorCollisions();
		AudioLogPlaying();

		if (storageCheckCounter <= 0) {
			CheckLasers();
			storageCheckCounter = 20;
		}
		else
		{
			storageCheckCounter--;
		}

		CheckImportantCollisionCubes();
	}

	SetStatusMessages();
	hudManager->update(frameDuration.count());
}

void APWatchdog::CheckSolvedPanels() {
	std::list<int64_t> solvedLocations;

	if (finalPanel != 0x09F7F && finalPanel != 0xFFF00 && ReadPanelDataIntentionallyUnsafe<int>(finalPanel, SOLVED) == 1 && !isCompleted) {
		isCompleted = true;
		ap->StatusUpdate(APClient::ClientStatus::GOAL);
	}
	if (finalPanel == 0x09F7F && !isCompleted)
	{
		float power = ReadPanelDataIntentionallyUnsafe<float>(0x17C6C, CABLE_POWER);

		if (power > 0.0f) {
			isCompleted = true;
			ap->StatusUpdate(APClient::ClientStatus::GOAL);
		}
	}
	if (finalPanel == 0xFFF00 && !isCompleted)
	{
		float power = ReadPanelDataIntentionallyUnsafe<float>(0x1800F, CABLE_POWER);

		if (power > 0.0f) {
			isCompleted = true;
			ap->StatusUpdate(APClient::ClientStatus::GOAL);
		}
	}

	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		int panelId = it->first;
		int locationId = it->second;

		if (allEPs.count(panelId)) {
			if (ReadPanelData<int>(panelId, EP_SOLVED)) //TODO: Check EP solved
			{
				solvedLocations.push_back(locationId);

				it = panelIdToLocationId.erase(it);
			}
			else
			{
				it++;
			}
			continue;
		}

		if (obeliskHexToEPHexes.count(panelId)) {
			std::set<int> EPSet = obeliskHexToEPHexes[panelId];

			bool anyNew = false;

			for (auto it2 = EPSet.begin(); it2 != EPSet.end();) {
				if (ReadPanelData<int>(*it2, EP_SOLVED)) //TODO: Check EP solved
				{
					anyNew = true;

					APClient::DataStorageOperation operation;
					operation.operation = "replace";
					operation.value = true;

					std::list<APClient::DataStorageOperation> operations;
					operations.push_back(operation);

					ap->Set("EP_" + std::to_string(*it2), NULL, false, operations);

					it2 = EPSet.erase(it2);
				}
				else
				{
					it2++;
				}
			}

			obeliskHexToEPHexes[panelId] = EPSet;

			if (EPSet.empty())
			{
				if(FirstEverLocationCheckDone) hudManager->queueBannerMessage(ap->get_location_name(locationId) + " Completed!");

				solvedLocations.push_back(locationId);

				it = panelIdToLocationId.erase(it);
			}
			else
			{
				if (anyNew) {
					int total = obeliskHexToAmountOfEPs[panelId];
					int done = total - EPSet.size();

					if (FirstEverLocationCheckDone) hudManager->queueBannerMessage(ap->get_location_name(locationId) + " Progress (" + std::to_string(done) + "/" + std::to_string(total) + ")");
				}
				it++;
			}
			continue;
		}

		if (panelId == 0x09FD2) {
			int cableId = 0x09FCD;

			if (ReadPanelData<int>(cableId, CABLE_POWER))
			{
				solvedLocations.push_back(locationId);

				it = panelIdToLocationId.erase(it);
			}
			else
			{
				it++;
			}
			continue;
		}

		if (panelId == 0x034E3) {
			int cableId = 0x034EB;

			if (ReadPanelData<int>(cableId, CABLE_POWER))
			{
				solvedLocations.push_back(locationId);

				it = panelIdToLocationId.erase(it);
			}
			else
			{
				it++;
			}
			continue;
		}

		if (panelId == 0x014D1) {
			int cableId = 0x00BED;

			if (ReadPanelData<int>(cableId, CABLE_POWER))
			{
				solvedLocations.push_back(locationId);

				it = panelIdToLocationId.erase(it);
			}
			else
			{
				it++;
			}
			continue;
		}

		if (panelId == 0x17C34) {
			if (ReadPanelData<int>(0x2FAD4, DOOR_OPEN) && ReadPanelData<int>(0x2FAD6, DOOR_OPEN) && ReadPanelData<int>(0x2FAD7, DOOR_OPEN))
			{
				solvedLocations.push_back(locationId);

				it = panelIdToLocationId.erase(it);
			}
			else
			{
				it++;
			}
			continue;
		}

		else if (ReadPanelData<int>(panelId, SOLVED))
		{
			solvedLocations.push_back(locationId);

			it = panelIdToLocationId.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (!solvedLocations.empty())
		ap->LocationChecks(solvedLocations);

	FirstEverLocationCheckDone = true;
}

void APWatchdog::MarkLocationChecked(int locationId, bool collect)
{
	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		if (it->second == locationId) {
			if (!actuallyEveryPanel.count(it->first)) return;

			if (collect && !ReadPanelData<int>(it->first, SOLVED)) {
				if (it->first == 0x17DC4 || it->first == 0x17D6C || it->first == 0x17DA2 || it->first == 0x17DC6 || it->first == 0x17DDB || it->first == 0x17E61 || it->first == 0x014D1 || it->first == 0x09FD2 || it->first == 0x034E3) {
					std::vector<int> bridgePanels;
					if (it->first == 0x17DC4) bridgePanels = { 0x17D72, 0x17D8F, 0x17D74, 0x17DAC, 0x17D9E, 0x17DB9, 0x17D9C, 0x17DC2, };
					else if (it->first == 0x17D6C) bridgePanels = { 0x17DC8, 0x17DC7, 0x17CE4, 0x17D2D, };
					else if (it->first == 0x17DC6) bridgePanels = { 0x17D9B, 0x17D99, 0x17DAA, 0x17D97, 0x17BDF, 0x17D91, };
					else if (it->first == 0x17DDB) bridgePanels = { 0x17DB3, 0x17DB5, 0x17DB6, 0x17DC0, 0x17DD7, 0x17DD9, 0x17DB8, 0x17DD1, 0x17DDC, 0x17DDE, 0x17DE3, 0x17DEC, 0x17DAE, 0x17DB0, };
					else if (it->first == 0x17DA2) bridgePanels = { 0x17D88, 0x17DB4, 0x17D8C, 0x17DCD, 0x17DB2, 0x17DCC, 0x17DCA, 0x17D8E, 0x17DB1, 0x17CE3, 0x17DB7 };
					else if (it->first == 0x17E61) bridgePanels = { 0x17E3C, 0x17E4D, 0x17E4F, 0x17E5B, 0x17E5F, 0x17E52 };
					else if (it->first == 0x014D1) bridgePanels = { 0x00001, 0x014D2, 0x014D4 };
					else if (it->first == 0x09FD2) bridgePanels = { 0x09FCC, 0x09FCE, 0x09FCF, 0x09FD0, 0x09FD1 };
					else if (it->first == 0x034E3) bridgePanels = { 0x034E4 };

					if (panelLocker->PuzzleIsLocked(it->first)) panelLocker->PermanentlyUnlockPuzzle(it->first);
					Special::SkipPanel(it->first, "Collected", false);
					WritePanelData<__int32>(it->first, VIDEO_STATUS_COLOR, { COLLECTED }); 

					for (int panel : bridgePanels) {
						if (ReadPanelData<int>(panel, SOLVED)) continue;
						
						if (panelLocker->PuzzleIsLocked(panel)) panelLocker->PermanentlyUnlockPuzzle(panel);
						Special::SkipPanel(panel, "Collected", false);
						WritePanelData<__int32>(panel, VIDEO_STATUS_COLOR, { COLLECTED });
					}
				}
				else{
					if(panelLocker->PuzzleIsLocked(it->first)) panelLocker->PermanentlyUnlockPuzzle(it->first);
					Special::SkipPanel(it->first, "Collected", false);
					if(it->first != 0x01983 && it->first != 0x01983) WritePanelData<float>(it->first, POWER, { 1.0f, 1.0f });
					WritePanelData<__int32>(it->first, VIDEO_STATUS_COLOR, { COLLECTED }); // Videos can't be skipped, so this should be safe.
				}
			}

			it = panelIdToLocationId.erase(it);
		}
		else
			it++;
	}
}

void APWatchdog::ApplyTemporarySpeedBoost() {
	speedTime += 20.0f;
}

void APWatchdog::ApplyTemporarySlow() {
	speedTime -= 20.0f;
}


void APWatchdog::HandleMovementSpeed(float deltaSeconds) {
	if (speedTime != 0) {
		// Move the speed time closer to zero.
		speedTime = std::max(std::abs(speedTime) - deltaSeconds, 0.f) * (std::signbit(speedTime) ? -1 : 1);

		if (speedTime > 0) {
			WriteMovementSpeed(2.0f * baseSpeed);
		}
		else if (speedTime < 0) {
			WriteMovementSpeed(0.6f * baseSpeed);
		}
		else {
			WriteMovementSpeed(baseSpeed);
		}
	}
}

void APWatchdog::TriggerPowerSurge() {
	if (hasPowerSurge) {
		powerSurgeStartTime = std::chrono::system_clock::now();
	}
	else {
		hasPowerSurge = true;
		powerSurgeStartTime = std::chrono::system_clock::now();

		for (const auto& panelId : AllPuzzles) {
			if (ReadPanelData<int>(panelId, SOLVED))
				continue;

			std::vector<float> powerValues = ReadPanelData<float>(panelId, POWER, 2);

			if (powerValues[0] > 0) {
				//using -20f as offset to create a unique recogniseable value that is also stored in the save
				WritePanelData<float>(panelId, POWER, { -20.0f + powerValues[0], -20.0f + powerValues[1] });
			}
		}
	}
}

void APWatchdog::HandlePowerSurge() {
	if (hasPowerSurge && DateTime::since(powerSurgeStartTime).count() > 10000)
		ResetPowerSurge();
}

void APWatchdog::ResetPowerSurge() {
	hasPowerSurge = false;

	for (const auto& panelId : AllPuzzles) {
		std::vector<float> powerValues = ReadPanelData<float>(panelId, POWER, 2);

		if (powerValues[0] < -18.0f && powerValues[0] > -22.0f && powerValues[1] < -18.0f && powerValues[1] > -22.0f)
		{
			powerValues[0] -= -20.0f;
			powerValues[1] -= -20.0f;

			WritePanelData<float>(panelId, POWER, powerValues);
			WritePanelData<float>(panelId, NEEDS_REDRAW, { 1 });
		}
	}
}

int APWatchdog::CheckIfCanSkipPuzzle() {
	SetWindowText(availableSkips, (L"Available Skips: " + std::to_wstring(foundPuzzleSkips - skippedPuzzles)).c_str());
	puzzleSkipInfoMessage.clear();


	int id = GetActivePanel();

	if ((id == 0x181F5 || id == 0x334D8) && !Hard) { // Swamp Rotating Bridge and Town RGB Control should only be skippable in expert
		EnableWindow(skipButton, false);
		return -1;
	}

	if (std::find(actuallyEveryPanel.begin(), actuallyEveryPanel.end(), id) == actuallyEveryPanel.end()) {
		EnableWindow(skipButton, false);
		return -1;
	}

	// Evaluate Cost

	int cost = 1;


	if (id == 0x03612) {
		if (ReadPanelData<int>(0x288E9, DOOR_OPEN) && ReadPanelData<int>(0x28AD4, DOOR_OPEN)) {
			EnableWindow(skipButton, false);
			return -1;
		}
		else
		{
			puzzleSkipInfoMessage = "Skipping this panel costs\n"
									"1 Puzzle Skip per unopened latch.";
		}

		if (!ReadPanelData<int>(0x288E9, DOOR_OPEN) && !ReadPanelData<int>(0x28AD4, DOOR_OPEN)) {
			cost = 2;
		}
	}

	if (id == 0x1C349) {
		if (ReadPanelData<int>(0x28AE8, DOOR_OPEN)) {
			EnableWindow(skipButton, false);
			return -1;
		}
		else
		{
			puzzleSkipInfoMessage = "Skipping this panel costs 2 Puzzle Skips.";
		}

		cost = 2;
	}

	if (id == 0x09FDA) { // Metapuzzle
		for (int id : {0x09FC1, 0x09F8E, 0x09F01, 0x09EFF}) {
			__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

			if (skipped < PUZZLE_SKIPPED || skipped > PUZZLE_SKIPPED_MAX) {
				//
				//  TODO (blastron, 11/24/22): The concept of only showing a puzzle skip message once isn't really supported in the new paradigm. Update this.
				// 
				//if(!metaPuzzleMessageHasBeenDisplayed) audioLogMessageBuffer[49] = { "", "Skipping this panel requires", "Skipping all the small puzzles.", 8.0f, true };
				//metaPuzzleMessageHasBeenDisplayed = true;
				//
				
				EnableWindow(skipButton, false);
				return -1;
				break;
			}
		}

		cost = 1;
	}

	// Cost Evaluated

	if (foundPuzzleSkips - cost < skippedPuzzles) {
		EnableWindow(skipButton, false);
		return -1;
	}

	if (id == 0x0A3A8) {
		id = 0x033EA;
	}
	if (id == 0x0A3B9) {
		id = 0x01BE9;
	}
	if (id == 0x0A3BB) {
		id = 0x01CD3;
	}
	if (id == 0x0A3AD) {
		id = 0x01D3F;
	}

	if (id == -1 || skip_completelyExclude.count(id) || panelLocker->PuzzleIsLocked(id)) {
		EnableWindow(skipButton, false);
		return -1;
	}

	__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

	if ((skipped >= PUZZLE_SKIPPED && skipped <= PUZZLE_SKIPPED_MAX) || skipped == COLLECTED) {
		EnableWindow(skipButton, false);
		return -1;
	}

	// Puzzle can be skipped from here on out, calculate cost

	EnableWindow(skipButton, true);
	return cost;
}

void APWatchdog::SkipPuzzle()
{
	int id = GetActivePanel();

	int cost = CheckIfCanSkipPuzzle();

	if (cost == -1) return;

	skippedPuzzles+= cost;

	// Special Skipping Animations

	if (id == 0x03612) {
		if (!ReadPanelData<int>(0x288E9, DOOR_OPEN))
		{
			_memory->OpenDoor(0x288E9);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		if (!ReadPanelData<int>(0x28AD4, DOOR_OPEN))
		{
			_memory->OpenDoor(0x28AD4);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	if (id == 0x1C349) {
		_memory->OpenDoor(0x28AE8);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		_memory->OpenDoor(0x28AED);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	// Special Skipping Animations Done

	Special::SkipPanel(id, true);

	WritePanelData<__int32>(id, VIDEO_STATUS_COLOR, { PUZZLE_SKIPPED + cost }); // Videos can't be skipped, so this should be safe.

	CheckIfCanSkipPuzzle();
}

void APWatchdog::SkipPreviouslySkippedPuzzles() {
	for (int id : actuallyEveryPanel) {
		__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

		if (skipped >= PUZZLE_SKIPPED && skipped <= PUZZLE_SKIPPED_MAX) {
			Special::SkipPanel(id);
			skippedPuzzles += skipped - PUZZLE_SKIPPED;
		}
		else if (skipped == COLLECTED) {
			Special::SkipPanel(id, "Collected", false);
		}
	}
}

void APWatchdog::AddPuzzleSkip() {
	foundPuzzleSkips++;
}

void APWatchdog::UnlockDoor(int id) {
	if (actuallyEveryPanel.count(id)) {
		_memory->WritePanelData<float>(id, POWER, { 1.0f, 1.0f });
		_memory->WritePanelData<int>(id, NEEDS_REDRAW, {1});
		return;
	}

	if (allLasers.count(id)) {
		_memory->ActivateLaser(id);
		return;
	}

	// Is a door

	if (id == 0x0CF2A) { // River to Garden door
		disableCollisionList.insert(id);

		_memory->WritePanelData<float>(0x17CAA, POSITION, { 36.694f, -41.883f, 16.570f });
		_memory->WritePanelData<float>(0x17CAA, SCALE, { 1.2f });

		_memory->UpdateEntityPosition(0x17CAA);
	}

	if (ReadPanelData<int>(id, DOOR_OPEN)) {
		std::wstringstream s;
		s << std::hex << id << " is already open.\n";
		OutputDebugStringW(s.str().c_str());
		return;
	}

	if (id == 0x0C310) {
		_memory->WritePanelData<float>(0x02886, POSITION + 8, { 12.8f });

		_memory->UpdateEntityPosition(0x02886);
	}

	if (id == 0x2D73F) {
		_memory->WritePanelData<float>(0x021D7, POSITION + 8, { 10.0f });

		_memory->UpdateEntityPosition(0x021D7);
	}

	if (doorCollisions.find(id) != doorCollisions.end()){
		for (int collisionId : doorCollisions[id]) {
			collisionsToRefresh[collisionId] = 10;
			collisionPositions[collisionId] = ReadPanelData<float>(collisionId, POSITION, 8);
		}
	}

	_memory->OpenDoor(id);
}

void APWatchdog::SeverDoor(int id) {
	if (actuallyEveryPanel.count(id)) {
		_memory->WritePanelData<float>(id, POWER, { 0.0f, 0.0f });
	}

	if (severTargetsById.count(id)) {
		severedDoorsList.insert(id);

		if (id == 0x01A0E) {
			_memory->WritePanelData<int>(0x01A0F, TARGET, { 0x0360E + 1 });
			return;
		}

		if (id == 0x01D3F) {
			_memory->WritePanelData<int>(0x01D3F, TARGET, { 0x03317 + 1 });
			return;
		}

		std::vector<Connection> conns = severTargetsById[id];

		for (auto& conn : conns) {
			if (id == 0x01954 || id == 0x018CE || id == 0x019D8 || id == 0x019B5 || id == 0x019E6 || id == 0x0199A || id == 0x18482 || id == 0x0A1D6)
			{
				DoubleDoorTargetHack(id);
				continue;
			}

			if (conn.id == 0x28A0D) {
				_memory->WritePanelData<float>(conn.id, POSITION, { -31.0f, 7.1f });

				_memory->UpdateEntityPosition(conn.id);
			}

			if (conn.id == 0x17CAB) {
				_memory->WritePanelData<float>(0x0026D, POWER, { 1.0f, 1.0f });
				_memory->WritePanelData<int>(0x0026D, NEEDS_REDRAW, { 1 });
			}

			if (conn.id == 0x01D8D) {
				_memory->WritePanelData<float>(conn.id, POSITION, { -57.8f, 157.2f, 3.45f });

				_memory->UpdateEntityPosition(conn.id);
			}

			if (conn.target_no == ENTITY_NAME) {
				std::stringstream stream;
				stream << std::hex << conn.id;
				std::string result(stream.str());
				std::vector<char> v(result.begin(), result.end());

				_memory->WriteArray<char>(conn.id, ENTITY_NAME, v);
				continue;
			}
			_memory->WritePanelData<int>(conn.id, conn.target_no, { 0 });
		}
		return;
	}
	else
	{
		std::wstringstream s;

		s << "Don't know how to Sever Door " << std::hex << id << "!!!!\n";
		OutputDebugStringW(s.str().c_str());
	}
}

void APWatchdog::DoubleDoorTargetHack(int id) {
	if (id == 0x01954) { // Exit Door 1->2
		if (severedDoorsList.count(0x018CE)) {
			_memory->WritePanelData<int>(0x00139, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x00521, CABLE_TARGET_2, { 0 });
		return;
	}

	if (id == 0x018CE) { // Shortcut Door 1
		if (severedDoorsList.count(0x01954)) {
			_memory->WritePanelData<int>(0x00139, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x00139, TARGET, { 0x01954 + 1 });
		return;
	}



	if (id == 0x019D8) { // Exit Door 2->3
		if (severedDoorsList.count(0x019B5)) {
			_memory->WritePanelData<int>(0x019DC, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x00525, CABLE_TARGET_2, { 0 });
		return;
	}

	if (id == 0x019B5) { // Shortcut Door 2
		if (severedDoorsList.count(0x019D8)) {
			_memory->WritePanelData<int>(0x019DC, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x019DC, TARGET, { 0x019D8 + 1 });
		return;
	}



	if (id == 0x019E6) { // Exit Door 3->4
		if (severedDoorsList.count(0x0199A)) {
			_memory->WritePanelData<int>(0x019E7, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x00529, CABLE_TARGET_2, { 0 });
		return;
	}

	if (id == 0x0199A) { // Shortcut Door 3
		if (severedDoorsList.count(0x019E6)) {
			_memory->WritePanelData<int>(0x019E7, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x019E7, TARGET, { 0x019E6 + 1 });
		return;
	}



	if (id == 0x18482) { // Swamp Blue
		if (severedDoorsList.count(0x0A1D6)) {
			_memory->WritePanelData<int>(0x00E3A, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x00BDB, CABLE_TARGET_0, { 0 });
		return;
	}

	if (id == 0x0A1D6) { // Swamp Purple
		if (severedDoorsList.count(0x18482)) {
			_memory->WritePanelData<int>(0x00E3A, TARGET, { 0 });
			return;
		}

		_memory->WritePanelData<int>(0x2FD5F, CABLE_TARGET_1, { 0 });
		return;
	}
}

void APWatchdog::HandleKeyTaps() {
	std::vector<InputButton> tapEvents = InputWatchdog::get()->consumeTapEvents();
	for (const InputButton& tappedButton : tapEvents) {
		switch (tappedButton) {
#if CHEAT_KEYS_ENABLED
		case InputButton::KEY_MINUS:
			hudManager->queueBannerMessage("Cheat: adding Slowness.", { 1.f, 0.f, 0.f }, 2.f);
			ApplyTemporarySlow();
			break;
		case InputButton::KEY_EQUALS:
			hudManager->queueBannerMessage("Cheat: adding Speed Boost.", { 1.f, 0.f, 0.f }, 2.f);
			ApplyTemporarySpeedBoost();
			break;
		case InputButton::KEY_0:
			hudManager->queueBannerMessage("Cheat: adding Puzzle Skip.", { 1.f, 0.f, 0.f }, 2.f);
			AddPuzzleSkip();
			break;
#endif
		};
	}
}

void APWatchdog::DisableCollisions() {
	for (int id : disableCollisionList) {
		_memory->RemoveMesh(id);
	}
}

void APWatchdog::RefreshDoorCollisions() {
	if (collisionsToRefresh.size() == 0) {
		return;
	}

	std::vector<float> playerPosition;

	try{
		playerPosition = _memory->ReadPlayerPosition();
	}
	catch (std::exception e) {
		return;
	}

	//std::wstringstream s;
	//s << playerPosition[0] << ", " << playerPosition[1] << ", " << playerPosition[2] << "\n";
	//OutputDebugStringW(s.str().c_str());

	for(auto const& p : collisionsToRefresh){
		auto collisionToUpdate = p.first;
		auto val = p.second;

		if (val == -10) return;

		if (val > 0) {
			collisionsToRefresh[collisionToUpdate] = val - 1;
			continue;
		}

		std::vector<float> originalPosition = collisionPositions[collisionToUpdate];

		if (pow(playerPosition[0] - originalPosition[0], 2) + pow(playerPosition[1] - originalPosition[1], 2) + pow(playerPosition[2] - originalPosition[2], 2) > 400) {
			continue;
		}

		std::wstringstream s1;
		s1 << std::hex << collisionToUpdate << " in range. Testing...\n";
		OutputDebugStringW(s1.str().c_str());

		collisionsToRefresh[collisionToUpdate] = -10;

		std::vector<float> newPositions = ReadPanelData<float>(collisionToUpdate, POSITION, 8);

		if (originalPosition != newPositions) {
			continue;
		}

		std::wstringstream s;
		s << std::hex << collisionToUpdate << " didn't move!!! Updating manually...\n";
		OutputDebugStringW(s.str().c_str());

		try {  
			_memory->UpdateEntityPosition(collisionToUpdate);
		}
		catch (const std::exception& e) { 
			collisionsToRefresh[collisionToUpdate] = 2;
			return;
		}

		if(!knownIrrelevantCollisions.count(collisionToUpdate)){
			collisionsToRefresh[collisionToUpdate] = 10;
		}
	}

	for (auto it = collisionsToRefresh.cbegin(), next_it = it; it != collisionsToRefresh.cend(); it = next_it)
	{
		++next_it;
		if (collisionsToRefresh[it->first] == -10)
		{
			collisionsToRefresh.erase(it);
		}
	}

	//OutputDebugStringW(L"-----\n");

	//Updates a tenth of the collisions every second. Will investigate if this needs to be increased potentially.
}

void APWatchdog::AudioLogPlaying() {
	std::string line1 = "";
	std::string line2 = "";
	std::string line3 = "";

	for (int logId : audioLogs) {
		bool logPlaying = ReadPanelData<int>(logId, AUDIO_LOG_IS_PLAYING) != 0;
		if (logPlaying && logId != currentAudioLog) {
			currentAudioLog = logId;

			std::string message = audioLogMessages[logId].first;
			hudManager->showSubtitleMessage(message, 12.0f);

			APClient::DataStorageOperation operation;
			operation.operation = "replace";
			operation.value = true;

			std::list<APClient::DataStorageOperation> operations;
			operations.push_back(operation);

			ap->Set("AL_" + std::to_string(logId), NULL, false, operations);

			int locationId = audioLogMessages[logId].second;

			if (locationId != -1) {
				ap->LocationScouts({ locationId });
			}
		}
		else if (!logPlaying && logId == currentAudioLog) {
			currentAudioLog = -1;
			hudManager->clearSubtitleMessage();
		}
	}
}

void APWatchdog::CheckLasers() {
	int laserCount = 0;
	
	for (int laser : allLasers) {
		if (ReadPanelData<int>(laser, LASER_TARGET)) {
			laserCount += 1;
		}
	}

	if (laserCount != state->activeLasers) {
		state->activeLasers = laserCount;
		panelLocker->UpdatePuzzleLock(*state, 0x0A332);
		panelLocker->UpdatePuzzleLock(*state, 0x3D9A9);

		if (laserCount >= state->requiredChallengeLasers && !laserRequirementMet) {
			laserRequirementMet = true;
		}
	}

	if (laserIDsToLasers.empty()) {
		int pNO = ap->get_player_number();

		for (int laser : allLasers) {
			std::string laserID = std::to_string(pNO) + "-" + std::to_string(laser);

			laserIDsToLasers[laserID] = laser;
			laserIDs.push_back(laserID);

			APClient::DataStorageOperation operation;
			operation.operation = "default";
			operation.value = false;

			std::list<APClient::DataStorageOperation> operations;

			operations.push_back(operation);

			nlohmann::json a;
		
			ap->Set(laserID, a, false, operations);
		}
	}

	auto laserStates = ap->Get(laserIDs);
}

void APWatchdog::HandleLaserResponse(const std::map <std::string, nlohmann::json> response, bool collect) {
	for (auto& [key, value] : response) {
		int playerNo = std::stoi(key.substr(0, key.find("-")));
		int laserNo = std::stoi(key.substr(key.find("-") + 1, key.length()));

		bool laserActiveAccordingToDataPackage = value == true;

		bool laserActiveInGame = ReadPanelData<int>(laserNo, LASER_TARGET) != 0;

		if (laserActiveInGame == laserActiveAccordingToDataPackage) continue;

		if (laserActiveInGame) {
			APClient::DataStorageOperation operation;
			operation.operation = "replace";
			operation.value = true;

			std::list<APClient::DataStorageOperation> operations;
			operations.push_back(operation);

			ap->Set(key, NULL, false, operations);
		}

		else if(collect)
		{
			if (laserNo == 0x012FB) _memory->OpenDoor(0x01317);
			_memory->ActivateLaser(laserNo);
			hudManager->queueBannerMessage(laserNames[laserNo] + " Laser Activated Remotely (Coop)");
		}
	}
}

void APWatchdog::InfiniteChallenge(bool enable) {
	_memory->SetInfiniteChallenge(enable);

	if (enable) hudManager->queueBannerMessage("Challenge Timer disabled.");
	if (!enable) hudManager->queueBannerMessage("Challenge Timer reenabled.");
}

void APWatchdog::CheckImportantCollisionCubes() {
	std::vector<float> playerPosition;

	try {
		playerPosition = _memory->ReadPlayerPosition();
	}
	catch (std::exception e) {
		return;
	}

	// bonsai panel dots requirement
	if (bonsaiCollisionCube.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x09d9b)) {
		hudManager->setWorldMessage("Needs Dots.");
	}
	else if (tutorialPillarCube.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0xc335)) {
		hudManager->setWorldMessage("Stone Pillar needs Triangles.");
	}
	else if ((riverVaultLowerCube.containsPoint(playerPosition) || riverVaultUpperCube.containsPoint(playerPosition)) && panelLocker->PuzzleIsLocked(0x15ADD)) {
		hudManager->setWorldMessage("Needs Dots, Black/White Squares.");
	}
	else if (bunkerPuzzlesCube.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x09FDC)) {
		hudManager->setWorldMessage("Bunker panels need B/W and Colored Squares.");
	}
	else if (quarryLaserPanel.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x03612)) {
		if (Hard) hudManager->setWorldMessage("Needs Eraser, Triangles,\n"
											  "Stars, Stars + Same Colored Symbol.");
		else hudManager->setWorldMessage("Needs Shapers and Eraser.");
	}
	else if (symmetryUpperPanel.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x1C349)) {
		if (Hard) hudManager->setWorldMessage("Needs Symmetry and Triangles.");
		else hudManager->setWorldMessage("Needs Symmetry and Dots.");
	}
	else if (ReadPanelData<int>(0x0C179, 0x1D4) && panelLocker->PuzzleIsLocked(0x01983)) {
		if (ReadPanelData<int>(0x0C19D, 0x1D4) && panelLocker->PuzzleIsLocked(0x01987)) {
			hudManager->setWorldMessage("Left Panel needs Stars and Shapers.\n"
										"Right Panel needs Dots and Colored Squares.");
		}
		else
		{
			hudManager->setWorldMessage("Left Panel needs Stars and Shapers.");
		}
	}
	else if (ReadPanelData<int>(0x0C19D, 0x1D4) && panelLocker->PuzzleIsLocked(0x01987)) {
		hudManager->setWorldMessage("Right Panel needs Dots and Colored Squares.");
	}
	else {
		hudManager->clearWorldMessage();
	}
}

bool APWatchdog::CheckPanelHasBeenSolved(int panelId) {
	return panelIdToLocationId.count(panelId);
}

void APWatchdog::SetItemReward(const int& id, const APClient::NetworkItem& item) {
	if (!actuallyEveryPanel.count(id)) return;

	Color backgroundColor;
	if (item.flags & APClient::ItemFlags::FLAG_ADVANCEMENT)
		backgroundColor = { 0.686f, 0.6f, 0.937f, 1.0f };
	else if (item.flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE)
		backgroundColor = { 0.427f, 0.545f, 0.91f, 1.0f };
	else if (item.flags & APClient::ItemFlags::FLAG_TRAP)
		backgroundColor = { 0.98f, 0.502f, 0.447f, 1.0f };
	else
		backgroundColor = { 0.0f , 0.933f, 0.933f, 1.0f };

	if (id == 0x28998 || id == 0x28A69 || id == 0x17CAA || id == 0x00037 || id == 0x09FF8 || id == 0x09DAF || id == 0x0A01F || id == 0x17E67) {
		WritePanelData<Color>(id, SUCCESS_COLOR_A, { backgroundColor });
	}
	else
	{
		WritePanelData<Color>(id, BACKGROUND_REGION_COLOR, { backgroundColor });
	}
	WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

void APWatchdog::CheckEPSkips() {
	if (!EPShuffle) return;

	std::set<int> panelsToSkip = {};
	std::set<int> panelsToRemoveSilently = {};

	for (int panel : panelsThatHaveToBeSkippedForEPPurposes) {
		__int32 skipped = ReadPanelData<__int32>(panel, VIDEO_STATUS_COLOR);

		if ((skipped >= PUZZLE_SKIPPED && skipped <= PUZZLE_SKIPPED_MAX) || skipped == COLLECTED) {
			panelsToRemoveSilently.insert(panel);
			continue;
		}

		if (panel == 0x09E86 || panel == 0x09ED8) {
			// TODO: This needs to check the exit is solved.
			if (ReadPanelData<int>(0x09E86, FLASH_MODE) == 1 && ReadPanelData<int>(0x09ED8, FLASH_MODE) == 1) { 
				int num_front_traced_edges = ReadPanelData<int>(0x09ED8, TRACED_EDGES);
				std::vector<int> front_traced_edges = ReadArray<int>(0x09ED8, TRACED_EDGE_DATA, num_front_traced_edges * 23);

				if(num_front_traced_edges >= 0 && front_traced_edges[num_front_traced_edges*13 - 13] == 0x1B && front_traced_edges[num_front_traced_edges * 13 - 12] == 0x27){
					panelsToSkip.insert(panel); 
					continue;
				}
				
				int num_back_traced_edges = ReadPanelData<int>(0x09E86, TRACED_EDGES);
				std::vector<int> back_traced_edges = ReadArray<int>(0x09E86, TRACED_EDGE_DATA, num_back_traced_edges * 13);

				if (num_back_traced_edges >= 0 && back_traced_edges[num_back_traced_edges * 13 - 13] == 0x1B && back_traced_edges[num_back_traced_edges * 13 - 12] == 0x27) {
					panelsToSkip.insert(panel);
					continue;
				}
			}
		}

		if (panel == 0x033EA || panel == 0x01BE9 || panel == 0x01CD3 || panel == 0x01D3F || panel == 0x181F5) {
			if (ReadPanelData<int>(panel, SOLVED)) {
				panelsToSkip.insert(panel);
				continue;
			}
		}

		if (panel == 0x334D8) { // Town RGB Control
			if (ReadPanelData<int>(0x334D8, SOLVED) && ReadPanelData<int>(0x03C0C, SOLVED) && ReadPanelData<int>(0x03C08, SOLVED)) {
				panelsToSkip.insert(panel);
				continue;
			}
		}
	}

	for (int panel : panelsToRemoveSilently) {
		panelsThatHaveToBeSkippedForEPPurposes.erase(panel);
	}

	for (int panel : panelsToSkip) {
		panelsThatHaveToBeSkippedForEPPurposes.erase(panel);
		Special::SkipPanel(panel, "Skipped", false);
		WritePanelData<__int32>(panel, VIDEO_STATUS_COLOR, { PUZZLE_SKIPPED }); // Cost == 0
	}
}

void APWatchdog::QueueItemMessages() {
	if (newItemsJustIn) {
		newItemsJustIn = false;
		return;
	}

	processingItemMessages = true;

	std::map<std::string, int> itemCounts;
	std::map<std::string, RgbColor> itemColors;
	std::vector<std::string> receivedItems;

	for (auto it = queuedItems.begin(); it != queuedItems.end();) {
		__int64 item = it->at(0);
		__int64 flags = it->at(1);
		__int64 realitem = it->at(2);

		if (ap->get_item_name(realitem) == "Unknown" || ap->get_item_name(item) == "Unknown") {
			it++;
			continue;
		}

		std::string name = ap->get_item_name(item);

		if (realitem != item && realitem > 0) {
			name += " (" + ap->get_item_name(realitem) + ")";
		}

		// Track the quantity of this item received in this batch.
		if (itemCounts.find(name) == itemCounts.end()) {
			itemCounts[name] = 1;
			receivedItems.emplace_back(name);
		}
		else {
			itemCounts[name]++;
		}

		// Assign a color to the item.
		itemColors[name] = getColorByItemFlag(flags);

		it = queuedItems.erase(it);
	}

	for (std::string name : receivedItems) {
		// If we received more than one of an item in this batch, add the quantity to the output string.
		std::string count = "";
		if (itemCounts[name] > 1) {
			count = " (x" + std::to_string(itemCounts[name]) + ")";
		}

		hudManager->queueBannerMessage("Received " + name + count + ".", itemColors[name]);
	}

	processingItemMessages = false;
}

void APWatchdog::QueueReceivedItem(std::vector<__int64> item) {
	newItemsJustIn = true;

	queuedItems.push_back(item);
}

void APWatchdog::SetStatusMessages() {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState == InteractionState::Walking) {
		int speedTimeInt = (int)std::ceil(std::abs(speedTime));

		if (speedTime > 0) {
			hudManager->setStatusMessage("Speed Boost active for " + std::to_string(speedTimeInt) + " seconds.");
		}
		else if (speedTime < 0) {
			hudManager->setStatusMessage("Slowness active for " + std::to_string(speedTimeInt) + " seconds.");
		}
		else {
			hudManager->clearStatusMessage();
		}
	}
	else if (interactionState == InteractionState::Focusing || interactionState == InteractionState::Solving) {
		hudManager->setStatusMessage(puzzleSkipInfoMessage);
	}
	else {
		hudManager->clearStatusMessage();
	}
}