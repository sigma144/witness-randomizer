#include "Input.h"
#include "Memory.h"
#include "Utilities.h"


#define PRINT_INPUT_DEBUG 1

InputWatchdog* InputWatchdog::_singleton = nullptr;


void InputWatchdog::initialize() {
	if (_singleton == nullptr) {
		_singleton = new InputWatchdog();
	}
}

InputWatchdog* InputWatchdog::get() {
	return _singleton;
}

InputWatchdog::InputWatchdog() : Watchdog(0.01f) {
	memory = std::make_shared<Memory>("witness64_d3d11.exe");
	findGlobalOffsets();
}

void InputWatchdog::action() {
	updateKeyState();
	updateInteractionState();
}

bool InputWatchdog::getButtonState(InputButton key) const {
	// NOTE: The actual value for any given key could be non-zero.
	return currentKeyState[static_cast<int>(key)] != 0;
}

InteractionState InputWatchdog::getInteractionState() const {
	if (currentMenuOpenPercent >= 1.f) {
		return InteractionState::Menu;
	}
	else if (currentInteractMode == 0x0) {
		return InteractionState::Focusing;
	}
	else if (currentInteractMode == 0x1) {
		return InteractionState::Solving;
	}
	else {
		return InteractionState::Walking;
	}
}

void InputWatchdog::updateKeyState() {
	// First, find the address of the key state in memory.
	const std::vector<int> offsets = {
		// Input devices are contained within the Gamelib_Renderer struct. Conveniently enough, Gamelib_Renderer::input_devices is literally the first field
		//   in the struct, so as soon as we dereference the pointer to the renderer we can immediately dereference the input device array.
		Memory::GAMELIB_RENDERER,
		0x0,

		// In the most recent build, the relevant Keyboard struct in input_devices is 0x90 bytes offset from the start of the input_devices array. Note that
		//   this differs from the build with the PDBs due to differences in the input data structures.
		// This value was determined by decompiling InputDevices::get_keyboard_button_state(), which does nothing but retrieve the pointer to the keyboard and
		//   call Keyboard::get_button_state().
		0x90,

		// Finally, the actual input data table is 8 bytes offset from the start of the keyboard struct. This is consistent across builds.
		0x8
	};

	uint64_t keyStateAddress = reinterpret_cast<uint64_t>(memory->ComputeOffset(offsets));

	// Read the new state into memory.
	int32_t newKeyState[INPUT_KEYSTATE_SIZE];
	if (!memory->ReadAbsolute(reinterpret_cast<void*>(keyStateAddress), &newKeyState, 0x200 * sizeof(int32_t))) {
		return;
	}

#if PRINT_INPUT_DEBUG
	std::vector<int> changedKeys;
	for (int keyIndex = 1; keyIndex < INPUT_KEYSTATE_SIZE; keyIndex++) {
		// Only report values changed to/from zero. The input code sets a value of 3 for the first frame a button is pressed and 5 for
		//   the frame it is released, so reporting the value every time it changes adds unnecessary noise.
		if (newKeyState[keyIndex] != currentKeyState[keyIndex] && (newKeyState[keyIndex] == 0 || currentKeyState[keyIndex] == 0)) {
			changedKeys.push_back(keyIndex);
		}
	}

	if (changedKeys.size() > 0) {
		std::wostringstream activeKeyString;
		activeKeyString << "HELD KEYS:";
		for (int keyIndex : changedKeys) {
			if (newKeyState[keyIndex] != 0) {
				activeKeyString << " 0x" << std::hex << keyIndex;
			}
		}

		activeKeyString << std::endl;

		OutputDebugStringW(activeKeyString.str().c_str());
	}
#endif

	// Save the new key state.
	std::copy(newKeyState, newKeyState + INPUT_KEYSTATE_SIZE, currentKeyState);
}

void InputWatchdog::updateInteractionState() {
	InteractionState oldState = getInteractionState();

	if (!memory->ReadRelative(reinterpret_cast<void*>(interactModeOffset), &currentInteractMode, sizeof(int32_t))) {
		currentInteractMode = 0x2; // fall back to not solving
	}

	if (!memory->ReadRelative(reinterpret_cast<void*>(menuOpenOffset), &currentMenuOpenPercent, sizeof(float))) {
		currentMenuOpenPercent = 0.f; // fall back to not open
	}

#if PRINT_INPUT_DEBUG
	if (getInteractionState() != oldState) {
		std::wostringstream interactionStateString;
		interactionStateString << "INTERACTION STATE: ";
		switch (getInteractionState()) {
		case InteractionState::Walking:
			interactionStateString << "WALKING" << std::endl;
			break;
		case InteractionState::Focusing:
			interactionStateString << "FOCUSING" << std::endl;
			break;
		case InteractionState::Solving:
			interactionStateString << "SOLVING" << std::endl;
			break;
		case InteractionState::Menu:
			interactionStateString << "MENU" << std::endl;
			break;
		default:
			interactionStateString << "UNKNOWN" << std::endl;
		}

		OutputDebugStringW(interactionStateString.str().c_str());
	}
#endif
}

void InputWatchdog::findGlobalOffsets() {
	// The interact mode is within the GLOBALS struct and has a fixed offset within all recent builds. Note that this offset
	//   is different than in the PDB build.
	interactModeOffset = Memory::GLOBALS + 0x524;

	// In order to find menu_open_t, we need to find a usage of it. draw_floating_symbols has a unique entry point:
	uint64_t floatingSymbolOffset = memory->executeSigScan({
		0x48, 0x63, 0xC3,			// MOVSXD RAX,EBX
		0x48, 0x6B, 0xC8, 0x7C}		// IMUL RCX,RAX,0x7C
	);

	// Skip ahead to the actual read:
	//  0x48, 0x03, 0x0D			// |
	//  0x40, 0x45, 0x43, 0x00		// ADD RCX, qword ptr [floating_symbols.data]
	//  0x0F, 0x2F, 0x79, 0x6C		// COMISS XMM7, dword ptr [RCX + 0x6C]			// floating_symbols.data[N].lifetime_total
	//  0x73, 0x11					// JNC
	//  0xF3, 0x0F, 0x10, 0x0D		// |
	//  ____, ____, ____, ____		// MOVSS XMM1, dword ptr [menu_open_t]
	floatingSymbolOffset += 7 + 17;

	menuOpenOffset = 0;
	if (memory->ReadRelative(reinterpret_cast<void*>(floatingSymbolOffset), &menuOpenOffset, 0x4)) {
		// Since menu_open_t is a global, any access to it uses an address that's relative to the instruction doing the access.
		menuOpenOffset += floatingSymbolOffset + 0x4;
	}
	else {
		menuOpenOffset = 0;
	}
}