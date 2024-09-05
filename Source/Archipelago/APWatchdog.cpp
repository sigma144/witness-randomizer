#include "windows.h"
#include "mmsystem.h"

#include "../../App/resource.h"

#include "APWatchdog.h"

#include "../DataTypes.h"
#include "../HudManager.h"
#include "DrawIngameManager.h"
#include "../Input.h"
#include "../Memory.h"
#include "../Panels.h"
#include "../Quaternion.h"
#include "../Special.h"
#include "PanelLocker.h"
#include "SkipSpecialCases.h"
#include <thread>
#include "../Randomizer.h"
#include "../DateTime.h"
#include "../ClientWindow.h"
#include "PanelRestore.h"
#include "APAudioPlayer.h"
#include "ASMPayloadManager.h"
#include "../Utilities.h"
#include "LockablePuzzle.h"


#define CHEAT_KEYS_ENABLED 0
#define SKIP_HOLD_DURATION 1.f

APWatchdog::APWatchdog(APClient* client, std::map<int, int> mapping, int lastPanel, PanelLocker* p, std::map<int, inGameHint> a, std::map<int, std::set<int>> o, bool ep, int puzzle_rando, APState* s, float smsf, bool elev, std::string col, std::string dis, std::set<int> disP, std::set<int> hunt, std::map<int, std::set<int>> iTD, std::map<int, std::vector<int>> pI, int dlA, std::map<int, int> dToI, std::vector<std::string> warps, bool sync) : Watchdog(0.033f) {
	populateWarpLookup();
	
	generator = std::make_shared<Generate>();
	ap = client;
	panelIdToLocationId = mapping;

	for (auto [key, value] : panelIdToLocationId) {
		panelIdToLocationId_READ_ONLY[key] = value;
		locationIdToPanelId_READ_ONLY[value] = key;
	}

	DeathLinkAmnesty = dlA;
	finalPanel = lastPanel;
	panelLocker = p;
	inGameHints = a;
	state = s;
	EPShuffle = ep;
	obeliskHexToEPHexes = o;
	solveModeSpeedFactor = smsf;
	CollectText = col;
	Collect = col;
	DisabledPuzzlesBehavior = dis;
	DisabledEntities = disP;
	ElevatorsComeToYou = elev;
	doorToItemId = dToI;
	progressiveItems = pI;
	itemIdToDoorSet = iTD;
	SyncProgress = sync;
	
	for (int huntEntity : hunt) {
		huntEntityToSolveStatus[huntEntity] = false;
		huntEntitySphereOpacities[huntEntity] = 0.0f;
		huntEntityPositions[huntEntity] = Vector3(0, 0, 0);
		huntEntityKeepActive[huntEntity] = 0.0f;
	}

	for (auto [area_name, entities] : areaNameToEntityIDs) {
		huntEntitiesPerArea[area_name] = {};

		for (int entity : entities) {
			if (hunt.count(entity)) {
				huntEntitiesPerArea[area_name].insert(entity);
			}
		}
	}

	for (auto [sideHex, epSet] : obeliskHexToEPHexes) {
		for (int epHex : epSet) {
			epToObeliskSides[epHex] = sideHex;
		}
	}

	for (auto warpname : warps) {
		unlockableWarps[warpname] = false;
	};
	
	if (Collect == "Free Skip + Unlock") {
		Collect = "Free Skip";
		CollectUnlock = true;
	}
	if (Collect == "Auto-Skip + Unlock") {
		Collect = "Auto-Skip";
		CollectUnlock = true;
	}

	mostRecentItemId = Memory::get()->ReadPanelData<int>(0x0064, VIDEO_STATUS_COLOR + 12);
	if (mostRecentItemId == 1065353216) { // Default value. Hopefully noone launches their game after the ~1 billionth item was sent, exactly.
		mostRecentItemId = -1;
		Memory::get()->WritePanelData<int>(0x64, VIDEO_STATUS_COLOR + 12, { -1 });
	}

	speedTime = ReadPanelData<float>(0x3D9A7, VIDEO_STATUS_COLOR);
	if (speedTime == 0.6999999881f) { // original value
		speedTime = 0;
	}

	for (auto [key, value] : obeliskHexToEPHexes) {
		obeliskHexToAmountOfEPs[key] = (int)value.size();
	}

	PuzzleRandomization = puzzle_rando;

	panelsThatHaveToBeSkippedForEPPurposes = {
		0x09E86, 0x09ED8, // light controllers 2 3
		0x033EA, 0x01BE9, 0x01CD3, 0x01D3F, // Pressure Plates
	};

	if (puzzle_rando == SIGMA_EXPERT) {
		panelsThatHaveToBeSkippedForEPPurposes.insert(0x181F5);
		panelsThatHaveToBeSkippedForEPPurposes.insert(0x334D8);
		panelsThatHaveToBeSkippedForEPPurposes.insert(0x03629); // Tutorial Gate Open
	}

	DeathLinkDataStorageKey = "WitnessDeathLink" + std::to_string(ap->get_player_number());
	if (SyncProgress) DeathLinkDataStorageKey += "_" + Utilities::wstring_to_utf8(Utilities::GetUUID());
	ap->SetNotify({ "WitnessDeathLink" + std::to_string(ap->get_player_number()) });
	ap->Set("WitnessDeathLink" + std::to_string(ap->get_player_number()), NULL, true, { {"default", 0} });

	lastFrameTime = std::chrono::system_clock::now();
	DrawIngameManager::create();
}

void APWatchdog::action() {
	auto currentFrameTime = std::chrono::system_clock::now();
	float frameDuration = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
	lastFrameTime = currentFrameTime;

	timePassedSinceRandomisation += frameDuration;
	halfSecondCountdown -= frameDuration;
	if (firstJinglePlayed) {
		timePassedSinceFirstJinglePlayed += frameDuration;
	}

	PlayFirstJingle();

	HandleReceivedItems();

	HandleInteractionState();

	HandleEncumberment(frameDuration, halfSecondCountdown <= 0);
	HandleWarp(frameDuration);
	HandleMovementSpeed(frameDuration);
	
	HandleKeyTaps();

	UpdatePuzzleSkip(frameDuration);

	CheckDeathLink();

	CheckUnlockedWarps();
	DrawSpheres(frameDuration);

	FlickerCable();

	if (Utilities::isAprilFools()) {
		DoAprilFoolsEffects(frameDuration);
	}

	if (halfSecondCountdown <= 0) {
		if (!Memory::get()->isProcessStillRunning()) throw std::exception("Game was closed.");

		halfSecondCountdown += 0.5f;

		QueueItemMessages();

		HandleInGameHints(0.5f);

		CheckFinalRoom();

		CheckSolvedPanels();

		if(PuzzleRandomization != NO_PUZZLE_RANDO) CheckEPSkips();

		HandlePowerSurge();
		HandleDeathLink();
		DisableCollisions();

		UpdateInfiniteChallenge();

		panelLocker->UpdatePPEPPuzzleLocks(*state);

		if (storageCheckCounter <= 0) {
			CheckLasers();
			CheckEPs();
			CheckPanels();
			CheckDoors();
			CheckHuntEntities();

			firstStorageCheckDone = true;

			storageCheckCounter = 20;
		}
		else
		{
			storageCheckCounter--;
		}
	}

	CheckImportantCollisionCubes();
	LookingAtLockedEntity();
	PettingTheDog(frameDuration);

	SetStatusMessages();
	HudManager::get()->update(frameDuration);

	firstActionDone = true;
}

void APWatchdog::HandleInteractionState() {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	bool stateChanged = InputWatchdog::get()->consumeInteractionStateChange();

	if (interactionState == InteractionState::Solving) {
		if (stateChanged) {
			// The player has started solving a puzzle. Update our active panel ID to match.
			activePanelId = GetActivePanel();
			mostRecentActivePanelId = activePanelId;
		}
	}
	else if (interactionState != InteractionState::Focusing) {
		// We're not in a puzzle-solving state. Clear the active panel, if any.
		activePanelId = -1;
	}

	if (timePassedSinceRandomisation > 6.0f) {
		if (!stateChanged && firstTimeActiveEntity) return;
		firstTimeActiveEntity = true;

		if (mostRecentActivePanelId == -1) {
			return;
		}

		std::string entity = "";
		if (entityToName.contains(mostRecentActivePanelId)) {
			entity = entityToName[mostRecentActivePanelId];
		}
		else {
			int realEntityID = mostRecentActivePanelId;

			// Weird PP allocation stuff
			for (int ppEP : {0x1AE8, 0x01C0D, 0x01DC7, 0x01E12, 0x01E52}) {
				int ppStartPointID = Memory::get()->ReadPanelData<int>(ppEP, PRESSURE_PLATE_PATTERN_POINT_ID) - 1;

				if (ppStartPointID > 0 && activePanelId == ppStartPointID) {
					realEntityID = ppEP;
				}
			}

			if (startPointToEPs.contains(realEntityID)) {
				std::vector<int> associatedEPs = startPointToEPs.find(realEntityID)->second;
				int representativeEP = associatedEPs[0];

				if (entityToName.contains(representativeEP)) {
					entity = entityToName[representativeEP];
				}
				else {
					entity = "Unknown EP";
				}
				if (associatedEPs.size() > 1) {
					entity += " (" + std::to_string(associatedEPs.size()) + ")";
				}
			}
		}

		if (!entity.empty() && activePanelId == -1) {
			entity += " (previous)";
		}

		ClientWindow::get()->setActiveEntityString(entity);
	}
}

bool APWatchdog::IsPanelSolved(int id, bool ensureSet) {
	if (id == 0x09F7F) return ReadPanelData<float>(0x17C6C, CABLE_POWER) > 0.0f; // Short box
	if (id == 0xFFF00) return ReadPanelData<float>(0x1800F, CABLE_POWER) > 0.0f; // Long box
	if (id == 0xFFF80) return sentDog; // Dog
	if (allEPs.count(id)) return ReadPanelData<int>(id, EP_SOLVED) > 0;
	if (id == 0x17C34) return ReadPanelData<int>(0x2FAD4, DOOR_OPEN) && ReadPanelData<int>(0x2FAD6, DOOR_OPEN) && ReadPanelData<int>(0x2FAD7, DOOR_OPEN); // Mountain Entry
	if (id == 0x079DF) return ReadPanelData<float>(0x034F1, CABLE_POWER) > 0.0f; // Town Triple
	if (ensureSet) {
		if (id == 0x09FD2) return ReadPanelData<int>(0x09FCD, CABLE_POWER) > 0.0f; // Multipanel
		if (id == 0x034E3) return ReadPanelData<int>(0x034EB, CABLE_POWER) > 0.0f; // Sound Room
		if (id == 0x014D1) return ReadPanelData<int>(0x00BED, CABLE_POWER) > 0.0f; // Red Underwater
	}
	return ReadPanelData<int>(id, SOLVED) == 1;
}

std::vector<int> APWatchdog::CheckCompletedHuntEntities() {
	std::vector<int> completedHuntEntities = {};

	for (auto [huntEntity, currentSolveStatus] : huntEntityToSolveStatus) {
		if (!currentSolveStatus) {
			if (IsPanelSolved(huntEntity, false)) {
				huntEntityToSolveStatus[huntEntity] = true;
				huntEntityKeepActive[huntEntity] = 7.0f;
				completedHuntEntities.push_back(huntEntity);
			}
		}
	}

	if (completedHuntEntities.size()) {
		int huntSolveTotal = 0;
		for (auto [huntEntity, currentSolveStatus] : huntEntityToSolveStatus) {
			if (currentSolveStatus) huntSolveTotal++;
		}

		state->solvedHuntEntities = huntSolveTotal;
		panelLocker->UpdatePuzzleLock(*state, 0x03629);

		HudManager::get()->queueNotification("Panel Hunt: " + std::to_string(huntSolveTotal) + "/" + std::to_string(state->requiredHuntEntities));
	}

	return completedHuntEntities;
}

void APWatchdog::CheckSolvedPanels() {
	for (int id : recolorWhenSolved) {
		if (!panelIdToLocationId_READ_ONLY.contains(id)) recolorWhenSolved.erase(id);
		if (!IsPanelSolved(id, true)) continue;

		if (FirstEverLocationCheckDone) {
			PotentiallyColorPanel(panelIdToLocationId_READ_ONLY[id], true);
			HudManager::get()->queueNotification("This location was previously collected.", { 0.7f, 0.7f, 0.7f });
		}
		recolorWhenSolved.erase(id);
	}

	std::list<int> solvedEntityIDs;
	std::vector<int> completedHuntEntities = {};

	if (!isCompleted) {
		if (finalPanel == 0x03629) {
			completedHuntEntities = CheckCompletedHuntEntities();

			if (!eee && ReadPanelData<float>(0x338A4, POSITION + 8) < 45) { // Some random audio log that changes Z position when EEE happens
				eee = true;

				InputWatchdog* inputWatchdog = InputWatchdog::get();
				InteractionState iState = inputWatchdog->getInteractionState();

				if (isKnockedOut) {
					return;
				}

				if (iState == InteractionState::Sleeping) {
					inputWatchdog->setSleep(false);
				}
			}

			std::vector<float> playerPosition = Memory::get()->ReadPlayerPosition();
			if (playerPosition[2] > 8) Memory::get()->WritePanelData<float>(0x03629, MAX_BROADCAST_DISTANCE, 0.0001f);
			else Memory::get()->WritePanelData<float>(0x03629, MAX_BROADCAST_DISTANCE, 24.f);

			if (EEEGate->containsPoint(playerPosition)) {
				isCompleted = true;
				eee = true;
				HudManager::get()->queueBannerMessage("Victory!");
				ap->StatusUpdate(APClient::ClientStatus::GOAL);
			}
		}
		else if (IsPanelSolved(finalPanel, false)){
			isCompleted = true;
			HudManager::get()->queueBannerMessage("Victory!");

			if (timePassedSinceRandomisation > 10.0f) {
				if (ClientWindow::get()->getJinglesSettingSafe() != "Off") APAudioPlayer::get()->PlayAudio(APJingle::Victory, APJingleBehavior::PlayImmediate);
			}
			ap->StatusUpdate(APClient::ClientStatus::GOAL);
		}
	}

	locationCheckInProgress = true;
	for (auto [entityID, locationID] : panelIdToLocationId)
	{
		if (obeliskHexToEPHexes.count(entityID)) {
			std::set<int> EPSet = obeliskHexToEPHexes[entityID];

			bool anyNew = false;

			for (auto it2 = EPSet.begin(); it2 != EPSet.end();) {
				if (ReadPanelData<int>(*it2, EP_SOLVED))
				{
					anyNew = true;
					ap->Set("WitnessEP" + std::to_string(ap->get_player_number()) + "-" + std::to_string(*it2), NULL, false, { {"replace", true} });

					it2 = EPSet.erase(it2);
				}
				else
				{
					it2++;
				}
			}

			obeliskHexToEPHexes[entityID] = EPSet;

			if (EPSet.empty())
			{
				if(FirstEverLocationCheckDone) HudManager::get()->queueBannerMessage(ap->get_location_name(locationID, "The Witness") + " Completed!");
				solvedEntityIDs.push_back(entityID);
			}
			else
			{
				if (anyNew) {
					int total = obeliskHexToAmountOfEPs[entityID];
					int done = total - EPSet.size();

					if (FirstEverLocationCheckDone) {
						std::string message = ap->get_location_name(locationID, "The Witness");
						message += " Progress (" + std::to_string(done) + "/" + std::to_string(total) + ")";
						HudManager::get()->queueBannerMessage(message);
					}
				}
			}
			continue;
		}

		else if (IsPanelSolved(entityID, true))
		{
			solvedEntityIDs.push_back(entityID);
		}
	}

	std::list<int64_t> solvedLocations = {};
	for (int solvedEntityID : solvedEntityIDs) {
		solvedLocations.push_back(panelIdToLocationId[solvedEntityID]);
		panelIdToLocationId.erase(solvedEntityID);
	}

	if (!solvedLocations.empty()) {
		ap->LocationChecks(solvedLocations);
	}

	locationCheckInProgress = false;
	FirstEverLocationCheckDone = true;

	// Maybe play panel hunt jingle
	if (completedHuntEntities.size()) {
		// Don't play if we're about to play a panel completion jingle for a progression or useful item
		for (int solvedLocation : solvedLocations) {
			// If we don't know flags yet, don't play a jingle
			if (!locationIdToItemFlags.count(solvedLocation)) {
				return;
			}
			// Play panel hunt jingle when the item is filler, otherwise don't
			if (locationIdToItemFlags[solvedLocation] != APClient::ItemFlags::FLAG_NONE) {
				return;
			}
			// Prefer epic filler over panel hunt
			if (PanelShouldPlayEpicVersion(locationIdToPanelId_READ_ONLY[solvedLocation])) {
				return;
			}
		}
		// Don't interrupt panel ramping chain
		APAudioPlayer::get()->ResetRampingCooldownPanel();
		// but play panel hunt jingle instead of normal
		PlayEntityHuntJingle(completedHuntEntities.front());
	}
}

void APWatchdog::SkipPanel(int id, std::string reason, bool kickOut, int cost, bool allowRecursion) {
	if (dont_touch_panel_at_all.count(id) or !allPanels.count(id) || PuzzlesSkippedThisGame.count(id)) {
		return;
	}

	// Knock-on skip effects
	if (allowRecursion) {
		if (reason == "Skipped" && skipTogether.find(id) != skipTogether.end()) {
			std::vector<int> otherPanels = skipTogether.find(id)->second;

			for (auto it = otherPanels.rbegin(); it != otherPanels.rend(); ++it)
			{
				int panel = *it;

				if (panel == id) continue; // Let's not make infinite recursion by accident
				if (ReadPanelData<int>(panel, SOLVED)) continue;

				SkipPanel(panel, reason, false, 0, false);
			}
		}

		if (reason == "Collected" || reason == "Excluded") { // Should this be for "disabled" too?
			if (collectTogether.find(id) != collectTogether.end()) {
				std::vector<int> otherPanels = collectTogether.find(id)->second;

				for (auto it = otherPanels.rbegin(); it != otherPanels.rend(); ++it)
				{
					int panel = *it;

					if (panel == id) continue; // Let's not make infinite recursion by accident
					if (panelIdToLocationId_READ_ONLY.count(panel)) break;
					// TODO: Add panel hunt panels to this later?

					if (ReadPanelData<int>(panel, SOLVED)) continue;

					SkipPanel(panel, reason, false, 0); // Allowing recursion is fine here for now
				}
			}
		}
	}

	if (reason == "Collected" && Collect == "Unchanged") {
		if (!IsPanelSolved(id, false)) recolorWhenSolved.insert(id);
		return;
	}	
	if (reason == "Excluded" && Collect == "Unchanged") return;
	if (reason == "Disabled" && DisabledPuzzlesBehavior == "Unchanged") return;

	if (reason != "Collected" && reason != "Excluded" || CollectUnlock) {
		if (panelLocker->PuzzleIsLocked(id)) panelLocker->PermanentlyUnlockPuzzle(id, *state);
		if (!dont_power.count(id)) WritePanelData<float>(id, POWER, { 1.0f, 1.0f });
	}

	if (reason != "Skipped" && !(reason == "Disabled" && (DisabledPuzzlesBehavior == "Auto-Skip" || DisabledPuzzlesBehavior == "Prevent Solve")) && !((reason == "Collected" || reason == "Excluded") && Collect == "Auto-Skip")) {
		if (!panelLocker->PuzzleIsLocked(id)) {
			Special::ColorPanel(id, reason);
		}
	}
	else if (reason == "Disabled" && DisabledPuzzlesBehavior == "Prevent Solve") {
		PuzzlesSkippedThisGame.insert(id);
		Special::SkipPanel(id, "Disabled Completely", kickOut);
		return;
	}
	else
	{
		if (!panelLocker->PuzzleIsLocked(id)) {
			if (reason == "Skipped") {
				if (ReadPanelData<int>(id, VIDEO_STATUS_COLOR) == COLLECTED) {
					PuzzlesSkippedThisGame.insert(id);
					Special::SkipPanel(id, "Collected", kickOut);
					return;
				}
				if (ReadPanelData<int>(id, VIDEO_STATUS_COLOR) == EXCLUDED) {
					PuzzlesSkippedThisGame.insert(id);
					Special::SkipPanel(id, "Excluded", kickOut);
					return;
				}
				if (ReadPanelData<int>(id, VIDEO_STATUS_COLOR) == DISABLED) {
					PuzzlesSkippedThisGame.insert(id);
					Special::SkipPanel(id, "Disabled", kickOut);
					return;
				}
			}
			Special::SkipPanel(id, reason, kickOut);
			PuzzlesSkippedThisGame.insert(id);
		}
	}

	if (reason == "Skipped") WritePanelData<int>(id, VIDEO_STATUS_COLOR, { PUZZLE_SKIPPED + cost });

	if (!skip_completelyExclude.count(id)) {
		if (reason == "Collected") WritePanelData<int>(id, VIDEO_STATUS_COLOR, { COLLECTED });
		if (reason == "Disabled") WritePanelData<int>(id, VIDEO_STATUS_COLOR, { DISABLED });
		if (reason == "Excluded") WritePanelData<int>(id, VIDEO_STATUS_COLOR, { EXCLUDED });
	}
}

void APWatchdog::MarkLocationChecked(int64_t locationId)
{
	while (!FirstEverLocationCheckDone) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(10)));
	}
	checkedLocations.insert(locationId);

	if (!locationIdToPanelId_READ_ONLY.count(locationId)) return;
	int panelId = locationIdToPanelId_READ_ONLY[locationId];

	if (allPanels.count(panelId)) {
		if (!IsPanelSolved(panelId, false)) {
			if (PuzzlesSkippedThisGame.count(panelId)) {
				while (locationCheckInProgress) {
					std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(10)));
				}
				if (panelIdToLocationId.count(panelId)) {
					panelIdToLocationId.erase(panelId);
				}
				return;
			}

			SkipPanel(panelId, "Collected", false);
		}

		PotentiallyColorPanel(locationId);
	}

	else if (allEPs.count(panelId) && Collect != "Unchanged") {
		int eID = panelId;

		Memory::get()->SolveEP(eID);
		panelLocker->PermanentlyUnlockPuzzle(eID, *state);
		if (precompletableEpToName.count(eID) && precompletableEpToPatternPointBytes.count(eID) && EPShuffle) {
			Memory::get()->MakeEPGlow(precompletableEpToName.at(eID), precompletableEpToPatternPointBytes.at(eID));
		}
	}

	while (locationCheckInProgress) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(10)));
	}
	if (panelIdToLocationId.count(panelId)) {
		panelIdToLocationId.erase(panelId);
	}

	if (obeliskHexToEPHexes.count(panelId) && Collect != "Unchanged") {
		for (int epHex : obeliskHexToEPHexes[panelId]) {
			if (!ReadPanelData<int>(epHex, EP_SOLVED)) {
				Memory::get()->SolveEP(epHex);
				if (precompletableEpToName.count(epHex) && precompletableEpToPatternPointBytes.count(epHex) && EPShuffle) {
					Memory::get()->MakeEPGlow(precompletableEpToName.at(epHex), precompletableEpToPatternPointBytes.at(epHex));
				}
			}
		}
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
		if (interactionState != InteractionState::Walking || IsEncumbered()) factor = solveModeSpeedFactor;

		speedTime = std::max(std::abs(speedTime) - deltaSeconds * factor, 0.f) * (std::signbit(speedTime) ? -1 : 1);

		if (IsEncumbered()) {
			WriteMovementSpeed(10.0f);
		}
		else if (speedTime > 0) {
			WriteMovementSpeed(2.0f);
		}
		else if (speedTime < 0) {
			WriteMovementSpeed(0.6f);
		}
		else {
			WriteMovementSpeed(1.0f);
		}
	}
	else {
		if (IsEncumbered()) {
			WriteMovementSpeed(10.0f);
		}
		else {
			WriteMovementSpeed(1.0f);
		}
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
	}
}

void APWatchdog::HandlePowerSurge() {
	if (hasPowerSurge)
	{
		if (DateTime::since(powerSurgeStartTime).count() > 10000)
			ResetPowerSurge();
		else {
			for (const auto& panelId : allPanels) {
				/*if (ReadPanelData<int>(panelId, SOLVED))
					continue;*/

				std::vector<float> powerValues = ReadPanelData<float>(panelId, POWER, 2, movingMemoryPanels.count(panelId));

				if (powerValues.size() == 0) {
					HudManager::get()->queueBannerMessage("Error reading power values on panel: " + std::to_string(panelId));
					continue;
				}

				if (powerValues[1] > 0) {
					//using -20f as offset to create a unique recogniseable value that is also stored in the save
					WritePanelData<float>(panelId, POWER, { -19.0f, -19.0f }, movingMemoryPanels.count(panelId));
				}
			}
		}
	}
}

void APWatchdog::ResetPowerSurge() {
	hasPowerSurge = false;
	bool weirdStuff = false;

	for (const auto& panelId : allPanels) {
		std::vector<float> powerValues = ReadPanelData<float>(panelId, POWER, 2, movingMemoryPanels.count(panelId));

		if (powerValues[1] < -18.0f && powerValues[1] > -22.0f)
		{
			powerValues[1] -= -20.0f;
			powerValues[0] = powerValues[1];

			WritePanelData<float>(panelId, POWER, powerValues, movingMemoryPanels.count(panelId));
			WritePanelData<float>(panelId, NEEDS_REDRAW, { 1 }, movingMemoryPanels.count(panelId));
		}
		else if (powerValues[0] < 0) {
			weirdStuff = true;
			WritePanelData<float>(panelId, POWER, { 1.0f, 1.0f }, movingMemoryPanels.count(panelId));
		}
	}

	if (weirdStuff) {
		HudManager::get()->queueBannerMessage("Something strange happened when trying to reset a power surge.");
	}
}

void APWatchdog::TriggerBonk() {
	if (eee && isCompleted) return;

	if (ClientWindow::get()->getJinglesSettingSafe() != "Off") APAudioPlayer::get()->PlayAudio(APJingle::DeathLink, APJingleBehavior::PlayImmediate);

	if (isKnockedOut) {
		knockOutStartTime = std::chrono::system_clock::now();
	}
	else {
		isKnockedOut = true;
		knockOutStartTime = std::chrono::system_clock::now();
	}
}

void APWatchdog::HandleDeathLink() {
	if (isKnockedOut)
	{
		if (DateTime::since(knockOutStartTime).count() > DEATHLINK_DURATION * 1000) ResetDeathLink();
	}
}

bool APWatchdog::IsEncumbered() {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();

	return isKnockedOut || interactionState == InteractionState::Sleeping || interactionState == InteractionState::Warping || interactionState == InteractionState::MenuAndSleeping;
}

void APWatchdog::HandleEncumberment(float deltaSeconds, bool doFunctions) {
	Memory *memory = Memory::get();

	if (IsEncumbered()) {
		memory->EnableVision(false);
		Memory::get()->EnableMovement(false);
		Memory::get()->EnableSolveMode(false);
		memory->MoveVisionTowards(0.0f, isKnockedOut ? 1.0f : deltaSeconds * 2);

		int interactMode = InputWatchdog::get()->readInteractMode();
		if ((interactMode == 0 || interactMode == 1) && doFunctions) {
			ASMPayloadManager::get()->ExitSolveMode();
		}
	}
	else {
		Memory::get()->EnableMovement(true);
		Memory::get()->EnableSolveMode(true);
		std::pair<float, float> previousAndNewBrightness = memory->MoveVisionTowards(1.0f, isKnockedOut ? deltaSeconds : deltaSeconds * 2);
		if (previousAndNewBrightness.second == 1.0f) memory->EnableVision(true);
	}
}

void APWatchdog::HandleWarp(float deltaSeconds){
	if (selectedWarp == NULL && unlockedWarps.size()) selectedWarp = unlockedWarps[0];

	Memory *memory = Memory::get();
	InputWatchdog* inputWatchdog = InputWatchdog::get();
	if (inputWatchdog->getInteractionState() == InteractionState::Warping) {
		if (!(ReadPanelData<char>(selectedWarp->entityToCheckForLoading, 0xBC) & 2)) {
			inputWatchdog->allowWarpCompletion();
		}
		
		inputWatchdog->updateWarpTimer(deltaSeconds);

		float warpTime = inputWatchdog->getWarpTime();

		if (warpTime > 0.0f) {
			memory->FloatWithoutMovement(true);

			if (warpTime <= WARPTIME - 0.5f + (selectedWarp->tutorial ? LONGWARPBUFFER : 0)) {
				memory->WritePlayerPosition(selectedWarp->playerPosition);
				memory->WriteCameraAngle(selectedWarp->cameraAngle);
				hasTeleported = true;
			}
		}

		// Warp time has been run out. Warp can be ended.
		else {
			if (hasTriedExitingNoclip) {
				auto now = std::chrono::system_clock::now();

				// We need to check whether the player actually safely landed.

				auto currentCameraPosition = memory->ReadPlayerPosition();
				if (pow(currentCameraPosition[0] - selectedWarp->playerPosition[0], 2) + pow(currentCameraPosition[1] - selectedWarp->playerPosition[1], 2) + pow(currentCameraPosition[2] - selectedWarp->playerPosition[2], 2) < 0.3) {
					// If it has, we need to give some time to fail. It doesn't fail instantly, it takes a frame.
					if (std::chrono::duration<float>(now - attemptedWarpCompletion).count() > 0.3f) {
						// If the player has been standing safely for 0.3 seconds, we complete the warp.
						memory->WritePlayerPosition(selectedWarp->playerPosition);
						memory->WriteCameraAngle(selectedWarp->cameraAngle);

						if (!selectedWarp->tutorial) {
							ASMPayloadManager::get()->ActivateMarker(0x033ED);
						}

						inputWatchdog->endWarp();
					}
				}
				else {
					// If they have not landed safely, we have to retry the warp.

					hasTriedExitingNoclip = false;

					warpRetryTime += 0.5f;
					if (warpRetryTime > 2.0f) warpRetryTime = 2.0f;

					if (selectedWarp->tutorial) {
						ASMPayloadManager::get()->ActivateMarker(0x034F6);
					}
				}
			}
			else {
				// Player is ready to be exited from warp. 

				// If there was a recent failed warp attempt, chill for a bit.
				auto now = std::chrono::system_clock::now();
				if (std::chrono::duration<float>(now - attemptedWarpCompletion).count() < warpRetryTime) {
					memory->FloatWithoutMovement(true);
					memory->WritePlayerPosition(selectedWarp->playerPosition);
					memory->WriteCameraAngle(selectedWarp->cameraAngle);
				}
				// Else, let them down onto the ground.
				else
				{
					memory->FloatWithoutMovement(false);

					int random_value = std::rand() % 2;
					bool debug_warp_fails = false;

					if (random_value == 0 && debug_warp_fails) {
						std::vector<float> a = { selectedWarp->playerPosition[0] + 5, selectedWarp->playerPosition[1] + 5, selectedWarp->playerPosition[2] + 5 };
						memory->WritePlayerPosition(a);
					}
					else {
						memory->WritePlayerPosition(selectedWarp->playerPosition);
					}
					memory->WriteCameraAngle(selectedWarp->cameraAngle);
					hasTriedExitingNoclip = true;

					attemptedWarpCompletion = std::chrono::system_clock::now();
				}
			}
		}
	}
	else {
	}
}

void APWatchdog::ResetDeathLink() {
	Memory::get()->EnableMovement(true);
	Memory::get()->EnableSolveMode(true);

	isKnockedOut = false;
}

void APWatchdog::StartRebindingKey(CustomKey key)
{
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState == InteractionState::Walking) {
		InputWatchdog::get()->beginCustomKeybind(key);
	}
}

void APWatchdog::UpdatePuzzleSkip(float deltaSeconds) {
	std::vector<int> recentlyUnlockedPuzzles = panelLocker->getAndFlushRecentlyUnlockedPuzzles();
	for (int id : recentlyUnlockedPuzzles) {
		if (!PuzzlesSkippedThisGame.count(id)) {
			__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);
			if (skipped == COLLECTED) {
				SkipPanel(id, "Collected", false);
			}
			else if (skipped == DISABLED) {
				SkipPanel(id, "Disabled", false);
			}
			else if (skipped == EXCLUDED) {
				SkipPanel(id, "Excluded", false);
			}
		}
	}

	const InputWatchdog* inputWatchdog = InputWatchdog::get();
	InteractionState interactionState = inputWatchdog->getInteractionState();
	if (interactionState == InteractionState::Solving || interactionState == InteractionState::Focusing) {
		// If we're tracking a panel for the purpose of puzzle skips, update our internal logic.
		if (activePanelId != -1) {
			// Check to see if the puzzle is in a skippable state.
			if (PuzzleIsSkippable(activePanelId)) {
				// The puzzle is still skippable. Update the panel's cost in case something has changed, like a latch
				//   being opened remotely.
				puzzleSkipCost = CalculatePuzzleSkipCost(activePanelId, puzzleSkipInfoMessage);
				
				int statusColor = ReadPanelData<int>(activePanelId, VIDEO_STATUS_COLOR);
				if (puzzleSkipCost && (statusColor == COLLECTED) || (statusColor == DISABLED) || (statusColor == EXCLUDED)) {
					puzzleSkipCost = 0;
					puzzleSkipInfoMessage = "Skipping this panel is free.";
				}

				// Update skip button logic.
				InputButton skipButton = inputWatchdog->getCustomKeybind(CustomKey::SKIP_PUZZLE);
				bool skipButtonHeld = skipButton != InputButton::NONE && inputWatchdog->getButtonState(skipButton);
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

				if (apology_panels.count(activePanelId)) {
					puzzleSkipInfoMessage = "This puzzle can't be skipped for technical reasons. (Sorry!)";
				}
			}
		}
	}
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

	// Verify that this is, indeed, a panel.
	if (std::find(allPanels.begin(), allPanels.end(), puzzleId) == allPanels.end()) {
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
	else if ((PuzzleRandomization == SIGMA_NORMAL || PuzzleRandomization == NO_PUZZLE_RANDO || PuzzleRandomization == UMBRA_VARIETY) && skip_excludeOnNormal.count(puzzleId) != 0) {
		// Puzzle is excluded on Normal.
		return false;
	}

	// Locked puzzles are unskippable.
	if (panelLocker->PuzzleIsLocked(puzzleId)) {
		return false;
	}

	// Check skippability based on panel state.
	if (PuzzlesSkippedThisGame.count(puzzleId)) {
		// Puzzle has already been skipped.
		return false;
	}

	std::set<int> still_let_skip_states = { COLLECTED, DISABLED, EXCLUDED };

	if (ReadPanelData<int>(puzzleId, SOLVED) != 0 && skip_multisolvePuzzles.count(puzzleId) == 0 && !still_let_skip_states.count(ReadPanelData<int>(puzzleId, VIDEO_STATUS_COLOR))) {
		// Puzzle has already been solved and thus cannot be skipped.
		return false;
	}

	return true;
}

int APWatchdog::CalculatePuzzleSkipCost(int puzzleId, std::string& specialMessage) const {
	// Check for special cases.
	
	if (puzzleId == 0x17C34) {
		// Mountain Entry
		int unopenedLatches = 0;
		unopenedLatches += ReadPanelData<int>(0x2fad4, DOOR_OPEN) == 0;
		unopenedLatches += ReadPanelData<int>(0x2fad7, DOOR_OPEN) == 0;
		unopenedLatches += ReadPanelData<int>(0x2fad6, DOOR_OPEN) == 0;

		if (unopenedLatches) {
			specialMessage = "Skipping this panel costs 1 Puzzle Skip per unopened latch.";
			return unopenedLatches;
		}
		else {
			specialMessage = "";
			return -1;
		}
	}

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
	else if (puzzleId == 0x03629) {
		// Tutorial Gate Open. Check for latch.
		int latchAmount = 0;

		latchAmount += ReadPanelData<int>(0x288E8, DOOR_OPEN) == 0;
		latchAmount += ReadPanelData<int>(0x288F3, DOOR_OPEN) == 0;
		latchAmount += ReadPanelData<int>(0x28942, DOOR_OPEN) == 0;

		if (latchAmount) {
			specialMessage = "Skipping this panel costs 1 Puzzle Skip per unopened latch.";
			return latchAmount;
		}
		else {
			specialMessage = "";
			return -1;
		}
	}
	else if (puzzleId == 0x09FDA) {
		// Metapuzzle. This can only be skipped if all child puzzles are skipped too.
		//   TODO: This flow here is a little confusing to read, because PuzzleIsSkippable returns true, but the cost
		//   returned here indicates that it isn't.
		for (int smallPuzzleId : {0x09FC1, 0x09F8E, 0x09F01, 0x09EFF}) {
			if (!PuzzlesSkippedThisGame.count(smallPuzzleId)) {
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

	SkipPanel(activePanelId, "Skipped", true, puzzleSkipCost);
}

void APWatchdog::SkipPreviouslySkippedPuzzles() {
	for (int id : allPanels) {
		__int32 skipped = ReadPanelData<__int32>(id, VIDEO_STATUS_COLOR);

		if (skipped >= PUZZLE_SKIPPED && skipped <= PUZZLE_SKIPPED_MAX) {
			SkipPanel(id, "Skipped", false, skipped - PUZZLE_SKIPPED);
			spentPuzzleSkips += skipped - PUZZLE_SKIPPED;
		}
		else if (skipped == COLLECTED) {
			SkipPanel(id, "Collected", false, skipped - PUZZLE_SKIPPED);
		}
		else if (skipped == EXCLUDED) {
			SkipPanel(id, "Excluded", false, skipped - PUZZLE_SKIPPED);
		}
		else if (skipped == DISABLED) {
			//APRandomizer.cpp will do it again anyway
		}
	}
}

void APWatchdog::AddPuzzleSkip() {
	foundPuzzleSkips++;
}

void APWatchdog::UnlockDoor(int id) {
	if (DisabledEntities.count(id)) return;

	if (allPanels.count(id)) {
		WritePanelData<float>(id, POWER, { 1.0f, 1.0f });
		state->keysReceived.insert(id);
		panelLocker->UpdatePuzzleLock(*state, id);
		return;
	}

	if (std::count(LockablePuzzles.begin(), LockablePuzzles.end(), id)) {
		state->keysReceived.insert(id);
		panelLocker->UpdatePuzzleLock(*state, id);
		return;
	}

	if (allLasers.count(id)) {
		Memory::get()->ActivateLaser(id);
		return;
	}

	// Is a door

	unlockedDoors.insert(id);

	if (id == 0x0CF2A) { // River to Garden door
		disableCollisionList.insert(id);

		WritePanelData<float>(0x17CAA, POSITION, { 37.194f, -41.883f, 16.645f });
		WritePanelData<float>(0x17CAA, SCALE, { 1.15f });

		ASMPayloadManager::get()->UpdateEntityPosition(0x17CAA);
	}

	if (allObelisks.count(id)) {
		disableCollisionList.erase(id);
	}

	if (id == 0x0C310) {
		WritePanelData<float>(0x02886, POSITION + 8, { 12.8f });

		ASMPayloadManager::get()->UpdateEntityPosition(0x02886);
	}

	if (id == 0x2D73F) {
		WritePanelData<float>(0x021D7, POSITION + 8, { 10.0f });

		ASMPayloadManager::get()->UpdateEntityPosition(0x021D7);
	}

	// Un-distance-gate Shadows Laser Panel
	if (id == 0x19665 || id == 0x194B2) {
		Memory::get()->WritePanelData<float>(0x19650, MAX_BROADCAST_DISTANCE, { -1 });
	}

	if (ReadPanelData<int>(id, DOOR_OPEN)) {
		std::wstringstream s;
		s << std::hex << id << " is already open.\n";
		OutputDebugStringW(s.str().c_str());
		return;
	}

	ASMPayloadManager::get()->OpenDoor(id);
}

void APWatchdog::SeverDoor(int id) {
	// Disabled doors should behave as vanilla
	if (DisabledEntities.count(id)) return;

	bool isDoor = true;
	
	if (std::count(LockablePuzzles.begin(), LockablePuzzles.end(), id)) {
		state->keysInTheGame.insert(id);
	}

	if (allEPs.count(id)) return; // EPs don't need any "severing"

	if (allPanels.count(id)) {
		WritePanelData<float>(id, POWER, { 1.0f, 1.0f });
		isDoor = false;
	}

	if (allObelisks.count(id)) {
		disableCollisionList.insert(id);
		isDoor = false;
	}

	if (isDoor) {
		lockedDoors.insert(id);
	}

	if (severTargetsById.count(id)) {
		severedDoorsList.insert(id);

		if (id == 0x01A0E) {
			WritePanelData<int>(0x01A0F, TARGET, { 0x0360E + 1 });
			return;
		}

		if (id == 0x01D3F) {
			WritePanelData<int>(0x01D3F, TARGET, { 0x03317 + 1 });
			return;
		}

		if (id == 0x012FB || id == 0x01317) {
			Memory::get()->StopDesertLaserPropagation();

			// If the Desert Elevator has already been severed and we're trying to sever the Laser, the Panel should do nothing.
			if (id == 0x012FB && severedDoorsList.count(0x01317)) {
				WritePanelData<int>(0x03608, TARGET, { 0 });
			}

			else if (id == 0x01317) {
				// If the Laser has been severed and we're trying to sever the Elevator, the Panel should do nothing.
				if (severedDoorsList.count(0x012FB)) {
					WritePanelData<int>(0x03608, TARGET, { 0 });
				}
				// HOWEVER, if the Laser has NOT been severed and we're severing the Elevator, the Laser Panel should now directly activate the laser.
				// If the Laser is going to be severed too, that is taken care of by the above if condition.
				else
				{
					WritePanelData<int>(0x03608, TARGET, { 0x012FB + 1 });
				}
			}
		}

		std::vector<Connection> conns = severTargetsById[id];

		for (auto& conn : conns) {
			if (id == 0x01954 || id == 0x018CE || id == 0x019D8 || id == 0x019B5 || id == 0x019E6 || id == 0x0199A || id == 0x18482 || id == 0x0A1D6)
			{
				DoubleDoorTargetHack(id);
				continue;
			}

			if (conn.id == 0x28A0D) {
				WritePanelData<float>(conn.id, POSITION, { -31.0f, 7.1f });

				ASMPayloadManager::get()->UpdateEntityPosition(conn.id);
			}

			if (conn.id == 0x17CAB) {
				WritePanelData<float>(0x0026D, POWER, { 1.0f, 1.0f });
				WritePanelData<int>(0x0026D, NEEDS_REDRAW, { 1 });
			}

			if (conn.id == 0x01D8D) {
				WritePanelData<float>(conn.id, POSITION, { -57.8f, 157.2f, 3.45f });

				ASMPayloadManager::get()->UpdateEntityPosition(conn.id);
			}

			if (conn.id == 0x012FB) {
				WritePanelData<float>(0x012FB, POSITION, { -124.4, 90.37, 13 });
				ASMPayloadManager::get()->UpdateEntityPosition(0x012FB);
				Memory::get()->DoFullPositionUpdate();
			}

			if (conn.id == 0x34BD2) {
				WritePanelData<float>(0x34BD2, POSITION, { -124.399971, 90.37002563, 13 });
				ASMPayloadManager::get()->UpdateEntityPosition(0x34BD2);
			}

			if (conn.id == 0x34F1E) {
				WritePanelData<float>(0x34F1E, POSITION, { -124.3999557, 90.37004852, 13 });
				ASMPayloadManager::get()->UpdateEntityPosition(0x34F1E);
			}

			if (conn.target_no == ENTITY_NAME) {
				std::stringstream stream;
				stream << std::hex << conn.id;
				std::string result(stream.str());
				std::vector<char> v(result.begin(), result.end());

				WriteArray<char>(conn.id, ENTITY_NAME, v);
				continue;
			}
			if (conn.target_no == LIGHTMAP_TABLE) WritePanelData<INT64>(conn.id, conn.target_no, { 0 });
			else WritePanelData<int>(conn.id, conn.target_no, { 0 });
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
			WritePanelData<int>(0x00139, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x00521, CABLE_TARGET_2, { 0 });
		return;
	}

	if (id == 0x018CE) { // Shortcut Door 1
		if (severedDoorsList.count(0x01954)) {
			WritePanelData<int>(0x00139, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x00139, TARGET, { 0x01954 + 1 });
		return;
	}



	if (id == 0x019D8) { // Exit Door 2->3
		if (severedDoorsList.count(0x019B5)) {
			WritePanelData<int>(0x019DC, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x00525, CABLE_TARGET_2, { 0 });
		return;
	}

	if (id == 0x019B5) { // Shortcut Door 2
		if (severedDoorsList.count(0x019D8)) {
			WritePanelData<int>(0x019DC, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x019DC, TARGET, { 0x019D8 + 1 });
		return;
	}



	if (id == 0x019E6) { // Exit Door 3->4
		if (severedDoorsList.count(0x0199A)) {
			WritePanelData<int>(0x019E7, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x00529, CABLE_TARGET_2, { 0 });
		return;
	}

	if (id == 0x0199A) { // Shortcut Door 3
		if (severedDoorsList.count(0x019E6)) {
			WritePanelData<int>(0x019E7, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x019E7, TARGET, { 0x019E6 + 1 });
		return;
	}



	if (id == 0x18482) { // Swamp Blue
		if (severedDoorsList.count(0x0A1D6)) {
			WritePanelData<int>(0x00E3A, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x00BDB, CABLE_TARGET_0, { 0 });
		return;
	}

	if (id == 0x0A1D6) { // Swamp Purple
		if (severedDoorsList.count(0x18482)) {
			WritePanelData<int>(0x00E3A, TARGET, { 0 });
			return;
		}

		WritePanelData<int>(0x2FD5F, CABLE_TARGET_1, { 0 });
		return;
	}
}

void APWatchdog::HandleKeyTaps() {
	InputWatchdog* inputWatchdog = InputWatchdog::get();
	InteractionState interactionState = inputWatchdog->getInteractionState();
	std::vector<InputButton> tapEvents = inputWatchdog->consumeTapEvents();
	std::vector<MovementDirection> directionEvents = inputWatchdog->consumeDirectionalEvents();

	for (const InputButton& tappedButton : tapEvents) {
		if (interactionState == InteractionState::Keybinding) {
			CustomKey bindingKey = inputWatchdog->getCurrentlyRebindingKey();
			if (tappedButton == InputButton::KEY_ESCAPE) {
				inputWatchdog->cancelCustomKeybind();
			}
			else if (!inputWatchdog->isValidForCustomKeybind(tappedButton)) {
				std::string keyName = inputWatchdog->getNameForInputButton(tappedButton);
				std::string bindName = inputWatchdog->getNameForCustomKey(bindingKey);
				HudManager::get()->queueBannerMessage("Can't bind [" + keyName + "] to " + bindName + ".", { 1.f, 0.25f, 0.25f }, 2.f);
			}
			else if (inputWatchdog->trySetCustomKeybind(tappedButton)) {

				std::string keyName = inputWatchdog->getNameForInputButton(tappedButton);
				std::string bindName = inputWatchdog->getNameForCustomKey(bindingKey);
				HudManager::get()->queueBannerMessage("Bound [" + keyName + "] to " + bindName + ".", { 1.f, 1.f, 1.f }, 2.f);
			}
			else {
				std::string keyName = inputWatchdog->getNameForInputButton(tappedButton);
				std::string bindName = inputWatchdog->getNameForCustomKey(bindingKey);
				HudManager::get()->queueBannerMessage("Failed to bind [" + keyName + "] to " + bindName + ".", { 1.f, 0.25f, 0.25f }, 2.f);
			}
		}
		else if (interactionState == InteractionState::Sleeping) {
			if (tappedButton == inputWatchdog->getCustomKeybind(CustomKey::SKIP_PUZZLE)) {
				TryWarp();
			}
		}
		else {
			switch (tappedButton) {
#if CHEAT_KEYS_ENABLED
			case InputButton::KEY_MINUS:
				HudManager::get()->queueNotification("Cheat: adding Slowness.", getColorByItemFlag(APClient::ItemFlags::FLAG_TRAP));
				ApplyTemporarySlow();
				break;
			case InputButton::KEY_EQUALS:
				HudManager::get()->queueNotification("Cheat: adding Speed Boost.", getColorByItemFlag(APClient::ItemFlags::FLAG_NEVER_EXCLUDE));
				ApplyTemporarySpeedBoost();
				break;
			case InputButton::KEY_0:
				HudManager::get()->queueNotification("Cheat: adding Puzzle Skip.", getColorByItemFlag(APClient::ItemFlags::FLAG_ADVANCEMENT));
				if (spentPuzzleSkips > foundPuzzleSkips) {
					// We've spent more skips than we've found, almost certainly because we cheated ourselves some in a
					//   previous app launch. Reset to zero before adding a new one.
					foundPuzzleSkips = spentPuzzleSkips;
				}

				AddPuzzleSkip();
				break;
			case InputButton::KEY_NUMPAD_0:
				for (int spamCount = 0; spamCount < 100; spamCount++) {
					std::stringstream spamString;
					spamString << "spam message " << spamCount + 1 << "/100";
					HudManager::get()->queueNotification(spamString.str());
				}
				HudManager::get()->queueNotification("sorry (not sorry)");
#endif
			};
		}

		if (tappedButton == inputWatchdog->getCustomKeybind(CustomKey::SLEEP)) {
			if (interactionState != InteractionState::Menu && interactionState != InteractionState::MenuAndSleeping) {
				ToggleSleep();
			}
		}
	}

	for (const MovementDirection& directionEvent : directionEvents) {
		if (interactionState == InteractionState::Sleeping) {
			if (selectedWarp == NULL) continue;

			auto i = distance(unlockedWarps.begin(), find(unlockedWarps.begin(), unlockedWarps.end(), selectedWarp));

			if (directionEvent == MovementDirection::RIGHT) {
				if (i >= unlockedWarps.size()) {
					i = 1;
				}

				i--;

				if (i < 0) i = unlockedWarps.size() - 1;

				selectedWarp = unlockedWarps[i];
			}

			if (directionEvent == MovementDirection::LEFT) {
				i++;
				if (i >= unlockedWarps.size()) {
					i = 0;
				}

				selectedWarp = unlockedWarps[i];
			}
		}
	}
}

void APWatchdog::DisableCollisions() {
	for (int id : disableCollisionList) {
		Memory::get()->RemoveMesh(id);
	}
}

void APWatchdog::HandleInGameHints(float deltaSeconds) {
	if (!FirstEverLocationCheckDone) return;

	std::string line1 = "";
	std::string line2 = "";
	std::string line3 = "";

	int pNO = ap->get_player_number();

	if (!firstStorageCheckDone) {
		for (int logId : audioLogs) {
			std::string key = "WitnessAudioLog" + std::to_string(pNO) + "-" + std::to_string(logId);
			ap->SetNotify({ key });
			ap->Set(key, false, true, { { "default", false } });
		}

		for (int laserID : allLasers) {
			std::string key = "WitnessLaserHint" + std::to_string(pNO) + "-" + std::to_string(laserID);
			ap->SetNotify({ key });
			ap->Set(key, false, true, { { "default", false } });
		}
	}

	std::set<inGameHint> seenMessages = {};

	// Laser Hints

	InteractionState interactionState = InputWatchdog::get()->getInteractionState();

	int candidate = -1;
	int currentLaser = -1;
	int currentAudioLog = -1;

	std::vector<float> headPosition = Memory::get()->ReadPlayerPosition();

	for (int laserID : allLasers) {
		std::vector<float> laserPosition = ReadPanelData<float>(laserID, POSITION, 3);

		if (pow(headPosition[0] - laserPosition[0], 2) + pow(headPosition[1] - laserPosition[1], 2) + pow(headPosition[2] - laserPosition[2], 2) > 200) {
			continue;
		}

		candidate = laserID;
	}

	if (candidate != -1 && ReadPanelData<int>(candidate, LASER_TARGET) == 0 && inGameHints.count(candidate)) {
		if (candidate != 0x012FB) {
			if (laserCollisions[candidate]->containsPoint(headPosition)) {
				currentLaser = candidate;
			}
		}
		else {
			std::vector<float> laserPosition = ReadPanelData<float>(candidate, POSITION, 3);

			if (abs(-140.690979 - laserPosition[0]) < 1 && abs(118.1552734 - laserPosition[1]) < 1) {
				if (abs(laserPosition[2] - headPosition[2]) < 2 && laserCollisions[candidate]->containsPoint(headPosition)) {
					currentLaser = candidate;
				}
			}
			else {
				if (pow(headPosition[0] - laserPosition[0], 2) + pow(headPosition[1] - laserPosition[1], 2) + pow(headPosition[2] - laserPosition[2], 2) < 120) {
					currentLaser = candidate;
				}
			}
		}
	}


	for (int laserID : allLasers) {
		if (inGameHints.count(laserID)) {
			bool laserHasBeenSeen = ReadPanelData<float>(laserID, 0x108) > 1;
			bool laserIsCurrentlyNearby = currentLaser == laserID;
			
			if (laserHasBeenSeen || laserIsCurrentlyNearby) {
				seenMessages.insert(inGameHints[laserID]);
				if (!seenLasers.count(laserID)) {
					WritePanelData<float>(laserID, 0x108, { 1.0001f });
					int64_t locationId = inGameHints[laserID].locationID;
					if (locationId != -1 && inGameHints[laserID].playerNo == pNO && inGameHints[laserID].allowScout && !checkedLocations.count(locationId)) ap->LocationScouts({locationId}, 2);
					ap->Set("WitnessLaserHint" + std::to_string(pNO) + "-" + std::to_string(laserID), NULL, false, { {"replace", true} });
					seenLasers.insert(laserID);
				}
			}
		}
	}

	// Audiolog Hints
	for (int logId : audioLogs) {
		bool audioLogHasBeenPlayed = ReadPanelData<int>(logId, AUDIO_LOG_PLAYED);
		bool logPlaying = ReadPanelData<int>(logId, AUDIO_LOG_IS_PLAYING) != 0;

		if (audioLogHasBeenPlayed || logPlaying) {
			seenMessages.insert(inGameHints[logId]);
			if (!seenAudioLogs.count(logId)) {
				int64_t locationId = inGameHints[logId].locationID;
				if (locationId != -1 && inGameHints[logId].playerNo == pNO && inGameHints[logId].allowScout && !checkedLocations.count(locationId)) ap->LocationScouts({ locationId }, 2);
				ap->Set("WitnessAudioLog" + std::to_string(pNO) + "-" + std::to_string(logId), NULL, false, { {"replace", true} });
				seenAudioLogs.insert(logId);
			}
		}
		if (logPlaying) {
			currentAudioLog = logId;
		}
	}

	// If no hint has to be shown, don't show a hint.
	if (currentAudioLog == -1 && currentLaser == -1) {
		currentHintEntity = -1;
	}
	// If it is unambiguous what to show, show that.
	else if (currentAudioLog != -1 && currentLaser == -1) {
		if (currentHintEntity != currentAudioLog) currentHintEntityDuration = 8.0f;
		currentHintEntity = currentAudioLog;
	}
	else if (currentAudioLog == -1 && currentLaser != -1) {
		currentHintEntityDuration = 0.5f;
		currentHintEntity = currentLaser;
	}
	// If both a laser and an audio log are playing... 
	else if (currentAudioLog != -1 && currentLaser != -1) {
		// ...if one of them *just* started playing, give priority to that ("focus request")
		if (lastLaser != currentLaser) {
			currentHintEntityDuration = 0.5f;
			currentHintEntity = currentLaser;
		}
		else if (lastAudioLog != currentAudioLog) {
			if (currentHintEntity != currentAudioLog) currentHintEntityDuration = 8.0f;
			currentHintEntity = currentAudioLog;
		}

		// otherwise, continue showing the hint that has been showing.
		else {
			if (currentHintEntity == currentLaser) {
				currentHintEntityDuration = 0.5f;
				currentHintEntity = currentLaser;
			}
			else {
				if (currentHintEntity != currentAudioLog) currentHintEntityDuration = 8.0f;
				currentHintEntity = currentAudioLog;
			}
		}
	}

	lastLaser = currentLaser;
	lastAudioLog = currentAudioLog;
	lastRealHintEntity = (currentHintEntity == -1) ? lastRealHintEntity : currentHintEntity;

	// If we're solving, don't show hint anymore if it's a laser hint
	if (interactionState != InteractionState::Walking && !audioLogs.count(lastRealHintEntity)) {
		HudManager::get()->clearInformationalMessage(InfoMessageCategory::ApHint);
	}
	else if (currentHintEntity != -1) {
		// We're playing an audio log or near a laser. Show its hint, unless it's already been on screen for a while.
		if (currentHintEntityDuration <= 0.f) {
			HudManager::get()->clearInformationalMessage(InfoMessageCategory::ApHint);
		}
		else
		{
			std::string message = inGameHints[currentHintEntity].message;
			HudManager::get()->showInformationalMessage(InfoMessageCategory::ApHint, message);
		}

		currentHintEntityDuration -= deltaSeconds;
	}
	else if (currentHintEntityDuration > 0.f) {
		// We don't have an audio log playing and aren't near a laser. Run out the timer on any hint that may still be playing.
		currentHintEntityDuration -= deltaSeconds;
		if (currentHintEntityDuration <= 0.f) {
			HudManager::get()->clearInformationalMessage(InfoMessageCategory::ApHint);
		}
	}
	else {
		HudManager::get()->clearInformationalMessage(InfoMessageCategory::ApHint);
	}
	std::vector<inGameHint> seenMessagesVec(seenMessages.begin(), seenMessages.end());
	std::sort(seenMessagesVec.begin(), seenMessagesVec.end(), [pNO](inGameHint a, inGameHint b) {
		bool aIsArea = a.areaHint != "";
		bool bIsArea = b.areaHint != "";
		
		if (aIsArea && bIsArea) {
			return a.message.compare(b.message) < 0;
		}
		else if (aIsArea) {
			return true;
		}
		else if (bIsArea) {
			return false;
		}
		
		if (a.locationID > b.locationID) {
			return true;
		}
		
		return a.message.compare(b.message) < 0;
	});
	std::vector<inGameHint> seenMessagesVecCleaned = {};
	for (inGameHint audioLogHint : seenMessagesVec) {
		if (audioLogHint.locationID == -1 && audioLogHint.playerNo == -1) {
			continue;
		}
		seenMessagesVecCleaned.push_back(audioLogHint);
	}

	std::vector<std::string> seenMessagesStrings = {};
	std::vector<std::string> fullyClearedAreas = {};
	std::vector<std::string> deadChecks = {};
	std::vector<std::string> otherPeoplesDeadChecks = {};

	std::map<std::string, bool> implicitlyClearedLocations = {};

	for (auto audioLogHint : seenMessagesVecCleaned) {
		std::string cleanedMessage = audioLogHint.message;
		std::replace(cleanedMessage.begin(), cleanedMessage.end(), '\n', ' ');

		if (audioLogHint.areaHint == "") {
			if (audioLogHint.playerNo != -1 && audioLogHint.playerNo != ap->get_player_number()) {
				if (locationsThatContainedItemsFromOtherPlayers.count({ audioLogHint.playerNo, audioLogHint.locationID })) {
					otherPeoplesDeadChecks.push_back(ap->get_location_name(audioLogHint.locationID, ap->get_player_game(audioLogHint.playerNo)) + " (" + ap->get_player_alias(audioLogHint.playerNo) + ")");

					continue;
				}
			}

			if (audioLogHint.playerNo == ap->get_player_number() && locationIdToItemFlags.count(audioLogHint.locationID)) {
				if (checkedLocations.count(audioLogHint.locationID) || (!(locationIdToItemFlags[audioLogHint.locationID] & APClient::ItemFlags::FLAG_ADVANCEMENT) && audioLogHint.allowScout)) {
					std::string name = ap->get_location_name(audioLogHint.locationID, "The Witness");
					if (locationIdToItemFlags[audioLogHint.locationID] & APClient::ItemFlags::FLAG_NEVER_EXCLUDE && !(locationIdToItemFlags[audioLogHint.locationID] & APClient::ItemFlags::FLAG_ADVANCEMENT)) {
						name += " (Useful)";
					}
					deadChecks.push_back(name);

					implicitlyClearedLocations[std::to_string(audioLogHint.locationID)] = true;

					continue;
				}
			}
		}
		else {
			int foundProgression = 0;
			int unchecked = 0;
			int unfoundHuntPanels = 0;
			bool hasHuntPanels = false;

			std::set<int64_t> associatedChecks = {};

			for (int64_t locationID : areaNameToLocationIDs[audioLogHint.areaHint]) {
				if (locationIdToItemFlags.count(locationID)) {
					associatedChecks.insert(locationID);

					if (checkedLocations.count(locationID)) {
						if (locationIdToItemFlags[locationID] & APClient::ItemFlags::FLAG_ADVANCEMENT) {
							foundProgression += 1;
						}
					}
					else {
						unchecked += 1;
					}
				}
			}

			for (int huntEntity : huntEntitiesPerArea[audioLogHint.areaHint]) {
				hasHuntPanels = true;
				if (!huntEntityToSolveStatus[huntEntity]) unfoundHuntPanels++;
			}

			if (audioLogHint.locationID == -1) {
				cleanedMessage += " (";

				if (hasHuntPanels) {
					if (unfoundHuntPanels == 0) {
						cleanedMessage += "All Hunt Panels found, ";
					}
					else if (unfoundHuntPanels == 1) {
						cleanedMessage += "1 Hunt Panel remaining, ";
					}
					else {
						cleanedMessage += std::to_string(unfoundHuntPanels) + " Hunt Panels remaining, ";
					}
				}

				if (audioLogHint.areaProgression != -1 && foundProgression >= audioLogHint.areaProgression) {
					if (!unfoundHuntPanels) {
						for (int64_t id : associatedChecks) {
							implicitlyClearedLocations[std::to_string(id)] = true;
						}

						fullyClearedAreas.push_back(audioLogHint.areaHint /* + " (" + std::to_string(foundProgression) + ")"*/);
						continue;
					}

					cleanedMessage += "all progression found";
				}
				else {

					cleanedMessage += std::to_string(audioLogHint.areaProgression - foundProgression);
					if (hasHuntPanels) cleanedMessage += " progression";
					cleanedMessage += " remaining, ";
					cleanedMessage += std::to_string(unchecked) + " unchecked";
				}

				cleanedMessage += ")";
			}
			else
			{
				if (unchecked == 0 || checkedLocations.count(audioLogHint.locationID)) {
					deadChecks.push_back(ap->get_location_name(audioLogHint.locationID, "The Witness"));
					continue;
				}
				cleanedMessage += " (" + std::to_string(unchecked) + " unchecked)";
			}
		}

		seenMessagesStrings.push_back(cleanedMessage);
	}

	if (lastDeadChecks != implicitlyClearedLocations) {
		ap->Set("WitnessDeadChecks" + std::to_string(pNO), nlohmann::json::object(), false, { { "update" , implicitlyClearedLocations } });
		lastDeadChecks = implicitlyClearedLocations;
	}

	ClientWindow::get()->displaySeenAudioHints(seenMessagesStrings, fullyClearedAreas, deadChecks, otherPeoplesDeadChecks);
}

void APWatchdog::CheckEPs() {
	if (!firstStorageCheckDone) {
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
	if (!firstStorageCheckDone) {
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

void APWatchdog::CheckPanels() {
	int pNO = ap->get_player_number();

	if (!firstStorageCheckDone) {
		ap->SetNotify({ "WitnessSolvedPanels" + std::to_string(pNO) });
		ap->Set("WitnessSolvedPanels" + std::to_string(pNO), nlohmann::json::object(), true, { { "default", nlohmann::json::object() } });
	}

	std::map<std::string, bool> newlySolvedPanels = {};

	for (int panel : allPanels) {
		if (solvedPanels.count(panel)) continue;

		if (ReadPanelData<int>(panel, SOLVED)) {
			std::stringstream stream;
			stream << std::hex << panel;
			std::string panel_str(stream.str());

			while (panel_str.size() < 5) {
				panel_str = "0" + panel_str;
			}

			panel_str = "0x" + panel_str;

			newlySolvedPanels[panel_str] = true;
			solvedPanels.insert(panel);
		}
	}

	if (newlySolvedPanels.size()) {
		ap->Set("WitnessSolvedPanels" + std::to_string(pNO), nlohmann::json::object(), false, { { "update" , newlySolvedPanels } });
	}
}

void APWatchdog::CheckHuntEntities() {
	if (!FirstEverLocationCheckDone) return;

	int pNO = ap->get_player_number();

	if (!firstStorageCheckDone) {
		ap->SetNotify({ "WitnessHuntEntityStatus" + std::to_string(pNO) });
		ap->Set("WitnessHuntEntityStatus" + std::to_string(pNO), nlohmann::json::object(), true, { { "default", nlohmann::json::object() } });
	}

	std::map<std::string, bool> newlySolvedHuntEntities = {};
	
	for (auto [huntEntity, solveStatus] : huntEntityToSolveStatus) {
		if (!solveStatus) continue;
		if (solvedHuntEntitiesDataStorage.count(huntEntity)) continue;
		
		std::stringstream stream;
		stream << std::hex << huntEntity;
		std::string panel_str(stream.str());

		while (panel_str.size() < 5) {
			panel_str = "0" + panel_str;
		}

		panel_str = "0x" + panel_str;

		newlySolvedHuntEntities[panel_str] = true;
	}

	if (newlySolvedHuntEntities.size()) {
		ap->Set("WitnessHuntEntityStatus" + std::to_string(pNO), nlohmann::json::object(), false, { { "update" , newlySolvedHuntEntities } });
	}
}

void APWatchdog::CheckDoors() {
	int pNO = ap->get_player_number();

	if (openedDoors.empty()) {
		ap->SetNotify({ "WitnessOpenedDoors" + std::to_string(pNO) });
		ap->Set("WitnessOpenedDoors" + std::to_string(pNO), nlohmann::json::object(), true, { { "default", nlohmann::json::object() } });
	}

	std::map<std::string, bool> newlyOpenedDoors = {};

	for (int door: UnlockableDoors) {
		if (openedDoors.count(door)) continue;

		if (ReadPanelData<int>(door, DOOR_OPEN)) {
			std::stringstream stream;
			stream << std::hex << door;
			std::string door_str(stream.str());

			while (door_str.size() < 5) {
				door_str = "0" + door_str;
			}

			door_str = "0x" + door_str;

			newlyOpenedDoors[door_str] = true;
			openedDoors.insert(door);
		}
	}

	if (newlyOpenedDoors.size()) {
		ap->Set("WitnessOpenedDoors" + std::to_string(pNO), nlohmann::json::object(), false, { { "update" , newlyOpenedDoors } });
	}
}

void APWatchdog::SetValueFromServer(std::string key, nlohmann::json value) {
	if (key == DeathLinkDataStorageKey) {
		if (DeathLinkAmnesty == -1) return;
		if (DeathLinkCount != value) {
			DeathLinkCount = value;
			HudManager::get()->queueNotification("Updated Death Link Amnesty from Server. Remaining: " + std::to_string(DeathLinkAmnesty - DeathLinkCount) + ".");
		}
	}

	if (key.find("WitnessDisabledDeathLink") != std::string::npos) {
		bool disableDeathLink = value == true;

		if (DeathLinkAmnesty == -1) return;

		if (disableDeathLink) {
			DeathLinkAmnesty = -1;

			if (deathLinkFirstResponse) {
				std::list<std::string> newTags = { };
				ap->ConnectUpdate(false, 7, true, newTags);
			}
			HudManager::get()->queueNotification("Death Link has been permanently disabled on this device for this slot.");
		}
		else {
			if (!deathLinkFirstResponse) {
				std::list<std::string> newTags = { "DeathLink" };
				ap->ConnectUpdate(false, 7, true, newTags);
			}
		}
		

		deathLinkFirstResponse = true;
		ClientWindow::get()->EnableDeathLinkDisablingButton(!disableDeathLink);
	}
}

void APWatchdog::HandleLaserResponse(std::string laserID, nlohmann::json value) {
	int laserNo = laserIDsToLasers[laserID];

	bool laserActiveAccordingToDataStore = value == true;

	bool laserActiveInGame = ReadPanelData<int>(laserNo, LASER_TARGET) != 0;

	if (laserActiveInGame == laserActiveAccordingToDataStore) return;

	if(!laserActiveInGame && SyncProgress)
	{
		Memory::get()->ActivateLaser(laserNo);
		HudManager::get()->queueNotification(laserNames[laserNo] + " Laser Activated Remotely (Coop)", getColorByItemFlag(APClient::ItemFlags::FLAG_ADVANCEMENT));
	}
}

void APWatchdog::HandleWarpResponse(nlohmann::json value) {
	std::map<std::string, bool> warpsAcquiredAccordingToDataStorage = value;
	
	std::vector<std::string> unlockedWarps = {};
	for (auto [warp, val] : warpsAcquiredAccordingToDataStorage) {
		unlockedWarps.push_back(warp);
	}
	
	UnlockWarps(unlockedWarps);
}

void APWatchdog::HandleHuntEntityResponse(nlohmann::json value) {
	std::map<std::string, bool> entitiesSolvedAccordingToDataStorage = value;
	
	std::set<int> remotelySolvedHuntEntities;
	
	for (auto [entityString, dataStorageSolveStatus] : entitiesSolvedAccordingToDataStorage) {
		if (!dataStorageSolveStatus) continue;

		int entityID = std::stoul(entityString, nullptr, 16);

		if (!huntEntityToSolveStatus.count(entityID)) {
			HudManager::get()->queueBannerMessage("Got faulty EntityHunt response for entity " + entityString);
			continue;
		}

		solvedHuntEntitiesDataStorage.insert(entityID);

		if (!SyncProgress) continue;

		if (huntEntityToSolveStatus[entityID]) continue;

		huntEntityToSolveStatus[entityID] = true;
		remotelySolvedHuntEntities.insert(entityID);
	}

	if (remotelySolvedHuntEntities.empty()) return;

	int huntSolveTotal = 0;
	for (auto [huntEntity, currentSolveStatus] : huntEntityToSolveStatus) {
		if (currentSolveStatus) huntSolveTotal++;
	}

	state->solvedHuntEntities = huntSolveTotal;
	panelLocker->UpdatePuzzleLock(*state, 0x03629);

	std::string message = "Hunt Panels were ";
	if (remotelySolvedHuntEntities.size() == 1) {
		int entityID = *(remotelySolvedHuntEntities.begin());
		message = "Hunt Panel ";
		if (entityToName.count(entityID)) {
			message += entityToName[entityID];
		}
		message += " was ";
	}
	message += "solved remotely (Coop). New total: ";
	message += std::to_string(huntSolveTotal) + "/" + std::to_string(state->requiredHuntEntities);

	HudManager::get()->queueNotification(message, getColorByItemFlag(APClient::ItemFlags::FLAG_ADVANCEMENT));
}

void APWatchdog::HandleEPResponse(std::string epID, nlohmann::json value) {
	int epNo = EPIDsToEPs[epID];

	bool epActiveAccordingToDataPackage = value == true;

	bool epActiveInGame = ReadPanelData<int>(epNo, EP_SOLVED) != 0;

	if (epActiveInGame == epActiveAccordingToDataPackage) return;

	if (!epActiveInGame && (SyncProgress))
	{
		Memory::get()->SolveEP(epNo);
		if (precompletableEpToName.count(epNo) && precompletableEpToPatternPointBytes.count(epNo) && EPShuffle) {
			Memory::get()->MakeEPGlow(precompletableEpToName.at(epNo), precompletableEpToPatternPointBytes.at(epNo));
		}
		if (SyncProgress){
			HudManager::get()->queueNotification("EP Activated Remotely (Coop)", getColorByItemFlag(APClient::ItemFlags::FLAG_ADVANCEMENT)); //TODO: Names
		}
	}
}

void APWatchdog::HandleAudioLogResponse(std::string logIDstr, nlohmann::json value) {
	bool audioLogSeen = value == true;
	int logID = stoi(logIDstr.substr(logIDstr.find("-") + 1));

	if (!audioLogSeen) return;

	if (SyncProgress)
	{
		WritePanelData<int>(logID, AUDIO_LOG_PLAYED, { 1 });
		seenAudioLogs.insert(logID);
	}
}

void APWatchdog::HandleLaserHintResponse(std::string laserIDstr, nlohmann::json value) {
	bool laserHintSeen = value == true;
	int laserID = stoi(laserIDstr.substr(laserIDstr.find("-") + 1));

	if (!laserHintSeen) return;

	if (SyncProgress)
	{
		WritePanelData<float>(laserID, 0x108, { 1.0001 });
		seenLasers.insert(laserID);
	}
}

void APWatchdog::HandleSolvedPanelsResponse(nlohmann::json value) {
	std::wstringstream s;
	s << "Solved Panels: " << value.dump().c_str() << "\n";

	OutputDebugStringW(s.str().c_str());
}

void APWatchdog::HandleOpenedDoorsResponse(nlohmann::json value) {
	std::wstringstream s;
	s << "Solved Doors: " << value.dump().c_str() << "\n";

	OutputDebugStringW(s.str().c_str());
}

void APWatchdog::setLocationItemFlag(int64_t location, unsigned int flags) {
	locationIdToItemFlags[location] = flags;

	PotentiallyColorPanel(location);
}

void APWatchdog::PotentiallyColorPanel(int64_t location, bool overrideRecolor) {
	if (!locationIdToPanelId_READ_ONLY.count(location)) return;
	if (!checkedLocations.count(location)) return;
	int panelId = locationIdToPanelId_READ_ONLY[location];

	if (recolorWhenSolved.contains(panelId) && !overrideRecolor) return;
	if (!allPanels.count(panelId) || PuzzlesSkippedThisGame.count(panelId)) return;
	if (!locationIdToItemFlags.count(location)) return;

	alreadyColored.insert(location);
	APWatchdog::SetItemRewardColor(panelId, locationIdToItemFlags[location]);
}

void APWatchdog::InfiniteChallenge(bool enable) {
	if (enable) HudManager::get()->queueBannerMessage("Challenge Timer disabled.");
	if (!enable) HudManager::get()->queueBannerMessage("Challenge Timer reenabled.");
}

void APWatchdog::CheckImportantCollisionCubes() {
	std::vector<float> playerPosition;

	try {
		playerPosition = Memory::get()->ReadPlayerPosition();
	}
	catch (std::exception e) {
		return;
	}

	insideChallengeBoxRange = challengeTimer->containsPoint(playerPosition);

	
	if (severedDoorsList.count(0x012FB) || severedDoorsList.count(0x01317)) {
		if (ReadPanelData<int>(0x012FB, LASER_TARGET)) {
			if (ReadPanelData<float>(0x01317, DOOR_OPEN_T_TARGET) > 1.0f) WritePanelData<float>(0x01317, DOOR_OPEN_T_TARGET, { 1.0f });
		}

		else if (ReadPanelData<float>(0x01317, DOOR_OPEN_T) != 1.0f && ReadPanelData<float>(0x01317, DOOR_OPEN_T_TARGET) == 1.0f) WritePanelData<float>(0x01317, DOOR_OPEN_T_TARGET, { 1.001f });

		else if (!desertLaserHasBeenUpWhileConnected && ReadPanelData<float>(0x01317, DOOR_OPEN_T) == 1.0f && ReadPanelData<float>(0x01317, DOOR_OPEN_T_TARGET) > 1.0f) {
			WritePanelData<float>(0x01317, DOOR_OPEN_T_TARGET, { 1.0f });
			WritePanelData<float>(0x01317, DOOR_OPEN_T, { 0.99f });
		}
		else if (ReadPanelData<float>(0x01317, DOOR_OPEN_T) == 1.0f && ReadPanelData<float>(0x01317, DOOR_OPEN_T_TARGET) == 1.0f) {
			WritePanelData<INT64>(0x27877, LIGHTMAP_TABLE, { 0 });
			WritePanelData<INT64>(0x01317, LIGHTMAP_TABLE, { 0 });
			WritePanelData<float>(0x01317, DOOR_OPEN_T_TARGET, { 1.001f });
			desertLaserHasBeenUpWhileConnected = true;
		}
	}

	if (ElevatorsComeToYou){
		if (quarryElevatorUpper->containsPoint(playerPosition) && ReadPanelData<float>(0x17CC1, DOOR_OPEN_T) == 1.0f && ReadPanelData<float>(0x17CC1, DOOR_OPEN_T_TARGET) == 1.0f) {
			ASMPayloadManager::get()->BridgeToggle(0x17CC4, false);
		}

		if (quarryElevatorLower->containsPoint(playerPosition) && ReadPanelData<float>(0x17CC1, DOOR_OPEN_T) == 0.0f && ReadPanelData<float>(0x17CC1, DOOR_OPEN_T_TARGET) == 0.0f) {
			ASMPayloadManager::get()->BridgeToggle(0x17CC4, true);
		}

		if (bunkerElevatorCube->containsPoint(playerPosition)) {
			std::vector<int> allBunkerElevatorDoors = { 0x0A069, 0x0A06A, 0x0A06B, 0x0A06C, 0x0A070, 0x0A071, 0x0A072, 0x0A073, 0x0A074, 0x0A075, 0x0A076, 0x0A077 };

			bool problem = false;

			for (int id : allBunkerElevatorDoors) {
				if (ReadPanelData<float>(id, DOOR_OPEN_T) != ReadPanelData<float>(id, DOOR_OPEN_T_TARGET)) {
					problem = true; // elevator is currently moving
					break;
				}
			}

			if (ReadPanelData<float>(0x0A076, DOOR_OPEN_T) == 0.0f && ReadPanelData<float>(0x0A075, DOOR_OPEN_T) == 1.0f) problem = true; // Already in the correct spot

			if(!problem) ASMPayloadManager::get()->SendBunkerElevatorToFloor(5, true);
		}

		if (swampLongBridgeNear->containsPoint(playerPosition) && ReadPanelData<float>(0x17E74, DOOR_OPEN_T) == 1.0f) {
			if (ReadPanelData<float>(0x1802C, DOOR_OPEN_T) == ReadPanelData<float>(0x1802C, DOOR_OPEN_T_TARGET) && ReadPanelData<float>(0x17E74, DOOR_OPEN_T_TARGET) == 1.0f) {
				ASMPayloadManager::get()->ToggleFloodgate("floodgate_control_arm_a", false);
				ASMPayloadManager::get()->ToggleFloodgate("floodgate_control_arm_b", true);
			}
		}

		if (swampLongBridgeFar->containsPoint(playerPosition) && ReadPanelData<float>(0x1802C, DOOR_OPEN_T) == 1.0f) {
			if (ReadPanelData<float>(0x17E74, DOOR_OPEN_T) == ReadPanelData<float>(0x17E74, DOOR_OPEN_T_TARGET) && ReadPanelData<float>(0x1802C, DOOR_OPEN_T_TARGET) == 1.0f) {
				ASMPayloadManager::get()->ToggleFloodgate("floodgate_control_arm_a", true);
				ASMPayloadManager::get()->ToggleFloodgate("floodgate_control_arm_b", false);
			}
		}

		if (townRedRoof->containsPoint(playerPosition) && ReadPanelData<float>(0x2897C, DOOR_OPEN_T) != 1.0f && ReadPanelData<float>(0x2897C, DOOR_OPEN_T) == ReadPanelData<float>(0x2897C, DOOR_OPEN_T_TARGET)) {
			ASMPayloadManager::get()->OpenDoor(0x2897C);
		}
	}


	if (timePassedSinceRandomisation <= 4.0f) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::Settings,
			"Collect Setting: " + CollectText + ".\nDisabled Setting: " + DisabledPuzzlesBehavior + ".");
		return;
	}
	else if (0.0f < timePassedSinceFirstJinglePlayed && timePassedSinceFirstJinglePlayed < 10.0f && ClientWindow::get()->getJinglesSettingSafe() != "Off") {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::Settings,
			"Change Volume of jingles using the Windows Volume Mixer, where the Randomizer Client will show up as its own app.");
		return;
	}

	HudManager::get()->clearInformationalMessage(InfoMessageCategory::Settings);

	if (tutorialPillarCube->containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0xc335)) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol,
			"Stone Pillar needs Triangles.");
	}
	else if ((riverVaultLowerCube->containsPoint(playerPosition) || riverVaultUpperCube->containsPoint(playerPosition)) && panelLocker->PuzzleIsLocked(0x15ADD)) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol,
			"Needs Dots, Black/White Squares.");
	}
	else if (bunkerPuzzlesCube->containsPoint(playerPosition) && panelLocker->PuzzleIsLocked(0x09FDC)) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol,
			"Most Bunker panels need Black/White Squares and Colored Squares.");
	}
	else if (ReadPanelData<int>(0x0C179, 0x1D4) && panelLocker->PuzzleIsLocked(0x01983)) {
		if (ReadPanelData<int>(0x0C19D, 0x1D4) && panelLocker->PuzzleIsLocked(0x01987)) {
			HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol,
				"Left Panel needs Stars and Shapers.\n"
				"Right Panel needs Dots and Colored Squares.");
		}
		else
		{
			HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol,
				"Left Panel needs Stars and Shapers.");
		}
	}
	else if (ReadPanelData<int>(0x0C19D, 0x1D4) && panelLocker->PuzzleIsLocked(0x01987)) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol,
			"Right Panel needs Dots and Colored Squares.");
	}
	else {
		HudManager::get()->clearInformationalMessage(InfoMessageCategory::MissingSymbol);
	}

	if (bunkerLaserPlatform->containsPoint(playerPosition) && Utilities::isAprilFools()) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::FlavorText,
			"what if we kissed,,, on the Bunker Laser Platform,,,\nhaha jk,,,, unless??");
	}
	else
	{
		HudManager::get()->clearInformationalMessage(InfoMessageCategory::FlavorText);
	}
}

bool APWatchdog::CheckPanelHasBeenSolved(int panelId) {
	return !panelIdToLocationId.count(panelId);
}

void APWatchdog::SetItemRewardColor(const int& id, const int& itemFlags) {
	if (!allPanels.count(id)) return;

	Color backgroundColor;

	if (recolorWhenSolved.contains(id)) {
		backgroundColor = { 0.07f, 0.07f, 0.07f, 1.0f };
	}
	else if (itemFlags & APClient::ItemFlags::FLAG_ADVANCEMENT){
		backgroundColor = { 0.686f, 0.6f, 0.937f, 1.0f };
	}
	else if (itemFlags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
		backgroundColor = { 0.427f, 0.545f, 0.91f, 1.0f };
	}
	else if (itemFlags & APClient::ItemFlags::FLAG_TRAP){
		backgroundColor = { 0.98f, 0.502f, 0.447f, 1.0f };
	}
	else {
		backgroundColor = { 0.0f , 0.933f, 0.933f, 1.0f };
	}

	WriteRewardColorToPanel(id, backgroundColor);
}

void APWatchdog::WriteRewardColorToPanel(int id, Color color) {
	if (id == 0x28998 || id == 0x28A69 || id == 0x17CAA || id == 0x00037 || id == 0x09FF8 || id == 0x09DAF || id == 0x0A01F || id == 0x17E67) {
		WritePanelData<Color>(id, SUCCESS_COLOR_A, { color });
	}
	else
	{
		WritePanelData<Color>(id, BACKGROUND_REGION_COLOR, { color });
	}
	WritePanelData<int>(id, NEEDS_REDRAW, { 1 });
}

bool APWatchdog::PanelShouldPlayEpicVersion(const int& id) {
	// No such thing as an epic version on Understated
	if (ClientWindow::get()->getJinglesSettingSafe() != "Full") return false;

	if (hardPanels.count(id)) {
		return true;
	}
	
	if (id == 0x0360D || id == 0x03616 || id == 0x03608 || id == 0x17CA4 || id == 0x032F5 || id == 0x09DE0 || id == 0x03613) { // Symmetry, Jungle, Desert, Monastery, Town, Bunker, Treehouse Laser Panel
		return true;
	}
	if (id == 0x03612) { // Quarry Laser Panel: Make sure it wasn't skipped with neither Boathouse nor Stoneworks done
		int skip = ReadPanelData<int>(id, VIDEO_STATUS_COLOR);

		return !(skip >= PUZZLE_SKIPPED + 2 && skip <= PUZZLE_SKIPPED_MAX) || !severedDoorsList.count(id);
	}
	if (id == 0x19650) { // Shadows Laser Panel: In door shuffle, this is trivial
		return !severedDoorsList.count(0x19665) || !severedDoorsList.count(0x194B2) || !severedDoorsList.count(id);
	}
	if (id == 0x0360E || id == 0x03317) { // Keep Panels: Make sure they weren't skipped in a doors mode
		int skip = ReadPanelData<int>(id, VIDEO_STATUS_COLOR);

		return !(skip >= PUZZLE_SKIPPED + 2 && skip <= PUZZLE_SKIPPED_MAX) || !severedDoorsList.count(0x01BEC) || !severedDoorsList.count(id);
	}
	if (id == 0x03615) { // Swamp Laser Panel: Make sure it wasn't acquired through the shortcut in a doors mode
		return !(severedDoorsList.count(0x2D880) && ReadPanelData<int>(0x2D880, DOOR_OPEN)) || !severedDoorsList.count(id);
	}

	return false;
}

void APWatchdog::PlaySentJingle(const int& id, const int& itemFlags) {
	if (ClientWindow::get()->getJinglesSettingSafe() == "Off") return;
	if (0.0f < timePassedSinceFirstJinglePlayed && timePassedSinceFirstJinglePlayed < 10.0f) return;

	if (alreadyPlayedHuntEntityJingle.count(id)) return;

	bool isEP = allEPs.count(id);

	bool epicVersion = PanelShouldPlayEpicVersion(id);

	if (ClientWindow::get()->getJinglesSettingSafe() == "Understated") {
		if (itemFlags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
			APAudioPlayer::get()->PlayAudio(APJingle::UnderstatedProgression, APJingleBehavior::Queue, epicVersion);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
			APAudioPlayer::get()->PlayAudio(APJingle::UnderstatedUseful, APJingleBehavior::Queue, epicVersion);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_TRAP) {
			APAudioPlayer::get()->PlayAudio(APJingle::UnderstatedTrap, APJingleBehavior::Queue, epicVersion);
		}
		else {
			APAudioPlayer::get()->PlayAudio(APJingle::UnderstatedFiller, APJingleBehavior::Queue, epicVersion);
		}
		return;
	}

	if (isEP){
		if (itemFlags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
			APAudioPlayer::get()->PlayAudio(APJingle::EPProgression, APJingleBehavior::Queue, epicVersion);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
			APAudioPlayer::get()->PlayAudio(APJingle::EPUseful, APJingleBehavior::Queue, epicVersion);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_TRAP) {
			APAudioPlayer::get()->PlayAudio(APJingle::EPTrap, APJingleBehavior::Queue, epicVersion);
		}
		else {
			APAudioPlayer::get()->PlayAudio(APJingle::EPFiller, APJingleBehavior::Queue, epicVersion);
		}
		return;
	}

	if (id == 0xFFF80) {
		if (itemFlags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
			APAudioPlayer::get()->PlayAudio(APJingle::DogProgression, APJingleBehavior::Queue, false);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
			APAudioPlayer::get()->PlayAudio(APJingle::DogUseful, APJingleBehavior::Queue, false);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_TRAP) {
			APAudioPlayer::get()->PlayAudio(APJingle::DogTrap, APJingleBehavior::Queue, false);
		}
		else {
			APAudioPlayer::get()->PlayAudio(APJingle::DogFiller, APJingleBehavior::Queue, false);
		}
		return;
	}

	if (finalRoomMusicTimer != -1) {
		if (itemFlags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
			APAudioPlayer::get()->PlayFinalRoomJingle("progression", APJingleBehavior::Queue, finalRoomMusicTimer);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
			APAudioPlayer::get()->PlayFinalRoomJingle("useful", APJingleBehavior::Queue, finalRoomMusicTimer);
		}
		else if (itemFlags & APClient::ItemFlags::FLAG_TRAP) {
			APAudioPlayer::get()->PlayFinalRoomJingle("trap", APJingleBehavior::Queue, finalRoomMusicTimer);
		}
		else {
			APAudioPlayer::get()->PlayFinalRoomJingle("filler", APJingleBehavior::Queue, finalRoomMusicTimer);
		}
		return;
	}

	if (itemFlags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
		APAudioPlayer::get()->PlayAudio(APJingle::PanelProgression, APJingleBehavior::Queue, epicVersion);
	}
	else if (itemFlags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
		APAudioPlayer::get()->PlayAudio(APJingle::PanelUseful, APJingleBehavior::Queue, epicVersion);
	}
	else if (itemFlags & APClient::ItemFlags::FLAG_TRAP) {
		APAudioPlayer::get()->PlayAudio(APJingle::PanelTrap, APJingleBehavior::Queue, epicVersion);
	}
	else {
		APAudioPlayer::get()->PlayAudio(APJingle::PanelFiller, APJingleBehavior::Queue, epicVersion);
	}
}

void APWatchdog::PlayReceivedJingle(const int& itemFlags) {
	if (ClientWindow::get()->getJinglesSettingSafe() == "Off") return;
	if (0.0f < timePassedSinceFirstJinglePlayed && timePassedSinceFirstJinglePlayed < 10.0f) return;

	if (itemFlags & APClient::ItemFlags::FLAG_ADVANCEMENT) {
		APAudioPlayer::get()->PlayAudio(APJingle::IncomingProgression, APJingleBehavior::DontQueue);
	}
	else if (itemFlags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE) {
		APAudioPlayer::get()->PlayAudio(APJingle::IncomingUseful, APJingleBehavior::DontQueue);
	}
	else if (itemFlags & APClient::ItemFlags::FLAG_TRAP) {
		APAudioPlayer::get()->PlayAudio(APJingle::IncomingTrap, APJingleBehavior::DontQueue);
	}
	else {
		APAudioPlayer::get()->PlayAudio(APJingle::IncomingFiller, APJingleBehavior::DontQueue);
	}
}

void APWatchdog::PlayFirstJingle() {
	if (firstJinglePlayed) return;

	if (ClientWindow::get()->getJinglesSettingSafe() != "Off") {
		APAudioPlayer::get()->PlayAudio(APJingle::FirstJingle, APJingleBehavior::PlayImmediate);
		firstJinglePlayed = true;
	}
}

void APWatchdog::PlayEntityHuntJingle(const int& huntEntity) {
	if (0.0f < timePassedSinceFirstJinglePlayed && timePassedSinceFirstJinglePlayed < 10.0f) return;

	alreadyPlayedHuntEntityJingle.insert(huntEntity);

	if (ClientWindow::get()->getJinglesSettingSafe() == "Off") return;

	if (panelIdToLocationId_READ_ONLY.count(huntEntity) && !CheckPanelHasBeenSolved(huntEntity)) return;

	float percentage = (float) state->solvedHuntEntities / (float) state->requiredHuntEntities;

	if (ClientWindow::get()->getJinglesSettingSafe() == "Understated") {
		APAudioPlayer::get()->PlayAudio(APJingle::UnderstatedEntityHunt, APJingleBehavior::Queue);
	}
	else {
		APAudioPlayer::get()->PlayAudio(APJingle::EntityHunt, APJingleBehavior::Queue, percentage);
	}
}

void APWatchdog::CheckEPSkips() {
	if (!EPShuffle) return;

	std::set<int> panelsToSkip = {};
	std::set<int> panelsToRemoveSilently = {};

	for (int panel : panelsThatHaveToBeSkippedForEPPurposes) {
		if (PuzzlesSkippedThisGame.count(panel)) {
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

		if (panel == 0x033EA || panel == 0x01BE9 || panel == 0x01CD3 || panel == 0x01D3F || panel == 0x181F5 || panel == 0x03629) {
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
		HudManager::get()->queueBannerMessage("Puzzle symbols removed to make Environmental Pattern solvable.");
	}

	for (int panel : panelsToSkip) {
		panelLocker->PermanentlyUnlockPuzzle(panel, *state);
		panelsThatHaveToBeSkippedForEPPurposes.erase(panel);
		SkipPanel(panel, "Skipped", false, 0);
	}
}

void APWatchdog::QueueItemMessages() {
	if (newItemsJustIn) {
		newItemsJustIn = false;
		return;
	}

	std::map<std::string, int> itemCounts;
	std::map<std::string, RgbColor> itemColors;
	std::vector<std::string> receivedItems;

	for (auto it = queuedItems.begin(); it != queuedItems.end();) {
		__int64 item = it->at(0);
		__int64 flags = it->at(1);
		__int64 realitem = it->at(2);

		if (ap->get_item_name(realitem, "The Witness") == "Unknown" || ap->get_item_name(item, "The Witness") == "Unknown") {
			it++;
			continue;
		}

		std::string name = ap->get_item_name(item, "The Witness");

		if (realitem != item && realitem > 0) {
			name += " (" + ap->get_item_name(realitem, "The Witness") + ")";
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

		HudManager::get()->queueNotification("Received " + name + count + ".", itemColors[name]);
	}
}

void APWatchdog::QueueReceivedItem(std::vector<__int64> item) {
	newItemsJustIn = true;

	queuedItems.push_back(item);
}

void APWatchdog::SetStatusMessages() {
	const InputWatchdog* inputWatchdog = InputWatchdog::get();
	InteractionState interactionState = inputWatchdog->getInteractionState();
	if (interactionState == InteractionState::Walking) {
		if (eee) {
			HudManager::get()->clearWalkStatusMessage();
		}
		else {
			if (isKnockedOut) {
				int secondsRemainingDeathLink = DEATHLINK_DURATION - DateTime::since(knockOutStartTime).count() / 1000;

				if (secondsRemainingDeathLink <= 0) {
					HudManager::get()->clearWalkStatusMessage();
				}
				else {
					HudManager::get()->setWalkStatusMessage("Knocked out for " + std::to_string(secondsRemainingDeathLink) + " seconds.");
				}
			}

			else {
				int speedTimeInt = (int)std::ceil(std::abs(speedTime));

				if (speedTime > 0) {
					HudManager::get()->setWalkStatusMessage("Speed Boost active for " + std::to_string(speedTimeInt) + " seconds.");
				}
				else if (speedTime < 0) {
					HudManager::get()->setWalkStatusMessage("Slowness active for " + std::to_string(speedTimeInt) + " seconds.");
				}
				else {
					HudManager::get()->clearWalkStatusMessage();
				}
			}
		}
	}
	else if (interactionState == InteractionState::Focusing || interactionState == InteractionState::Solving) {
		// Always show the number of puzzle skips available while in focus mode.
		int availableSkips = GetAvailablePuzzleSkips();
		std::string skipMessage = "Have " + std::to_string(availableSkips) + " Puzzle Skip" + (availableSkips != 1 ? "s" : "") + ".";

		if (lookingAtLockedEntity != -1) {
			std::string lockName = ap->get_item_name(doorToItemId[lookingAtLockedEntity], "The Witness");
			skipMessage = "Locked by: \"" + lockName + "\"\n" + skipMessage;
		}

		if (activePanelId != -1) {
			int solvingPressurePlateStartPoint = 0;
			int solvingPressurePlateAssociatedEPID = 0;

			// Weird PP allocation stuff
			for (int ppEP : {0x1AE8, 0x01C0D, 0x01DC7, 0x01E12, 0x01E52}) {
				int ppStartPointID = Memory::get()->ReadPanelData<int>(ppEP, PRESSURE_PLATE_PATTERN_POINT_ID) - 1;


				if (ppStartPointID > 0 && activePanelId == ppStartPointID) {
					solvingPressurePlateStartPoint = ppStartPointID;
					solvingPressurePlateAssociatedEPID = startPointToEPs.find(ppEP)->second[0];
				}
			}

			// If we have a special skip message for the current puzzle, show it above the skip count.
			if (puzzleSkipInfoMessage.size() > 0) {
				skipMessage = puzzleSkipInfoMessage + "\n" + skipMessage;
			}

			if (solvingPressurePlateStartPoint || (startPointToEPs.count(activePanelId) && !(activePanelId == EPtoStartPoint.find(0x3352F)->second && finalPanel == 0x03629))) {
				bool allDisabled = true;
				bool someDisabled = false;

				bool hasLockedEPs = false;

				std::string name = "(Unknown Key)";

				for(auto [ep, startPoint] : EPtoStartPoint){
					if ((solvingPressurePlateStartPoint == activePanelId && ep == solvingPressurePlateAssociatedEPID || startPoint == activePanelId) && ep) {
						bool disabled = find(DisabledEntities.begin(), DisabledEntities.end(), ep) != DisabledEntities.end();

						allDisabled = allDisabled && disabled;
						someDisabled = someDisabled || disabled;

						if(panelLocker->PuzzleIsLocked(ep)){
							hasLockedEPs = hasLockedEPs || !disabled;
							if (doorToItemId.count(ep)) {
								name = ap->get_item_name(doorToItemId[ep], "The Witness");
							}
						}
					}
				}

				if (interactionState == InteractionState::Solving) {
					if (allDisabled) {
						if (DisabledPuzzlesBehavior == "Prevent Solve") {
							HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol, "This EP is disabled.");
						}
						else {
							skipMessage = "This EP is disabled.\n" + skipMessage;
						}
					}
					else if (hasLockedEPs) {
						HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol, "This EP cannot be solved until you receive the " + name + ".");
					}
					else if (someDisabled) {
						skipMessage = "Some EPs for this start point are disabled.\n" + skipMessage;
					}
				}
			}

			if (DeathLinkAmnesty != -1) {
				if (activePanelId != -1 && allPanels.count(activePanelId)) {
					if (deathlinkExcludeList.count(activePanelId) || PuzzleRandomization == SIGMA_EXPERT && deathlinkExpertExcludeList.count(activePanelId) || PuzzleRandomization == UMBRA_VARIETY && deathlinkVarietyExcludeList.count(activePanelId)) {
						skipMessage = "This panel is excluded from DeathLink.\n" + skipMessage;
					}
					else {
						if (DeathLinkAmnesty != 0) {
							if (DeathLinkCount == DeathLinkAmnesty) {
								skipMessage = "The next panel fail will cause a DeathLink.\n" + skipMessage;
							}
							else {
								skipMessage = "Remaining DeathLink Amnesty: " + std::to_string(DeathLinkAmnesty - DeathLinkCount) + ".\n" + skipMessage;
							}
						}
					}
				}
			}

			if (CanUsePuzzleSkip()) {
				// We have a selected, skippable, affordablSe puzzle. Show an input hint.
				skipMessage += " Hold [" + inputWatchdog->getNameForInputButton(inputWatchdog->getCustomKeybind(CustomKey::SKIP_PUZZLE)) + "] to use.";

				// Show special costs, if any.
				if (puzzleSkipCost == 0) {
					skipMessage += " (Free!)";
				}
				else if (puzzleSkipCost > 1) {
					skipMessage += " (Costs " + std::to_string(puzzleSkipCost) + ".)";
				}
			}
			else if (availableSkips > 0 && puzzleSkipCost > availableSkips) {
				// The player has some puzzle skips available, but not enough to solve the selected puzzle. Note that this
				//   message will only show for puzzles that cost more than one skip.
				skipMessage += "(Costs " + std::to_string(puzzleSkipCost) + ".)";
			}
		}

		HudManager::get()->setSolveStatusMessage(skipMessage);
	}
	else if (interactionState == InteractionState::Keybinding) {
//		CustomKey currentKey = inputWatchdog->getCurrentlyRebindingKey();
//		HudManager::get()->setStatusMessage("Press key to bind to " + inputWatchdog->getNameForCustomKey(currentKey) + "... (ESC to cancel)");
	}
	else if (interactionState == InteractionState::Sleeping || interactionState == InteractionState::MenuAndSleeping) {
		if (selectedWarp == NULL) {
			HudManager::get()->setWalkStatusMessage("Sleeping.");
		} else {
			std::string message = "Sleeping. Press [" + inputWatchdog->getNameForInputButton(inputWatchdog->getCustomKeybind(CustomKey::SKIP_PUZZLE)) + "] to warp to: " + selectedWarp->name;
			if (unlockedWarps.size() > 1 && ClientWindow::get()->getSetting(ClientToggleSetting::ExtraInfo)) {
				message = "Use your left & right movement keys to cycle between warps.\n" + message;
			}
			HudManager::get()->setWalkStatusMessage(message);
		}
	}
	else if (interactionState == InteractionState::Warping) {
		if (inputWatchdog->warpIsGoingOvertime()) {
			HudManager::get()->setWalkStatusMessage("Warping... This is taking longer than expected.\n(Please have the game in focus while warping!)");
		}
		else if (warpRetryTime > 0.0f) {
			HudManager::get()->setWalkStatusMessage("Warping... This is taking longer than expected.\n(Please have the game in focus while warping!)");
		}
		else {
			HudManager::get()->setWalkStatusMessage("Warping...");
		}
	}
	else {
//		HudManager::get()->clearStatusMessage();
	}
}

int APWatchdog::GetActivePanel() {
	try {
		int puzzle = Memory::get()->GetActivePanel();

		if (puzzle == 0x0A3A8) {
			puzzle = 0x033EA;
		}
		else if (puzzle == 0x0A3B9) {
			puzzle = 0x01BE9;
		}
		else if (puzzle == 0x0A3BB) {
			puzzle = 0x01CD3;
		}
		else if (puzzle == 0x0A3AD) {
			puzzle = 0x01D3F;
		}

		return puzzle;
	}
	catch (std::exception& e) {
		OutputDebugStringW(L"Couldn't get active panel");
		return -1;
	}
}

void APWatchdog::WriteMovementSpeed(float currentSpeed) {
	try {
		return Memory::get()->WriteMovementSpeed(currentSpeed);
	}
	catch (std::exception& e) {
		OutputDebugStringW(L"Couldn't get active panel");
	}
}

Vector3 APWatchdog::getCachedEntityPosition(int id) {
	if (!recordedEntityPositions.count(id)) recordedEntityPositions[id] = Vector3(ReadPanelData<float>(id, POSITION, 3));
	return recordedEntityPositions[id];
}

Vector3 APWatchdog::getCameraDirection() {
	std::vector<float> cameraAngle = Memory::get()->ReadCameraAngle();

	float pitch = cameraAngle[0];
	float yaw = cameraAngle[1];

	float xzLen = cos(pitch);
	return Vector3(xzLen * cos(yaw), sin(pitch), xzLen * sin(-yaw));
}

void APWatchdog::LookingAtLockedEntity() {
	lookingAtLockedEntity = -1;

	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState != InteractionState::Focusing) {
		HudManager::get()->clearInformationalMessage(InfoMessageCategory::EnvironmentalPuzzle);
		return;
	}

	Vector3 headPosition = Vector3(Memory::get()->ReadPlayerPosition());
	Vector3 cursorDirection = InputWatchdog::get()->getMouseDirection();
	Vector3 cameraDirection = getCameraDirection();

	// EP Stuff

	int lookingAtEP = -1;
	float distanceToCenter = 10000000.0f;

	for (int epID : allEPs) {
		Vector3 obeliskPosition = getCachedEntityPosition(epID);

		if ((headPosition - obeliskPosition).length() > 7) {
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

		Vector3 facingVec = Vector3(facing);

		float dotProduct = cursorDirection * facingVec;

		// not facing the right direction
		if (dotProduct > 0) continue;

		Vector3 entityPosition = Vector3(ReadPanelData<float>(epID, POSITION, 3));

		Vector3 v = entityPosition - headPosition;
		float t = cursorDirection * v;

		// not in front of it
		if (t < 0) continue;
		Vector3 p = headPosition + (cursorDirection * t);

		float distance = (p - entityPosition).length();

		float boundingRadius = ReadPanelData<float>(epID, BOUNDING_RADIUS);

		if (distance < boundingRadius && distance < distanceToCenter) {
			distanceToCenter = distance;
			lookingAtEP = epID;
		}
	}

	if (entityToName.count(lookingAtEP)) {
		std::string epName = entityToName[lookingAtEP];

		if (epToObeliskSides.count(lookingAtEP)) {
			int obeliskSideHex = epToObeliskSides[lookingAtEP];
			if (panelIdToLocationId.count(obeliskSideHex)) {
				epName += " (" + ap->get_location_name(panelIdToLocationId[obeliskSideHex], "The Witness") + ")";
			}

			HudManager::get()->showInformationalMessage(InfoMessageCategory::EnvironmentalPuzzle, entityToName[lookingAtEP] + " (" + + ")");
		}

		HudManager::get()->showInformationalMessage(InfoMessageCategory::EnvironmentalPuzzle, epName);
	}
	else {
		HudManager::get()->clearInformationalMessage(InfoMessageCategory::EnvironmentalPuzzle);
	}

	// Door stuff

	if (!ClientWindow::get()->getSetting(ClientToggleSetting::ExtraInfo)) {
		return;
	}

	//if (activePanelId != -1) return;

	std::set<int> candidateEntities;

	for (int lockedPanelID : state->keysInTheGame) {
		if (!allPanels.count(lockedPanelID)) continue;

		Vector3 entityPosition = getCachedEntityPosition(lockedPanelID);
		if ((headPosition - entityPosition).length() > 5) {
			continue;
		}

		if (!omniDirectionalEntities.count(lockedPanelID)) {
			std::vector<float> qData = ReadPanelData<float>(lockedPanelID, ORIENTATION, 4);

			Quaternion q;
			q.x = qData[0];
			q.y = qData[1];
			q.z = qData[2];
			q.w = qData[3];

			std::vector<float> facing = { 1, 0, 0 };
			q.RotateVector(facing);

			Vector3 facingVec = Vector3(facing);

			float dotProduct = cursorDirection * facingVec;

			// panel is facing away from camera
			if (dotProduct > 0) continue;
		}
		
		Vector3 v = (entityPosition - headPosition).normalized();
		float t = v * cameraDirection;

		// not facing it enough
		if (t < 0.35) continue;

		candidateEntities.insert(lockedPanelID);
	}

	for (int doorID : lockedDoors) {
		Vector3 entityPosition = getCachedEntityPosition(doorID);
		if ((headPosition - entityPosition).length() > 7) {
			continue;
		}

		Vector3 v = (entityPosition - headPosition).normalized();
		float t = v * cameraDirection;

		// not facing it enough (camera)
		if (t < 0.35 && !omniDirectionalEntities.count(doorID)) continue;

		candidateEntities.insert(doorID);
	}

	std::set<int> candidates;

	int lookingAtLockedEntityCandidate = -1;
	float distanceToNearestLockedEntity = 1000000.f;

	// find closest
	for (int id : candidateEntities) {
		Vector3 entityPosition = Vector3(ReadPanelData<float>(id, POSITION, 3));

		float distance = (entityPosition - entityPosition).length();

		if (distance > distanceToNearestLockedEntity) continue;

		distanceToNearestLockedEntity = distance;
		lookingAtLockedEntityCandidate = id;
	}

	if (lookingAtLockedEntityCandidate == -1 || state->keysReceived.count(lookingAtLockedEntityCandidate) || unlockedDoors.count(lookingAtLockedEntityCandidate)) {
		return;
	}
	if (doorToItemId.count(lookingAtLockedEntityCandidate)) {
		lookingAtLockedEntity = lookingAtLockedEntityCandidate;
	}
	else {
		if (doorToItemId.empty()) return;
		/*
		std::stringstream s;
		s << std::hex << lookingAtLockedEntityCandidate;
		HudManager::get()->showInformationalMessage(InfoMessageCategory::MissingSymbol, "Locked entity with unknown name: " + s.str());
		*/
	}
}

void APWatchdog::PettingTheDog(float deltaSeconds) {
	Memory* memory = Memory::get();

	if (!LookingAtTheDog()) {
		letGoSinceInteractModeOpen = false;
		dogPettingDuration = 0;

		memory->writeCursorSize(1.0f);
		memory->writeCursorColor({ 1.0f, 1.0f, 1.0f });

		if (dogBarkDuration > 0.f) {
			dogBarkDuration -= deltaSeconds;
			if (dogBarkDuration <= 1.f) {
				HudManager::get()->clearInformationalMessage(InfoMessageCategory::Dog);
			}
		}
		else {
			HudManager::get()->clearInformationalMessage(InfoMessageCategory::Dog);
		}

		return;
	}
	else if (dogBarkDuration > 0.f) {
		dogBarkDuration -= deltaSeconds;
		if (dogBarkDuration <= 1.f) {
			HudManager::get()->clearInformationalMessage(InfoMessageCategory::Dog);
		}

		return;
	}

	memory->writeCursorColor({ 1.0f, 0.5f, 1.0f });

	if (dogPettingDuration >= 4.0f) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::Dog, "Woof Woof!");

		Memory::get()->writeCursorSize(1.0f);
		memory->writeCursorColor({ 1.0f, 1.0f, 1.0f });


		dogPettingDuration = 0.f;
		dogBarkDuration = 4.f;
		sentDog = true;
	}
	else if (dogPettingDuration > 0.f) {
		HudManager::get()->showInformationalMessage(InfoMessageCategory::Dog, "Petting progress: " + std::to_string((int)(dogPettingDuration * 100 / 4.0f)) + "%");
	}

	if (!InputWatchdog::get()->getButtonState(InputButton::MOUSE_BUTTON_LEFT) && !InputWatchdog::get()->getButtonState(InputButton::CONTROLLER_FACEBUTTON_DOWN)) {
		letGoSinceInteractModeOpen = true;

		Memory::get()->writeCursorSize(2.0f);
		return;
	}

	if (!letGoSinceInteractModeOpen) {
		Memory::get()->writeCursorSize(2.0f);
		return;
	}

	Memory::get()->writeCursorSize(1.0f);

	const Vector3& currentMouseDirection = InputWatchdog::get()->getMouseDirection();
	if (lastMouseDirection != currentMouseDirection)
	{
		dogPettingDuration += deltaSeconds;
	}

	lastMouseDirection = currentMouseDirection;
}

bool APWatchdog::LookingAtTheDog() const {
	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState != InteractionState::Focusing) {
		return false;
	}

	std::vector<float> headPosition = Memory::get()->ReadPlayerPosition();
	const Vector3& cursorDirection = InputWatchdog::get()->getMouseDirection();

	std::vector<std::vector<float>> dogPositions = { {-107.954, -101.36, 4.078}, {-107.9, -101.312, 4.1},{-107.866, -101.232, 4.16},{ -107.871, -101.150, 4.22 }, { -107.86, -101.06, 4.3 }, { -107.88, -100.967, 4.397 } };

	float distanceToDog = pow(headPosition[0] - dogPositions[1][0], 2) + pow(headPosition[1] - dogPositions[1][1], 2) + pow(headPosition[2] - dogPositions[1][2], 2);

	if (distanceToDog > 49) {
		return false;
	}

	float smallestDistance = 10000000.0f;
	float boundingRadius = 0.105f;

	for (auto dogPosition : dogPositions) {
		std::vector<float> v = { dogPosition[0] - headPosition[0], dogPosition[1] - headPosition[1], dogPosition[2] - headPosition[2] };
		float t = v[0] * cursorDirection.X + v[1] * cursorDirection.Y + v[2] * cursorDirection.Z;
		std::vector<float> p = { headPosition[0] + t * cursorDirection.X, headPosition[1] + t * cursorDirection.Y, headPosition[2] + t * cursorDirection.Z };

		float distance = sqrt(pow(p[0] - dogPosition[0], 2) + pow(p[1] - dogPosition[1], 2) + pow(p[2] - dogPosition[2], 2));

		if (distance < smallestDistance) {
			smallestDistance = distance;
		}
	}

	if (smallestDistance > boundingRadius) {
		return false;
	}

	return true;
}

APServerPoller::APServerPoller(APClient* client) : Watchdog(0.1f) {
	ap = client;
}

void APServerPoller::action() {
	ap->poll();
}


void APWatchdog::CheckDeathLink() {
	if (DeathLinkAmnesty == -1) return;

	int panelIdToConsider = mostRecentActivePanelId;

	for (int panelId : alwaysDeathLinkPanels) {
		if (ReadPanelData<int>(panelId, FLASH_MODE) == 2) panelIdToConsider = panelId;
	}

	if (panelIdToConsider == -1 || !allPanels.count(panelIdToConsider)) return;
	if (deathlinkExcludeList.count(panelIdToConsider)) return;
	if (PuzzleRandomization == SIGMA_EXPERT && deathlinkExpertExcludeList.count(panelIdToConsider) || PuzzleRandomization == UMBRA_VARIETY && deathlinkVarietyExcludeList.count(panelIdToConsider)) return;

	int newState = ReadPanelData<int>(panelIdToConsider, FLASH_MODE, 1, movingMemoryPanels.count(panelIdToConsider))[0];

	if (mostRecentPanelState != newState) {
		mostRecentPanelState = newState;

		if (newState == 2) {
			DeathLinkCount++;
			if (DeathLinkCount > DeathLinkAmnesty) {
				SendDeathLink(mostRecentActivePanelId);
				HudManager::get()->queueNotification("Death Sent.", getColorByItemFlag(APClient::ItemFlags::FLAG_TRAP));
				DeathLinkCount = 0;
			}
			else {
				int remain_amnesty = DeathLinkAmnesty - DeathLinkCount;
				if (remain_amnesty == 0) {
					HudManager::get()->queueNotification("Panel failed. The next panel fail will cause a DeathLink.", getColorByItemFlag(APClient::ItemFlags::FLAG_TRAP));
				}
				else {
					HudManager::get()->queueNotification("Panel failed. Remaining DeathLink Amnesty: " + std::to_string(remain_amnesty) + ".", getColorByItemFlag(APClient::ItemFlags::FLAG_TRAP));
				}
			}
			ap->Set(DeathLinkDataStorageKey, NULL, false, { {"replace", DeathLinkCount} });
		}
	}
}

void APWatchdog::SendDeathLink(int panelId)
{
	auto now = std::chrono::system_clock::now();
	double nowDouble = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 1000;

	std::string entityName = std::to_string(panelId);

	if (entityToName.count(panelId)) {
		entityName = entityToName[panelId];
	}

	deathLinkTimestamps.insert(nowDouble);

	auto data = nlohmann::json{
		{"time", nowDouble},
		{"cause", ap->get_player_alias(ap->get_player_number()) + " failed " + entityName + "."},
		{"source", ap->get_player_alias(ap->get_player_number())}
	};
	ap->Bounce(data, {}, {}, { "DeathLink" });
}

void APWatchdog::ProcessDeathLink(double time, std::string cause, std::string source) {
	if (eee || DeathLinkAmnesty == -1) return;

	TriggerBonk();

	double a = -1;

	for (double b : deathLinkTimestamps) {
		if (fabs(time - b) < std::numeric_limits<double>::epsilon() * fmax(fabs(time), fabs(b))) { // double equality with some leeway because of conversion back and forth from/to JSON
			a = b;
		}
	}

	if (a != -1) {
		deathLinkTimestamps.erase(a);
		return;
	}

	std::string firstSentence = "Received death.";
	std::string secondSentence = "";

	if (cause != "") {
		secondSentence = " Reason: " + cause;
	}
	else if (source != "") {
		firstSentence = "Received death from " + source + ".";
	}

	HudManager::get()->queueNotification(firstSentence + secondSentence, getColorByItemFlag(APClient::ItemFlags::FLAG_TRAP));
}

void APWatchdog::UpdateInfiniteChallenge() {
	int recordPlayerState = ReadPanelData<int>(0x00BFF, 0xE8);
	bool challengeIsGoing = recordPlayerState > 0 && recordPlayerState <= 4;

	APAudioPlayer* player = APAudioPlayer::get();

	bool isChecked = ClientWindow::get()->getSetting(ClientToggleSetting::ChallengeTimer);

	bool customChallenge = APAudioPlayer::get()->HasCustomChallengeSong();
	if (customChallenge) {
		bool challengeIsGoingReal = challengeIsGoing && ReadPanelData<float>(0x0088E, POWER + 4) > 0;

		if (!player->challengeSongIsPlaying && challengeIsGoingReal) {
			player->StartCustomChallengeSong();
		}

		if (!challengeIsGoingReal && player->challengeSongIsPlaying) {
			player->StopCustomChallengeSong();
		}

		if (challengeIsGoingReal && player->challengeSongIsPlaying && player->ChallengeSongHasEnded() && !isChecked) {
			Memory::get()->ForceStopChallenge();
		}
	}

	if (!insideChallengeBoxRange || challengeIsGoing) {
		infiniteChallengeIsValid = false;
		return;
	}

	bool shouldDisableVanillaSound = isChecked || customChallenge;

	if (shouldDisableVanillaSound != infiniteChallenge || !infiniteChallengeIsValid) {
		bool success = Memory::get()->SetInfiniteChallenge(shouldDisableVanillaSound);

		if (success) {
			infiniteChallenge = shouldDisableVanillaSound;
			infiniteChallengeIsValid = true;
		}
	}
}

void APWatchdog::QueueItem(APClient::NetworkItem i) {
	locationsThatContainedItemsFromOtherPlayers.insert({ i.player, i.location });

	queuedReceivedItems.push(i);
}

void APWatchdog::HandleReceivedItems() {
	Memory::get()->WritePanelData<int>(0x0064, VIDEO_STATUS_COLOR + 12, { mostRecentItemId });

	while (!queuedReceivedItems.empty()) {
		auto item = queuedReceivedItems.front();
		InteractionState iState = InputWatchdog::get()->getInteractionState();

		if (item.item == ITEM_BONK_TRAP && (iState == InteractionState::Sleeping || iState == InteractionState::MenuAndSleeping || iState == InteractionState::Menu || iState == InteractionState::Warping)) {
			return;
		}

		queuedReceivedItems.pop();
		int realitem = item.item;
		int advancement = item.flags;

		if (progressiveItems.count(realitem)) {
			if (progressiveItems[realitem].size() == 0) {
				continue;
			}

			realitem = progressiveItems[realitem][0];
			progressiveItems[item.item].erase(progressiveItems[item.item].begin());
		}

		bool unlockLater = false;

		if (item.item != ITEM_TEMP_SPEED_BOOST && item.item != ITEM_TEMP_SPEED_REDUCTION && item.item != ITEM_POWER_SURGE && item.item != ITEM_BONK_TRAP) {
			unlockItem(realitem);
			panelLocker->UpdatePuzzleLocks(*state, realitem);
		}
		else {
			unlockLater = true;
		}

		if (itemIdToDoorSet.count(realitem)) {
			for (int doorHex : itemIdToDoorSet[realitem]) {
				UnlockDoor(doorHex);
			}
		}

		if (mostRecentItemId >= item.index + 1) continue;

		if (unlockLater) {
			unlockItem(realitem);
		}

		mostRecentItemId = item.index + 1;

		Memory::get()->WritePanelData<int>(0x0064, VIDEO_STATUS_COLOR + 12, { mostRecentItemId });

		QueueReceivedItem({ item.item, advancement, realitem });
		if (item.item != ITEM_BONK_TRAP && item.player != ap->get_player_number()) PlayReceivedJingle(item.flags);
	}
}

void APWatchdog::unlockItem(int item) {
	switch (item) {
	case ITEM_DOTS:									state->unlockedDots = true;							break;
	case ITEM_COLORED_DOTS:							state->unlockedColoredDots = true;				break;
	case ITEM_FULL_DOTS:									state->unlockedFullDots = true;							break;
	case ITEM_SOUND_DOTS:							state->unlockedSoundDots = true;					break;
	case ITEM_SYMMETRY:								state->unlockedSymmetry = true;					break;
	case ITEM_TRIANGLES:								state->unlockedTriangles = true;					break;
	case ITEM_ERASOR:									state->unlockedErasers = true;						break;
	case ITEM_TETRIS:									state->unlockedTetris = true;						break;
	case ITEM_TETRIS_ROTATED:						state->unlockedTetrisRotated = true;				break;
	case ITEM_TETRIS_NEGATIVE:						state->unlockedTetrisNegative = true;			break;
	case ITEM_STARS:									state->unlockedStars = true;						break;
	case ITEM_STARS_WITH_OTHER_SYMBOL:			state->unlockedStarsWithOtherSimbol = true;	break;
	case ITEM_B_W_SQUARES:							state->unlockedStones = true;						break;
	case ITEM_COLORED_SQUARES:						state->unlockedColoredStones = true;				break;
	case ITEM_SQUARES: state->unlockedStones = state->unlockedColoredStones = true;				break;
	case ITEM_ARROWS: state->unlockedArrows = true; break;

		//Powerups
	case ITEM_TEMP_SPEED_BOOST:					ApplyTemporarySpeedBoost();				break;
	case ITEM_PUZZLE_SKIP:							AddPuzzleSkip(); break;

		//Traps
	case ITEM_POWER_SURGE:							TriggerPowerSurge();						break;
	case ITEM_BONK_TRAP:							TriggerBonk();						  break;
	case ITEM_TEMP_SPEED_REDUCTION:				ApplyTemporarySlow();						break;
	}
}

void APWatchdog::DisablePuzzle(int id) {
	auto memory = Memory::get();

	if (allEPs.count(id)) {
		memory->SolveEP(id);

		if (id == 0x3352F && finalPanel == 0x03629) { // Gate EP in Panel Hunt: Be normal challenge difficulty impossible
			panelLocker->PermanentlyUnlockPuzzle(id, *state);
			return;
		}
		if ((DisabledPuzzlesBehavior == "Auto-Skip" || DisabledPuzzlesBehavior == "Prevent Solve") && precompletableEpToName.count(id) && precompletableEpToPatternPointBytes.count(id)) {
			memory->MakeEPGlow(precompletableEpToName.at(id), precompletableEpToPatternPointBytes.at(id));
		}
		if (DisabledPuzzlesBehavior == "Prevent Solve") {
			panelLocker->DisablePuzzle(id);
		}
	}
	if (std::count(LockablePuzzles.begin(), LockablePuzzles.end(), id) || allPanels.count(id)) {
		SkipPanel(id, "Disabled", false);
	}
}

void APWatchdog::CheckFinalRoom() {
	uint64_t entity_pointer = ASMPayloadManager::get()->FindSoundByName("amb_ending_water");

	if (entity_pointer == 0) {
		finalRoomMusicTimer = -1;
		return;
	}

	float volumeScale;
	Memory::get()->ReadAbsolute(reinterpret_cast<LPCVOID>(entity_pointer + 0xd8), &volumeScale, sizeof(float));
	
	if (volumeScale < 0.8) {
		finalRoomMusicTimer = -1;
		return;
	}

	uint64_t soundStreamPointer = Memory::get()->getSoundStream(entity_pointer);

	if (soundStreamPointer == 0) {
		finalRoomMusicTimer = -1;
		return;
	}

	Memory::get()->ReadAbsolute(reinterpret_cast<LPCVOID>(soundStreamPointer + 0x70), &finalRoomMusicTimer, sizeof(double));
}

void APWatchdog::DoAprilFoolsEffects(float deltaSeconds) {
	// Windmill
	
	Memory* memory = Memory::get();

	bool windmill_turning;
	memory->ReadAbsolute(reinterpret_cast<LPCVOID>(memory->windmillCurrentlyTurning), &windmill_turning, sizeof(bool));

	float currentSpeed;
	float targetSpeed;
	memory->ReadAbsolute(reinterpret_cast<LPCVOID>(memory->windmillCurrentTurnSpeed), &currentSpeed, sizeof(float));
	float normalMaxSpeed;
	memory->ReadAbsolute(reinterpret_cast<LPCVOID>(memory->windmillMaxTurnSpeed), &normalMaxSpeed, sizeof(float));

	if (windmill_turning) {
		if (memory->GetActivePanel() == 0x17d90) {
			targetSpeed = normalMaxSpeed;
		}
		else
		{
			targetSpeed = 20;
		}
	}
	else
	{
		if (currentSpeed < normalMaxSpeed) return;
		targetSpeed = normalMaxSpeed;
	}

	if (targetSpeed > currentSpeed) {
		float delta = (targetSpeed - currentSpeed) * deltaSeconds;
		if (delta > 5 * deltaSeconds) delta = 5 * deltaSeconds;
		if (currentSpeed - normalMaxSpeed < 2) {
			delta = deltaSeconds * 5 * ((currentSpeed - normalMaxSpeed) / 2);
			if (delta < deltaSeconds) delta = deltaSeconds;
		}

		currentSpeed += delta;
		if (currentSpeed > targetSpeed) currentSpeed = targetSpeed;
	}
	else {
		float delta = (currentSpeed - targetSpeed) * deltaSeconds;
		if (delta > 5 * deltaSeconds) delta = 5 * deltaSeconds;
		if (delta < deltaSeconds * 0.5) delta = deltaSeconds * 0.5;
		currentSpeed -= (currentSpeed - targetSpeed) * deltaSeconds;
		if (currentSpeed < targetSpeed) currentSpeed = targetSpeed;
	}

	memory->WriteAbsolute(reinterpret_cast<LPVOID>(memory->windmillCurrentTurnSpeed), &currentSpeed, sizeof(float));
}

Vector3 APWatchdog::getHuntEntitySpherePosition(int huntEntity) {
	if (finalElevatorPanelPositions.count(huntEntity) && !ReadPanelData<int>(0x3D9a7, SOLVED)) {
		return finalElevatorPanelPositions[huntEntity];
	}

	if (!treehouseBridgePanels.count(huntEntity) || ReadPanelData<int>(huntEntity, SOLVED)) {
		Vector3 panelPosition = Vector3(ReadPanelData<float>(huntEntity, POSITION, 3));
		if (huntEntity == 0x34BC5) panelPosition.Y += 0.15f;
		else if (huntEntity == 0x34BC6) panelPosition.Y -= 0.15f;
		return panelPosition;
	}

	if (treehousePanelPreSolvePositions.count(huntEntity)) {
		return treehousePanelPreSolvePositions[huntEntity];
	}

	int directionalPanel = 0;
	int panelAfterDirectionalPanel = 0;
	std::vector<std::map<int, Vector3>> representativeMaps = {};

	if (leftOrange10to15Straight.count(huntEntity)) {
		directionalPanel = 0x17DD1;
		panelAfterDirectionalPanel = 0x17DDE;
		representativeMaps = { leftOrange10to15Straight, leftOrange10to15Leftwards, leftOrange10to15Right };
	}
	else if (rightOrange11To12StraightThenStraight.count(huntEntity)) {
		// Special case

		// If right orange 10 has not been solved, use 4 as the reference & assume it will be solved straight
		if (!ReadPanelData<int>(0x17DB7, SOLVED)) {
			directionalPanel = 0x17CE3;
			panelAfterDirectionalPanel = 0x17DCD;
			representativeMaps = { rightOrange5To12Straight, rightOrange5To12Left, rightOrange5To12Right };
		}
		else {
			// If it has been solved, it becomes the reference point
			directionalPanel = 0x17DB7;
			panelAfterDirectionalPanel = 0x17DB1;

			representativeMaps = { rightOrange11To12LeftThenLeft, rightOrange11To12LeftThenStraight, rightOrange11To12LeftThenRight,
								   rightOrange11To12StraightThenLeft, rightOrange11To12StraightThenStraight, rightOrange11To12StraightThenRight,
								   rightOrange11To12RightThenLeft, rightOrange11To12RightThenStraight, rightOrange11To12RightThenRight, };
		}
	}
	else if (rightOrange5To12Straight.count(huntEntity)) {
		directionalPanel = 0x17CE3;
		panelAfterDirectionalPanel = 0x17DCD;
		representativeMaps = { rightOrange5To12Straight, rightOrange5To12Left, rightOrange5To12Right };
	}
	else if (greenBridge5to7Straight.count(huntEntity)) {
		directionalPanel = 0x17E52;
		panelAfterDirectionalPanel = 0x17E5B;
		representativeMaps = { greenBridge5to7Straight, greenBridge5to7Leftwards, greenBridge5to7Rightwards };
	}

	if (directionalPanel == 0) return { -1, -1, -1 };

	Vector3 panelAfterDirectionalPosition = Vector3(ReadPanelData<float>(panelAfterDirectionalPanel, POSITION, 3));
	int directionalPanelSolved = ReadPanelData<int>(directionalPanel, SOLVED);

	// If the directional panel has not been solved, assume the bridge is going straight.

	if (!directionalPanelSolved) {
		return representativeMaps[0][huntEntity];
	}

	// If it has been solved, check in which direction, then use that for the upcoming hunt panel positions.

	for (auto map : representativeMaps) {
		if ((panelAfterDirectionalPosition - map[panelAfterDirectionalPanel]).length() < 0.5) {
			return map[huntEntity];
		}
	}

	return { -1000, -1, -1 };
}

void APWatchdog::DrawSpheres(float deltaSeconds) {
	std::vector<WitnessDrawnSphere> spheresToDraw = {};

	for (auto [warpname, unlocked] : unlockableWarps) {
		Vector3 position = warpLookup[warpname].playerPosition;

		if (warpPositionUnlockPointOverrides.contains(warpname)) position = warpPositionUnlockPointOverrides[warpname];

		position.Z -= 1.6f;
		RgbColor color = unlocked ? RgbColor(0.2f, 0.9f, 0.2f, 0.9f) : RgbColor(0.9f, 0.2f, 0.2f, 0.9f);

		spheresToDraw.push_back(WitnessDrawnSphere({ position, WARP_SPHERE_RADIUS, color, true }));
	}

	Vector3 headPosition = Vector3(Memory::get()->ReadPlayerPosition());
	DrawIngameManager* drawManager = DrawIngameManager::get();

	for (auto [huntEntity, previousOpacity] : huntEntitySphereOpacities) {
		float positionEntity = huntEntity;
		if (huntEntity == 0xFFF00) positionEntity = 0x09F7F; // Use short box for long box position

		Vector3 position = getCachedEntityPosition(positionEntity);
		
		float distance = (position - headPosition).length();

		if (distance > 60) {
			huntEntitySphereOpacities[huntEntity] = 0;
			continue;
		}

		Vector3 actualPosition = getHuntEntitySpherePosition(positionEntity);

		bool hide = false;
		if (actualPosition.X == -1000) { // -1000 is a special value to indicate that the position of this entity should be assumed to be the previous position.
			actualPosition = huntEntityPositions[huntEntity];
			hide = true;
		}

		huntEntityPositions[huntEntity] = actualPosition;

		// TODO: make better thing for special positions
		if (huntEntity == 0x09F7F) {
			actualPosition.Y -= 1;
		}
		if (huntEntity == 0xFFF00) {
			actualPosition.Y += 1;
		}

		float workingDistance = (actualPosition - headPosition).length();

		float targetOpacity = 0;

		InteractionState interactionState = InputWatchdog::get()->getInteractionState();
		if (hide) targetOpacity = 0;
		else {
			if (interactionState == InteractionState::Focusing || interactionState == InteractionState::Solving) {
				workingDistance -= 2;
				targetOpacity = workingDistance / 10 * 0.1;
				if (targetOpacity > 0.1) targetOpacity = 0.1;
				if (targetOpacity < 0) targetOpacity = 0;
			}
			else {
				workingDistance += 2;
				targetOpacity = workingDistance / 10 * 0.6;

				if (targetOpacity > 0.6) targetOpacity = 0.6;
				// if (targetOpacity < 0) targetOpacity = 0; impossible
			}

			if (huntEntityKeepActive[huntEntity] > 0) {
				float potentialHigherTargetOpacity = huntEntityKeepActive[huntEntity] / 15.0f;
				if (potentialHigherTargetOpacity > targetOpacity) targetOpacity = potentialHigherTargetOpacity;
				huntEntityKeepActive[huntEntity] -= deltaSeconds;
			}
		}

		float delta = (previousOpacity + 0.01) / 7;
		float newOpacity = previousOpacity;


		if (newOpacity > targetOpacity) {
			newOpacity -= delta;
			if (newOpacity < targetOpacity) newOpacity = targetOpacity;
		}
		else {
			newOpacity += delta;
			if (newOpacity > targetOpacity) newOpacity = targetOpacity;
		}


		huntEntitySphereOpacities[huntEntity] = newOpacity;

		if (newOpacity == 0) continue;

		bool solveStatus = huntEntityToSolveStatus[huntEntity];
		float radius = 0.7;
		RgbColor color = solveStatus ? RgbColor(0.0f, 1.0f, 0.0f, newOpacity) : RgbColor(1.0f, 0.0f, 0.0f, newOpacity);
		bool cull = true;

		spheresToDraw.push_back(WitnessDrawnSphere({ actualPosition, radius, color, cull }));
	}

	drawManager->drawSpheres(spheresToDraw);
}

void APWatchdog::FlickerCable() {
	if (PuzzleRandomization != SIGMA_EXPERT) return;
	if (tutorialCableStateChangedRecently > 0) {
		tutorialCableStateChangedRecently -= 1;
		return;
	}

	if (tutorialCableFlashState == 0) {
		std::uniform_int_distribution<std::mt19937::result_type> dist13(0, 13);
		if (dist13(rng) != 0) return;
	}
	else {
		std::uniform_int_distribution<std::mt19937::result_type> dist4(0, 4);
		if (dist4(rng) != 0) return;
	}

	tutorialCableStateChangedRecently = 2;

	std::uniform_int_distribution<std::mt19937::result_type> dist3(0, 3);
	tutorialCableFlashState += dist3(rng);
	if (tutorialCableFlashState > 3) tutorialCableFlashState = 0;

	float cableColor = 0;
	if (tutorialCableFlashState == 1 || tutorialCableFlashState == 3) {
		cableColor = 0.027;
	}

	if (tutorialCableFlashState == 2) {
		cableColor = 0.1;
	}

	InteractionState interactionState = InputWatchdog::get()->getInteractionState();
	if (interactionState == InteractionState::Focusing || interactionState == InteractionState::Solving) {
		cableColor *= 0.75;
	}

	Memory::get()->WritePanelData<float>(0x51, 0x158, { cableColor });
}

void APWatchdog::ToggleSleep() {
	InputWatchdog* inputWatchdog = InputWatchdog::get();
	InteractionState iState = inputWatchdog->getInteractionState();

	if (isKnockedOut || iState == InteractionState::Cutscene || eee) {
		HudManager::get()->displayBannerMessageIfQueueEmpty("Can't go into sleep mode right now.");
		return;
	}
	if (iState == InteractionState::Warping) return;

	if (iState == InteractionState::Sleeping) {
		inputWatchdog->setSleep(false);
	}
	else {
		inputWatchdog->setSleep(true);
	}
}

void APWatchdog::TryWarp() {
	InputWatchdog* inputWatchdog = InputWatchdog::get();
	InteractionState iState = inputWatchdog->getInteractionState();

	if (iState != InteractionState::Sleeping) return;
	if (selectedWarp == NULL) return;
	if (eee) {
		return;
	}

	if (selectedWarp->tutorial) {
		ASMPayloadManager::get()->ActivateMarker(0x034F6);
	}

	inputWatchdog->startWarp(selectedWarp->tutorial);
	hasTeleported = false;
	hasDoneTutorialMarker = false;
	hasTriedExitingNoclip = false;
	warpRetryTime = 0.0f;
	ClientWindow::get()->focusGameWindow();
}

void APWatchdog::CheckUnlockedWarps() {
	int pNO = ap->get_player_number();

	if (!firstActionDone) {
		ap->SetNotify({ "WitnessUnlockedWarps" + std::to_string(pNO) });

		if (SyncProgress) {
			ap->SetNotify({ "WitnessUnlockedWarps" + std::to_string(pNO) + "_" + Utilities::wstring_to_utf8(Utilities::GetUUID()) });
		}

		std::map<std::string, bool> startwarp = { };
		if (unlockableWarps.contains(startingWarp)) startwarp = { { startingWarp, true } };


		ap->Set("WitnessUnlockedWarps" + std::to_string(pNO), nlohmann::json::object(), true, { { "default", startwarp } });
		ap->Set("WitnessUnlockedWarps" + std::to_string(pNO) + "_" + Utilities::wstring_to_utf8(Utilities::GetUUID()), startwarp, true, { {"default", nlohmann::json::object()} });
	}

	Vector3 playerPosition = Vector3(Memory::get()->ReadPlayerPosition());
	bool newunlocks = false;

	for (auto [warpname, unlocked] : unlockableWarps) {
		if (unlocked) continue;

		Vector3 warpPosition = warpLookup[warpname].playerPosition;
		if (warpPositionUnlockPointOverrides.contains(warpname)) warpPosition = warpPositionUnlockPointOverrides[warpname];
		
		if ((playerPosition - warpPosition).length() > WARP_SPHERE_RADIUS + 1.2f) continue;

		UnlockWarps({ warpname });
		return;
	}
}

void APWatchdog::UnlockWarps(std::vector<std::string> warps) {
	std::vector<std::string> newWarps = {};
	
	for (auto warp : warps) {
		if (!unlockableWarps.contains(warp)) {
			HudManager::get()->queueNotification("Server reported unlocked warp \"" + warp + "\", but this is not an unlockable warp for this slot.", getColorByItemFlag(APClient::ItemFlags::FLAG_TRAP));
			continue;
		}

		if (!unlockableWarps[warp]) {
			newWarps.push_back(warp);
			unlockableWarps[warp] = true;
		}
	}

	if (newWarps.empty()) return;

	std::string message = "Unlocked Warp";
	if (newWarps.size() > 1) message += "s";
	message += ":";
	for (auto warp : newWarps) {
		message += " " + warp + ",";
	}
	message.pop_back();
	message += ".";

	std::vector<Warp*> newUnlockedWarps = { };

	for (auto warp : allPossibleWarps) {
		if (unlockableWarps.contains(warp.name) && unlockableWarps[warp.name]) newUnlockedWarps.push_back(&warpLookup[warp.name]);
	}

	unlockedWarps = newUnlockedWarps;
	HudManager::get()->queueNotification(message, getColorByItemFlag(APClient::ItemFlags::FLAG_ADVANCEMENT));

	std::map<std::string, bool> warpsToSignalToDataStore;
	for (std::string warp : newWarps) {
		warpsToSignalToDataStore[warp] = true;
	}

	int pNO = ap->get_player_number();

	ap->Set("WitnessUnlockedWarps" + std::to_string(pNO), nlohmann::json::object(), false, { { "update" , warpsToSignalToDataStore } });
	ap->Set("WitnessUnlockedWarps" + std::to_string(pNO) + "_" + Utilities::wstring_to_utf8(Utilities::GetUUID()), nlohmann::json::object(), false, { { "update" , warpsToSignalToDataStore } });
}
