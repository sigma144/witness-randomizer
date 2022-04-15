#include "APWatchdog.h"
#include "..\Quaternion.h"
#include "..\Special.h"
#include <thread>

void APWatchdog::action() {
	CheckSolvedPanels();
	HandleMovementSpeed();
	HandlePowerSurge();
}

void APWatchdog::CheckSolvedPanels() {
	std::list<int64_t> solvedLocations;
	bool isCompleted = false;

	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		int panelId = it->first;
		int locationId = it->second;

		if (ReadPanelData<int>(panelId, SOLVED))
		{
			if (panelId == 0x3D9A9)
				isCompleted = true;			

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

	if (isCompleted)
		ap->StatusUpdate(APClient::ClientStatus::GOAL);
}

void APWatchdog::MarkLocationChecked(int locationId)
{
	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		if (it->second == locationId)
		{
			it = panelIdToLocationId.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void APWatchdog::ApplyTemporarySpeedBoost() {
	temporarySpeedModdificationStartTime = std::chrono::system_clock::now();
	currentSpeed = 2.5f;
	hasTemporarySpeedModdification = true;
}

void APWatchdog::ApplyTemporarySlow() {
	temporarySpeedModdificationStartTime = std::chrono::system_clock::now();
	currentSpeed = 0.4f;
	hasTemporarySpeedModdification = true;
}

void APWatchdog::HandleMovementSpeed() {
	if (hasTemporarySpeedModdification && DateTime::since(temporarySpeedModdificationStartTime).count() > 30000) {
		hasTemporarySpeedModdification = false;

		currentSpeed = baseSpeed;
	}

	_memory->WriteMovementSpeed(currentSpeed);
}

void APWatchdog::TriggerPowerSurge() {
	if (!hasPowerSurge)
	{
		for (const auto& panelId : AllPuzzles) {
			if (ReadPanelData<int>(panelId, SOLVED))
				continue;

			std::vector<float> powerValues = ReadPanelData<float>(panelId, POWER, 2);

			if (powerValues[0] > 0) {
				powerSurgedPanels.insert({ panelId, powerValues });

				WritePanelData<float>(panelId, POWER, { 0.0f, 0.0f });
			}
		}
	}

	powerSurgeStartTime = std::chrono::system_clock::now();
	hasPowerSurge = true;
}

void APWatchdog::HandlePowerSurge() {
	if (hasPowerSurge && DateTime::since(powerSurgeStartTime).count() > 10000)
	{
		hasPowerSurge = false;

		for (auto& [panelId, powerValues] : powerSurgedPanels) {
			WritePanelData<float>(panelId, POWER, powerValues);
		}
	}
}
