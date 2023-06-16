// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <algorithm>
#include "Memory.h"
#include "Randomizer.h"
#include "Panels.h"
#include "PuzzleList.h"
#include <string>
#include <iostream>
#include <numeric>
#include "Random.h"
#include "Quaternion.h"
#include "ClientWindow.h"

std::vector<int> copyWithoutElements(const std::vector<int>& input, const std::vector<int>& toRemove) {
	std::vector<int> result;
	result.reserve(input.size());
	for (int val : input) {
		if (std::find(toRemove.begin(), toRemove.end(), val) == toRemove.end()) {
			result.push_back(val);
		}
	}
	return result;
}

void Randomizer::GenerateNormal() {
	std::shared_ptr<PuzzleList> puzzles = std::make_shared<PuzzleList>();
	puzzles->setSeed(seed, seedIsRNG, colorblind);
	puzzles->GenerateAllN();
	if (doubleMode) ShufflePanels(false);
}

void Randomizer::GenerateHard() {
	std::shared_ptr<PuzzleList> puzzles = std::make_shared<PuzzleList>();
	puzzles->setSeed(seed, seedIsRNG, colorblind);
	puzzles->GenerateAllH();
	if (doubleMode) ShufflePanels(true);

	ClientWindow* clientWindow = ClientWindow::get();
	clientWindow->setStatusMessage("Starting expert symbol watchdogs...");
	Panel::StartArrowWatchdogs(_shuffleMapping);
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
	memory->WritePanelData<float>(0x09F95, OPEN_RATE, { 0.04f });  // Desert Surface Final Control, 4x
	memory->WritePanelData<float>(0x03839, OPEN_RATE, { 0.7f }); // Mill Ramp, 3x
	memory->WritePanelData<float>(0x021BA, OPEN_RATE, { 1.5f }); // Mill Lift, 3x
	memory->WritePanelData<float>(0x17CC1, OPEN_RATE, { 0.8f }); // Mill Elevator, 4x
	memory->WritePanelData<float>(0x0061A, OPEN_RATE, { 0.1f }); // Swamp Sliding Bridge, 4x
	memory->WritePanelData<float>(0x09EEC, OPEN_RATE, { 0.1f }); // Mountain 2 Elevator, 4x
	memory->WritePanelData<float>(0x17E74, OPEN_RATE, { 0.03f }); // Swamp Flood gate (inner), 2x //Keeping these slower for now to help with EP's
	memory->WritePanelData<float>(0x1802C, OPEN_RATE, { 0.03f }); // Swamp Flood gate (outer), 2x
	memory->WritePanelData<float>(0x005A2, OPEN_RATE, { 0.04f }); // Swamp Rotating Bridge, 4x
	memory->WritePanelData<float>(0x17C6A, OPEN_RATE, { 0.25f }); // Ramp Angle, 5x
	memory->WritePanelData<float>(0x17F02, OPEN_RATE, { 0.15f }); // Ramp Position, 4x
	memory->WritePanelData<float>(0x17C50, OPEN_RATE, { 0.3f }); //Boathouse Barrier, 2x

	memory->WritePanelData<float>(0x00030, BOAT_DELTA_SPEED, { 1.0f }); // Boat accel, ~3x

	memory->WritePanelData<float>(0x17CC1, OPEN_RATE, { 0.8f }); // Quarry Elevator, 4x
	memory->WritePanelData<float>(0x0A069, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A06A, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A06B, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A06C, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A070, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A071, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A072, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A073, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A074, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A075, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A076, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x
	memory->WritePanelData<float>(0x0A077, OPEN_RATE, { 0.6f }); // Bunker Elevator, 4x

	memory->WritePanelData<float>(0x38ACB, OPEN_RATE, { 2.0f }); // UTM Elevator


	memory->WritePanelData<float>(0x09DA0, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E02, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E03, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E15, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E16, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E1D, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E1E, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E26, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09E98, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09ED4, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09EE3, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F10, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F11, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F21, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F2A, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F2B, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F3C, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F3D, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x09F3E, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x0C160, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x0C161, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
	memory->WritePanelData<float>(0x0C164, OPEN_RATE, { 0.5f }); // Monastery Shutters, 2x
}

void Randomizer::RandomizeDesert() {
	std::vector<int> puzzles = desertPanels;
	std::vector<int> valid1 = { 0x00698, 0x0048F, 0x09F92, 0x09DA6, 0x0078D, 0x04D18, 0x0117A, 0x17ECA, 0x0A02D };
	std::vector<int> valid2 = { 0x00698, 0x0048F, 0x09F92, 0x0A036, 0x0A049, 0x0A053, 0x00422, 0x006E3, 0x00C72, 0x008BB, 0x0078D, 0x04D18, 0x01205, 0x181AB, 0x012D7, 0x17ECA, 0x0A02D };
	std::vector<int> valid3 = { 0x00698, 0x0048F, 0x09F92, 0x0A036, 0x0A049, 0x00422, 0x008BB, 0x0078D, 0x18313, 0x01205 };
	std::vector<int> validSurfaceSeven = { 0x00698, 0x0048F, 0x09F92, 0x0A036, 0x0A049, 0x0A053, 0x00422, 0x006E3, 0x0A02D, 0x00C72, 0x0129D, 0x008BB, 0x0078D, 0x18313, 0x04D18, 0x01205, 0x181AB, 0x17ECA, 0x012D7 };
	std::vector<int> validSurfaceThree = { 0x00698, 0x0048F, 0x09F92, 0x0A049, 0x006E3, 0x008BB, 0x0078D, 0x01205, 0x012D7, 0x17ECA, 0x0A02D };
	int endIndex = static_cast<int>(desertPanels.size());
	for (int i = 0; i < endIndex - 1; i++) {
		const int target = Random::rand() % (endIndex - i) + i;
		//Prevent ambiguity caused by shadows, and ensure all latches on Surface 7 and Light 3 must be opened
		if (i == target || i == 1 && std::find(valid1.begin(), valid1.end(), desertPanels[target]) == valid1.end() || 
			i == 2 && std::find(validSurfaceThree.begin(), validSurfaceThree.end(), desertPanels[target]) == validSurfaceThree.end() ||
			i == 9 && std::find(valid2.begin(), valid2.end(), desertPanels[target]) == valid2.end() ||
			i == 6 && std::find(validSurfaceSeven.begin(), validSurfaceSeven.end(), desertPanels[target]) == validSurfaceSeven.end() ||
			i == 10 && std::find(valid3.begin(), valid3.end(), desertPanels[target]) == valid3.end()) {
			i--;
			continue;
		}
		if (i != target) {
			SwapPanels(puzzles[i], puzzles[target], SWAP::LINES);
			std::swap(desertPanels[i], desertPanels[target]);
		}
		Memory::get()->WritePanelData<float>(puzzles[i], PATH_WIDTH_SCALE, {0.8f});
	}
	/* TODO: Add into v1.3 after doing more experimentation
	if (_memory->ReadPanelData<float>(0x00295, POWER) < 1)
		for (auto panel : desertPanels) {
			auto rotationData = _memory->ReadPanelData<float>(panel, ORIENTATION, 4);
			auto q = Quaternion{rotationData[3], rotationData[0], rotationData[1], rotationData[2]};
			q = q.Rotate90();
			//q = q.Reflect(); Figure this out at some point
			_memory->WritePanelData<float>(panel, ORIENTATION, {(float)q.x, (float)q.y, (float)q.z, (float)q.w});
		}*/
	Special::setPower(0x09F94, false); // Turn off desert surface 8
	Special::setTargetAndDeactivate(0x17ECA, 0x18076); // Change desert floating target to desert flood final
}

void Randomizer::Randomize(std::vector<int>& panels, int flags) {
	return RandomizeRange(panels, flags, 0, panels.size());
}

// Range is [start, end)
void Randomizer::RandomizeRange(std::vector<int> panels, int flags, size_t startIndex, size_t endIndex) {
	if (panels.size() == 0) return;
	if (startIndex >= endIndex) return;
	if (endIndex >= panels.size()) endIndex = static_cast<int>(panels.size());
	for (size_t i = endIndex - 1; i > startIndex; i--) {
		const int target = (Random::rand() % (static_cast<int>(i) - static_cast<int>(startIndex) + 1)) + static_cast<int>(startIndex);
		if (i != target) {
			SwapPanels(panels[i], panels[target], flags);
			std::swap(panels[i], panels[target]); // Panel indices in the array
		}
	}
}

//Isn't actualy working
void Randomizer::RandomizeAudiologs()
{
	std::vector<int> audiologs = {
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
}

void Randomizer::SwapPanels(int panel1, int panel2, int flags) {
	if (!_shuffleMapping.count(panel1)) {
		_shuffleMapping[panel1] = panel1;
	}
	if (!_shuffleMapping.count(panel2)) {
		_shuffleMapping[panel2] = panel2;
	}
	std::swap(_shuffleMapping[panel1], _shuffleMapping[panel2]);

	std::map<int, int> offsets;

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
	if (flags & SWAP::LINES) {
		offsets[TRACED_EDGES] = 16;
		offsets[AUDIO_PREFIX] = sizeof(void*);
		offsets[PATH_WIDTH_SCALE] = sizeof(float);
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

	Memory* memory = Memory::get();
	for (auto const&[offset, size] : offsets) {
		std::vector<byte> panel1data = memory->ReadPanelData<byte>(panel1, offset, size);
		std::vector<byte> panel2data = memory->ReadPanelData<byte>(panel2, offset, size);
		memory->WritePanelData<byte>(panel2, offset, panel1data);
		memory->WritePanelData<byte>(panel1, offset, panel2data);
	}
	memory->WritePanelData<int>(panel1, NEEDS_REDRAW, { 1 });
	memory->WritePanelData<int>(panel2, NEEDS_REDRAW, { 1 });
}

void Randomizer::ReassignTargets(const std::vector<int>& panels, const std::vector<int>& order, std::vector<int> targets) {
	Memory* memory = Memory::get();

	if (targets.empty()) {
		// This list is offset by 1, so the target of the Nth panel is in position N (aka the N+1th element)
		// The first panel may not have a wire to power it, so we use the panel ID itself.
		targets = { panels[0] + 1 };
		for (const int panel : panels) {
			int target = memory->ReadPanelData<int>(panel, TARGET, 1)[0];
			targets.push_back(target);
		}
	}

	for (size_t i = 0; i < order.size() - 1; i++) {
		// Set the target of order[i] to order[i+1], using the "real" target as determined above.
		const int panelTarget = targets[order[i + 1]];
		memory->WritePanelData<int>(panels[order[i]], TARGET, { panelTarget });
	}
}

void Randomizer::SwapWithRandomPanel(int panel1, const std::vector<int>& possiblePanels, int flags) {
	int toSwap = -1;
	do {
		const int target = Random::rand() % static_cast<int>(possiblePanels.size());
		toSwap = possiblePanels[target];
	} while (_alreadySwapped.count(toSwap));
	if (panel1 != toSwap) {
		SwapPanels(panel1, toSwap, flags);
	}
	_alreadySwapped.insert(toSwap);
}

// Range is [start, end)
void Randomizer::ShuffleRange(std::vector<int>& order, size_t startIndex, size_t endIndex) {
	if (order.size() == 0) return;
	if (startIndex >= endIndex) return;
	if (endIndex >= order.size()) endIndex = static_cast<int>(order.size());
	for (size_t i = endIndex - 1; i > startIndex; i--) {
		const int target = (Random::rand() % (static_cast<int>(i) - static_cast<int>(startIndex) + 1)) + static_cast<int>(startIndex); 
		std::swap(order[i], order[target]);
	}
}

void Randomizer::ShufflePanels(bool hard) {
	Memory* memory = Memory::get();

	_alreadySwapped.clear();

	// General shuffles.
	if (hard) {
		SwapWithRandomPanel(0x0A3B5, tutorialBackLeftExpertOptions, SWAP::LINES | SWAP::COLORS); // Tutorial Back Left
	} else {
		SwapWithRandomPanel(0x0A3B5, tutorialBackLeftOptions, SWAP::LINES | SWAP::COLORS); // Tutorial Back Left
	}
	Randomize(utmElevatorControls, SWAP::LINES | SWAP::COLORS);
	Randomize(treehousePivotSet, SWAP::LINES | SWAP::COLORS);
	Randomize(utmPerspectiveSet, SWAP::LINES | SWAP::COLORS);
	if (!hard) {
		Randomize(symmetryLaserYellows, SWAP::LINES | SWAP::COLORS);
		Randomize(symmetryLaserBlues, SWAP::LINES | SWAP::COLORS);
		Randomize(squarePanels, SWAP::LINES | SWAP::COLORS);
	} else {
		std::vector<int> panelsToRandom = copyWithoutElements(squarePanels, squarePanelsExpertBanned);
		std::vector<int> firstPass = copyWithoutElements(panelsToRandom, arrowPanels);
		std::vector<int> secondPass = copyWithoutElements(panelsToRandom, glassPanels);

		Randomize(firstPass, SWAP::LINES | SWAP::COLORS);
		Randomize(secondPass, SWAP::LINES | SWAP::COLORS);
	}
	Randomize(desertPanelsWide, SWAP::LINES);
	Randomize(mountainMultipanel, SWAP::LINES | SWAP::COLORS);

	// Symmetry transparent.
	std::vector<int> transparentRandomOrder(transparent.size(), 0);
	std::iota(transparentRandomOrder.begin(), transparentRandomOrder.end(), 0);
	ShuffleRange(transparentRandomOrder, 1, 5);
	ReassignTargets(transparent, transparentRandomOrder);

	// Orchard.
	std::vector<int> orchardRandomOrder(orchard.size() + 1, 0);
	std::iota(orchardRandomOrder.begin(), orchardRandomOrder.end(), 0);
	ShuffleRange(orchardRandomOrder, 1, 5);
	// Ensure that we open the gate before the final puzzle (by swapping)
	int panel3Index = find(orchardRandomOrder, 3);
	int panel4Index = find(orchardRandomOrder, 4);
	orchardRandomOrder[std::min(panel3Index, panel4Index)] = 3;
	orchardRandomOrder[std::max(panel3Index, panel4Index)] = 4;
	ReassignTargets(orchard, orchardRandomOrder);

	// Don't power off Town Apple Tree on fail on Expert.
	if (hard) {
		Memory::get()->WritePanelData<int>(0x28938, POWER_OFF_ON_FAIL, {0});
	}

	// Monastery.
	std::vector<int> monasteryRandomOrder(monasteryPanels.size(), 0);
	std::iota(monasteryRandomOrder.begin(), monasteryRandomOrder.end(), 0);
	ShuffleRange(monasteryRandomOrder, 3, 9); // Outer 2 & 3, Inner 1-4
	ReassignTargets(monasteryPanels, monasteryRandomOrder);

	// Bunker.
	std::vector<int> bunkerRandomOrder(bunkerPanels.size(), 0);
	std::iota(bunkerRandomOrder.begin(), bunkerRandomOrder.end(), 0);
	// Randomize Tutorial 2-Advanced Tutorial 4 + Glass 1
	// Tutorial 1 cannot be randomized, since no other panel can start on
	// Glass 1 will become door + glass 1, due to the targetting system
	ShuffleRange(bunkerRandomOrder, 1, 10);
	// Randomize Glass 1-3 into everything after the door/glass 1
	const size_t glass1Index = find(bunkerRandomOrder, 9);
	ShuffleRange(bunkerRandomOrder, glass1Index + 1, 12);
	ReassignTargets(bunkerPanels, bunkerRandomOrder);

	// Jungle.
	std::vector<int> jungleRandomOrder(junglePanels.size(), 0);
	std::iota(jungleRandomOrder.begin(), jungleRandomOrder.end(), 0);
	ShuffleRange(jungleRandomOrder, 8, 13); // Pitches 1-6
	ReassignTargets(junglePanels, jungleRandomOrder);

	// Randomize final pillars order
	std::vector<int> pillarTargets = { pillars[0] + 1 };
	for (const int pillar : pillars) {
		int target = memory->ReadPanelData<int>(pillar, TARGET, 1)[0];
		pillarTargets.push_back(target);
	}
	pillarTargets[5] = pillars[5] + 1;
	std::vector<int> pillarRandomOrder(pillars.size(), 0);
	std::iota(pillarRandomOrder.begin(), pillarRandomOrder.end(), 0);
	ShuffleRange(pillarRandomOrder, 0, 4); // Left Pillars 1-4
	ShuffleRange(pillarRandomOrder, 5, 9); // Right Pillars 1-4
	ReassignTargets(pillars, pillarRandomOrder, pillarTargets);
	// Turn off original starting panels
	memory->WritePanelData<float>(pillars[0], POWER, { 0.0f, 0.0f });
	memory->WritePanelData<float>(pillars[5], POWER, { 0.0f, 0.0f });
	// Turn on new starting panels
	memory->WritePanelData<float>(pillars[pillarRandomOrder[0]], POWER, { 1.0f, 1.0f });
	memory->WritePanelData<float>(pillars[pillarRandomOrder[5]], POWER, { 1.0f, 1.0f });
}
