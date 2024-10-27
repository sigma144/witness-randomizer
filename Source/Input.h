#pragma once

#include "DataTypes.h"
#include "Watchdog.h"
#include <cstdint>
#include <optional>

#ifndef INPUT_KEYSTATE_SIZE
// The size of the Keyboard::key_state array, which in the most recent build is an int[256]. Note that this differs from the PDB build.
#define INPUT_KEYSTATE_SIZE 0x200
#define WARPTIME 2.5f
#define LONGWARPBUFFER 0.5f
#define WARPFINALISATIONTIME 0.5f
#endif


// Custom keys for randomizer-specific functionality.
enum class CustomKey : int {
	SLEEP,
	SKIP_PUZZLE,
	COUNT
};

// A mapping of human-readable button names onto the hex codes used by the game.
enum class InputButton : int {
	// Letters
	KEY_A				= 0x61,
	KEY_B				= 0x62,
	KEY_C				= 0x63,
	KEY_D				= 0x64,
	KEY_E				= 0x65,
	KEY_F				= 0x66,
	KEY_G				= 0x67,
	KEY_H				= 0x68,
	KEY_I				= 0x69,
	KEY_J				= 0x6a,
	KEY_K				= 0x6b,
	KEY_L				= 0x6c,
	KEY_M				= 0x6d,
	KEY_N				= 0x6e,
	KEY_O				= 0x6f,
	KEY_P				= 0x70,
	KEY_Q				= 0x71,
	KEY_R				= 0x72,
	KEY_S				= 0x73,
	KEY_T				= 0x74,
	KEY_U				= 0x75,
	KEY_V				= 0x76,
	KEY_W				= 0x77,
	KEY_X				= 0x78,
	KEY_Y				= 0x79,
	KEY_Z				= 0x7a,
	
	// Numerals (not numpad)
	KEY_0				= 0x30,
	KEY_1				= 0x31,
	KEY_2				= 0x32,
	KEY_3				= 0x33,
	KEY_4				= 0x34,
	KEY_5				= 0x35,
	KEY_6				= 0x36,
	KEY_7				= 0x37,
	KEY_8				= 0x38,
	KEY_9				= 0x39,

	// Function keys
	KEY_F1				= 0x112,
	KEY_F2				= 0x113,
	KEY_F3				= 0x114,
	KEY_F4				= 0x115,
	KEY_F5				= 0x116,
	KEY_F6				= 0x117,
	KEY_F7				= 0x118,
	KEY_F8				= 0x119,
	KEY_F9				= 0x11a,
	KEY_F10				= 0x11b,
	KEY_F11				= 0x11c,
	KEY_F12				= 0x11d,

	// Modifier keys
	KEY_SHIFT			= 0x10E,
	KEY_CTRL			= 0x10D,
	KEY_ALT				= 0x10F,
	MODIFIER_HELD		= 0x152, // present when ctrl, shift, or alt are held

	// Miscellaneous symbols
	KEY_BACKTICK		= 0x60, // aka tilde
	KEY_MINUS			= 0x2D,
	KEY_EQUALS			= 0x3D, // aka plus
	KEY_TAB				= 0x09,
	KEY_LEFTBRACKET		= 0x5B,
	KEY_RIGHTBRACKET	= 0x5D,
	KEY_BACKSLASH		= 0x5C,
	KEY_SEMICOLON		= 0x3B,
	KEY_QUOTE			= 0x27,

	// Arrow keys
	KEY_ARROW_UP		= 0x100,
	KEY_ARROW_DOWN		= 0x101,
	KEY_ARROW_LEFT		= 0x102,
	KEY_ARROW_RIGHT		= 0x103,

	// Control inputs
	KEY_ESCAPE			= 0x1B,
	KEY_SPACEBAR		= 0x20,
	KEY_RETURN			= 0x0D, // also used on the numpad
	KEY_BACKSPACE		= 0x08,
	KEY_DELETE			= 0x7F,
	KEY_PAGE_UP			= 0x120,
	KEY_PAGE_DOWN		= 0x121,
	KEY_INSERT			= 0x122,
	KEY_PAUSE			= 0x123,
	KEY_PRINT_SCREEN	= 0x124,
	KEY_HOME			= 0x11E,
	KEY_END				= 0x11F,

	// Numpad keys
	KEY_NUMPAD_0		= 0x126,
	KEY_NUMPAD_1		= 0x127,
	KEY_NUMPAD_2		= 0x128,
	KEY_NUMPAD_3		= 0x129,
	KEY_NUMPAD_4		= 0x12A,
	KEY_NUMPAD_5		= 0x12B,
	KEY_NUMPAD_6		= 0x12C,
	KEY_NUMPAD_7		= 0x12D,
	KEY_NUMPAD_8		= 0x12E,
	KEY_NUMPAD_9		= 0x12F,
	KEY_NUMPAD_PLUS		= 0x130,
	KEY_NUMPAD_MINUS	= 0x131,
	KEY_NUMPAD_MULTIPLY	= 0x132,
	KEY_NUMPAD_DIVIDE	= 0x133,

	// Mouse buttons
	MOUSE_BUTTON_LEFT	= 0x104,
	MOUSE_BUTTON_MIDDLE	= 0x105,
	MOUSE_BUTTON_RIGHT	= 0x106,
	MOUSE_BUTTON_4		= 0x14D,
	MOUSE_BUTTON_5		= 0x14E,

	// Controller face buttons (a/b/x/y, circle/cross/triangle/square, etc.)
	CONTROLLER_FACEBUTTON_DOWN	= 0x135, // A on xbox
	CONTROLLER_FACEBUTTON_RIGHT	= 0x136, // B on xbox
	CONTROLLER_FACEBUTTON_LEFT	= 0x137, // X on xbox
	CONTROLLER_FACEBUTTON_UP	= 0x138, // Y on xbox

	// Inner controller buttons (select/start, -/+ on switch, view/menu on xbox, etc.)
	CONTROLLER_SPECIAL_LEFT		= 0x13E, // select
	CONTROLLER_SPECIAL_RIGHT	= 0x13D, // start

	// Left controller stick. The up/down/left/right axes are presented as binary on/off rather than a range.
	CONTROLLER_LSTICK_UP		= 0x143,
	CONTROLLER_LSTICK_DOWN		= 0x144,
	CONTROLLER_LSTICK_LEFT		= 0x145,
	CONTROLLER_LSTICK_RIGHT		= 0x146,
	CONTROLLER_LSTICK_CLICK		= 0x14B,

	// Right controller stick. The up/down/left/right axes are presented as binary on/off rather than a range.
	CONTROLLER_RSTICK_UP		= 0x147,
	CONTROLLER_RSTICK_DOWN		= 0x148,
	CONTROLLER_RSTICK_LEFT		= 0x149,
	CONTROLLER_RSTICK_RIGHT		= 0x14A,
	CONTROLLER_RSTICK_CLICK		= 0x14C,

	// D-pad on the controller
	CONTROLLER_DPAD_UP			= 0x13F,
	CONTROLLER_DPAD_DOWN		= 0x140,
	CONTROLLER_DPAD_LEFT		= 0x141,
	CONTROLLER_DPAD_RIGHT		= 0x142,

	// Shoulder/trigger buttons on the controller. Trigger values are presented as binary on/off and do not report pressure values.
	CONTROLLER_SHOULDER_LEFT	= 0x139,
	CONTROLLER_SHOULDER_RIGHT	= 0x13a,
	CONTROLLER_TRIGGER_LEFT		= 0x13b,
	CONTROLLER_TRIGGER_RIGHT	= 0x13c,

	// Miscellaneous
	NONE						= 0x0
};

enum MovementDirection {
	NONE = 0,
	LEFT = 1,
	UP = 2,
	RIGHT = 3,
	DOWN = 4,
};

enum InteractionState {
	Walking,	// Free look.
	Focusing,	// Cursor shown, but no puzzle selected
	Solving,	// Actively solving a puzzle
	Cutscene,	// In the ending cutscene
	Menu,		// A menu is shown and blocking game input
	MenuAndSleeping, // A menu is shown AND the player is in "sleep mode"
	Keybinding,	// The randomizer is intercepting input in order to register a keybind.
	Sleeping,   // "Sleep mode", for warps
	Warping,    // Warping, NYI
};


// A manager class for reading input values from game memory and processing them into useful data.
class InputWatchdog : public Watchdog {
public:

	// Instantiates and starts the watchdog.
	static void initialize();

	// Gets the input singleton. Returns nullptr if initialize() has not been called yet.
	static InputWatchdog* get();

	virtual void action() override;

	// Returns the state (pressed/released) for the given key.
	bool getButtonState(InputButton key) const;

	// Returns the button associated with the custom keybind.
	InputButton getCustomKeybind(CustomKey key) const;

	// Returns the player's current interaction mode.
	InteractionState getInteractionState() const;

	// Returns whether or not the interaction state has changed since the last time this function was called.
	bool consumeInteractionStateChange();

	// Returns any key taps (quick presses and releases) generated since the last time this function was called.
	std::vector<InputButton> consumeTapEvents();
	std::vector<MovementDirection> consumeDirectionalEvents();

	// Returns the most recently calculated direction of the mouse.
	const Vector3& getMouseDirection() const;

	static std::string getNameForCustomKey(CustomKey key);
	static std::string getNameForInputButton(InputButton button);

	void loadCustomKeybind(CustomKey key, InputButton button);

	void beginCustomKeybind(CustomKey key);
	bool trySetCustomKeybind(InputButton button);
	void cancelCustomKeybind();

	CustomKey getCurrentlyRebindingKey() const;

	// Determines whether or not the given button is valid for use as a custom keybind.
	bool isValidForCustomKeybind(InputButton button) const;

	void setSleep(bool sleep);
	void startWarp(bool longwarp);
	void endWarp();
	void allowWarpCompletion();
	bool warpIsGoingOvertime() const;
	float getWarpTime();
	void updateWarpTimer(float deltaSeconds);
	int readInteractMode();

	Vector2 getMouseScreenPosition();
	std::vector<Vector3> getCone();
	Vector3 getMouseDirectionFromScreenPosition(Vector2 screenPosition);
	Vector3 calculateMouseVector();

private:
	InputWatchdog();
	static InputWatchdog* _singleton;

	// The raw state data retrieved from game memory. Updated once per tick.
	int32_t currentKeyState[INPUT_KEYSTATE_SIZE];

	void updateKeyState();

	// The direction, in world coordinates, of the player's mouse. Updated once per tick.
	Vector3 mouseDirection;
	
	void updateMouseVector();

	// The raw interact mode integer retrieved from game memory. Updated once per tick.
	int32_t currentInteractMode;

	// The interact mode that was set at the last time consumeInteractionStateChange() was called.
	InteractionState previousInteractionState = InteractionState::Walking;

	// A value between 0 and 1 representing how faded in the pause menu is, with 1 being fully active.
	float currentMenuOpenPercent;

	void updateInteractionState();

	void findInteractModeOffset();
	void findMenuOpenOffset();

	std::optional<CustomKey> currentlyRebindingKey;
	std::map<CustomKey, InputButton> customKeybinds;

	uint64_t interactModeOffset;
	uint64_t menuOpenOffset;
	
	std::map<InputButton, std::chrono::system_clock::time_point> pressTimes;
	std::vector<InputButton> pendingTapEvents;
	MovementDirection lastDirection;
	std::vector<MovementDirection> pendingDirectionEvents;

	bool isAsleep = false;
	float warpTimer = -1.0f;
	bool isAllowedToCompleteWarp = false;
};