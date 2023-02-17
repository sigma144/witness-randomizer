#include "APWatchdog.h"

#include "../Input.h"
#include "../Panels.h"
#include "../Quaternion.h"
#include "SkipSpecialCases.h"

#include <thread>


#define CHEAT_KEYS_ENABLED 0
#define SKIP_HOLD_DURATION 1.f


void APWatchdog::action() {
	auto currentFrameTime = std::chrono::system_clock::now();
	std::chrono::duration<float> frameDuration = currentFrameTime - lastFrameTime;
	lastFrameTime = currentFrameTime;

	HandleInteractionState();
	HandleMovementSpeed(frameDuration.count());
	
	HandleKeyTaps();

	UpdatePuzzleSkip(frameDuration.count());

	halfSecondCountdown -= frameDuration.count();
	if (halfSecondCountdown <= 0) {
		halfSecondCountdown += 0.5f;

		QueueItemMessages();

		CheckSolvedPanels();

		if(PuzzleRandomization != NO_PUZZLE_RANDO) CheckEPSkips();

		HandlePowerSurge();
		DisableCollisions();
		RefreshDoorCollisions();
		AudioLogPlaying();

		if (storageCheckCounter <= 0) {
			CheckLasers();
			CheckEPs();
			storageCheckCounter = 20;
		}
		else
		{
			storageCheckCounter--;
		}
	}

	hudManager->clearWorldMessage();

	CheckImportantCollisionCubes();
	LookingAtObelisk();
	LookingAtTheDog(frameDuration.count());

	SetStatusMessages();
	hudManager->update(frameDuration.count());
}

void APWatchdog::HandleInteractionState() {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	bool stateChanged = InputWatchdog::get()->consumeInteractionStateChange();

	if (interactionState == InteractionState::Solving) {
		if (stateChanged) {
			// The player has started solving a puzzle. Update our active panel ID to match.
			activePanelId = GetActivePanel();
		}
	}
	else if (interactionState != InteractionState::Focusing) {
		// We're not in a puzzle-solving state. Clear the active panel, if any.
		activePanelId = -1;
	}
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

		if (panelId == 0xFFF80) {
			if(sentDog)
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

					ap->Set("WitnessEP" + std::to_string(ap->get_player_number()) + "-" + std::to_string(*it2), NULL, false, operations);

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
			if (actuallyEveryPanel.count(it->first)) {
				if (collect && !ReadPanelData<int>(it->first, SOLVED)) {
					__int32 skipped = ReadPanelData<__int32>(it->first, VIDEO_STATUS_COLOR);

					if (skipped == COLLECTED || skipped == DISABLED || skipped >= PUZZLE_SKIPPED && skipped <= PUZZLE_SKIPPED_MAX) {
						it = panelIdToLocationId.erase(it);
						continue;
					}

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
					else {
						if (panelLocker->PuzzleIsLocked(it->first)) panelLocker->PermanentlyUnlockPuzzle(it->first);
						Special::SkipPanel(it->first, "Collected", false);
						if (it->first != 0x01983 && it->first != 0x01983) WritePanelData<float>(it->first, POWER, { 1.0f, 1.0f });
						if(!skip_completelyExclude.count(it->first)) WritePanelData<__int32>(it->first, VIDEO_STATUS_COLOR, { COLLECTED }); // Videos can't be skipped, so this should be safe.
						WritePanelData<__int32>(it->first, NEEDS_REDRAW, { 1 });
					}
				}
			}

			else if (allEPs.count(it->first) && collect) {
				int eID = it->first;

				_memory->SolveEP(eID);
				if (precompletableEpToName.count(eID) && precompletableEpToPatternPointBytes.count(eID)) {
					_memory->MakeEPGlow(precompletableEpToName.at(eID), precompletableEpToPatternPointBytes.at(eID));
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
		float factor = 1;

		InteractionState interactionState = InputWatchdog::get()->getInteractionState();
		if (interactionState != InteractionState::Walking) factor = solveModeSpeedFactor;

		speedTime = std::max(std::abs(speedTime) - deltaSeconds * factor, 0.f) * (std::signbit(speedTime) ? -1 : 1);

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
	else {
		WriteMovementSpeed(baseSpeed);
	}

	if (speedTime == 0.6999999881) { // avoid original value
		speedTime = 0.699;
	}

	WritePanelData<float>(0x3D9A7, VIDEO_STATUS_COLOR, { speedTime });
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

void APWatchdog::UpdatePuzzleSkip(float deltaSeconds) {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState == InteractionState::Solving || interactionState == InteractionState::Focusing) {
		// If we're tracking a panel for the purpose of puzzle skips, update our internal logic.
		if (activePanelId != -1) {
			// Check to see if the puzzle is in a skippable state.
			if (PuzzleIsSkippable(activePanelId)) {
				// The puzzle is still skippable. Update the panel's cost in case something has changed, like a latch
				//   being opened remotely.
				puzzleSkipCost = CalculatePuzzleSkipCost(activePanelId, puzzleSkipInfoMessage);

				// Update skip button logic.
				bool skipButtonHeld = InputWatchdog::get()->getButtonState(InputButton::KEY_T);
				if (skipButtonHeld) {
					skipButtonHeldTime += deltaSeconds;
					if (skipButtonHeldTime > SKIP_HOLD_DURATION) {
						SkipPuzzle();
					}
				}
				else {
					skipButtonHeldTime = 0.f;
				}
			}
			else {
				// The puzzle is not in a skippable state.
				puzzleSkipInfoMessage = "";
				puzzleSkipCost = -1;
			}
		}
	}

	// Update the client.
	SetWindowText(availableSkips, (L"Available Skips: " + std::to_wstring(GetAvailablePuzzleSkips())).c_str());
	EnableWindow(skipButton, CanUsePuzzleSkip());
}

bool APWatchdog::CanUsePuzzleSkip() const {
	return activePanelId != -1 && puzzleSkipCost != -1 && GetAvailablePuzzleSkips() >= puzzleSkipCost;
}

bool APWatchdog::PuzzleIsSkippable(int puzzleId) const {
	if (puzzleId == -1) {
		return false;
	}

	// Special case: For pressure plate puzzles, the player can't actually select the puzzle directly, so we instead
	//   detect when they've selected the reset line and then switch to the actual panel they want to interact with.
	if (puzzleId == 0x0A3A8) {
		puzzleId = 0x033EA;
	}
	else if (puzzleId == 0x0A3B9) {
		puzzleId = 0x01BE9;
	}
	else if (puzzleId == 0x0A3BB) {
		puzzleId = 0x01CD3;
	}
	else if (puzzleId == 0x0A3AD) {
		puzzleId = 0x01D3F;
	}

	// Verify that this is, indeed, a panel.
	if (std::find(actuallyEveryPanel.begin(), actuallyEveryPanel.end(), puzzleId) == actuallyEveryPanel.end()) {
		return false;
	}

	// Check for hardcoded exclusions.
	if (skip_completelyExclude.count(puzzleId) != 0) {
		// Puzzle is always excluded.
		return false;
	}
	else if (PuzzleRandomization == SIGMA_EXPERT && skip_excludeOnHard.count(puzzleId) != 0) {
		// Puzzle is excluded on Hard.
		return false;
	}
	else if ((PuzzleRandomization == SIGMA_NORMAL || PuzzleRandomization == NO_PUZZLE_RANDO) && skip_excludeOnNormal.count(puzzleId) != 0) {
		// Puzzle is excluded on Normal.
		return false;
	}

	// Locked puzzles are unskippable.
	if (panelLocker->PuzzleIsLocked(puzzleId)) {
		return false;
	}

	// Check skippability based on panel state.
	uint32_t statusColor = ReadPanelData<uint32_t>(puzzleId, VIDEO_STATUS_COLOR);
	if (statusColor >= PUZZLE_SKIPPED && statusColor <= PUZZLE_SKIPPED_MAX) {
		// Puzzle has already been skipped.
		return false;
	}
	else if (statusColor == COLLECTED || statusColor == DISABLED) {
		// Puzzle has been collected and is not worth skipping.
		return false;
	}
	else if (ReadPanelData<int>(puzzleId, SOLVED) != 0 && skip_multisolvePuzzles.count(puzzleId) == 0) {
		// Puzzle has already been solved and thus cannot be skipped.
		return false;
	}

	return true;
}

int APWatchdog::CalculatePuzzleSkipCost(int puzzleId, std::string& specialMessage) const {
	// Check for special cases.
	if (puzzleId == 0x03612) {
		// Quarry laser panel. Check for latches.
		bool leftLatchOpen = ReadPanelData<int>(0x288E9, DOOR_OPEN) != 0;
		bool rightLatchOpen = ReadPanelData<int>(0x28AD4, DOOR_OPEN) != 0;
		if (!leftLatchOpen && !rightLatchOpen) {
			specialMessage = "Skipping this panel costs 1 Puzzle Skip per unopened latch.";
			return 2;
		}
		else if (!leftLatchOpen || !rightLatchOpen) {
			specialMessage = "Skipping this panel costs 1 Puzzle Skip per unopened latch.";
			return 1;
		}
		else {
			specialMessage = "";
			return 1;
		}
	}
	else if (puzzleId == 0x1C349) {
		// Symmetry island upper panel. Check for latch.
		bool latchOpen = ReadPanelData<int>(0x28AE8, DOOR_OPEN) != 0;
		if (!latchOpen) {
			specialMessage = "Skipping this panel costs 2 Puzzle Skips while latched.";
			return 2;
		}
		else {
			specialMessage = "";
			return 1;
		}
	}
	else if (puzzleId == 0x09FDA) {
		// Metapuzzle. This can only be skipped if all child puzzles are skipped too.
		//   TODO: This flow here is a little confusing to read, because PuzzleIsSkippable returns true, but the cost
		//   returned here indicates that it isn't.
		for (int smallPuzzleId : {0x09FC1, 0x09F8E, 0x09F01, 0x09EFF}) {
			uint32_t statusColor = ReadPanelData<uint32_t>(smallPuzzleId, VIDEO_STATUS_COLOR);
			if (statusColor < PUZZLE_SKIPPED || statusColor > PUZZLE_SKIPPED_MAX) {
				specialMessage = "Skipping this panel requires skipping all the small puzzles.";
				return -1;
			}
		}

		return 1;
	}

	specialMessage = "";
	return 1;
}

int APWatchdog::GetAvailablePuzzleSkips() const {
	return std::max(0, foundPuzzleSkips - spentPuzzleSkips);
}

void APWatchdog::SkipPuzzle() {
	if (!CanUsePuzzleSkip()) {
		return;
	}

	spentPuzzleSkips += puzzleSkipCost;
	skipButtonHeldTime = 0.f;

	Special::SkipPanel(activePanelId, true);
	WritePanelData<__int32>(activePanelId, VIDEO_STATUS_COLOR, { PUZZLE_SKIPPED + puzzleSkipCost }); // Videos can't be skipped, so this should be safe.
}

void APWatchdog::SkipPreviouslySkippedPuzzles() {
	for (int id : actuallyEveryPanel) {
		__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

		if (skipped >= PUZZLE_SKIPPED && skipped <= PUZZLE_SKIPPED_MAX) {
			Special::SkipPanel(id);
			spentPuzzleSkips += skipped - PUZZLE_SKIPPED;
		}
		else if (skipped == COLLECTED) {
			Special::SkipPanel(id, "Collected", false);
		}
		else if (skipped == DISABLED) {
			//Panellocker will do it again anyway
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
			if (spentPuzzleSkips > foundPuzzleSkips) {
				// We've spent more skips than we've found, almost certainly because we cheated ourselves some in a
				//   previous app launch. Reset to zero before adding a new one.
				foundPuzzleSkips = spentPuzzleSkips;
			}

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
			OutputDebugStringW(L"All good.\n");
			continue;
		}
		if (PanelRestore::HasPositions(collisionToUpdate) && newPositions == PanelRestore::GetPositions(collisionToUpdate)) {
			OutputDebugStringW(L"All good.\n");
			continue;
		}

		std::wstringstream s;
		s << std::hex << collisionToUpdate << " didn't move!!! ";
		OutputDebugStringW(s.str().c_str());

		if (alreadyTriedUpdatingNormally.count(collisionToUpdate)) {
			OutputDebugStringW(L"Force-Updating...\n");

			if (PanelRestore::HasPositions(collisionToUpdate)) {
				WritePanelData<int>(collisionToUpdate, MOUNT_PARENT_ID, { 0 });
				WritePanelData<float>(collisionToUpdate, POSITION, PanelRestore::GetPositions(collisionToUpdate));
				_memory->UpdateEntityPosition(collisionToUpdate);
			}

			return;
		}

		try {  
			OutputDebugStringW(L"Updating using ingame function...\n");
			_memory->UpdateEntityPosition(collisionToUpdate);
			alreadyTriedUpdatingNormally.insert(collisionToUpdate);
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
			hudManager->clearSubtitleMessage();
			hudManager->showSubtitleMessage(message, 12.0f);

			APClient::DataStorageOperation operation;
			operation.operation = "replace";
			operation.value = true;

			std::list<APClient::DataStorageOperation> operations;
			operations.push_back(operation);

			int pNO = ap->get_player_number();

			ap->Set("WitnessAudioLog" + std::to_string(pNO) + "-" + std::to_string(logId), NULL, false, operations);

			int locationId = audioLogMessages[logId].second;

			if (locationId != -1 && !seenAudioMessages.count(locationId)) {
				ap->LocationScouts({ locationId }, 2);
				seenAudioMessages.insert(locationId);
			}
		}
		else if (!logPlaying && logId == currentAudioLog) {
			currentAudioLog = -1;
		}
	}
}

void APWatchdog::CheckEPs() {
	if (EPIDsToEPs.empty()) {
		int pNO = ap->get_player_number();

		for (int ep : allEPs) {
			std::string epID = "WitnessEP" + std::to_string(pNO) + "-" + std::to_string(ep);

			EPIDsToEPs[epID] = ep;
			EPStates[ep] = false;
			EPIDs.push_back(epID);

			APClient::DataStorageOperation operation;
			operation.operation = "default";
			operation.value = false;

			std::list<APClient::DataStorageOperation> operations;

			operations.push_back(operation);


			nlohmann::json a;

			ap->Set(epID, a, false, operations);

			EPIDs.push_back(epID);
		}

		ap->SetNotify(EPIDs);
		ap->Get(EPIDs);
	}

	for (auto [epID, ep] : EPIDsToEPs) {
		if (ReadPanelData<int>(ep, EP_SOLVED)) {
			if (!EPStates[ep]) {
				EPStates[ep] = true;

				APClient::DataStorageOperation operation;
				operation.operation = "replace";
				operation.value = true;

				std::list<APClient::DataStorageOperation> operations;
				operations.push_back(operation);

				ap->Set(epID, NULL, false, operations);
			}
		}
	}
}

void APWatchdog::CheckLasers() {
	if (laserIDsToLasers.empty()) {
		int pNO = ap->get_player_number();

		for (int laser : allLasers) {
			std::string laserID = "WitnessLaser" + std::to_string(pNO) + "-" + std::to_string(laser);

			laserIDsToLasers[laserID] = laser;
			laserStates[laser] = false;
			laserIDs.push_back(laserID);

			APClient::DataStorageOperation operation;
			operation.operation = "default";
			operation.value = false;

			std::list<APClient::DataStorageOperation> operations;

			operations.push_back(operation);


			nlohmann::json a;

			ap->Set(laserID, a, false, operations);

			laserIDs.push_back(laserID);
		}

		ap->SetNotify(laserIDs);
		ap->Get(laserIDs);
	}

	int laserCount = 0;
	
	for (auto [laserID, laser] : laserIDsToLasers) {
		if (ReadPanelData<int>(laser, LASER_TARGET)) {
			laserCount += 1;

			if (!laserStates[laser]) {
				laserStates[laser] = true;

				APClient::DataStorageOperation operation;
				operation.operation = "replace";
				operation.value = true;

				std::list<APClient::DataStorageOperation> operations;
				operations.push_back(operation);

				ap->Set(laserID, NULL, false, operations);
			}
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
}

void APWatchdog::HandleLaserResponse(std::string laserID, nlohmann::json value, bool collect) {
	int laserNo = laserIDsToLasers[laserID];

	bool laserActiveAccordingToDataPackage = value == true;

	bool laserActiveInGame = ReadPanelData<int>(laserNo, LASER_TARGET) != 0;

	if (laserActiveInGame == laserActiveAccordingToDataPackage) return;

	if(!laserActiveInGame)
	{
		if (laserNo == 0x012FB) _memory->OpenDoor(0x01317);
		_memory->ActivateLaser(laserNo);
		hudManager->queueBannerMessage(laserNames[laserNo] + " Laser Activated Remotely (Coop)");
	}
}

void APWatchdog::HandleEPResponse(std::string epID, nlohmann::json value, bool collect) {
	int epNo = EPIDsToEPs[epID];

	bool epActiveAccordingToDataPackage = value == true;

	bool epActiveInGame = ReadPanelData<int>(epNo, EP_SOLVED) != 0;

	if (epActiveInGame == epActiveAccordingToDataPackage) return;

	if (!epActiveInGame)
	{
		_memory->SolveEP(epNo);
		if (precompletableEpToName.count(epNo) && precompletableEpToPatternPointBytes.count(epNo)) {
			_memory->MakeEPGlow(precompletableEpToName.at(epNo), precompletableEpToPatternPointBytes.at(epNo));
		}
		hudManager->queueBannerMessage("EP Activated Remotely (Coop)"); //TODO: Names
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
		hudManager->setWorldMessage("Most Bunker panels need Black/White Squares and Colored Squares.");
	}
	else if (quarryLaserPanel.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x03612)) {
		if (PuzzleRandomization == SIGMA_EXPERT) hudManager->setWorldMessage("Needs Eraser, Triangles,\n"
											  "Stars, Stars + Same Colored Symbol.");
		else hudManager->setWorldMessage("Needs Shapers and Eraser.");
	}
	else if (symmetryUpperPanel.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x1C349)) {
		if (PuzzleRandomization == SIGMA_EXPERT) hudManager->setWorldMessage("Needs Symmetry and Triangles.");
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

		if ((skipped >= PUZZLE_SKIPPED && skipped <= PUZZLE_SKIPPED_MAX) || skipped == COLLECTED || skipped == DISABLED) {
			panelsToRemoveSilently.insert(panel);
			continue;
		}

		if (panel == 0x09E86 || panel == 0x09ED8) {
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

	if (!panelsToSkip.empty()) {
		hudManager->queueBannerMessage("Puzzle symbols removed to make Environmental Pattern solvable.");
	}

	for (int panel : panelsToSkip) {
		panelLocker->PermanentlyUnlockPuzzle(panel);
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

		hudManager->clearActionHint();
	}
	else if (interactionState == InteractionState::Focusing || interactionState == InteractionState::Solving) {
		if (activePanelId != -1) {
			hudManager->setStatusMessage(puzzleSkipInfoMessage);

			if (CanUsePuzzleSkip()) {
				if (puzzleSkipCost == 0) {
					hudManager->setActionHint("Hold [T] to use Puzzle Skip. (Free!)");
				}
				else if (puzzleSkipCost == 1) {
					hudManager->setActionHint("Hold [T] to use Puzzle Skip. (Have " + std::to_string(GetAvailablePuzzleSkips()) + ".)");
				}
				else {
					hudManager->setActionHint("Hold [T] to use Puzzle Skip. (Have " + std::to_string(GetAvailablePuzzleSkips()) + ", costs " + std::to_string(puzzleSkipCost) + ".)");
				}
			}
			else if (GetAvailablePuzzleSkips() > 0 && puzzleSkipCost > GetAvailablePuzzleSkips()) {
				// The player has some puzzle skips available, but not enough to solve the selected puzzle.
				hudManager->setActionHint("Insufficient Puzzle Skips. (Have " + std::to_string(GetAvailablePuzzleSkips()) + ", costs " + std::to_string(puzzleSkipCost) + ".)");
			}
			else if (GetAvailablePuzzleSkips() == 0 && puzzleSkipCost != -1) {
				// The player has no puzzle skips available, but the puzzle is skippable.
				hudManager->setActionHint("No Puzzle Skips available.");
			}
			else {
				// We can't use the puzzle skip for a different reason.
				hudManager->clearActionHint();
			}
		}
		else {
			hudManager->clearStatusMessage();
			hudManager->clearActionHint();
		}
	}
	else {
		hudManager->clearStatusMessage();
		hudManager->clearActionHint();
	}
}

void APWatchdog::LookingAtObelisk() {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState != InteractionState::Focusing) return;

	std::set<int> candidates;

	auto ray = InputWatchdog::get()->getMouseRay();

	std::vector<float> headPosition = ray.first;
	std::vector<float> cursorDirection = ray.second;

	for (int epID : allEPs) {
		std::vector<float> obeliskPosition = _memory->ReadPanelData<float>(epID, POSITION, 3);

		if (pow(headPosition[0] - obeliskPosition[0], 2) + pow(headPosition[1] - obeliskPosition[1], 2) + pow(headPosition[2] - obeliskPosition[2], 2) > 49) {
			continue;
		}

		std::vector<float> qData = ReadPanelData<float>(epID, ORIENTATION, 4);

		Quaternion q;
		q.x = qData[0];
		q.y = qData[1];
		q.z = qData[2];
		q.w = qData[3];

		std::vector<float> facing = { 1, 0, 0 };

		q.RotateVector(facing);

		float dotProduct = cursorDirection[0] * facing[0] + cursorDirection[1] * facing[1] + cursorDirection[2] * facing[2];

		if (dotProduct < 0) candidates.insert(epID);

		continue;
	}

	int lookingAtEP = -1;
	float distanceToCenter = 10000000.0f;

	for (int epID : candidates) {
		std::vector<float> epPosition = _memory->ReadPanelData<float>(epID, POSITION, 3);

		std::vector<float> v = { epPosition[0] - headPosition[0], epPosition[1] - headPosition[1], epPosition[2] - headPosition[2] };
		float t = v[0] * cursorDirection[0] + v[1] * cursorDirection[1] + v[2] * cursorDirection[2];
		std::vector<float> p = { headPosition[0] + t * cursorDirection[0], headPosition[1] + t * cursorDirection[1], headPosition[2] + t * cursorDirection[2] };
		
		float distance = sqrt(pow(p[0] - epPosition[0], 2) + pow(p[1] - epPosition[1], 2) + pow(p[2] - epPosition[2], 2));

		float boundingRadius = _memory->ReadPanelData<float>(epID, BOUNDING_RADIUS);

		if (distance < boundingRadius && distance < distanceToCenter) {
			distanceToCenter = distance;
			lookingAtEP = epID;
		}
	}

	if (epToName.count(lookingAtEP)) {
		hudManager->setWorldMessage(epToName[lookingAtEP]);
	}

	return;
}

void APWatchdog::LookingAtTheDog(float frameLength) {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState != InteractionState::Focusing) {
		letGoSinceInteractModeOpen = false;
		dogFrames = 0;
		_memory->writeCursorSize(1.0f);
		_memory->writeCursorColor({ 1.0f, 1.0f, 1.0f });
		return;
	}

	auto ray = InputWatchdog::get()->getMouseRay();

	std::vector<float> headPosition = ray.first;
	std::vector<float> cursorDirection = ray.second;

	std::vector<std::vector<float>> dogPositions = { {-107.954, -101.36, 4.078}, {-107.9, -101.312, 4.1},{-107.866, -101.232, 4.16},{ -107.871, -101.150, 4.22 }, { -107.86, -101.06, 4.3 }, { -107.88, -100.967, 4.397 } };

	float distanceToDog = pow(headPosition[0] - dogPositions[1][0], 2) + pow(headPosition[1] - dogPositions[1][1], 2) + pow(headPosition[2] - dogPositions[1][2], 2);

	if (distanceToDog > 49) {
		return;
	}

	float smallestDistance = 10000000.0f;
	float boundingRadius = 0.105f;

	for (auto dogPosition : dogPositions) {
		std::vector<float> v = { dogPosition[0] - headPosition[0], dogPosition[1] - headPosition[1], dogPosition[2] - headPosition[2] };
		float t = v[0] * cursorDirection[0] + v[1] * cursorDirection[1] + v[2] * cursorDirection[2];
		std::vector<float> p = { headPosition[0] + t * cursorDirection[0], headPosition[1] + t * cursorDirection[1], headPosition[2] + t * cursorDirection[2] };

		float distance = sqrt(pow(p[0] - dogPosition[0], 2) + pow(p[1] - dogPosition[1], 2) + pow(p[2] - dogPosition[2], 2));

		if (distance < smallestDistance) {
			smallestDistance = distance;
		}
	}

	if (smallestDistance > boundingRadius) {
		_memory->writeCursorSize(1.0f);
		_memory->writeCursorColor({ 1.0f, 1.0f, 1.0f });
		return;
	}

	_memory->writeCursorColor({ 1.0f, 0.5f, 1.0f });

	if (dogFrames >= 4.0f) {
		hudManager->setWorldMessage("Woof Woof!");

		sentDog = true;
	}
	else
	{
		hudManager->setWorldMessage("Petting progress: " + std::to_string((int)(dogFrames * 100 / 4.0f)) + "%");
	}

	if (!InputWatchdog::get()->getButtonState(InputButton::MOUSE_BUTTON_LEFT) && !InputWatchdog::get()->getButtonState(InputButton::CONTROLLER_FACEBUTTON_DOWN)) {
		letGoSinceInteractModeOpen = true;

		_memory->writeCursorSize(2.0f);
		return;
	}

	if (!letGoSinceInteractModeOpen) {
		_memory->writeCursorSize(2.0f);
		return;
	}

	_memory->writeCursorSize(1.0f);

	if (lastMousePosition != cursorDirection)
	{
		dogFrames += frameLength;
	}

	lastMousePosition = cursorDirection;
}