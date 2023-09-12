#pragma once


#include "../Watchdog.h"
#include "APGameData.h"
#include "APState.h"
#include "nlohmann/json.hpp"

#include <chrono>
#include <map>

class APClient;
class Generate;
class HudManager;
class PanelLocker;


class APWatchdog : public Watchdog {
public:
	APWatchdog(APClient* client, std::map<int, int> mapping, int lastPanel, PanelLocker* p, std::map<int, std::string> epn, std::map<int, std::pair<std::string, int64_t>> a, std::map<int, std::set<int>> o, bool ep, int puzzle_rando, APState* s, float smsf, bool dl, bool elev, std::string col, std::string dis, std::set<int> disP);

	int spentPuzzleSkips = 0;
	int foundPuzzleSkips = 0;

	APState* state;

	virtual void action();

	void MarkLocationChecked(int locationId);
	void ApplyTemporarySpeedBoost();
	void ApplyTemporarySlow();
	void TriggerPowerSurge();
	void ResetPowerSurge();

	void StartRebindingKey(enum class CustomKey key);

	void AddPuzzleSkip();
	void SkipPuzzle();
	void SkipPreviouslySkippedPuzzles();

	void UnlockDoor(int id);
	void SeverDoor(int id);

	void SkipPanel(int id, std::string reason, bool kickOut) {
		SkipPanel(id, reason, kickOut, 0);
	}
	void SkipPanel(int id, std::string reason, bool kickOut, int cost);

	void DoubleDoorTargetHack(int id);

	void SetItemRewardColor(const int& id, const int& itemFlags);
	void PlaySentJingle(const int& id, const int& itemFlags);
	void PlayReceivedJingle(const int& itemFlags);

	bool CheckPanelHasBeenSolved(int panelId);

	void HandleLaserResponse(std::string laserID, nlohmann::json value, bool collect);
	void HandleEPResponse(std::string epID, nlohmann::json value, bool collect);

	void InfiniteChallenge(bool enable);

	void ProcessDeathLink(double time, std::string cause, std::string source);


	bool processingItemMessages = false;
	bool newItemsJustIn = false;

	void QueueReceivedItem(std::vector<__int64> item);

	HudManager* getHudManager() const { return hudManager.get(); }

	std::set<int> seenAudioMessages;

private:
	APClient* ap;
	PanelLocker* panelLocker;
	std::shared_ptr<Generate> generator;
	std::map<int, int> panelIdToLocationId;
	std::map<int, int> locationIdToPanelId_READ_ONLY;
	int finalPanel;
	bool isCompleted = false;

	std::chrono::system_clock::time_point lastFrameTime;
	float halfSecondCountdown = 0.f;
	float timePassedSinceRandomisation = 0.0f;

	int halfSecondCounter = 0;

	std::shared_ptr<HudManager> hudManager;

	bool DeathLink = false;
	bool ElevatorsComeToYou = false;
	bool EPShuffle = false;
	int PuzzleRandomization = 0;

	std::string Collect = "Unchanged";
	std::string DisabledPuzzlesBehavior = "Prevent Solve";
	std::set<int> DisabledEntities;

	bool FirstEverLocationCheckDone = false;

	bool hasPowerSurge = false;
	std::chrono::system_clock::time_point powerSurgeStartTime;

	std::set<double> deathLinkTimestamps;

	const float baseSpeed = 2.0f;

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

	void AudioLogPlaying(float deltaSeconds);

	void CheckSolvedPanels();
	void HandleMovementSpeed(float deltaSeconds);
	void HandlePowerSurge();

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

	void CheckImportantCollisionCubes();

	void SetStatusMessages();

	int GetActivePanel();
	void WriteMovementSpeed(float currentSpeed);

	void UpdateInfiniteChallenge();

	std::map<std::string, int> laserIDsToLasers;
	std::list<std::string> laserIDs;
	std::map<int, bool> laserStates;

	std::map<std::string, int> EPIDsToEPs;
	std::list<std::string> EPIDs;
	std::map<int, bool> EPStates;

	std::set<int> PuzzlesSkippedThisGame = {};

	int currentAudioLog = -1;
	float currentAudioLogDuration = 0.f;

	int activePanelId = -1;
	int mostRecentActivePanelId = -1;
	int mostRecentPanelState = -1;

	std::string puzzleSkipInfoMessage;
	float skipButtonHeldTime = 0.f; // Tracks how long the skip button has been held.

	// The cost to skip the currently-selected puzzle. -1 if the puzzle cannot be
	//   skipped for whatever reason.
	int puzzleSkipCost = -1;

	std::map<int, std::pair<std::string, int64_t>> audioLogMessages = {};
	std::map<int, std::set<int>> obeliskHexToEPHexes = {};
	std::map<int, int> obeliskHexToAmountOfEPs = {};
	std::map<int, std::string> entityToName = {};

	Vector3 lastMouseDirection;
	float dogPettingDuration = 0.f;
	float dogBarkDuration = 0.f;
	bool sentDog = false;
	bool letGoSinceInteractModeOpen = false;

	CollisionCube townRedRoof = CollisionCube(-23.53f, -22.34f, 14.95f, -27.8f, -19.9f, 17.34f);
	CollisionCube swampLongBridgeFar = CollisionCube(189.75f, 5.86f, 3.5f, 194.2f, -1.6f, 1.2f);
	CollisionCube swampLongBridgeNear = CollisionCube(200.75f, 14.5f, 0.5f, 207.7f, 18.15f, 2.85f);
	CollisionCube bunkerElevatorCube = CollisionCube(161.9f, -86.2f, 28.0f, 158.1f, -91.0f, 29.2f);
	CollisionCube quarryElevatorUpper = CollisionCube(-61.1f, 175.6f, 11.5f, -66.3f, 171.8f, 14.2f);
	CollisionCube quarryElevatorLower = CollisionCube(-69.0f, 165.7f, 2.1f, -54.7f, 174.8f, 4.5f);
	CollisionCube riverVaultUpperCube = CollisionCube(52, -51, 19, 44, -47, 23);
	CollisionCube riverVaultLowerCube = CollisionCube(40, -56, 16, 46, -47, 20.5f);
	CollisionCube bunkerPuzzlesCube = CollisionCube(161.2f, -96.3f, 5.8f, 172.3f, -101.1f, 11.5f);
	CollisionCube tutorialPillarCube = CollisionCube(-152, -150.9f, 5, -148, -154.8f, 9);
	CollisionCube quarryLaserPanel = CollisionCube(-59, 90, 17, -67, 100, 21);
	CollisionCube symmetryUpperPanel = CollisionCube(-180, 31, 12.6f, -185, 37, 17);
	CollisionCube challengeTimer = CollisionCube(-27, -40, -20, -50, -20, -4);

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