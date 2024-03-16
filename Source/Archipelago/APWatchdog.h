#pragma once


#include "../Watchdog.h"
#include "APGameData.h"
#include "APWitnessData.h"
#include "APState.h"
#include "nlohmann/json.hpp"
#include "Client/apclientpp/apclient.hpp"

#include <chrono>
#include <map>
#include <queue>

class Generate;
class HudManager;
class PanelLocker;


class APWatchdog : public Watchdog {
public:
	APWatchdog(APClient* client, std::map<int, int> mapping, int lastPanel, PanelLocker* p, std::map<int, std::string> epn, std::map<int, inGameHint> a, std::map<int, std::set<int>> o, bool ep, int puzzle_rando, APState* s, float smsf, bool elev, std::string col, std::string dis, std::set<int> disP, std::map<int, std::set<int>> iTD, std::map<int, std::vector<int>> pI, int dlA, std::map<int, int> dToI);

	int DEATHLINK_DURATION = 15;

	int spentPuzzleSkips = 0;
	int foundPuzzleSkips = 0;

	APState* state;

	virtual void action();

	void MarkLocationChecked(int64_t locationId);
	void ApplyTemporarySpeedBoost();
	void ApplyTemporarySlow();
	void TriggerPowerSurge();
	void ResetPowerSurge();

	void TriggerBonk();
	void ResetDeathLink();

	void StartRebindingKey(enum class CustomKey key);

	void QueueItem(APClient::NetworkItem);
	void HandleReceivedItems();

	void AddPuzzleSkip();
	void SkipPuzzle();
	void SkipPreviouslySkippedPuzzles();

	void UnlockDoor(int id);
	void SeverDoor(int id);

	void SkipPanel(int id, std::string reason, bool kickOut) {
		SkipPanel(id, reason, kickOut, 0);
	}
	void SkipPanel(int id, std::string reason, bool kickOut, int cost);

	void DisablePuzzle(int id);

	void DoubleDoorTargetHack(int id);

	void SetItemRewardColor(const int& id, const int& itemFlags);
	void PlaySentJingle(const int& id, const int& itemFlags);
	void PlayReceivedJingle(const int& itemFlags);
	void PlayFirstJingle();

	bool CheckPanelHasBeenSolved(int panelId);

	void SetValueFromServer(std::string key, nlohmann::json value);
	void HandleLaserResponse(std::string laserID, nlohmann::json value, bool syncProgress);
	void HandleEPResponse(std::string epID, nlohmann::json value, bool syncProgress);
	void HandleAudioLogResponse(std::string logIDstr, nlohmann::json value, bool syncprogress);
	void HandleLaserHintResponse(std::string laserIDstr, nlohmann::json value, bool syncprogress);
	void HandleSolvedPanelsResponse(nlohmann::json value, bool syncProgress);
	void HandleOpenedDoorsResponse(nlohmann::json value, bool syncProgress);
	void setLocationItemFlag(int64_t location, unsigned int flags);

	void InfiniteChallenge(bool enable);

	void ProcessDeathLink(double time, std::string cause, std::string source);

	bool newItemsJustIn = false;

	void QueueReceivedItem(std::vector<__int64> item);

	HudManager* getHudManager() const { return hudManager.get(); }

	std::set<int> seenAudioLogs;
	std::set<int> seenLasers;

private:
	APClient* ap;
	PanelLocker* panelLocker;
	std::shared_ptr<Generate> generator;
	std::map<int, int> panelIdToLocationId;
	std::set<int> panelsThatAreLocations;
	std::map<int, int> locationIdToPanelId_READ_ONLY;
	std::map<int64_t, unsigned int> locationIdToItemFlags;
	std::set<int64_t> checkedLocations;
	std::set<std::pair<int, int64_t>> locationsThatContainedItemsFromOtherPlayers;
	int finalPanel;
	bool isCompleted = false;
	bool desertLaserHasBeenUpWhileConnected = false;

	std::queue<APClient::NetworkItem> queuedReceivedItems = {};

	std::chrono::system_clock::time_point lastFrameTime;
	float halfSecondCountdown = 0.f;
	float timePassedSinceRandomisation = 0.0f;
	float timePassedSinceFirstJinglePlayed = 0.0f;

	int halfSecondCounter = 0;

	int mostRecentItemId = -1;

	std::map<int, std::set<int>> itemIdToDoorSet;
	std::map<int, std::vector<int>> progressiveItems;

	std::shared_ptr<HudManager> hudManager;

	int DeathLinkAmnesty = 0;
	int DeathLinkCount = 0;

	bool ElevatorsComeToYou = false;
	bool EPShuffle = false;
	int PuzzleRandomization = 0;

	std::string Collect = "Unchanged";
	std::string CollectText = "Unchanged";
	bool CollectUnlock = false;
	std::string DisabledPuzzlesBehavior = "Prevent Solve";
	std::set<int> DisabledEntities;

	std::map<int, int> doorToItemId;

	bool FirstEverLocationCheckDone = false;
	bool firstStorageCheckDone = false;
	bool firstJinglePlayed = false;

	bool hasPowerSurge = false;
	bool hasDeathLink = false;
	std::chrono::system_clock::time_point powerSurgeStartTime;
	std::chrono::system_clock::time_point deathLinkStartTime;

	std::set<double> deathLinkTimestamps;

	bool laserRequirementMet = false;

	std::set<int> disableCollisionList;

	std::set<int> severedDoorsList;
	std::map<int, std::vector<float>> collisionPositions;
	std::set<int> alreadyTriedUpdatingNormally;

	int storageCheckCounter = 6;

	float speedTime = 0.0f;
	float solveModeSpeedFactor = 0.0f;

	bool infiniteChallenge = false;
	bool insideChallengeBoxRange = true;

	bool symmetryMessageDelivered = false;
	bool ppMessageDelivered = false;

	void HandleKeyTaps();

	void HandleInteractionState();
	
	void CheckEPSkips();

	void CheckDeathLink();
	void SendDeathLink(int panelId);

	void QueueItemMessages();

	void DisableCollisions();

	void HandleInGameHints(float deltaSeconds);

	void CheckSolvedPanels();
	void HandleMovementSpeed(float deltaSeconds);
	void HandlePowerSurge();
	void HandleDeathLink();
	void HandleVision(float deltaSeconds);

	void LookingAtObelisk();

	void PettingTheDog(float deltaSeconds);
	bool LookingAtTheDog() const;

	// Updates puzzle skip logic.
	void UpdatePuzzleSkip(float deltaSeconds);

	// Whether or not the player can use a puzzle skip. Checks if the player has a
	//   puzzle selected, if they can afford the cost, etc.
	bool CanUsePuzzleSkip() const;

	// Whether or not the puzzle is in a skippable state (unlocked, not skipped, etc.),
	//   NOT whether or not the player can skip the puzzle.
	bool PuzzleIsSkippable(int puzzleId) const;

	// Computes the cost to skip the puzzle. If the cost is unusual, will set
	//   specialMessage to indicate the reason why.
	int CalculatePuzzleSkipCost(int puzzleId, std::string& specialMessage) const;

	// Returns the number of skips currently available to the player.
	int GetAvailablePuzzleSkips() const;

	void CheckLasers();
	void CheckEPs();
	void CheckPanels();
	void CheckDoors();

	void CheckImportantCollisionCubes();

	void SetStatusMessages();

	int GetActivePanel();
	void WriteMovementSpeed(float currentSpeed);

	void UpdateInfiniteChallenge();

	void unlockItem(int item);

	void CheckFinalRoom();

	void DoAprilFoolsEffects(float deltaSeconds);

	std::map<std::string, int> laserIDsToLasers;
	std::list<std::string> laserIDs;
	std::map<int, bool> laserStates;
	std::map<int, std::string> laserMessages;

	std::set<int> solvedPanels;
	std::set<int> openedDoors;
	std::map<std::string, bool> lastDeadChecks;

	std::map<std::string, int> EPIDsToEPs;
	std::list<std::string> EPIDs;
	std::map<int, bool> EPStates;

	std::set<int> PuzzlesSkippedThisGame = {};

	int currentHintEntity = -1;
	int lastRealHintEntity = -1;
	int lastLaser = -1;
	int lastAudioLog = -1;
	float currentHintEntityDuration = 0.f;

	int activePanelId = -1;
	int mostRecentActivePanelId = -1;
	int mostRecentPanelState = -1;

	std::string puzzleSkipInfoMessage;
	float skipButtonHeldTime = 0.f; // Tracks how long the skip button has been held.

	// The cost to skip the currently-selected puzzle. -1 if the puzzle cannot be
	//   skipped for whatever reason.
	int puzzleSkipCost = -1;

	std::map<int, inGameHint> inGameHints = {};
	std::map<int, std::set<int>> obeliskHexToEPHexes = {};
	std::map<int, int> obeliskHexToAmountOfEPs = {};
	std::map<int, std::string> entityToName = {};

	Vector3 lastMouseDirection;
	float dogPettingDuration = 0.f;
	float dogBarkDuration = 0.f;
	bool sentDog = false;
	bool letGoSinceInteractModeOpen = false;

	double finalRoomMusicTimer = -1;

	std::set<int> panelsThatHaveToBeSkippedForEPPurposes = {};

	bool metaPuzzleMessageHasBeenDisplayed = false;

	std::vector<std::vector<__int64>> queuedItems;
};

class APServerPoller : public Watchdog {
public:

	APServerPoller(APClient* client);
	virtual void action();

private:
	APClient* ap;
};
