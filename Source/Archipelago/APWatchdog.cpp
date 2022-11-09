#include "APWatchdog.h"
#include "..\Quaternion.h"
#include <thread>
#include "../Panels.h"
#include "SkipSpecialCases.h"

void APWatchdog::action() {
	HandleMovementSpeed();
	DisplayAudioLogMessage();

	if (halfSecondCounter > 0) {
		halfSecondCounter -= 1;
		return;
	}
	else
	{
		halfSecondCounter = 4;
	}

	CheckSolvedPanels();

	HandlePowerSurge();
	CheckIfCanSkipPuzzle();
	DisplayMessage();
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

void APWatchdog::DisplayAudioLogMessage() {
	bool displayingMessage = false;

	auto it = audioLogMessageBuffer.begin();
	while (it != audioLogMessageBuffer.end())
	{
		int priority = it->first;
		AudioLogMessage* message = &it->second;



		if (!displayingMessage || message->countWhileOutprioritized) message->time -= 0.1f;

		if (message->time < 0) {
			it = audioLogMessageBuffer.erase(it);
			continue;
		}
		else
		{
			if (!displayingMessage) _memory->DisplaySubtitles(message->line1, message->line2, message->line3);
			displayingMessage = true;
		}

		++it;
	}

	if (!displayingMessage) {
		_memory->DisplaySubtitles("", "", "");
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
}

void APWatchdog::MarkLocationChecked(int locationId, bool collect)
{
	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		if (it->second == locationId) {
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
					WritePanelData<float>(it->first, POWER, { 1.0f, 1.0f });
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
	speedTime += 30.0f;
}

void APWatchdog::ApplyTemporarySlow() {
	speedTime -= 30.0f;
}


void APWatchdog::HandleMovementSpeed() {
	if (!hasEverModifiedSpeed) {
		temporarySpeedModificationTime = std::chrono::system_clock::now();
		hasEverModifiedSpeed = true;
	}

	auto rightnow = std::chrono::system_clock::now();

	std::chrono::duration<float> elapsed_seconds = rightnow - temporarySpeedModificationTime;
	float secondsInFloat = elapsed_seconds.count();

	temporarySpeedModificationTime = rightnow;

	if (speedTime == 0.0f) {
		WriteMovementSpeed(baseSpeed);
		audioLogMessageBuffer[100000] = { "", "", "", 0.8f, true };
		return;
	}

	int speedTimeInt = (int) std::round((std::abs(speedTime) + 0.49f));

	if (speedTime > 0.0f) {
		audioLogMessageBuffer[100000] = { "", "", "Speed Boost active for " + std::to_string(speedTimeInt) + " seconds.", 0.8f, true};

		WriteMovementSpeed(2.0f * baseSpeed);
		if (speedTime - secondsInFloat < 0.0f) speedTime = 0.0f;
		else speedTime -= secondsInFloat;
		return;
	}
	
	if (speedTime < 0.0f) {
		audioLogMessageBuffer[100000] = { "", "", "Slowness active for " + std::to_string(speedTimeInt) + " seconds.", 0.8f, true };

		WriteMovementSpeed(0.6f * baseSpeed);
		if (speedTime + secondsInFloat > 0.0f) speedTime = 0.0f;
		else speedTime += secondsInFloat;
		return;
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
	SetWindowText(availableSkips, (L"Available Skips: " + std::to_wstring(availablePuzzleSkips - skippedPuzzles)).c_str());



	int id = GetActivePanel();

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
			audioLogMessageBuffer[49] = { "", "Skipping this panel costs", "1 Puzzle Skip per unopened latch.", 0.8f, true };
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
			audioLogMessageBuffer[49] = { "", "Skipping this panel costs", "2 Puzzle Skips.", 0.8f, true };
		}

		cost = 2;
	}

	if (id == 0x09FDA) { // Metapuzzle
		for (int id : {0x09FC1, 0x09F8E, 0x09F01, 0x09EFF}) {
			__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

			if (skipped < PUZZLE_SKIPPED || skipped > PUZZLE_SKIPPED_MAX) {
				if(!metaPuzzleMessageHasBeenDisplayed) audioLogMessageBuffer[49] = { "", "Skipping this panel requires", "Skipping all the small puzzles.", 8.0f, true };
				metaPuzzleMessageHasBeenDisplayed = true;
				EnableWindow(skipButton, false);
				return -1;
				break;
			}
		}

		cost = 1;
	}

	// Cost Evaluated

	if (availablePuzzleSkips - cost < skippedPuzzles) {
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
	availablePuzzleSkips++;
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

void APWatchdog::DisplayMessage() {
	if (messageCounter > 0) {
		messageCounter--;
		return;
	}

	if (outstandingMessages.empty()) return;

	std::string message = outstandingMessages.front();
	outstandingMessages.pop();
	_memory->DisplayHudMessage(message);
	
	messageCounter = 8;
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

	std::wstringstream s;
	s << playerPosition[0] << ", " << playerPosition[1] << ", " << playerPosition[2] << "\n";
	OutputDebugStringW(s.str().c_str());

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

	OutputDebugStringW(L"-----\n");

	//Updates a tenth of the collisions every second. Will investigate if this needs to be increased potentially.
}

void APWatchdog::AudioLogPlaying() {
	std::string line1 = "";
	std::string line2 = "";
	std::string line3 = "";

	for (int id : audioLogs) {
		if (ReadPanelData<int>(id, AUDIO_LOG_IS_PLAYING)) {
			if (currentAudioLog != id) {
				currentAudioLog = id;

				if (audioLogMessages.count(id)) {
					std::vector<std::string> message = audioLogMessages[id].first;

					audioLogMessageBuffer[100] = { message[0], message[1], message[2], 12.0f, true};
				}
				else if (audioLogMessageBuffer.count(100)) {
					audioLogMessageBuffer.erase(100);
				}
			}

			return;
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
			queueMessage(laserNames[laserNo] + " Laser Activated Remotely (Coop)");
		}
	}
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
		audioLogMessageBuffer[50] = { "", "Needs Dots.", "", 0.8f, false };
	}

	if (tutorialPillarCube.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0xc335)) {
		audioLogMessageBuffer[50] = { "", "Stone Pillar needs Triangles.", "", 0.8f, false };
	}

	if ((riverVaultLowerCube.containsPoint(playerPosition) || riverVaultUpperCube.containsPoint(playerPosition)) && panelLocker->PuzzleIsLocked(0x15ADD)) {
		audioLogMessageBuffer[50] = { "", "Needs Dots, Black/White Squares.", "", 0.8f, false };
	}

	if (bunkerPuzzlesCube.containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x09FDC)) {
		audioLogMessageBuffer[50] = { "", "Panels in the Bunker need", "Black/White Squares, Colored Squares.", 0.8f, false };
	}
}