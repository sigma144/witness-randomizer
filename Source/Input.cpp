#include "Input.h"
#include "Memory.h"
#include "Utilities.h"


#define PRINT_KEYSTATE_DEBUG 1

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
	findKeyStateAddress();
}

void InputWatchdog::action() {
	if (keyStateAddress == 0) return;

	int32_t newKeyState[INPUT_KEYSTATE_SIZE];
	if (!memory->ReadAbsolute(reinterpret_cast<void*>(keyStateAddress), &newKeyState, 0x200 * sizeof(int32_t))) {
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

bool InputWatchdog::findKeyStateAddress() {
	keyStateAddress = 0;

	// First, find Overlay::begin, which has a lot of useful information.
	uint64_t overlayFunctionOffset = memory->executeSigScan({ 0x45, 0x33, 0xC9, 0x89, 0x41, 0x40, 0x48, 0x8B, 0x0D });
	if (overlayFunctionOffset == UINT64_MAX) {
		throw "Unable to locate Overlay::begin().";
		return false;
	}

	overlayFunctionOffset -= 0x100; // Back up to a little before the actual start of the function.

	// Read the entirety of Overlay::begin into memory.
	std::vector<byte> overlayFunctionBody(0x200);
	if (!memory->ReadRelative(reinterpret_cast<void*>(overlayFunctionOffset), &overlayFunctionBody[0], overlayFunctionBody.size())) {
		throw "Unable to load Overlay::begin() into memory.";
		return false;
	}

	// Next, find the pointer to gamelib_render. What we searched for above is the code that reads it:
	//   0x45, 0x33, 0xC9       <- (part of a previous operation)
	//   0x89, 0x41, 0x40       <- (part of a previous operation)
	//   0x48, 0x8B, 0x0D,      <-
	//   0x??, 0x??, 0x??, 0x??  | MOV RCX, qword ptr [????]
	// Since referencing global values is a relative operation, our final address is equal to the offset passed to the MOV operation, which is a 32-bit
	//   integer, plus the address of the instruction immediately after the call.
	int rendererPointerIndex = 0x100 + 0x9; // Move back to our found address, then 9 bytes after to get to the pointer.
	uint32_t rendererPointerJumpLength = *(uint32_t*)(&overlayFunctionBody[rendererPointerIndex]);
	uint64_t rendererPointerOffset = rendererPointerJumpLength + overlayFunctionOffset + rendererPointerIndex + 0x4;

	// Next, dereference the gamelib_render pointer. This will be a value on the heap.
	uint64_t rendererAddress;
	if (!memory->ReadRelative(reinterpret_cast<void*>(rendererPointerOffset), &rendererAddress, sizeof(uint64_t))) {
		throw "Unable to dereference gamelib_render.";
		return false;
	}

	// Next, we need to find gamelib_render->input_devices. Conveniently enough, that pointer is literally the first field, so we can instantly read it. Note
	//   that this pointer is on the heap and can thus contain any arbitrary value.
	uint64_t inputDevicesAddress;
	if (!memory->ReadAbsolute(reinterpret_cast<void*>(rendererAddress), &inputDevicesAddress, sizeof(uint64_t))) {
		throw "Unable to dereference gamelib_render->input_devices.";
		return false;
	}

	// Now that we have the address of the input_devices object, we need to find out where the Keyboard data lies within it, then find where the array of key
	//   data is. In order to do so, we need to find where that data is written to and read from so that we can figure out the relative position of these fields.
	// First, let's find the address of get_keyboard_button_state(). This takes two parameters: a pointer to the input device (which stored in RCX) and a byte
	//   that represents the key being pressed (stored in EDX). The actual key byte generally changes between versions, but there's always a call to check some
	//   sort of control marker at 0x0.
	std::vector<byte> keyboardFunctionSearchBytes = {
		0x33, 0xD2,				// XOR EDX,EDX <- clears the key parameter
		0xC1, 0xE8, 0x02,		// (part of a previous operation)
		0x48, 0x8B, 0xCF,		// MOV RCX,RDI <- move input_devices into RCX
		0x24, 0x01,				// (part of a previous operation)
		0x88, 0x43, 0x29,		// (part of a previous operation. this is brittle because it assumes input_devices->cursor_released is at index+0x29
		0xE8					// CALL ...
	};

	int keyboardFunctionIndex = Utilities::findSequence<byte>(overlayFunctionBody, keyboardFunctionSearchBytes);
	if (keyboardFunctionIndex == -1) {
		throw "Unable to find index of Input_Devices::get_keyboard_button_state pointer in Overlay::begin().";
		return false;
	}

	keyboardFunctionIndex += keyboardFunctionSearchBytes.size(); // Advance past the search sequence to start of pointer.

	// Since referencing global values is relative, we must compute the absolute value ourselves based on the address of the next operation.
	uint32_t keyboardFunctionJumpLength = *(uint32_t*)(&overlayFunctionBody[keyboardFunctionIndex]);
	uint32_t keyboardFunctionOffset = keyboardFunctionJumpLength + overlayFunctionOffset + keyboardFunctionIndex + 0x4;

	// Next, read the body of Input_Devices::get_keyboard_button_state().
	const SIZE_T keyboardButtonFunctionSize = 0x20;
	std::vector<byte> keyboardButtonFunctionBody(keyboardButtonFunctionSize);
	if (!memory->ReadRelative(reinterpret_cast<void*>(keyboardFunctionOffset), &keyboardButtonFunctionBody[0], keyboardButtonFunctionSize)) {
		throw "Unable to read Input_Devices::get_keyboard_button_state into memory.";
		return false;
	}

	// Verify that we have indeed jumped to the right function by checking to see if the first two bytes are a MOV operation.
	if (!(keyboardButtonFunctionBody[0] == 0x48 && keyboardButtonFunctionBody[1] == 0x8b)) {
		throw "Memory at pointer indicated by Input_Devices::get_keyboard_button_state did not start with the expected MOV operation.";
	}

	// InputDevices::get_keyboard_button_state() is simple: it retrieves the pointer to the keyboard from input_devices, then calls Keyboard::get_button_state().
	//   In order to retrieve the pointer ourselves, we need to find the offset within input_devices that represents the keyboard pointer, which we can do by
	//   reading the offset parameter passed to the MOV operation.
	// NOTE: The actual MOV call is slightly inconsistent between builds and may use either 1 or 4 bytes for its offset, so we need to detect that.
	uint64_t keyboardPointer = inputDevicesAddress;
	if (keyboardButtonFunctionBody[2] == 0x49) {
		keyboardPointer += *(uint8_t*)(&keyboardButtonFunctionBody[3]);
	}
	else if (keyboardButtonFunctionBody[2] == 0x89) {
		keyboardPointer += *(uint32_t*)(&keyboardButtonFunctionBody[3]);
	}
	else
	{
		throw "Unexpected argument to MOV when trying to retrieve the keyboard pointer.";
		return false;
	}

	// Finally, read the address of the keyboard data table. This is equal to the address pointed to by the keyboard pointer plus 0x8, which is the offset of the
	//   key_state within the Keyboard struct. (This seems to be consistent across builds, so we don't need to dig into get_button_state() to retrieve it.)
	uint64_t keyboardAddress;
	if (!memory->ReadAbsolute(reinterpret_cast<void*>(keyboardPointer), &keyboardAddress, sizeof(uint64_t))) {
		throw "Unable to dereference keyboard pointer.";
		return false;
	}

	// The value of the key_state array is 8 bytes after the start of the Keyboard struct. (This seems to be consistent across builds, so we don't need to read
	//   Keyboard::get_button_state() to retrieve it.
	keyStateAddress = keyboardAddress + 0x08;
	return true;
}