#include "APWatchdog.h"
#include "..\Quaternion.h"
#include <thread>
#include "../Panels.h"
#include "SkipSpecialCases.h"
#include "DoorSevers.h"

void APWatchdog::action() {
	CheckSolvedPanels();
	HandleMovementSpeed();
	HandlePowerSurge();
	UpdateChallengeLock();
	CheckIfCanSkipPuzzle();
	DisplayMessage();
	DisableCollisions();
	RefreshDoorCollisions();
}

void APWatchdog::CheckSolvedPanels() {
	std::list<int64_t> solvedLocations;

	if (finalPanel != 0x09F7F && finalPanel != 0xFFF00 && ReadPanelData<int>(finalPanel, SOLVED) && !isCompleted) {
		isCompleted = true;
		ap->StatusUpdate(APClient::ClientStatus::GOAL);
	}
	if (finalPanel == 0x09F7F && !isCompleted)
	{
		float power = ReadPanelData<float>(0x17C34, POWER);

		if (power != 0.0f) {
			isCompleted = true;
			ap->StatusUpdate(APClient::ClientStatus::GOAL);
		}
	}
	if (finalPanel == 0xFFF00 && !isCompleted)
	{
		float power = ReadPanelData<float>(0x17FA2, POWER);

		if (power != 0.0f) {
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
			int CABLE_POWER = 0xC4;

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

void APWatchdog::MarkLocationChecked(int locationId)
{
	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		if (it->second == locationId)
			it = panelIdToLocationId.erase(it);
		else
			it++;
	}
}

void APWatchdog::ApplyTemporarySpeedBoost() {
	temporarySpeedModdificationStartTime = std::chrono::system_clock::now();
	currentSpeed = baseSpeed * 2.0f;
	hasTemporarySpeedModdification = true;
}

void APWatchdog::ApplyTemporarySlow() {
	temporarySpeedModdificationStartTime = std::chrono::system_clock::now();
	currentSpeed = baseSpeed * 0.4f;
	hasTemporarySpeedModdification = true;
}

void APWatchdog::HandleMovementSpeed() {
	if (hasTemporarySpeedModdification && DateTime::since(temporarySpeedModdificationStartTime).count() > 30000) {
		hasTemporarySpeedModdification = false;

		currentSpeed = baseSpeed;
	}

	WriteMovementSpeed(currentSpeed);
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

void APWatchdog::UpdateChallengeLock() {
	float power = ReadPanelData<float>(0x17FA2, POWER); //Challenge is supposed to unlock when the long solution of the box is solved, which activates the Mountain Bottom Layer Discard

	if (power != 0.0f) {
		WritePanelData<float>(0x0A332, POWER, { 1.0, 1.0 });
	}
	else
	{
		WritePanelData<float>(0x0A332, POWER, { 0.0, 0.0 });
	}
}

boolean APWatchdog::CheckIfCanSkipPuzzle() {
	SetWindowText(availableSkips, (L"Available Skips: " + std::to_wstring(availablePuzzleSkips - skippedPuzzles)).c_str());

	if (availablePuzzleSkips <= skippedPuzzles) {
		EnableWindow(skipButton, false);
		return false;
	}

	int id = GetActivePanel();

	if (std::find(actuallyEveryPanel.begin(), actuallyEveryPanel.end(), id) == actuallyEveryPanel.end()) {
		return false;
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
		return false;
	}

	__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

	if (skipped == PUZZLE_SKIPPED) {
		EnableWindow(skipButton, false);
		return false;
	}

	EnableWindow(skipButton, true);
	return true;
}

void APWatchdog::SkipPuzzle()
{
	int id = GetActivePanel();

	if (!CheckIfCanSkipPuzzle()) return;

	skippedPuzzles++;

	Special::SkipPanel(id);

	WritePanelData<__int32>(id, VIDEO_STATUS_COLOR, { PUZZLE_SKIPPED }); // Videos can't be skipped, so this should be safe.

	CheckIfCanSkipPuzzle();
}

void APWatchdog::SkipPreviouslySkippedPuzzles() {
	for (int id : actuallyEveryPanel) {
		__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

		if (skipped == PUZZLE_SKIPPED) {
			Special::SkipPanel(id);
			skippedPuzzles++;
		}
	}
}

void APWatchdog::AddPuzzleSkip() {
	availablePuzzleSkips++;
}

void APWatchdog::UnlockDoor(int id) {
	if (id == 0x0CF2A) {
		disableCollisionList.insert(id);

		_memory->WritePanelData<float>(0x17CAA, POSITION, { 36.694f, -41.883f, 16.570f });
		_memory->WritePanelData<float>(0x17CAA, SCALE, { 1.2f });

		_memory->UpdateEntityPosition(0x17CAA);
	}

	if (id == 0x0C310) {
		_memory->WritePanelData<float>(0x02886, POSITION + 8, { 12.8f });

		_memory->UpdateEntityPosition(0x02886);
	}

	if (id == 0x2D73F) {
		_memory->WritePanelData<float>(0x021D7, POSITION + 8, { 10.0f });

		_memory->UpdateEntityPosition(0x021D7);
	}

	for (int collisionId : doorCollisions[id]) {
		collisionsToRefresh.insert(collisionId);
	}

	_memory->OpenDoor(id);
}

void APWatchdog::SeverDoor(int id) {
	if (severTargetsById.count(id)) {
		severedDoorsList.insert(id);

		if (id == 0x01A0E) {
			_memory->WritePanelData<int>(0x01A0F, TARGET, { 0x0360E + 1 });
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
	
	messageCounter = 6;
}

void APWatchdog::DisableCollisions() {
	for (int id : disableCollisionList) {
		_memory->RemoveMesh(id);
	}
}

void APWatchdog::RefreshDoorCollisions() {
	std::wstringstream s;

	for (int collisionId : collisionsToRefresh) {
		s << "0x" << std::hex << collisionId << ", ";
		_memory->UpdateEntityPosition(collisionId);
	}
	s << "\n";
	OutputDebugStringW(s.str().c_str());
}