#include <codecvt>
#include "Special.h"
#include "Converty.h"
#include "nlohmann/json.hpp"
#include <Richedit.h>

#include "ClientWindow.h"

#include "../App/Version.h"
#include "../App/Main.h"
#include "Input.h"

using json = nlohmann::json;

ClientWindow* ClientWindow::_singleton = nullptr;

#define SAVE_VERSION 4

#define CLIENT_WINDOW_WIDTH 600
#define CLIENT_MENU_CLASS_NAME L"WitnessRandomizer"
#define CLIENT_WINDOW_CLASS_NAME L"WitnessRandomizer"

#define CONTROL_MARGIN 10
#define CONTROL_HEIGHT 26

#define STATIC_TEXT_MARGIN CONTROL_MARGIN + 2
#define STATIC_TEXT_HEIGHT 16

#define SETTING_LABEL_WIDTH 160
#define SETTING_LABEL_Y_OFFSET ((CONTROL_HEIGHT - STATIC_TEXT_HEIGHT) / 2)
#define SETTING_CONTROL_LEFT (STATIC_TEXT_MARGIN + SETTING_LABEL_WIDTH)
#define SETTING_CONTROL_WIDTH (CLIENT_WINDOW_WIDTH - CONTROL_MARGIN - SETTING_CONTROL_LEFT)

#define KEYBIND_BUTTON_WIDTH 100
#define KEYBIND_BUTTON_LEFT (CLIENT_WINDOW_WIDTH - CONTROL_MARGIN - KEYBIND_BUTTON_WIDTH)
#define KEYBIND_LABEL_LEFT SETTING_CONTROL_LEFT
#define KEYBIND_LABEL_WIDTH (KEYBIND_LABEL_LEFT - KEYBIND_BUTTON_WIDTH - CONTROL_MARGIN)

#define LINE_SPACING 5

#define IDC_FIELD_AP_ADDRESS 0x400
#define IDC_FIELD_AP_SLOTNAME 0x401
#define IDC_FIELD_AP_PASSWORD 0x402

#define IDC_BUTTON_RANDOMIZE 0x440
#define IDC_BUTTON_LOADCREDENTIALS 0x441

// Root index for keybind controls.
#define IDC_BUTTON_KEYBIND 0x480

#define IDC_SETTING_COLORBLIND 0x500
#define IDC_SETTING_COLLECT 0x501
#define IDC_SETTING_CHALLENGE 0x502
#define IDC_SETTING_SYNCPROGRESS 0x503
#define IDC_SETTING_DISABLED 0x504
#define IDC_SETTING_HIGHCONTRAST 0x505
#define IDC_SETTING_JINGLES 0x510


void ClientWindow::create(HINSTANCE inAppInstance, int nCmdShow) {
	if (_singleton == nullptr) {
		_singleton = new ClientWindow(inAppInstance);
		_singleton->buildWindow();
		_singleton->show(nCmdShow);
		_singleton->setWindowMode(ClientWindowMode::Disabled);
	}
}

ClientWindow* ClientWindow::get() {
	return _singleton;
}

void ClientWindow::saveSettings()
{
	json data;

	data["saveVersion"] = SAVE_VERSION;

	data["challengeTimer"] = getSetting(ClientToggleSetting::ChallengeTimer);
	data["collect"] = getSetting(ClientDropdownSetting::Collect);
	data["disabled"] = getSetting(ClientDropdownSetting::DisabledPuzzles);
	data["colorblind"] = getSetting(ClientToggleSetting::ColorblindMode);
	data["syncprogress"] = getSetting(ClientToggleSetting::SyncProgress);
	data["highcontrast"] = getSetting(ClientToggleSetting::HighContrast);
	data["jingles"] = getSetting(ClientToggleSetting::Jingles);

	InputWatchdog* input = InputWatchdog::get();
	data["key_skipPuzzle"] = static_cast<int>(input->getCustomKeybind(CustomKey::SKIP_PUZZLE));

	data["apSlotName"] = getSetting(ClientStringSetting::ApSlotName);

	std::remove("WRPGconfig.txt");
	std::ofstream outputStream = std::ofstream("WRPGconfig.json");
	outputStream << data;
	outputStream.close();
}

void ClientWindow::loadSettings()
{
	std::ifstream inputStream = std::ifstream("WRPGconfig.json");
	bool loadedSettings = false;
	if (inputStream.good()) {

		json data;
		inputStream >> data;
		inputStream.close();

		int saveVersion = data.contains("saveVersion") ? data["saveVersion"].get<int>() : 0;
		if (saveVersion == SAVE_VERSION) {
			// Load game settings.
			setSetting(ClientToggleSetting::Jingles, data.contains("jingles") ? data["jingles"].get<bool>() : false);
			setSetting(ClientToggleSetting::ChallengeTimer, data.contains("challengeTimer") ? data["challengeTimer"].get<bool>() : false);

			setSetting(ClientToggleSetting::SyncProgress, data.contains("syncprogress") ? data["syncprogress"].get<bool>() : false);
			setSetting(ClientToggleSetting::HighContrast, data.contains("highcontrast") ? data["highcontrast"].get<bool>() : false);

			setSetting(ClientDropdownSetting::Collect, data.contains("collect") ? data["collect"].get<std::string>() : "Unchanged");
			setSetting(ClientDropdownSetting::DisabledPuzzles, data.contains("disabled") ? data["disabled"].get<std::string>() : "Prevent Solve");
			setSetting(ClientToggleSetting::ColorblindMode, data.contains("colorblind") ? data["colorblind"].get<bool>() : false);

			// Load keybinds.
			InputWatchdog* input = InputWatchdog::get();
			input->loadCustomKeybind(CustomKey::SKIP_PUZZLE,
				data.contains("key_skipPuzzle") ? static_cast<InputButton>(data["key_skipPuzzle"].get<int>()) : InputButton::KEY_T);
			
			loadedSettings = true;
		}

		refreshKeybind(CustomKey::SKIP_PUZZLE);
	}

	if (!loadedSettings) {
		// Set defaults.
		setSetting(ClientToggleSetting::Jingles, false);
		setSetting(ClientToggleSetting::ChallengeTimer, false);

		setSetting(ClientToggleSetting::SyncProgress, false);
		setSetting(ClientToggleSetting::HighContrast, false);

		setSetting(ClientDropdownSetting::Collect, "Unchanged");
		setSetting(ClientDropdownSetting::DisabledPuzzles, "Prevent Solve");
		setSetting(ClientToggleSetting::ColorblindMode, false);

		InputWatchdog* input = InputWatchdog::get();
		input->loadCustomKeybind(CustomKey::SKIP_PUZZLE, InputButton::KEY_T);
		
		refreshKeybind(CustomKey::SKIP_PUZZLE);
	}

#if _DEBUG
	auto defaultAddress = "localhost:38281";
	auto defaultUser = "Witness";
#else
	auto defaultAddress = "archipelago.gg:";
	auto defaultUser = "";
#endif

	setSetting(ClientStringSetting::ApAddress, defaultAddress);
	setSetting(ClientStringSetting::ApSlotName, defaultUser);
	setSetting(ClientStringSetting::ApPassword, "");
}

void ClientWindow::showMessageBox(std::string message) const {
	MessageBox(hwndRootWindow, Converty::Utf8ToWide(message).c_str(), NULL, MB_OK);
}

bool ClientWindow::showDialogPrompt(std::string message) const {
	return MessageBox(hwndRootWindow, Converty::Utf8ToWide(message).c_str(), NULL, MB_YESNO) == IDYES;
}

bool ClientWindow::getSetting(ClientToggleSetting setting) const {
	int buttonId = toggleSettingButtonIds.find(setting)->second;
	return IsDlgButtonChecked(hwndRootWindow, buttonId);
}

void ClientWindow::setSetting(ClientToggleSetting setting, bool value) const {
	int buttonId = toggleSettingButtonIds.find(setting)->second;
	CheckDlgButton(hwndRootWindow, buttonId, value);
}

std::string ClientWindow::getSetting(ClientStringSetting setting) const {
	HWND textBox = stringSettingTextBoxes.find(setting)->second;
	return readStringFromTextBox(textBox);
}

void ClientWindow::setSetting(ClientDropdownSetting setting, std::string value) const {
	HWND dropDown = dropdownBoxes.find(setting)->second;
	std::wstring valueWide = Converty::Utf8ToWide(value);
	SendMessage(dropDown, CB_SELECTSTRING, 0, (LPARAM)valueWide.c_str());
}

std::string ClientWindow::getSetting(ClientDropdownSetting setting) const {
	HWND dropDown = dropdownBoxes.find(setting)->second;
	return readStringFromDropdown(dropDown);
}

void ClientWindow::setSetting(ClientStringSetting setting, std::string value) const {
	HWND textBox = stringSettingTextBoxes.find(setting)->second;
	writeStringToTextBox(value, textBox);
}

void ClientWindow::refreshKeybind(const CustomKey& customKey) const {
	InputWatchdog* input = InputWatchdog::get();

	HWND label = customKeybindValues.find(customKey)->second;
	writeStringToTextBox(input->getNameForInputButton(input->getCustomKeybind(customKey)), label);
}

void ClientWindow::setStatusMessage(std::string statusMessage) const
{
	writeStringToTextBox(statusMessage, hwndStatusText);
}

void ClientWindow::setErrorMessage(std::string errorMessage) const
{
	writeStringToTextBox(errorMessage, hwndErrorText);
}

void ClientWindow::setWindowMode(ClientWindowMode mode)
{
	currentWindowMode = mode;

	switch (mode) {
	case ClientWindowMode::Disabled: {
		// Disable everything.
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApAddress)->second, false);
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApSlotName)->second, false);
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApPassword)->second, false);
		EnableWindow(hwndApLoadCredentials, false);
		EnableWindow(hwndApConnect, false);

		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::ColorblindMode)->second, false);
		EnableWindow(dropdownBoxes.find(ClientDropdownSetting::Collect)->second, false);
		EnableWindow(dropdownBoxes.find(ClientDropdownSetting::DisabledPuzzles)->second, false);
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::SyncProgress)->second, false);
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::HighContrast)->second, false);

		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::ChallengeTimer)->second, false);
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::Jingles)->second, false);

		for (auto keybindButton : customKeybindButtons) {
			EnableWindow(keybindButton.second, false);
		}

		break;
	}
	case ClientWindowMode::PreConnect: {
		// Enable Archipelago credentials and actions.
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApAddress)->second, true);
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApSlotName)->second, true);
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApPassword)->second, true);
		EnableWindow(hwndApLoadCredentials, true);
		EnableWindow(hwndApConnect, true);

		// Enable randomization settings.

		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::ColorblindMode)->second, true);
		EnableWindow(dropdownBoxes.find(ClientDropdownSetting::Collect)->second, true);
		EnableWindow(dropdownBoxes.find(ClientDropdownSetting::DisabledPuzzles)->second, true);
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::SyncProgress)->second, true);
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::HighContrast)->second, true);

		// Disable runtime settings.
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::ChallengeTimer)->second, false);

		// Enable "any time" settings.
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::Jingles)->second, true);

		// Disable keybinds until connected.
		for (auto keybindButton : customKeybindButtons) {
			EnableWindow(keybindButton.second, false);
		}

		// Focus on the first AP setting field.
		HWND focusField = stringSettingTextBoxes.find(ClientStringSetting::ApAddress)->second;
		std::string focusFieldContents = readStringFromTextBox(focusField);
		SetFocus(focusField);
		SendMessage(focusField, EM_SETSEL, focusFieldContents.length(), focusFieldContents.length());

		break;
	}
	case ClientWindowMode::Randomized: {
		// Disable Archipelago credentials and actions.
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApAddress)->second, false);
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApSlotName)->second, false);
		EnableWindow(stringSettingTextBoxes.find(ClientStringSetting::ApPassword)->second, false);
		EnableWindow(hwndApLoadCredentials, false);
		EnableWindow(hwndApConnect, false);

		// Disable randomization settings.
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::ColorblindMode)->second, false);
		EnableWindow(dropdownBoxes.find(ClientDropdownSetting::Collect)->second, false);
		EnableWindow(dropdownBoxes.find(ClientDropdownSetting::DisabledPuzzles)->second, false);
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::SyncProgress)->second, false);
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::HighContrast)->second, false);

		// Enable runtime settings.
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::ChallengeTimer)->second, true);

		// Enable "any time" settings.
		EnableWindow(toggleSettingCheckboxes.find(ClientToggleSetting::Jingles)->second, true);

		// Enable keybinds.
		for (auto keybindButton : customKeybindButtons) {
			EnableWindow(keybindButton.second, true);
		}

		break;
	}
	}
}

void ClientWindow::focusGameWindow()
{
	// Find the game window by enumerating over all active windows.
	struct WindowData {
		DWORD processId = Memory::get()->getProcessID();
		HWND windowHandle = HWND();
		bool found = false;
	} windowData;

	auto enumWindowCallback = [](HWND handle, LPARAM param) -> BOOL {
		WindowData& outData = *(WindowData*)param;
		DWORD processId = 0;

		GetWindowThreadProcessId(handle, &processId);
		if (processId != outData.processId || !IsWindowVisible(handle)) {
			// Continue.
			return true;
		}

		// Found a visible window matching our process ID.
		outData.windowHandle = handle;
		outData.found = true;
		return false;
	};

	bool foundWindow = EnumWindows(enumWindowCallback, (LPARAM)&windowData);
	if (windowData.found) {
		SetForegroundWindow(windowData.windowHandle);

		// If the window has been minimized (such as when alt-tabbing out of fullscreen), show it.
		WINDOWPLACEMENT windowPlacement = WINDOWPLACEMENT();
		GetWindowPlacement(windowData.windowHandle, &windowPlacement);
		if (windowPlacement.showCmd == SW_SHOWMINIMIZED) {
			ShowWindow(windowData.windowHandle, SW_NORMAL);
		}
	}
}

void ClientWindow::focusClientWindow()
{
	SetForegroundWindow(hwndRootWindow);
}

void ClientWindow::buildWindow() {

	// Register a class for the root window.
	WNDCLASSW wndClass = {
		CS_HREDRAW | CS_VREDRAW,
		singletonHandleWndProc,
		0,
		DLGWINDOWEXTRA,
		hAppInstance,
		NULL,
		LoadCursor(nullptr, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW),
		CLIENT_MENU_CLASS_NAME,
		CLIENT_WINDOW_CLASS_NAME
	};
	RegisterClassW(&wndClass);

	// Create the root window.
	hwndRootWindow = CreateWindowEx(WS_EX_CONTROLPARENT, CLIENT_WINDOW_CLASS_NAME, PRODUCT_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_TABSTOP | WS_MINIMIZEBOX,
		650, 200, CLIENT_WINDOW_WIDTH, 100, nullptr, nullptr, hAppInstance, nullptr);

	// HACK: Due to things like menu bar thickness, the root window won't actually be the requested size. Grab the actual size here for later comparison.
	RECT actualRootWindowSize;
	GetClientRect(hwndRootWindow, &actualRootWindowSize);
	windowWidthAdjustment = CLIENT_WINDOW_WIDTH - actualRootWindowSize.right;
	windowHeightAdjustment = 100 - actualRootWindowSize.bottom;

	int currentY = 10;

	addVersionDisplay(currentY);
	addHorizontalRule(currentY);
	addArchipelagoCredentials(currentY);
	addHorizontalRule(currentY);
	addGameOptions(currentY);
	addHorizontalRule(currentY);
	addKeybindings(currentY);
	addHorizontalRule(currentY);
	addErrorMessage(currentY);

	// Resize the window to match its contents.
	MoveWindow(hwndRootWindow, 650, 200,
		CLIENT_WINDOW_WIDTH + windowWidthAdjustment, currentY + windowHeightAdjustment, true);
}

void ClientWindow::addHorizontalRule(int& currentY) {

	currentY += LINE_SPACING + 4;

	CreateWindow(L"STATIC", L"",
		WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,
		8, currentY,
		CLIENT_WINDOW_WIDTH - 16, 2,
		hwndRootWindow, 0, hAppInstance, NULL);

	currentY += 2 + LINE_SPACING + 4;
}

void ClientWindow::addVersionDisplay(int& currentY) {
	const int halfWidth = CLIENT_WINDOW_WIDTH / 2 - STATIC_TEXT_MARGIN;

	// AP version name. Left-justified.
	CreateWindow(L"STATIC", L"Archipelago Version: " AP_VERSION_STR,
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY,
		halfWidth, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);

	// Client version display. Right-justified.
	CreateWindow(L"STATIC", L"Client Version: " VERSION_STR,
		WS_VISIBLE | WS_CHILD | SS_RIGHT,
		CLIENT_WINDOW_WIDTH - halfWidth - STATIC_TEXT_MARGIN, currentY,
		halfWidth, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);

	currentY += STATIC_TEXT_HEIGHT;
}

void ClientWindow::addArchipelagoCredentials(int& currentY) {
	// Server address.
	CreateWindow(L"STATIC", L"Enter an AP address:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY + SETTING_LABEL_Y_OFFSET,
		SETTING_LABEL_WIDTH, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);
	HWND hwndApAddress = CreateWindow(MSFTEDIT_CLASS, L"",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		SETTING_CONTROL_LEFT, currentY,
		SETTING_CONTROL_WIDTH, CONTROL_HEIGHT,
		hwndRootWindow, (HMENU)IDC_FIELD_AP_ADDRESS, hAppInstance, NULL);
	stringSettingTextBoxes[ClientStringSetting::ApAddress] = hwndApAddress;
	SendMessage(hwndApAddress, EM_SETEVENTMASK, NULL, ENM_CHANGE); // Notify on text change

	currentY += CONTROL_HEIGHT + LINE_SPACING;

	// Slot name.
	CreateWindow(L"STATIC", L"Enter slot name:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY + SETTING_LABEL_Y_OFFSET,
		SETTING_LABEL_WIDTH, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);
	HWND hwndApSlotName = CreateWindow(MSFTEDIT_CLASS, L"",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		SETTING_CONTROL_LEFT, currentY,
		SETTING_CONTROL_WIDTH, CONTROL_HEIGHT,
		hwndRootWindow, (HMENU)IDC_FIELD_AP_SLOTNAME, hAppInstance, NULL);
	stringSettingTextBoxes[ClientStringSetting::ApSlotName] = hwndApSlotName;
	SendMessage(hwndApSlotName, EM_SETEVENTMASK, NULL, ENM_CHANGE); // Notify on text change

	currentY += CONTROL_HEIGHT + LINE_SPACING;

	// Password.
	CreateWindow(L"STATIC", L"Enter password:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY + SETTING_LABEL_Y_OFFSET,
		SETTING_LABEL_WIDTH, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);
	HWND hwndApPassword = CreateWindow(MSFTEDIT_CLASS, L"",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		SETTING_CONTROL_LEFT, currentY,
		SETTING_CONTROL_WIDTH, CONTROL_HEIGHT,
		hwndRootWindow, (HMENU)IDC_FIELD_AP_PASSWORD, hAppInstance, NULL);
	stringSettingTextBoxes[ClientStringSetting::ApPassword] = hwndApPassword;
	SendMessage(hwndApPassword, EM_SETEVENTMASK, NULL, ENM_CHANGE); // Notify on text change

	currentY += CONTROL_HEIGHT + LINE_SPACING;

	// Load credentials button.
	const int apCredentialsButtonWidth = 150;
	hwndApLoadCredentials = CreateWindow(L"BUTTON", L"Restore Credentials",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		CONTROL_MARGIN, currentY,
		SETTING_LABEL_WIDTH - 6, CONTROL_HEIGHT,
		hwndRootWindow, (HMENU)IDC_BUTTON_LOADCREDENTIALS, hAppInstance, NULL);

	// Connect button. In line with the load credentials button.
	hwndApConnect = CreateWindow(L"BUTTON", L"Connect",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		SETTING_CONTROL_LEFT, currentY,
		SETTING_CONTROL_WIDTH, CONTROL_HEIGHT,
		hwndRootWindow, (HMENU)IDC_BUTTON_RANDOMIZE, hAppInstance, NULL);

	currentY += CONTROL_HEIGHT + LINE_SPACING;

	// Current randomization status.
	hwndStatusText = CreateWindow(L"STATIC", L"Not connected.",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		SETTING_CONTROL_LEFT + 2, currentY + 2,
		SETTING_CONTROL_WIDTH - 2, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);

	currentY += STATIC_TEXT_HEIGHT + 2;
}

void ClientWindow::addGameOptions(int& currentY) {
	// Musical jingles option.
	HWND hwndOptionMusic = CreateWindow(L"BUTTON", L"Jingles - Add musical sound effects for completing location checks.",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_MULTILINE,
		CONTROL_MARGIN, currentY,
		CLIENT_WINDOW_WIDTH - STATIC_TEXT_MARGIN, STATIC_TEXT_HEIGHT,
		hwndRootWindow, (HMENU)IDC_SETTING_JINGLES, hAppInstance, NULL);
	toggleSettingButtonIds[ClientToggleSetting::Jingles] = IDC_SETTING_JINGLES;
	toggleSettingCheckboxes[ClientToggleSetting::Jingles] = hwndOptionMusic;

	currentY += STATIC_TEXT_HEIGHT + LINE_SPACING;

	// Colorblind option. This is 3 lines tall.
	HWND hwndOptionColorblind = CreateWindow(L"BUTTON", L"Colorblind Mode - The colors on certain panels will be changed to be more accommodating to people with colorblindness. The puzzles themselves are identical to those generated without colorblind mode enabled.",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_MULTILINE,
		CONTROL_MARGIN, currentY,
		CLIENT_WINDOW_WIDTH - STATIC_TEXT_MARGIN, STATIC_TEXT_HEIGHT * 3,
		hwndRootWindow, (HMENU)IDC_SETTING_COLORBLIND, hAppInstance, NULL);
	toggleSettingButtonIds[ClientToggleSetting::ColorblindMode] = IDC_SETTING_COLORBLIND;
	toggleSettingCheckboxes[ClientToggleSetting::ColorblindMode] = hwndOptionColorblind;

	currentY += STATIC_TEXT_HEIGHT * 3 + LINE_SPACING;

	// Challenge timer option.
	HWND hwndOptionHighContrast = CreateWindow(L"BUTTON", L"High Contrast Mode",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_MULTILINE,
		CONTROL_MARGIN, currentY,
		CLIENT_WINDOW_WIDTH - STATIC_TEXT_MARGIN, STATIC_TEXT_HEIGHT,
		hwndRootWindow, (HMENU)IDC_SETTING_HIGHCONTRAST, hAppInstance, NULL);
	toggleSettingButtonIds[ClientToggleSetting::HighContrast] = IDC_SETTING_HIGHCONTRAST;
	toggleSettingCheckboxes[ClientToggleSetting::HighContrast] = hwndOptionHighContrast;

	currentY += STATIC_TEXT_HEIGHT + LINE_SPACING;

	// Challenge timer option.
	HWND hwndOptionSyncProgress = CreateWindow(L"BUTTON", L"Coop: Sync Lasers and EPs",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_MULTILINE,
		CONTROL_MARGIN, currentY,
		CLIENT_WINDOW_WIDTH - STATIC_TEXT_MARGIN, STATIC_TEXT_HEIGHT,
		hwndRootWindow, (HMENU)IDC_SETTING_SYNCPROGRESS, hAppInstance, NULL);
	toggleSettingButtonIds[ClientToggleSetting::SyncProgress] = IDC_SETTING_SYNCPROGRESS;
	toggleSettingCheckboxes[ClientToggleSetting::SyncProgress] = hwndOptionSyncProgress;

	currentY += STATIC_TEXT_HEIGHT + LINE_SPACING;

	// Challenge timer option.
	HWND hwndOptionChallenge = CreateWindow(L"BUTTON", L"Disable Challenge Timer - Disables the time limit on the Challenge.",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_MULTILINE,
		CONTROL_MARGIN, currentY,
		CLIENT_WINDOW_WIDTH - STATIC_TEXT_MARGIN, STATIC_TEXT_HEIGHT,
		hwndRootWindow, (HMENU)IDC_SETTING_CHALLENGE, hAppInstance, NULL);
	toggleSettingButtonIds[ClientToggleSetting::ChallengeTimer] = IDC_SETTING_CHALLENGE;
	toggleSettingCheckboxes[ClientToggleSetting::ChallengeTimer] = hwndOptionChallenge;

	currentY += STATIC_TEXT_HEIGHT + LINE_SPACING;

    // Option for collected panels
	HWND hwndOptionCollectText = CreateWindow(L"STATIC", L"Excluded/Collected Puzzles:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY + SETTING_LABEL_Y_OFFSET,
		SETTING_LABEL_WIDTH * 1.25, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);

	HWND hwndOptionCollect = CreateWindow(L"COMBOBOX", NULL,
		CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD,
		CONTROL_MARGIN + SETTING_LABEL_WIDTH * 1.25, currentY,
		SETTING_LABEL_WIDTH, STATIC_TEXT_HEIGHT * 6,
		hwndRootWindow, (HMENU)IDC_SETTING_COLLECT, hAppInstance, NULL);
	dropdownBoxes[ClientDropdownSetting::Collect] = hwndOptionCollect;
	dropdownIds[ClientDropdownSetting::Collect] = IDC_SETTING_COLLECT;

	SendMessage(hwndOptionCollect, CB_ADDSTRING, 0, (LPARAM)L"Unchanged");
	SendMessage(hwndOptionCollect, CB_ADDSTRING, 0, (LPARAM)L"Free Skip");
	SendMessage(hwndOptionCollect, CB_ADDSTRING, 0, (LPARAM)L"Auto-Skip");
	SendMessage(hwndOptionCollect, CB_SELECTSTRING, 0, (LPARAM)L"Unchanged");

	currentY += STATIC_TEXT_HEIGHT + LINE_SPACING * 2;

	// Option for disabled panels
	HWND hwndOptionDisabledText = CreateWindow(L"STATIC", L"Disabled Panels:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY + SETTING_LABEL_Y_OFFSET,
		SETTING_LABEL_WIDTH * 1.25, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);

	HWND hwndOptionDisabled = CreateWindow(L"COMBOBOX", NULL,
		CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD,
		CONTROL_MARGIN + SETTING_LABEL_WIDTH * 1.25, currentY,
		SETTING_LABEL_WIDTH, STATIC_TEXT_HEIGHT * 6,
		hwndRootWindow, (HMENU)IDC_SETTING_DISABLED, hAppInstance, NULL);
	dropdownBoxes[ClientDropdownSetting::DisabledPuzzles] = hwndOptionDisabled;
	dropdownIds[ClientDropdownSetting::DisabledPuzzles] = IDC_SETTING_DISABLED;

	SendMessage(hwndOptionDisabled, CB_ADDSTRING, 0, (LPARAM)L"Unchanged");
	SendMessage(hwndOptionDisabled, CB_ADDSTRING, 0, (LPARAM)L"Free Skip");
	SendMessage(hwndOptionDisabled, CB_ADDSTRING, 0, (LPARAM)L"Auto-Skip");
	SendMessage(hwndOptionDisabled, CB_ADDSTRING, 0, (LPARAM)L"Prevent Solve");
	SendMessage(hwndOptionDisabled, CB_SELECTSTRING, 0, (LPARAM)L"Unchanged");

	currentY += STATIC_TEXT_HEIGHT * 2;
}

void ClientWindow::addKeybindings(int& currentY)
{
	int customKeyCount = static_cast<int>(CustomKey::COUNT);
	if (customKeyCount < 1) return;

	CreateWindow(L"STATIC", L"Custom keybindings:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY + SETTING_LABEL_Y_OFFSET,
		SETTING_LABEL_WIDTH, STATIC_TEXT_HEIGHT,
		hwndRootWindow, NULL, hAppInstance, NULL);
	currentY += STATIC_TEXT_HEIGHT;

	InputWatchdog* input = InputWatchdog::get();

	for (int i = 0; i < customKeyCount; i++) {
		CustomKey currentKey = static_cast<CustomKey>(i);
		currentY += LINE_SPACING;

		HWND hwndLabel = CreateWindow(L"STATIC", L"",
			WS_VISIBLE | WS_CHILD | SS_RIGHT,
			STATIC_TEXT_MARGIN, currentY + SETTING_LABEL_Y_OFFSET,
			SETTING_LABEL_WIDTH - STATIC_TEXT_MARGIN, STATIC_TEXT_HEIGHT,
			hwndRootWindow, NULL, hAppInstance, NULL);
		writeStringToTextBox(input->getNameForCustomKey(currentKey) + ":", hwndLabel);

		HWND hwndValue = CreateWindow(L"STATIC", L"",
			WS_VISIBLE | WS_CHILD | SS_LEFT,
			KEYBIND_LABEL_LEFT, currentY + SETTING_LABEL_Y_OFFSET,
			KEYBIND_LABEL_WIDTH, STATIC_TEXT_HEIGHT,
			hwndRootWindow, NULL, hAppInstance, NULL);
		customKeybindValues[currentKey] = hwndValue;

		HWND hwndButton = CreateWindow(L"BUTTON", L"Bind",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			KEYBIND_BUTTON_LEFT, currentY,
			KEYBIND_BUTTON_WIDTH, CONTROL_HEIGHT,
			hwndRootWindow, (HMENU)(IDC_BUTTON_KEYBIND + i), hAppInstance, NULL);
		customKeybindButtons[currentKey] = hwndButton;

		currentY += CONTROL_HEIGHT;
	}
}

void ClientWindow::addPuzzleEditor(int& currentY) {
	// TODO
}

void ClientWindow::addErrorMessage(int& currentY) {

	// Most recent error.
	hwndErrorText = CreateWindow(L"STATIC", L"Most Recent Error:",
		WS_VISIBLE | WS_CHILD | SS_LEFT,
		STATIC_TEXT_MARGIN, currentY,
		CLIENT_WINDOW_WIDTH - STATIC_TEXT_MARGIN, STATIC_TEXT_HEIGHT * 3,
		hwndRootWindow, NULL, hAppInstance, NULL);

	currentY += STATIC_TEXT_HEIGHT * 3;
}

void ClientWindow::show(int nCmdShow) {
	focusAndHighlight(stringSettingTextBoxes.find(ClientStringSetting::ApAddress)->second);
	ShowWindow(hwndRootWindow, nCmdShow);
	UpdateWindow(hwndRootWindow);
}

void ClientWindow::focusAndHighlight(HWND hwndTextBox) {
	SetFocus(hwndTextBox);
	int len = GetWindowTextLength(hwndTextBox);
	SendMessage(hwndTextBox, EM_SETSEL, MAKELONG(len, len), MAKELONG(len, len));
}

void ClientWindow::toggleCheckbox(int buttonId) {
	bool isChecked = IsDlgButtonChecked(hwndRootWindow, buttonId);
	CheckDlgButton(hwndRootWindow, buttonId, !isChecked);
}

LRESULT CALLBACK ClientWindow::handleWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_CLOSE) {
		// The close button was pressed. Verify that the user wants to quit.
		if (MessageBox(hwnd, L"Warning: Some puzzles may be unsolvable if you close the randomizer while the game is running.\r\n\r\nExit anyway?", L"", MB_OKCANCEL) == IDOK)
			DestroyWindow(hwnd);
		return 0;
	}
	else if (message == WM_DESTROY) {
		// The window was destroyed. Inform the OS that we want to terminate.
		PostQuitMessage(0);
	}
	else if (message == WM_COMMAND) {
		if (currentWindowMode == ClientWindowMode::PreConnect && wParam == IDOK) {
			HWND currentFocus = GetFocus();
			if (currentFocus == stringSettingTextBoxes.find(ClientStringSetting::ApAddress)->second ||
				currentFocus == stringSettingTextBoxes.find(ClientStringSetting::ApSlotName)->second ||
				currentFocus == stringSettingTextBoxes.find(ClientStringSetting::ApPassword)->second) {
				// The user has pressed Enter while focused on an AP text box. Connect.
				Main::randomize();
				return 0;
			}
		}

		switch (LOWORD(wParam)) {
		// Setting checkboxes.
		case IDC_SETTING_COLORBLIND: {
			toggleCheckbox(IDC_SETTING_COLORBLIND);
			break;
		}
		case IDC_SETTING_CHALLENGE: {
			toggleCheckbox(IDC_SETTING_CHALLENGE);
			break;
		}
		case IDC_SETTING_SYNCPROGRESS: {
			toggleCheckbox(IDC_SETTING_SYNCPROGRESS);
			break;
		}
		case IDC_SETTING_HIGHCONTRAST: {
			toggleCheckbox(IDC_SETTING_HIGHCONTRAST);
			break;
		}
		case IDC_SETTING_JINGLES: {
			toggleCheckbox(IDC_SETTING_JINGLES);
			break;
		}
		// Buttons.
		case IDC_BUTTON_RANDOMIZE: {
			Main::randomize();
			break;
		}
		case IDC_BUTTON_LOADCREDENTIALS: {
			Main::loadCredentials();
			break;
		}
// Keybindings.
		case IDC_BUTTON_KEYBIND + static_cast<int>(CustomKey::SKIP_PUZZLE) : {
			CustomKey key = static_cast<CustomKey>(LOWORD(wParam) - IDC_BUTTON_KEYBIND);
			handleKeybind(key);
			break;
		}
		default:
			return DefDlgProc(hwnd, message, wParam, lParam);
		}
	}
	else {
		return DefDlgProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

std::string ClientWindow::readStringFromTextBox(HWND hwndTextBox) const {

	WCHAR wideBuffer[0x100];

	GetWindowText(hwndTextBox, &wideBuffer[0], 100);
	std::wstring wideString(wideBuffer);
	return Converty::WideToUtf8(wideString);
}

void ClientWindow::writeStringToTextBox(std::string string, HWND hwndTextBox) const {
	SetWindowText(hwndTextBox, Converty::Utf8ToWide(string).c_str());
}

std::string ClientWindow::readStringFromDropdown(HWND dropdownBox) const {
	WCHAR wideBuffer[0x100];

	GetWindowText(dropdownBox, &wideBuffer[0], 100);
	std::wstring wideString(wideBuffer);

	return Converty::WideToUtf8(wideString);
}

void ClientWindow::handleKeybind(const CustomKey& customKey)
{
	InputWatchdog::get()->beginCustomKeybind(customKey);
}
