// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "Randomizer.h"
#include "PanelLists.h"
#include "PuzzleList.h"
#include <string>
#include <iostream>
#include <numeric>
#include "Random.h"
#include "Quaternion.h"
#include "Watchdog.h"

std::vector<PanelID> copyWithoutElements(const std::vector<PanelID>& input, const std::vector<PanelID>& toRemove) {
	std::vector<PanelID> result;
	result.reserve(input.size());
	for (PanelID val : input) {
		if (std::find(toRemove.begin(), toRemove.end(), val) == toRemove.end()) {
			result.push_back(val);
		}
	}
	return result;
}

void Randomizer::GenerateNormal(HWND loadingHandle) {
	PuzzleList puzzles;
	puzzles.setLoadingHandle(loadingHandle);
	puzzles.setSeed(seed, seedIsRNG, colorblind);
	puzzles.GenerateAllN();
	if (doubleMode) ShufflePanels(false);
}

void Randomizer::GenerateHard(HWND loadingHandle) {
	PuzzleList puzzles;
	puzzles.setLoadingHandle(loadingHandle);
	puzzles.setSeed(seed, seedIsRNG, colorblind);
	puzzles.GenerateAllH();
	if (doubleMode) ShufflePanels(true);
	SetWindowText(loadingHandle, L"Done!");
	if (!HasBeenRandomized())
		MessageBox(GetActiveWindow(), L"Hi there! Thanks for trying out Expert Mode. It will be tough, but I hope you have fun!\r\n\r\n"
		L"Expert has some unique tricks up its sleeve. You will encounter some situations that may seem impossible at first glance (even in tutorial)! "
		L"In these situations, try to think of alternate approaches that weren't required in the base game.\r\n\r\n"
		L"For especially tough puzzles, the Solver folder has a solver that works for most puzzles, though it occasionally fails to find a solution.\r\n\r\n"
		L"The Github wiki also has a Hints page that can help with certain tricky puzzles.\r\n\r\n"
		L"Thanks for playing, and good luck!", L"Welcome", MB_OK);
}

void Randomizer::StartWatchdogs()
{
	if (watchdogsStarted) return;
	(new SymbolsWatchdog())->start();
	(new TownDoorWatchdog())->start();
	watchdogsStarted = true;
}

template <class T>
int find(const std::vector<T> &data, T search, size_t startIndex = 0) {
	for (size_t i = startIndex; i<data.size(); i++) {
		if (data[i] == search) return static_cast<int>(i);
	}
	std::cout << "Couldn't find " << search << " in data!" << std::endl;
	throw std::exception("Couldn't find value in data!");
}

void Randomizer::AdjustSpeed() {
	Memory* memory = Memory::get();
	memory->WritePanelData<float>(MV_DESERT_SUN_8, OPEN_RATE, 0.04f);  // 4x
	memory->WritePanelData<float>(MV_MILL_RAMP, OPEN_RATE, 0.7f); // 3x
	memory->WritePanelData<float>(MV_MILL_LIFT, OPEN_RATE, 1.5f); // 3x
	memory->WritePanelData<float>(MV_QUARRY_ELEVATOR, OPEN_RATE, 0.8f); // 4x
	memory->WritePanelData<float>(MV_SWAMP_SLIDING_BRIDGE, OPEN_RATE, 0.1f); //4x
	memory->WritePanelData<float>(MV_MOUNTAIN_ELEVATOR, OPEN_RATE, 0.1f); // 4x
	memory->WritePanelData<float>(MV_SWAMP_FLOODGATE_1, OPEN_RATE, 0.03f); // 2x
	memory->WritePanelData<float>(MV_SWAMP_FLOODGATE_2, OPEN_RATE, 0.03f); // 2x
	memory->WritePanelData<float>(MV_SWAMP_ROTATING_BRIDGE, OPEN_RATE, 0.04f); // 4x
	memory->WritePanelData<float>(MV_BOATHOUSE_RAMP_ANGLE, OPEN_RATE, 0.25f); // 5x
	memory->WritePanelData<float>(MV_BOATHOUSE_RAMP_SLIDE, OPEN_RATE, 0.15f); // 4x
	memory->WritePanelData<float>(MV_BOATHOUSE_SHORTCUT, OPEN_RATE, 0.3f); // 2x
}

void Randomizer::RandomizeDesert() {
	std::vector<PanelID> puzzles = desertPanels;
	int endIndex = static_cast<int>(desertPanels.size());
	for (int i = 0; i < endIndex - 1; i++) {
		const int target = Random::rand() % (endIndex - i) + i;
		//Prevent ambiguity caused by shadows, and ensure all latches on Surface 7 and Light 3 must be opened
		if (i == target ||
			i == 1 && !desertValidSun2.count(desertPanels[target]) ||
			i == 2 && !desertValidSun3.count(desertPanels[target]) ||
			i == 6 && !desertValidSun7.count(desertPanels[target]) ||
			i == 9 && !desertValidLightR.count(desertPanels[target]) ||
			i == 10 && !desertValidLightFloor.count(desertPanels[target])) {
			i--;
			continue;
		}
		if (i != target) {
			SwapPanels(puzzles[i], puzzles[target], SWAP::PATHS);
			std::swap(desertPanels[i], desertPanels[target]);
		}
		Memory::get()->WritePanelData<float>(puzzles[i], PATH_WIDTH_SCALE, 0.8f);
	}
	Randomizer::Randomize(desertPanelsWide, SWAP::PATHS);
	Special::setPower(DESERT_SUN_8, false);
	Special::setTargetAndDeactivate(DESERT_FLOOD_5, DESERT_FLOOD_FINAL);
}

void Randomizer::Randomize(std::vector<PanelID>& panels, int flags) {
	return RandomizeRange(panels, flags, 0, static_cast<int>(panels.size()));
}

// Range is [start, end)
void Randomizer::RandomizeRange(std::vector<PanelID> panels, int flags, int startIndex, int endIndex) {
	if (panels.size() == 0) return;
	if (startIndex >= endIndex) return;
	if (endIndex >= panels.size()) endIndex = static_cast<int>(panels.size());
	for (int i = endIndex - 1; i > startIndex; i--) {
		const int target = Random::rand(startIndex, i);
		if (i != target) {
			SwapPanels(panels[i], panels[target], flags);
			std::swap(panels[i], panels[target]); // Panel indices in the array
		}
	}
}

//Isn't actualy working
void Randomizer::RandomizeAudiologs()
{
	/*
	std::vector<PanelID> audiologs = {
		0x3C0F7, 0x3C0FD, 0x32A00, 0x3C0FE, 0x338B0, 0x338B7, 0x338AD,
		0x338A5, 0x338AE, 0x338AF, 0x338A7, 0x338A3, 0x338A4, 0x3C108,
		0x338EF, 0x336E5, 0x338A6, 0x3C100, 0x3C0F4, 0x3C102, 0x3C10D,
		0x3C10E, 0x3C10B, 0x0074F, 0x012C7, 0x329FF, 0x3C106, 0x33AFF,
		0x011F9, 0x00763, 0x32A08, 0x3C101, 0x3C0FF, 0x3C103, 0x00A0F,
		0x339A9, 0x015C0, 0x33B36, 0x3C10C, 0x32A0E, 0x329FE, 0x32A07,
		0x00761, 0x3C109, 0x33B37, 0x3C107, 0x3C0F3, 0x015B7, 0x3C10A,
		0x32A0A, 0x015C1, 0x3C12A, 0x3C104, 0x3C105, 0x339A8, 0x0050A,
		0x338BD, 0x3C135, 0x338C9, 0x338D7, 0x338C1, 0x338CA
	};
	Randomizer::RandomizeRange(audiologs, SWAP::AUDIO_NAMES, 0, audiologs.size());
	*/
}

void Randomizer::SwapPanels(PanelID panel1, PanelID panel2, int flags) {
	if (!_shuffleMapping.count(panel1)) {
		_shuffleMapping[panel1] = panel1;
	}
	if (!_shuffleMapping.count(panel2)) {
		_shuffleMapping[panel2] = panel2;
	}
	std::swap(_shuffleMapping[panel1], _shuffleMapping[panel2]);

	std::map<PanelVar, int> offsets;

	if (flags & SWAP::TARGETS) {
		offsets[TARGET] = sizeof(int);
	}
	if (flags & SWAP::AUDIO_NAMES) {
		offsets[AUDIO_LOG_NAME] = sizeof(void*); //This isn't actually working 
	}
	if (flags & SWAP::COLORS) {
		offsets[PATH_COLOR] = 16;
		offsets[REFLECTION_PATH_COLOR] = 16;
		offsets[DOT_COLOR] = 16;
		offsets[ACTIVE_COLOR] = 16;
		offsets[BACKGROUND_REGION_COLOR] = 12; // Not copying alpha to preserve transparency.
		offsets[SUCCESS_COLOR_A] = 16;
		offsets[SUCCESS_COLOR_B] = 16;
		offsets[STROBE_COLOR_A] = 16;
		offsets[STROBE_COLOR_B] = 16;
		offsets[ERROR_COLOR] = 16;
		offsets[PATTERN_POINT_COLOR] = 16;
		offsets[PATTERN_POINT_COLOR_A] = 16;
		offsets[PATTERN_POINT_COLOR_B] = 16;
		offsets[SYMBOL_A] = 16;
		offsets[SYMBOL_B] = 16;
		offsets[SYMBOL_C] = 16;
		offsets[SYMBOL_D] = 16;
		offsets[SYMBOL_E] = 16;
		offsets[PUSH_SYMBOL_COLORS] = sizeof(int);
		offsets[OUTER_BACKGROUND] = 16;
		offsets[OUTER_BACKGROUND_MODE] = sizeof(int);
		offsets[NUM_COLORED_REGIONS] = sizeof(int);
		offsets[COLORED_REGIONS] = sizeof(void*);
	}
	if (flags & SWAP::PATHS) {
		offsets[AUDIO_PREFIX] = sizeof(void*);
		offsets[PATH_WIDTH_SCALE] = sizeof(float);
		offsets[PATTERN_SCALE] = sizeof(float);
		offsets[STARTPOINT_SCALE] = sizeof(float);
		offsets[NUM_DOTS] = sizeof(int);
		offsets[NUM_CONNECTIONS] = sizeof(int);
		offsets[DOT_POSITIONS] = sizeof(void*);
		offsets[DOT_FLAGS] = sizeof(void*);
		offsets[DOT_CONNECTION_A] = sizeof(void*);
		offsets[DOT_CONNECTION_B] = sizeof(void*);
		offsets[DECORATIONS] = sizeof(void*);
		offsets[DECORATION_FLAGS] = sizeof(void*);
		offsets[DECORATION_COLORS] = sizeof(void*);
		offsets[NUM_DECORATIONS] = sizeof(int);
		offsets[REFLECTION_DATA] = sizeof(void*);
		offsets[GRID_SIZE_X] = sizeof(int);
		offsets[GRID_SIZE_Y] = sizeof(int);
		offsets[STYLE_FLAGS] = sizeof(int);
		offsets[SEQUENCE_LEN] = sizeof(int);
		offsets[SEQUENCE] = sizeof(void*);
		offsets[DOT_SEQUENCE_LEN] = sizeof(int);
		offsets[DOT_SEQUENCE] = sizeof(void*);
		offsets[DOT_SEQUENCE_LEN_REFLECTION] = sizeof(int);
		offsets[DOT_SEQUENCE_REFLECTION] = sizeof(void*);
		offsets[PANEL_TARGET] = sizeof(void*);
		offsets[SPECULAR_TEXTURE] = sizeof(void*);
	}
	if (flags & SWAP::DRAWN_LINES) {
		offsets[TRACED_EDGES] = 16;
	}

	Memory* memory = Memory::get();
	for (auto const&[offset, size] : offsets) {
		if (offset == PATH_WIDTH_SCALE)
			continue;
		if (offset == PATTERN_SCALE) {
			float panel1path = memory->ReadPanelData<float>(panel1, PATH_WIDTH_SCALE);
			float panel2path = memory->ReadPanelData<float>(panel2, PATH_WIDTH_SCALE);
			float panel1pattern = memory->ReadPanelData<float>(panel1, PATTERN_SCALE);
			float panel2pattern = memory->ReadPanelData<float>(panel2, PATTERN_SCALE);

			float computedFor1 = panel1path * panel1pattern / panel2path;
			float computedFor2 = panel2path * panel2pattern / panel1path;
			memory->WritePanelData<float>(panel2, PATTERN_SCALE, computedFor1);
			memory->WritePanelData<float>(panel1, PATTERN_SCALE, computedFor2);
			continue;
		}
		std::vector<byte> panel1data = memory->ReadPanelDataVector<byte>(panel1, offset, size);
		std::vector<byte> panel2data = memory->ReadPanelDataVector<byte>(panel2, offset, size);
		memory->WritePanelDataVector<byte>(panel2, offset, panel1data);
		memory->WritePanelDataVector<byte>(panel1, offset, panel2data);
	}
	memory->WritePanelData<int>(panel1, NEEDS_REDRAW, true);
	memory->WritePanelData<int>(panel2, NEEDS_REDRAW, true);
	memory->invalidateCache();
}

void Randomizer::ReassignTargets(const std::vector<PanelID>& panels, const std::vector<int>& order, std::vector<PanelID> targets) {
	Memory* memory = Memory::get();
	if (targets.empty()) {
		// This list is offset by 1, so the target of the Nth panel is in position N (aka the N+1th element)
		// The first panel may not have a wire to power it, so we use the panel ID itself.
		targets = { static_cast<PanelID>(panels[0] + 1) };
		for (const PanelID panel : panels) {
			PanelID target = memory->ReadPanelData<PanelID>(panel, TARGET);
			targets.push_back(target);
		}
	}
	for (size_t i = 0; i < order.size() - 1; i++) {
		// Set the target of order[i] to order[i+1], using the "real" target as determined above.
		const PanelID panelTarget = targets[order[i + 1]];
		memory->WritePanelData<PanelID>(panels[order[i]], TARGET, panelTarget);
	}
}

void Randomizer::SwapWithRandomPanel(PanelID panel1, const std::vector<PanelID>& possiblePanels, int flags) {
	PanelID toSwap;
	do {
		toSwap = Random::pickRandom(possiblePanels);
	} while (_alreadySwapped.count(toSwap));
	if (panel1 != toSwap) {
		SwapPanels(panel1, toSwap, flags);
	}
	_alreadySwapped.insert(toSwap);
}

// Range is [start, end)
void Randomizer::ShuffleRange(std::vector<int>& order, int startIndex,int endIndex) {
	if (order.size() == 0) return;
	if (startIndex >= endIndex) return;
	if (endIndex >= order.size()) endIndex = static_cast<int>(order.size());
	for (int i = endIndex - 1; i > startIndex; i--) {
		const int target = Random::rand(startIndex, i);
		std::swap(order[i], order[target]);
	}
}

void Randomizer::ShufflePanels(bool hard) {
	Memory* memory = Memory::get();
	_alreadySwapped.clear();

	Randomize(elevatorControls, SWAP::PATHS | SWAP::COLORS);
	Randomize(treehousePivotSet, SWAP::PATHS | SWAP::COLORS);
	Randomize(utmPerspectiveSet, SWAP::PATHS | SWAP::COLORS);
	if (!hard) {
		Randomize(symmetryLaserYellows, SWAP::PATHS | SWAP::COLORS);
		Randomize(symmetryLaserBlues, SWAP::PATHS | SWAP::COLORS);
		Randomize(squarePanels, SWAP::PATHS | SWAP::COLORS);
	} else {
		std::vector<PanelID> panelsToRandom = copyWithoutElements(squarePanels, squarePanelsExpertBanned);
		Randomize(panelsToRandom, SWAP::PATHS | SWAP::COLORS);
	}
	Randomize(mountainMultipanel, SWAP::PATHS | SWAP::COLORS);

	Randomize(pillars, SWAP::PATHS | SWAP::COLORS);
}

bool Randomizer::HasBeenPlayed() {
	return Memory::get()->ReadPanelData<float>(TUT_MAZE_2, POWER) > 0 || Memory::get()->ReadPanelData<int>(TUT_ENTER_1, TRACED_EDGES) > 0;
}

bool Randomizer::HasBeenRandomized() {
	return Memory::get()->ReadPanelData<int>(TUT_ENTER_1, BACKGROUND_HIDDEN_VAR) > 0;
}
