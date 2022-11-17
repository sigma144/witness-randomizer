#include "Input.h"
#include "Memory.h"
#include "Utilities.h"


#define PRINT_KEYSTATE_DEBUG 0

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
}

void InputWatchdog::action() {
	int32_t newKeyState[INPUT_KEYSTATE_SIZE];
	if (!memory->ReadAbsolute(reinterpret_cast<void*>(findKeyStateAddress()), &newKeyState, 0x200 * sizeof(int32_t))) {
		return;
	}

#if PRINT_KEYSTATE_DEBUG
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

bool InputWatchdog::getButtonState(InputButton key) const
{
	// NOTE: The actual value for any given key could be non-zer
	return currentKeyState[static_cast<int>(key)] != 0;
}

uint64_t InputWatchdog::findKeyStateAddress() {
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

	return reinterpret_cast<uint64_t>(memory->ComputeOffset(offsets));
}