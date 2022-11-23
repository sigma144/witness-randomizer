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

struct HudMessage {
	std::string text;
	std::array<float, 3> rgbColor;
};

struct AudioLogMessage {
	std::string line1;
	std::string line2;
	std::string line3;
	
	float time;

	bool countWhileOutprioritized;
};

class APWatchdog : public Watchdog {
public:
	APWatchdog(APClient* client, std::map<int, int> mapping, int lastPanel, PanelLocker* p, HWND skipButton1, HWND availableSkips1, std::map<int, std::pair<std::vector<std::string>, int64_t>> a, std::map<int, std::set<int>> o, bool ep, bool hard, APState* s) : Watchdog(0.1f) {
		generator = std::make_shared<Generate>();
		ap = client;
		panelIdToLocationId = mapping;
		finalPanel = lastPanel;
		skipButton = skipButton1;
		availableSkips = availableSkips1;
		panelLocker = p;
		audioLogMessages = a;
		state = s;
		EPShuffle = ep;
		obeliskHexToEPHexes = o;

		for (auto [key, value] : obeliskHexToEPHexes) {
			obeliskHexToAmountOfEPs[key] = value.size();
		}

		Hard = hard;

		panelsThatHaveToBeSkippedForEPPurposes = {
			0x09E86, 0x09ED8, // light controllers 2 3
			0x033EA, 0x01BE9, 0x01CD3, 0x01D3F, // Pressure Plates
		};

		if (hard) {
			panelsThatHaveToBeSkippedForEPPurposes.insert(0x181F5);
			panelsThatHaveToBeSkippedForEPPurposes.insert(0x334D8);
		}
	}

	int skippedPuzzles = 0;
	int availablePuzzleSkips = 0;

	HWND skipButton;
	HWND availableSkips;

	APState* state;

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

	void SetItemReward(const int& id, const APClient::NetworkItem& item);

	void queueMessage(std::string text) {
		HudMessage message;
		message.text = text;
		message.rgbColor = { 1.0f, 1.0f, 1.0f };

		outstandingMessages.push(message);
	}

	void queueMessage(std::string text, std::array<float, 3> rgbColor) {
		HudMessage message;
		message.text = text;
		message.rgbColor = rgbColor;

		outstandingMessages.push(message);
	}

	bool CheckPanelHasBeenSolved(int panelId);

	void HandleLaserResponse(const std::map<std::string, nlohmann::json> response, bool collect);

	void InfiniteChallenge(bool enable) {
		_memory->SetInfiniteChallenge(enable);

		if (enable) queueMessage("Challenge Timer disabled.");
		if (!enable) queueMessage("Challenge Timer reenabled.");
	}
private:
	APClient* ap;
	PanelLocker* panelLocker;
	std::shared_ptr<Generate> generator;
	std::map<int, int> panelIdToLocationId;
	int finalPanel;
	bool isCompleted = false;

	int halfSecondCounter = 0;

	bool EPShuffle = false;
	bool Hard = false;

	bool FirstEverLocationCheckDone = false;

	bool hasPowerSurge = false;
	std::chrono::system_clock::time_point powerSurgeStartTime;

	float baseSpeed = 2.0f;
	float currentSpeed = 2.0f;
	bool hasTemporarySpeedModdification = false;
	std::chrono::system_clock::time_point temporarySpeedModificationTime;
	bool hasEverModifiedSpeed = false;

	std::queue<HudMessage> outstandingMessages;
	int messageCounter = 0;

	std::map<int, AudioLogMessage> audioLogMessageBuffer;

	bool laserRequirementMet = false;

	std::set<int> disableCollisionList;

	std::set<int> severedDoorsList;
	std::map<int,int> collisionsToRefresh;
	std::map<int, std::vector<float>> collisionPositions;

	int storageCheckCounter = 6;

	float speedTime = 0.0f;

	void CheckEPSkips();

	void DisplayMessage();

	void DisableCollisions();

	void AudioLogPlaying();
	void DisplayAudioLogMessage();

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
	std::map<int, std::set<int>> obeliskHexToEPHexes = {};
	std::map<int, int> obeliskHexToAmountOfEPs = {};

	CollisionCube bonsaiCollisionCube = CollisionCube(18, -31.6f, 14, 21, -29, 17);
	CollisionCube riverVaultUpperCube = CollisionCube(52, -51, 19, 44, -47, 23);
	CollisionCube riverVaultLowerCube = CollisionCube(40, -56, 16, 46, -47, 20.5);
	CollisionCube bunkerPuzzlesCube = CollisionCube(161.2, -96.3, 5.8, 172.3, -101.1, 11.5);
	CollisionCube tutorialPillarCube = CollisionCube(-152, -150.9, 5, -148, -154.8, 9);
	CollisionCube quarryLaserPanel = CollisionCube(-59, 90, 17, -67, 100, 21);
	CollisionCube symmetryUpperPanel = CollisionCube(-180, 31, 12.6, -185, 37, 17);

	std::set<int> panelsThatHaveToBeSkippedForEPPurposes = {};

	bool metaPuzzleMessageHasBeenDisplayed = false;


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