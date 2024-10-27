#include "Input.h"

#include "Memory.h"
#include "Utilities.h"
#include "ClientWindow.h"
#include "HUDManager.h"


#define PRINT_INPUT_DEBUG 0
#define KEY_TAP_DURATION 0.4f

InputWatchdog* InputWatchdog::_singleton = nullptr;


void InputWatchdog::initialize() {
	if (_singleton == nullptr) {
		_singleton = new InputWatchdog();
	}
}

InputWatchdog* InputWatchdog::get() {
	return _singleton;
}

InputWatchdog::InputWatchdog() : Watchdog(0.016f) {
	findInteractModeOffset();
	findMenuOpenOffset();

	// TEMP: Set default keybinds.
	customKeybinds[CustomKey::SKIP_PUZZLE] = InputButton::KEY_T;
	customKeybinds[CustomKey::SLEEP] = InputButton::KEY_E;
}

void InputWatchdog::action() {
	updateKeyState();
	updateMouseVector();
	updateInteractionState();
}

bool InputWatchdog::getButtonState(InputButton key) const {
	// NOTE: The actual value for any given key could be non-zero.
	return currentKeyState[static_cast<int>(key)] != 0;
}

InputButton InputWatchdog::getCustomKeybind(CustomKey key) const {
	auto result = customKeybinds.find(key);
	return result != customKeybinds.end() ? result->second : InputButton::NONE;
}

InteractionState InputWatchdog::getInteractionState() const {
	if (warpTimer >= 0) {
		if (currentMenuOpenPercent >= 1.f) {
			return InteractionState::MenuAndSleeping;
		}
		return InteractionState::Warping;
	}
	if (currentlyRebindingKey.has_value()) {
		return InteractionState::Keybinding;
	}
	if (isAsleep) {
		if (currentMenuOpenPercent >= 1.f) {
			return InteractionState::MenuAndSleeping;
		}
		return InteractionState::Sleeping;
	}
	else if (currentMenuOpenPercent >= 1.f) {
		return InteractionState::Menu;
	}
	else {
		switch (currentInteractMode) {
		case 0x0:
			return InteractionState::Focusing;
		case 0x1:
			return InteractionState::Solving;
		case 0x2:
			return InteractionState::Walking;
		case 0x3:
			return InteractionState::Cutscene;
		default:
			// Invalid. Return Walking.
			return InteractionState::Walking;
		}
	}
}

bool InputWatchdog::consumeInteractionStateChange()
{
	InteractionState currentInteractionState = getInteractionState();
	if (currentInteractionState != previousInteractionState) {
		previousInteractionState = currentInteractionState;
		return true;
	}
	else {
		return false;
	}
}

std::vector<InputButton> InputWatchdog::consumeTapEvents() {
	std::vector<InputButton> output = pendingTapEvents;
	pendingTapEvents.clear();
	return output;
}

std::vector<MovementDirection> InputWatchdog::consumeDirectionalEvents() {
	std::vector<MovementDirection> output = pendingDirectionEvents;
	pendingDirectionEvents.clear();
	return output;
}

const Vector3& InputWatchdog::getMouseDirection() const {
	return mouseDirection;
}

void InputWatchdog::loadCustomKeybind(CustomKey key, InputButton button) {
	customKeybinds[key] = button;
}

void InputWatchdog::beginCustomKeybind(CustomKey key) {
	if (getInteractionState() == InteractionState::Sleeping) {
		HudManager::get()->queueBannerMessage("Cannot keybind while sleeping.");
	}
	currentlyRebindingKey = key;
	ClientWindow::get()->focusGameWindow();
}

bool InputWatchdog::trySetCustomKeybind(InputButton button) {
	if (currentlyRebindingKey.has_value() && isValidForCustomKeybind(button)) {
		ClientWindow* clientWindow = ClientWindow::get();

		customKeybinds[currentlyRebindingKey.value()] = button;
		clientWindow->refreshKeybind(currentlyRebindingKey.value());
		currentlyRebindingKey.reset();

		clientWindow->focusClientWindow();
		clientWindow->saveSettings();

		return true;
	}

	return false;
}

void InputWatchdog::cancelCustomKeybind() {
	currentlyRebindingKey.reset();
	ClientWindow::get()->focusClientWindow();
}

CustomKey InputWatchdog::getCurrentlyRebindingKey() const {
	return currentlyRebindingKey.has_value() ? currentlyRebindingKey.value() : CustomKey::COUNT;
}

bool InputWatchdog::isValidForCustomKeybind(InputButton button) const
{
	// Filter inputs that are either hard-coded by the game to map to particular actions or are impractical for keybinds.
	switch (button) {
	case InputButton::MOUSE_BUTTON_LEFT:
	case InputButton::MOUSE_BUTTON_RIGHT:
	case InputButton::KEY_SPACEBAR:
	case InputButton::KEY_ESCAPE:
		// These buttons are hard-coded into the game.
		return false;
	case InputButton::CONTROLLER_LSTICK_UP:
	case InputButton::CONTROLLER_LSTICK_DOWN:
	case InputButton::CONTROLLER_LSTICK_LEFT:
	case InputButton::CONTROLLER_LSTICK_RIGHT:
	case InputButton::CONTROLLER_RSTICK_UP:
	case InputButton::CONTROLLER_RSTICK_DOWN:
	case InputButton::CONTROLLER_RSTICK_LEFT:
	case InputButton::CONTROLLER_RSTICK_RIGHT:
		// Controller axes are not good for keybinds.
		return false;
	}

	// TODO: Identify players' current in-game keybinds and reject those.

	// Filter keys that are already bound to a different custom keybind.
	for (const std::pair<CustomKey, InputButton>& keybind : customKeybinds) {
		if (currentlyRebindingKey.has_value() && keybind.first == currentlyRebindingKey.value()) {
			// If we're rebinding a key, that key's current button is valid.
			continue;
		}

		if (keybind.second == button) {
			return false;
		}
	}

	return true;
}

void InputWatchdog::setSleep(bool sleep) {
	this->isAsleep = sleep;
}

void InputWatchdog::startWarp(bool longWarp) {
	this->warpTimer = WARPTIME;
	if (longWarp) this->warpTimer += LONGWARPBUFFER;
	this->isAllowedToCompleteWarp = false;
}

void InputWatchdog::endWarp() {
	this->setSleep(false);
	this->warpTimer = -1;
}

void InputWatchdog::allowWarpCompletion() {
	isAllowedToCompleteWarp = true;
}

bool InputWatchdog::warpIsGoingOvertime() const {
	return !isAllowedToCompleteWarp && this->warpTimer == WARPFINALISATIONTIME;
}

float InputWatchdog::getWarpTime() {
	return this->warpTimer;
}

void InputWatchdog::updateWarpTimer(float deltaSeconds) {
	if (this->warpTimer == -1) return;
	if (this->warpTimer - deltaSeconds <= 0) {
		this->warpTimer = 0;
	}
	else if (!isAllowedToCompleteWarp && this->warpTimer - deltaSeconds <= WARPFINALISATIONTIME) {
		this->warpTimer = WARPFINALISATIONTIME;
	}
	else {
		this->warpTimer -= deltaSeconds;
	}
}

int InputWatchdog::readInteractMode() {
	int32_t interactMode;

	bool success = Memory::get()->ReadRelative(reinterpret_cast<void*>(interactModeOffset), &interactMode, sizeof(int32_t));
	if (!success) {
		return 0x2;
	}
	else
	{
		return interactMode;
	}
}

void InputWatchdog::updateKeyState() {
	Memory* memory = Memory::get();

	// First, find the address of the key state in memory.
	const std::vector<int> offsets = {
		// Input devices are contained within the Gamelib_Renderer struct. Conveniently enough, Gamelib_Renderer::input_devices is literally the first field
		//   in the struct, so as soon as we dereference the pointer to the renderer we can immediately dereference the input device array.
		memory->GAMELIB_RENDERER,
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
#endif

	for (int keyIndex = 1; keyIndex < INPUT_KEYSTATE_SIZE; keyIndex++) {
		// Test for key changes. Note that we only report values changing to/from zero: the input code sets a value of 3 for the first frame a button is
		//   pressed and 5 for the frame it is released, so if we report an event every time the value changes we'll be adding unnecessary noise.
		//   Additionally, we don't want to explicitly react to 3s and 5s here because we're polling asynchronously, and we may miss the frame that those
		//   values are set.
		bool wasPressed = currentKeyState[keyIndex] != 0;
		bool isPressed = newKeyState[keyIndex] != 0;
		if (!wasPressed && isPressed) {
			// The key was pressed since our last poll. Record when it was pressed.
			pressTimes[static_cast<InputButton>(keyIndex)] = std::chrono::system_clock::now();
		}
		else if (wasPressed && !isPressed) {
			// The key was released since our last poll. Check to see if the press and release are close enough together to count as a tap, and if so,
			//   record it.
			auto result = pressTimes.find(static_cast<InputButton>(keyIndex));
			if (result != pressTimes.end()) {
				std::chrono::duration<float> holdDuration = std::chrono::system_clock::now() - result->second;
				if (holdDuration.count() <= KEY_TAP_DURATION) {
					pendingTapEvents.push_back(static_cast<InputButton>(keyIndex));

#if PRINT_INPUT_DEBUG
					std::wostringstream tapKeyString;
					tapKeyString << "TAPPED: 0x" << std::hex << keyIndex << std::endl;
					OutputDebugStringW(tapKeyString.str().c_str());			
#endif
				}

				pressTimes.erase(result);
			}
		}

#if PRINT_INPUT_DEBUG
		if (wasPressed != isPressed) {
			changedKeys.push_back(keyIndex);
		}
#endif
	}

	Vector3 desiredMovementDirection = Vector3(memory->ReadDesiredMovementDirection());

	MovementDirection currentDirection = MovementDirection::NONE;

	if (std::abs(desiredMovementDirection.X) >= std::abs(desiredMovementDirection.Y)) {
		if (desiredMovementDirection.X > 0.5) currentDirection = MovementDirection::UP;
		if (desiredMovementDirection.X < -0.5) currentDirection = MovementDirection::DOWN;
	}
	else {
		if (desiredMovementDirection.Y > 0.5) currentDirection = MovementDirection::RIGHT;
		if (desiredMovementDirection.Y < -0.5) currentDirection = MovementDirection::LEFT;
	}

	if (currentDirection != lastDirection) {
		lastDirection = currentDirection;
		if (currentDirection != MovementDirection::NONE) {
			pendingDirectionEvents.push_back(currentDirection);
		}
	}

#if PRINT_INPUT_DEBUG
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

Vector2 InputWatchdog::getMouseScreenPosition() {
	Memory* memory = Memory::get();

	uint64_t gestureDataAddress;
	memory->ReadAbsolute(reinterpret_cast<LPVOID>(memory->GESTURE_MANAGER), &gestureDataAddress, sizeof(uint64_t));

	Vector2 screenPosition = Vector2(0.5f, 0.5f);
	memory->ReadAbsolute(reinterpret_cast<LPVOID>(gestureDataAddress + 0x18), &screenPosition, sizeof(Vector2));
	return screenPosition;
}

std::vector<Vector3> InputWatchdog::getCone() {
	Vector2 mouseScreenPosition = InputWatchdog::get()->getMouseScreenPosition();
	Vector3 headPosition = Vector3(Memory::get()->ReadPlayerPosition());

	std::vector<Vector3> rayDirections = {};

	for (int i = -3; i < 4; i++) {
		for (int j = -3; j < 4; j++) {
			if (std::abs(i) + std::abs(j) >= 5) continue;
			Vector2 rayPosition = mouseScreenPosition + Vector2(0.0024f * i, 0.0024f * j);
			Vector3 rayDirection = InputWatchdog::get()->getMouseDirectionFromScreenPosition(rayPosition);

			rayDirections.push_back(rayDirection);
		}
	}

	return rayDirections;
}

Vector3 InputWatchdog::getMouseDirectionFromScreenPosition(Vector2 screenPosition) {
	Memory* memory = Memory::get();

	uint64_t gestureDataAddress;
	memory->ReadAbsolute(reinterpret_cast<LPVOID>(memory->GESTURE_MANAGER), &gestureDataAddress, sizeof(uint64_t));

	struct CursorParams {
		Vector2 strut;
		Vector3 origin;
		Vector3 corner;
		Vector3 e1;
		Vector3 e2;
	} cursorParams;

	memory->ReadAbsolute(reinterpret_cast<LPVOID>(gestureDataAddress + 0x94), &cursorParams, sizeof(CursorParams));

	Vector3 mouseDir = (cursorParams.e1 * 2.f * cursorParams.strut.X * screenPosition.X) +
		(cursorParams.e2 * 2.f * cursorParams.strut.X * screenPosition.Y) +
		cursorParams.corner - cursorParams.origin;
	return mouseDir.normalized();
}

Vector3 InputWatchdog::calculateMouseVector() {
	Memory* memory = Memory::get();

	return getMouseDirectionFromScreenPosition(getMouseScreenPosition());
}

void InputWatchdog::updateMouseVector()
{
	mouseDirection = calculateMouseVector();
}

void InputWatchdog::updateInteractionState() {
	Memory* memory = Memory::get();

	InteractionState oldState = getInteractionState();

	currentInteractMode = readInteractMode();

	if (menuOpenOffset == 0 || !memory->ReadRelative(reinterpret_cast<void*>(menuOpenOffset), &currentMenuOpenPercent, sizeof(float))) {
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
		case InteractionState::Cutscene:
			interactionStateString << "CUTSCENE" << std::endl;
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

void InputWatchdog::findInteractModeOffset() {
	Memory* memory = Memory::get();

	// get_cursor_delta_from_mouse_or_gamepad() has a reliable signature to scan for:
	uint64_t cursorDeltaOffset = memory->executeSigScan({
		0xF3, 0x0F, 0x59, 0xD7,		// MULSS XMM2,XMM7
		0xF3, 0x0F, 0x59, 0xCF,		// MULSS XMM1,XMM7
		0xF3, 0x0F, 0x59, 0xD0,		// MULSS XMM2,XMM0
		0xF3, 0x0F, 0x59, 0xC8,		// MULSS XMM1,XMM0
	});

	if (cursorDeltaOffset == UINT64_MAX) {
		interactModeOffset = 0;
		return;
	}

	// This set of instructions is executed immediately after the call to retrieve the pointer to globals.interact_mode, so we just need to read four bytes prior
	interactModeOffset = 0;
	if (memory->ReadRelative(reinterpret_cast<void*>(cursorDeltaOffset - 0x4), &interactModeOffset, 0x4)) {
		// Since menu_open_t is a global, any access to it uses an address that's relative to the instruction doing the access, which is conveniently our search offset.
		interactModeOffset += cursorDeltaOffset;
	}
	else {
		interactModeOffset = 0;
	}
}

void InputWatchdog::findMenuOpenOffset() {
	Memory* memory = Memory::get();

	// In order to find menu_open_t, we need to find a usage of it. draw_floating_symbols has a unique entry point:
	uint64_t floatingSymbolOffset = memory->executeSigScan({
		0x48, 0x63, 0xC3,			// MOVSXD RAX,EBX
		0x48, 0x6B, 0xC8, 0x7C		// IMUL RCX,RAX,0x7C
	});

	if (floatingSymbolOffset == UINT64_MAX) {
		menuOpenOffset = 0;
		return;
	}

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