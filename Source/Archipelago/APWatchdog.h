#pragma once

#include <map>
#include <chrono>
#include "..\Watchdog.h"
#include "..\Generate.h"
#include "..\DateTime.h"
#include "nlohmann\json.hpp"
#include "APGameData.h"
#include "APState.h"
#include "PanelLocker.h"

struct AudioLogMessage {
	std::string line1;
	std::string line2;
	std::string line3;
	
	float time;

	bool countWhileOutprioritized;
};

class APWatchdog : public Watchdog {
public:
	APWatchdog(APClient* client, std::map<int, int> mapping, int lastPanel, PanelLocker* p, HWND skipButton1, HWND availableSkips1, std::map<int, std::pair<std::vector<std::string>, int64_t>> a, APState s) : Watchdog(0.5f) {
		generator = std::make_shared<Generate>();
		ap = client;
		panelIdToLocationId = mapping;
		finalPanel = lastPanel;
		skipButton = skipButton1;
		availableSkips = availableSkips1;
		panelLocker = p;
		audioLogMessages = a;
		state = s;
	}

	virtual void start();

	int skippedPuzzles = 0;
	int availablePuzzleSkips = 0;

	HWND skipButton;
	HWND availableSkips;

	APState state;

	virtual void action();

	void MarkLocationChecked(int locationId, bool collect);
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

	void HandleLaserResponse(const std::map<std::string, nlohmann::json> response, bool collect);
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

	std::map<int, AudioLogMessage> audioLogMessageBuffer;

	bool laserRequirementMet = false;

	std::set<int> disableCollisionList;

	std::set<int> severedDoorsList;
	std::map<int,int> collisionsToRefresh;
	std::map<int, std::vector<float>> collisionPositions;

	int storageCheckCounter = 6;

	void DisplayMessage();

	void DisableCollisions();

	void AudioLogPlaying();

	void RefreshDoorCollisions();

	void CheckSolvedPanels();
	void HandleMovementSpeed();
	void HandlePowerSurge();

	void CheckLasers();

	void CheckImportantCollisionCubes();

	std::map<std::string, int> laserIDsToLasers;
	std::list<std::string> laserIDs;

	int currentAudioLog = -1;

	std::map<int, std::pair<std::vector<std::string>, int64_t>> audioLogMessages = {};

	CollisionCube bonsaiCollisionCube = CollisionCube(18, -31.6f, 14, 21, -29, 17);
	CollisionCube riverVaultUpperCube = CollisionCube(52, -51, 19, 44, -47, 23);
	CollisionCube riverVaultLowerCube = CollisionCube(40, -56, 16, 46, -47, 20.5);
	CollisionCube bunkerPuzzlesCube = CollisionCube(161.2, -96.3, 5.8, 172.3, -101.1, 11.5);


	int CheckIfCanSkipPuzzle();

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

class AudioLogMessageDisplayer : public Watchdog {
public:
	AudioLogMessageDisplayer(std::map<int, AudioLogMessage>* audioLogMessageBuffer) : Watchdog(0.1f) {
		this->audioLogMessageBuffer = audioLogMessageBuffer;
	}
	virtual void action();

private:
	std::map<int, AudioLogMessage>* audioLogMessageBuffer;
};
