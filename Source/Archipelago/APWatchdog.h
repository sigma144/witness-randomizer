#pragma once

#include <map>
#include <chrono>
#include "..\Watchdog.h"
#include "..\Generate.h"
#include "APRandomizer.h"
#include "..\DateTime.h"

class APWatchdog : public Watchdog {
public:
	APWatchdog(APClient* client, std::map<int, int> mapping, int lastPanel, PanelLocker* p, HWND skipButton1, HWND availableSkips1) : Watchdog(1) {
		generator = std::make_shared<Generate>();
		ap = client;
		panelIdToLocationId = mapping;
		finalPanel = lastPanel;
		skipButton = skipButton1;
		availableSkips = availableSkips1;
		panelLocker = p;
	}

	int skippedPuzzles = 0;
	int availablePuzzleSkips = 0;

	HWND skipButton;
	HWND availableSkips;

	virtual void action();

	void MarkLocationChecked(int locationId);
	void ApplyTemporarySpeedBoost();
	void ApplyTemporarySlow();
	void TriggerPowerSurge();
	void ResetPowerSurge();

	void AddPuzzleSkip();
	void SkipPuzzle();
	void SkipPreviouslySkippedPuzzles();

	void UnlockDoor(int id);
	void SeverDoor(int id);

	void DoubleDoorTargetHack(int id);

	void queueMessage(std::string message) {
		outstandingMessages.push(message);
	}
private:
	APClient* ap;
	PanelLocker* panelLocker;
	std::shared_ptr<Generate> generator;
	std::map<int, int> panelIdToLocationId;
	int finalPanel;
	bool isCompleted = false;

	bool hasPowerSurge = false;
	std::chrono::system_clock::time_point powerSurgeStartTime;

	float baseSpeed = 2.0f;
	float currentSpeed = 2.0f;
	bool hasTemporarySpeedModdification = false;
	std::chrono::system_clock::time_point temporarySpeedModdificationStartTime;

	std::queue<std::string> outstandingMessages;
	int messageCounter = 0;

	std::set<int> disableCollisionList;

	std::set<int> severedDoorsList;
	std::map<int, int> refreshDoorsMap;

	void DisplayMessage();

	void DisableCollisions();

	void RefreshDoors();

	void CheckSolvedPanels();
	void HandleMovementSpeed();
	void HandlePowerSurge();
	void UpdateChallengeLock();
	boolean CheckIfCanSkipPuzzle();

	int GetActivePanel() {
		try {
			return _memory->GetActivePanel();
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Couldn't get active panel");
		}
	}

	void WriteMovementSpeed(float currentSpeed) {
		try {
			return _memory->WriteMovementSpeed(currentSpeed);
		}
		catch (std::exception& e) {
			OutputDebugStringW(L"Couldn't get active panel");
		}
	}
};

class APServerPoller : public Watchdog {
public:
	APServerPoller(APClient* client) : Watchdog(0.1f) {
		ap = client;
	}

	virtual void action() {
		ap->poll();
	}

private:
	APClient* ap;
};
