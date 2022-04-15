#pragma once

#include <map>
#include <chrono>
#include "..\Watchdog.h"
#include "..\Generate.h"
#include "APRandomizer.h"
#include "..\DateTime.h"

class APWatchdog : public Watchdog {
public:
	APWatchdog(APClient* client, std::map<int, int> mapping) : Watchdog(1) {
		generator = std::make_shared<Generate>();
		ap = client;
		panelIdToLocationId = mapping;
	}

	virtual void action();

	void MarkLocationChecked(int locationId);
	void ApplyTemporarySpeedBoost();
	void ApplyTemporarySlow();
	void TriggerPowerSurge();

private:
	APClient* ap;
	std::shared_ptr<Generate> generator;
	std::map<int, int> panelIdToLocationId;

	bool hasPowerSurge = false;
	std::chrono::system_clock::time_point powerSurgeStartTime;
	std::map<int, std::vector<float>> powerSurgedPanels;

	float baseSpeed = 2.0f;
	float currentSpeed = 2.0f;
	bool hasTemporarySpeedModdification = false;
	std::chrono::system_clock::time_point temporarySpeedModdificationStartTime;

	void CheckSolvedPanels();
	void HandleMovementSpeed();
	void HandlePowerSurge();
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