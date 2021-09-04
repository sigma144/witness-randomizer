// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Memory.h"
#include "Randomizer.h"
#include "Panels.h"
#include "PuzzleList.h"
#include <string>
#include <iostream>
#include <numeric>
#include "Random.h"

void Randomizer::GenerateNormal(HWND loadingHandle) {
	std::shared_ptr<PuzzleList> puzzles = std::make_shared<PuzzleList>();
	puzzles->setLoadingHandle(loadingHandle);
	puzzles->setSeed(seed, seedIsRNG);
	puzzles->GenerateAllN();
	Panel::SavePanels(seed, false);
}

void Randomizer::GenerateHard(HWND loadingHandle) {
	std::shared_ptr<PuzzleList> puzzles = std::make_shared<PuzzleList>();
	puzzles->setLoadingHandle(loadingHandle);
	puzzles->setSeed(seed, seedIsRNG);
	puzzles->GenerateAllH();
	Panel::SavePanels(seed, true);
	MessageBox(GetActiveWindow(), L"Hi there! Thanks for trying out Expert Mode. It will be tough, but I hope you have fun!\r\n\r\n"
		L"Here are some things you should know before you get started:\r\n\r\n"
		L"- Your first big challenge will involve getting out of the tutorial area. A puzzle that you normally need to solve will be unsolvable the normal way.\r\n\r\n"
		L"- One puzzle in the Treehouse area is deactivated, making it harder to get to the laser.\r\n\r\n"
		L"- Progression in the Town is slightly different. You must complete the 3-color room to open one of the tower doors.\r\n\r\n"
		L"- The Keep has been made more challenging. You'll need to think creatively to figure out how to get around environmental obstacles.\r\n\r\n"
		L"- For negative tetris puzzles, make sure your solution has all tetris shapes lying within the bounds of the grid prior to cancellation.\r\n\r\n"
		L"- Remember to check the Hints page on the wiki if you get stuck.\r\n\r\n"
		L"- Good luck!", L"Welcome", MB_OK);
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
	_memory->WritePanelData<float>(0x09F95, OPEN_RATE, { 0.04f });  // Desert Surface Final Control, 4x
	_memory->WritePanelData<float>(0x03839, OPEN_RATE, { 0.7f }); // Mill Ramp, 3x
	_memory->WritePanelData<float>(0x021BA, OPEN_RATE, { 1.5f }); // Mill Lift, 3x
	_memory->WritePanelData<float>(0x17CC1, OPEN_RATE, { 0.8f }); // Mill Elevator, 4x
	_memory->WritePanelData<float>(0x0061A, OPEN_RATE, { 0.1f }); // Swamp Sliding Bridge, 4x
	_memory->WritePanelData<float>(0x09EEC, OPEN_RATE, { 0.1f }); // Mountain 2 Elevator, 4x
	// What I would also like to speed up, but haven't been able to figure it out yet: Swamp Flood Gate, Swamp Rotating Bridge
}

void Randomizer::StartWatching() {
	//test
	(new TestWatchdog())->start();
	//holy shit it works
}

void Randomizer::PreventSnipes()
{
	// Distance-gate shadows laser to prevent sniping through the bars
	_memory->WritePanelData<float>(0x19650, MAX_BROADCAST_DISTANCE, { 2.5 });
}

void Randomizer::RandomizeDesert() {
	std::vector<int> puzzles = desertPanels;
	std::vector<int> valid1 = { 0x00698, 0x0048F, 0x09F92, 0x09DA6, 0x0078D, 0x04D18, 0x0117A, 0x17ECA, 0x0A02D };
	std::vector<int> valid2 = { 0x00698, 0x09F92, 0x0A036, 0x0A049, 0x0A053, 0x00422, 0x006E3, 0x00C72, 0x008BB, 0x0078D, 0x01205, 0x181AB, 0x012D7, 0x17ECA, 0x0A02D };
	std::vector<int> valid3 = { 0x00698, 0x0048F, 0x09F92, 0x0A036, 0x0A049, 0x00422, 0x008BB, 0x0078D, 0x18313, 0x01205, 0x012D7 };
	int endIndex = static_cast<int>(desertPanels.size());
	for (int i = 0; i < endIndex - 1; i++) {
		const int target = Random::rand() % (endIndex - i) + i;
		//Prevent ambiguity caused by shadows
		if (i == target || i == 1 && std::find(valid1.begin(), valid1.end(), desertPanels[target]) == valid1.end() || 
			(i == 2 || i == 9) && std::find(valid2.begin(), valid2.end(), desertPanels[target]) == valid2.end() ||
			i == 10 && std::find(valid3.begin(), valid3.end(), desertPanels[target]) == valid3.end()) {
			i--;
			continue;
		}
		if (i != target) {
			SwapPanels(puzzles[i], puzzles[target], SWAP::LINES);
			std::swap(desertPanels[i], desertPanels[target]);
		}
	}
}

void Randomizer::Randomize(std::vector<int>& panels, int flags) {
	return RandomizeRange(panels, flags, 0, panels.size());
}

// Range is [start, end)
void Randomizer::RandomizeRange(std::vector<int> &panels, int flags, size_t startIndex, size_t endIndex) {
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

void Randomizer::SwapPanels(int panel1, int panel2, int flags) {
	std::map<int, int> offsets;

	if (flags & SWAP::TARGETS) {
		offsets[TARGET] = sizeof(int);
	}
	if (flags & SWAP::AUDIO_NAMES) {
		offsets[AUDIO_LOG_NAME] = sizeof(void*);
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

	for (auto const&[offset, size] : offsets) {
		std::vector<byte> panel1data = _memory->ReadPanelData<byte>(panel1, offset, size);
		std::vector<byte> panel2data = _memory->ReadPanelData<byte>(panel2, offset, size);
		_memory->WritePanelData<byte>(panel2, offset, panel1data);
		_memory->WritePanelData<byte>(panel1, offset, panel2data);
	}
	_memory->WritePanelData<int>(panel1, NEEDS_REDRAW, { 1 });
	_memory->WritePanelData<int>(panel2, NEEDS_REDRAW, { 1 });
}