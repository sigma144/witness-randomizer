// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com


#include "windows.h"

#include <Richedit.h>

#include "Main.h"

#include <string>
#include <sstream>
#include <iostream>

#include "Version.h"
#include "Randomizer.h"
#include "Panel.h"
#include "Generate.h"
#include "Special.h"
#include "PuzzleList.h"
#include "Watchdog.h"
#include "Random.h"
#include "Input.h"
#include "ClientWindow.h"

#include "Converty.h"
#include "Archipelago/APRandomizer.h"
#include <Archipelago/APGameData.h>
#include <Archipelago/ASMPayloadManager.h>


#define IDC_RANDOMIZE 0x401
#define IDC_TOGGLESPEED 0x402
#define IDC_SPEEDRUNNER 0x403
#define IDC_HARDMODE 0x404
#define IDC_READ 0x405
#define IDC_TEST 0x406
#define IDC_WRITE 0x407
#define IDC_DUMP 0x408
#define IDT_RANDOMIZED 0x409
#define IDC_TOGGLELASERS 0x410
#define IDC_TOGGLESNIPES 0x411
#define IDC_LOADCREDENTIALS 0x422
#define IDC_TAB 0x450
#define IDC_RETURN 0x451

#define IDC_ADD 0x301
#define IDC_REMOVE 0x302
#define IDC_ROTATED 0x303
#define IDC_NEGATIVE 0x304
#define IDC_SYMMETRYX 0x305
#define IDC_SYMMETRYY 0x306

#define IDC_DIFFICULTY_NORMAL 0x501
#define IDC_DIFFICULTY_EXPERT 0x502
#define IDC_COLORBLIND 0x503
#define IDC_DOUBLE 0x504
#define IDC_COLLECT 0x550
#define IDC_CHALLENGE 0x560

#define IDC_KEYBIND_PUZZLESKIP 0x570

#define SHAPE_11 0x1000
#define SHAPE_12 0x2000
#define SHAPE_13 0x4000
#define SHAPE_14 0x8000
#define SHAPE_21 0x0100
#define SHAPE_22 0x0200
#define SHAPE_23 0x0400
#define SHAPE_24 0x0800
#define SHAPE_31 0x0010
#define SHAPE_32 0x0020
#define SHAPE_33 0x0040
#define SHAPE_34 0x0080
#define SHAPE_41 0x0001
#define SHAPE_42 0x0002
#define SHAPE_43 0x0004
#define SHAPE_44 0x0008

#define ARROW_DOWN 0x700
#define ARROW_UP 0x701
#define ARROW_RIGHT 0x702
#define ARROW_LEFT 0x703
#define ARROW_DOWN_RIGHT 0x704
#define ARROW_UP_RIGHT 0x705
#define ARROW_UP_LEFT 0x706
#define ARROW_DOWN_LEFT 0x707

#define DEBUG false

//Panel to edit
int panel = 0x09E69;

//HWND hwndCol, hwndRow, hwndElem, hwndColor;						// Debug UI

std::shared_ptr<Panel> _panel;
std::shared_ptr<Randomizer> randomizer = std::make_shared<Randomizer>();
std::shared_ptr<APRandomizer> apRandomizer = std::make_shared<APRandomizer>();
std::shared_ptr<Generate> generator = std::make_shared<Generate>();
std::shared_ptr<Special> specialCase = std::make_shared<Special>(generator);
std::vector<byte> bytes;

// Panels for storing settings strings to the save file.
std::vector<int> addressPanels = { 0x09FA0, 0x09F86, 0x0C339, 0x09FAA, 0x0A249, 0x1C2DF, 0x1831E, 0x1C260, 0x1831C, 0x1C2F3, 0x1831D, 0x1C2B1, 0x1831B, 0x0A015 };
std::vector<int> slotNamePanels = { 0x17CC4, 0x275ED, 0x03678, 0x03679, 0x03675, 0x03676, 0x17CAC, 0x03852, 0x03858, 0x38663, 0x275FA, 0x334DB, 0x334DC, 0x09E49 };
std::vector<int> passwordPanels = { 0x0361B, 0x334D8, 0x2896A, 0x17D02, 0x03713, 0x00B10, 0x00C92, 0x09D9B, 0x17CAB, 0x337FA, 0x0A099, 0x34BC5, 0x34BC6, 0x17CBC };

int ctr = 0;
TCHAR text[30];
int x, y;
std::wstring debugwstr;
Decoration::Shape symbol;
Decoration::Color color;
int currentShape;
int currentDir;
int lastSeed;
int lastPuzzleRandomisation;
std::vector<long long> shapePos = { SHAPE_11, SHAPE_12, SHAPE_13, SHAPE_14, SHAPE_21, SHAPE_22, SHAPE_23, SHAPE_24, SHAPE_31, SHAPE_32, SHAPE_33, SHAPE_34, SHAPE_41, SHAPE_42, SHAPE_43, SHAPE_44 };
std::vector<long long> defaultShape = { SHAPE_21, SHAPE_31, SHAPE_32, SHAPE_33 }; //L-shape
std::vector<long long> directions = { ARROW_UP_RIGHT, ARROW_UP, ARROW_UP_LEFT, ARROW_LEFT, 0, ARROW_RIGHT, ARROW_DOWN_LEFT, ARROW_DOWN, ARROW_DOWN_RIGHT }; //Order of directional check boxes
float target;

void focusEdit(HWND hwnd) {
	SetFocus(hwnd);
	int len = GetWindowTextLength(hwnd);
	SendMessage(hwnd, EM_SETSEL, MAKELONG(len, len), MAKELONG(len, len));

}

//LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//	static bool seedIsRNG = false;
//



//		switch (LOWORD(wParam)) {
//
//		case IDC_TAB:
//			//if(GetFocus() == hwndAddress){
//			//	focusEdit(hwndUser);
//			//}
//			//else if (GetFocus() == hwndUser) {
//			//	focusEdit(hwndPassword);
//			//}
//			//else {
//			//	focusEdit(hwndAddress);
//			//}
//			break;
//
//			//Test button  - general testing (debug mode only)
//
//		case IDC_TEST:
//			generator->resetConfig();
//			generator->seed(static_cast<unsigned int>(time(NULL)));
//			specialCase->test();
//			break;
//
//		//case IDC_RETURN:
//			//if (GetFocus() != hwndUser && GetFocus() != hwndPassword) {
//			//	break;
//			//}

void Main::randomize() {
	ClientWindow* clientWindow = ClientWindow::get();
	clientWindow->setWindowMode(ClientWindowMode::Disabled);

	clientWindow->logLine("Attempting randomization.");

	Memory* memory = Memory::get();

	bool rerandomize = false;
	if (memory->ReadPanelData<int>(0x00064, NUM_DOTS) > 5) {
		if (clientWindow->showDialogPrompt("Game is currently randomized. Are you sure you want to randomize again? (Can cause glitches)") == true) {
			rerandomize = true;
		}
		else {
			clientWindow->setWindowMode(ClientWindowMode::PreConnect);
			return;
		}
	}

	clientWindow->logLine("Rerandomize: " + std::to_string(rerandomize));

	// Retrieve AP credentials from the main window.
	std::string apAddress = clientWindow->getSetting(ClientStringSetting::ApAddress);
	std::string apSlotName = clientWindow->getSetting(ClientStringSetting::ApSlotName);
	std::string apPassword = clientWindow->getSetting(ClientStringSetting::ApPassword);

	clientWindow->logLine("Credentials: " + apAddress + " | " + apSlotName + " | " + apPassword);

	randomizer->seedIsRNG = false;
	apRandomizer->SyncProgress = clientWindow->getSetting(ClientToggleSetting::SyncProgress);
	apRandomizer->CollectedPuzzlesBehavior = clientWindow->getSetting(ClientDropdownSetting::Collect);
	apRandomizer->DisabledPuzzlesBehavior = clientWindow->getSetting(ClientDropdownSetting::DisabledPuzzles);
	apRandomizer->solveModeSpeedFactor = 0.0f; // THIS VALUE IN THE FUTURE CAN BE USED TO MAKE SPEED BOOSTS TICK DOWN AT A SLOW RATE IN SOLVE MODE RATHER THAN STOP OR GO AT FULL SPEED

	clientWindow->setStatusMessage("Connecting to Archipelago...");
	if (!apRandomizer->Connect(apAddress, apSlotName, apPassword)) {
		clientWindow->setStatusMessage("");
		clientWindow->setWindowMode(ClientWindowMode::PreConnect);
		return;
	}

	int seed = apRandomizer->Seed;
	int puzzleRando = apRandomizer->PuzzleRandomization;

	memory->ClearOffsets();
	randomizer->AdjustSpeed(); //Makes certain moving objects move faster

	//If the save was previously randomized, check that seed and difficulty match with the save file
	int lastSeed = memory->ReadPanelData<int>(0x00064, VIDEO_STATUS_COLOR + 8);
	if (lastSeed != 1041865114 && !rerandomize && !DEBUG) {
		if (seed != lastSeed) {
			if (clientWindow->showDialogPrompt("This save file was previously randomized with a different seed, are you sure you want to randomize it with a new seed?") == false) {
				return;
			}

			memory->WritePanelData<float>(0x0064, VIDEO_STATUS_COLOR + 8, { 0.0f });
		}
		lastPuzzleRandomisation = memory->ReadPanelData<int>(0x00182, VIDEO_STATUS_COLOR + 8);
	}

	//If the save hasn't been randomized before, make sure it is a fresh, unplayed save file
	else if (Special::hasBeenPlayed() && !rerandomize && !DEBUG) {
		if (clientWindow->showDialogPrompt("Warning: It is recommended that you start a new game for the randomizer, to prevent corruption of your save file. Randomize on the current save file anyway?") == true) {
			randomizer->seedIsRNG = false;
		}
		else return;
	}

	clientWindow->logLine("Last seed: " + std::to_string(lastSeed));

	if (lastSeed == 0) {
		memory->WritePanelData<float>(0x0064, VIDEO_STATUS_COLOR + 8, { 0.0f });
	}

	clientWindow->logLine("Store AP credentials to in-game data.");
	Special::writeStringToPanels(apAddress, addressPanels);
	Special::writeStringToPanels(apSlotName, slotNamePanels);
	Special::writeStringToPanels(apPassword, passwordPanels);

	clientWindow->logLine("Store settings.");
	clientWindow->saveSettings();

	clientWindow->logLine("Configure base randomizer.");
	randomizer->seed = seed;
	randomizer->colorblind = clientWindow->getSetting(ClientToggleSetting::ColorblindMode);
	randomizer->doubleMode = false;

	clientWindow->logLine("Create ASMPayloadManager.");
	ASMPayloadManager::create();

	clientWindow->setStatusMessage("Initialising puzzles...");
	apRandomizer->InitPanels();

	if (clientWindow->getSetting(ClientToggleSetting::PanelEffects)) {
		clientWindow->logLine("Disabling color cycle effects.");
		apRandomizer->DisableColorCycle();
	}

	clientWindow->setStatusMessage("Restoring vanilla puzzles...");
	apRandomizer->RestoreOriginals();

	clientWindow->setStatusMessage("Randomizing puzzles...");
	if (puzzleRando == SIGMA_EXPERT)
		randomizer->GenerateHard();
	else if (puzzleRando == SIGMA_NORMAL)
		randomizer->GenerateNormal();
	else
		Random::seed(seed);

	// Run the AP randomizer.
	clientWindow->setStatusMessage("Setting up AP features...");
	apRandomizer->PreGeneration();

	clientWindow->logLine("Generating random puzzles.");
	if (puzzleRando == SIGMA_EXPERT)
		apRandomizer->GenerateHard();
	else if (puzzleRando == SIGMA_NORMAL || puzzleRando == NO_PUZZLE_RANDO)
		apRandomizer->GenerateNormal();

	memory->WritePanelData(0x00064, VIDEO_STATUS_COLOR + 8, seed);
	memory->WritePanelData(0x00182, VIDEO_STATUS_COLOR + 8, puzzleRando);

	if (clientWindow->getSetting(ClientToggleSetting::HighContrast)) {
		clientWindow->logLine("Setting up High Contrast Mode.");
		apRandomizer->HighContrastMode();
	}

	clientWindow->logLine("Start PostGeneration.");
	apRandomizer->PostGeneration();

	clientWindow->logLine("Start InputWatchdog.");
	InputWatchdog::get()->start();

	clientWindow->setStatusMessage("Connected.");
	clientWindow->setWindowMode(ClientWindowMode::Randomized);
}

void Main::loadCredentials() {
	ClientWindow* clientWindow = ClientWindow::get();

	std::string apAddress = Special::readStringFromPanels(addressPanels);
	if (apAddress.length() == 0) {
		clientWindow->showMessageBox("This save has not previously been randomized with Archipelago and has no stored credentials.");
		return;
	}

	clientWindow->setSetting(ClientStringSetting::ApAddress, apAddress);
	clientWindow->setSetting(ClientStringSetting::ApSlotName, Special::readStringFromPanels(slotNamePanels));
	clientWindow->setSetting(ClientStringSetting::ApPassword, Special::readStringFromPanels(passwordPanels));
}

//		//Add a symbol to the puzzle (debug mode only)
//		case IDC_ADD:
//			memset(&text, 0, sizeof(text));
//			GetWindowText(hwndElem, text, 30);
//			if (wcscmp(text, L"Stone") == 0) symbol = Decoration::Shape::Stone;
//			if (wcscmp(text, L"Star") == 0) symbol = Decoration::Shape::Star;
//			if (wcscmp(text, L"Eraser") == 0) symbol = Decoration::Shape::Eraser;
//			if (wcscmp(text, L"Shape") == 0) symbol = Decoration::Shape::Poly;
//			if (wcscmp(text, L"Triangle 1") == 0) symbol = Decoration::Shape::Triangle1;
//			if (wcscmp(text, L"Triangle 2") == 0) symbol = Decoration::Shape::Triangle2;
//			if (wcscmp(text, L"Triangle 3") == 0) symbol = Decoration::Shape::Triangle3;
//			if (wcscmp(text, L"Arrow 1") == 0) symbol = Decoration::Shape::Arrow1;
//			if (wcscmp(text, L"Arrow 2") == 0) symbol = Decoration::Shape::Arrow2;
//			if (wcscmp(text, L"Arrow 3") == 0) symbol = Decoration::Shape::Arrow3;
//			if (wcscmp(text, L"Dot (Intersection)") == 0) symbol = Decoration::Shape::Dot_Intersection;
//			if (wcscmp(text, L"Dot (Row)") == 0) symbol = Decoration::Shape::Dot_Row;
//			if (wcscmp(text, L"Dot (Column)") == 0) symbol = Decoration::Shape::Dot_Column;
//			if (wcscmp(text, L"Gap (Row)") == 0) symbol = Decoration::Shape::Gap_Row;
//			if (wcscmp(text, L"Gap (Column)") == 0) symbol = Decoration::Shape::Gap_Column;
//			if (wcscmp(text, L"Start") == 0) symbol = Decoration::Shape::Start;
//			if (wcscmp(text, L"Exit") == 0) symbol = Decoration::Shape::Exit;
//			memset(&text, 0, sizeof(text));
//			GetWindowText(hwndColor, text, 30);
//			if (wcscmp(text, L"Black") == 0) color = Decoration::Color::Black;
//			if (wcscmp(text, L"White") == 0) color = Decoration::Color::White;
//			if (wcscmp(text, L"Red") == 0) color = Decoration::Color::Red;
//			if (wcscmp(text, L"Orange") == 0) color = Decoration::Color::Orange;
//			if (wcscmp(text, L"Yellow") == 0) color = Decoration::Color::Yellow;
//			if (wcscmp(text, L"Green") == 0) color = Decoration::Color::Green;
//			if (wcscmp(text, L"Cyan") == 0) color = Decoration::Color::Cyan;
//			if (wcscmp(text, L"Blue") == 0) color = Decoration::Color::Blue;
//			if (wcscmp(text, L"Purple") == 0) color = Decoration::Color::Purple;
//			if (wcscmp(text, L"Magenta") == 0) color = Decoration::Color::Magenta;
//			if (wcscmp(text, L"None") == 0) color = Decoration::Color::None;
//			if (wcscmp(text, L"???") == 0) color = Decoration::Color::X;
//			debugwstr.reserve(30);
//			GetWindowText(hwndCol, &debugwstr[0], 30);
//			x = _wtoi(debugwstr.c_str());
//			GetWindowText(hwndRow, &debugwstr[0], 30);
//			y = _wtoi(debugwstr.c_str());
//
//			_panel->Read(panel);
//			if (symbol == Decoration::Shape::Poly)
//				_panel->SetShape(x, y, currentShape, IsDlgButtonChecked(hwnd, IDC_ROTATED), IsDlgButtonChecked(hwnd, IDC_NEGATIVE), color);
//			else if (symbol == Decoration::Shape::Arrow1 || symbol == Decoration::Shape::Arrow2 || symbol == Decoration::Shape::Arrow3)
//				_panel->SetShape(x, y, symbol | currentDir, IsDlgButtonChecked(hwnd, IDC_ROTATED), IsDlgButtonChecked(hwnd, IDC_NEGATIVE), color);
//			else _panel->SetSymbol(x, y, symbol, color);
//			_panel->Write(panel);
//			break;
//
//		//Remove a symbol from the puzzle (debug mode only)
//		case IDC_REMOVE:
//			GetWindowText(hwndCol, &debugwstr[0], 30);
//			x = _wtoi(debugwstr.c_str());
//			GetWindowText(hwndRow, &debugwstr[0], 30);
//			y = _wtoi(debugwstr.c_str());
//			GetWindowText(hwndElem, text, 30);
//
//			_panel->Read(panel);
//			if (wcscmp(text, L"Dot (Intersection)") == 0 ||
//				wcscmp(text, L"Start") == 0 || wcscmp(text, L"Exit") == 0)
//				_panel->ClearGridSymbol(x * 2, y * 2);
//			else if (wcscmp(text, L"Gap (Column)") == 0 ||
//				wcscmp(text, L"Dot (Column)") == 0)
//				_panel->ClearGridSymbol(x * 2, y * 2 + 1);
//			else if (wcscmp(text, L"Gap (Row)") == 0 ||
//				wcscmp(text, L"Dot (Row)") == 0)
//				_panel->ClearGridSymbol(x * 2 + 1, y * 2);
//			else
//				_panel->ClearSymbol(x, y);
//			_panel->Write(panel);
//			break;
//
//		//Debug mode checkboxes
//		case IDC_ROTATED:
//			CheckDlgButton(hwnd, IDC_ROTATED, !IsDlgButtonChecked(hwnd, IDC_ROTATED));
//			break;
//
//		case IDC_NEGATIVE:
//			CheckDlgButton(hwnd, IDC_NEGATIVE, !IsDlgButtonChecked(hwnd, IDC_NEGATIVE));
//			break;
//
//		case IDC_SYMMETRYX:
//			CheckDlgButton(hwnd, IDC_SYMMETRYX, !IsDlgButtonChecked(hwnd, IDC_SYMMETRYX));
//			if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX) && IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
//				_panel->symmetry = Panel::Symmetry::Rotational;
//			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX))
//				_panel->symmetry = Panel::Symmetry::Horizontal;
//			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
//				_panel->symmetry = Panel::Symmetry::Vertical;
//			else _panel->symmetry = Panel::Symmetry::None;
//			_panel->Write(panel);
//			break;
//
//		case IDC_SYMMETRYY:
//			CheckDlgButton(hwnd, IDC_SYMMETRYY, !IsDlgButtonChecked(hwnd, IDC_SYMMETRYY));
//			if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX) && IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
//				_panel->symmetry = Panel::Symmetry::Rotational;
//			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX))
//				_panel->symmetry = Panel::Symmetry::Horizontal;
//			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
//				_panel->symmetry = Panel::Symmetry::Vertical;
//			else _panel->symmetry = Panel::Symmetry::None;
//			_panel->Write(panel);
//			break;
//		//Tetris shape editing
//		case SHAPE_11: CheckDlgButton(hwnd, SHAPE_11, !IsDlgButtonChecked(hwnd, SHAPE_11)); currentShape ^= SHAPE_11; break;
//		case SHAPE_12: CheckDlgButton(hwnd, SHAPE_12, !IsDlgButtonChecked(hwnd, SHAPE_12)); currentShape ^= SHAPE_12; break;
//		case SHAPE_13: CheckDlgButton(hwnd, SHAPE_13, !IsDlgButtonChecked(hwnd, SHAPE_13)); currentShape ^= SHAPE_13; break;
//		case SHAPE_14: CheckDlgButton(hwnd, SHAPE_14, !IsDlgButtonChecked(hwnd, SHAPE_14)); currentShape ^= SHAPE_14; break;
//		case SHAPE_21: CheckDlgButton(hwnd, SHAPE_21, !IsDlgButtonChecked(hwnd, SHAPE_21)); currentShape ^= SHAPE_21; break;
//		case SHAPE_22: CheckDlgButton(hwnd, SHAPE_22, !IsDlgButtonChecked(hwnd, SHAPE_22)); currentShape ^= SHAPE_22; break;
//		case SHAPE_23: CheckDlgButton(hwnd, SHAPE_23, !IsDlgButtonChecked(hwnd, SHAPE_23)); currentShape ^= SHAPE_23; break;
//		case SHAPE_24: CheckDlgButton(hwnd, SHAPE_24, !IsDlgButtonChecked(hwnd, SHAPE_24)); currentShape ^= SHAPE_24; break;
//		case SHAPE_31: CheckDlgButton(hwnd, SHAPE_31, !IsDlgButtonChecked(hwnd, SHAPE_31)); currentShape ^= SHAPE_31; break;
//		case SHAPE_32: CheckDlgButton(hwnd, SHAPE_32, !IsDlgButtonChecked(hwnd, SHAPE_32)); currentShape ^= SHAPE_32; break;
//		case SHAPE_33: CheckDlgButton(hwnd, SHAPE_33, !IsDlgButtonChecked(hwnd, SHAPE_33)); currentShape ^= SHAPE_33; break;
//		case SHAPE_34: CheckDlgButton(hwnd, SHAPE_34, !IsDlgButtonChecked(hwnd, SHAPE_34)); currentShape ^= SHAPE_34; break;
//		case SHAPE_41: CheckDlgButton(hwnd, SHAPE_41, !IsDlgButtonChecked(hwnd, SHAPE_41)); currentShape ^= SHAPE_41; break;
//		case SHAPE_42: CheckDlgButton(hwnd, SHAPE_42, !IsDlgButtonChecked(hwnd, SHAPE_42)); currentShape ^= SHAPE_42; break;
//		case SHAPE_43: CheckDlgButton(hwnd, SHAPE_43, !IsDlgButtonChecked(hwnd, SHAPE_43)); currentShape ^= SHAPE_43; break;
//		case SHAPE_44: CheckDlgButton(hwnd, SHAPE_44, !IsDlgButtonChecked(hwnd, SHAPE_44)); currentShape ^= SHAPE_44; break;
//		//Arrow direction selection
//		case ARROW_UP_LEFT: CheckDlgButton(hwnd, ARROW_UP_LEFT, !IsDlgButtonChecked(hwnd, ARROW_UP_LEFT)); if (IsDlgButtonChecked(hwnd, ARROW_UP_LEFT)) currentDir = ARROW_UP_LEFT; break;
//		case ARROW_UP: CheckDlgButton(hwnd, ARROW_UP, !IsDlgButtonChecked(hwnd, ARROW_UP)); if (IsDlgButtonChecked(hwnd, ARROW_UP)) currentDir = ARROW_UP; break;
//		case ARROW_UP_RIGHT: CheckDlgButton(hwnd, ARROW_UP_RIGHT, !IsDlgButtonChecked(hwnd, ARROW_UP_RIGHT)); if (IsDlgButtonChecked(hwnd, ARROW_UP_RIGHT)) currentDir = ARROW_UP_RIGHT; break;
//		case ARROW_LEFT: CheckDlgButton(hwnd, ARROW_LEFT, !IsDlgButtonChecked(hwnd, ARROW_LEFT)); if (IsDlgButtonChecked(hwnd, ARROW_LEFT)) currentDir = ARROW_LEFT; break;
//		case ARROW_RIGHT: CheckDlgButton(hwnd, ARROW_RIGHT, !IsDlgButtonChecked(hwnd, ARROW_RIGHT)); if (IsDlgButtonChecked(hwnd, ARROW_RIGHT)) currentDir = ARROW_RIGHT; break;
//		case ARROW_DOWN_LEFT: CheckDlgButton(hwnd, ARROW_DOWN_LEFT, !IsDlgButtonChecked(hwnd, ARROW_DOWN_LEFT)); if (IsDlgButtonChecked(hwnd, ARROW_DOWN_LEFT)) currentDir = ARROW_DOWN_LEFT; break;
//		case ARROW_DOWN: CheckDlgButton(hwnd, ARROW_DOWN, !IsDlgButtonChecked(hwnd, ARROW_DOWN)); if (IsDlgButtonChecked(hwnd, ARROW_DOWN)) currentDir = ARROW_DOWN; break;
//		case ARROW_DOWN_RIGHT: CheckDlgButton(hwnd, ARROW_DOWN_RIGHT, !IsDlgButtonChecked(hwnd, ARROW_DOWN_RIGHT)); if (IsDlgButtonChecked(hwnd, ARROW_DOWN_RIGHT)) currentDir = ARROW_DOWN_RIGHT; break;
//		*/
//		}
//	}
//    return DefWindowProc(hwnd, message, wParam, lParam);
//}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)

{
	LoadLibrary(L"Msftedit.dll");

	ClientWindow::create(hInstance, nCmdShow);

	//Initialize memory globals constant depending on game version
	Memory::create();
	Memory* memory = memory->get();
	
	InputWatchdog::initialize();

	//Get the seed and difficulty previously used for this save file (if applicable)
	//int lastSeed = Special::ReadPanelData<int>(0x00064, BACKGROUND_REGION_COLOR + 12);
	//hard = (Special::ReadPanelData<int>(0x00182, BACKGROUND_REGION_COLOR + 12) > 0);
	//doubleMode = (Special::ReadPanelData<int>(0x0A3B2, BACKGROUND_REGION_COLOR + 12) > 0);

	//---------------------Debug/editing controls (debug mode only)---------------------

	//if (DEBUG) {

	//	////////
	//	// Horizontal rule.
	//	CreateWindow(L"STATIC", L"",
	//		WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,
	//		minX - 2, currentY + 4, maxX - minX + 4, 2, hwndRootWindow, 0, hInstance, NULL);
	//	currentY += 10 + lineSpacing;

	//	_panel = std::make_shared<Panel>();
	//	CreateWindow(L"STATIC", L"Col/Row:",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
	//		160, currentY, 100, 26, hwndRootWindow, NULL, hInstance, NULL);
	//	hwndCol = CreateWindow(MSFTEDIT_CLASS, L"",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
	//		220, currentY, 50, 26, hwndRootWindow, NULL, hInstance, NULL);
	//	hwndRow = CreateWindow(MSFTEDIT_CLASS, L"",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
	//		270, currentY, 50, 26, hwndRootWindow, NULL, hInstance, NULL);
	//	SetWindowText(hwndCol, L"0");
	//	SetWindowText(hwndRow, L"0");

	//	hwndElem = CreateWindow(L"COMBOBOX", L"",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWN | CBS_HASSTRINGS,
	//		160, currentY + 30, 150, 300, hwndRootWindow, NULL, hInstance, NULL);

	//	const int NUM_ELEMS = 17;
	//	TCHAR elems[NUM_ELEMS][24] =
	//	{
	//		TEXT("Stone"), TEXT("Star"), TEXT("Eraser"), TEXT("Shape"),
	//		TEXT("Triangle 1"), TEXT("Triangle 2"), TEXT("Triangle 3"),
	//		TEXT("Arrow 1"), TEXT("Arrow 2"), TEXT("Arrow 3"),
	//		TEXT("Start"), TEXT("Exit"), TEXT("Gap (Row)"), TEXT("Gap (Column)"),
	//		TEXT("Dot (Intersection)"), TEXT("Dot (Row)"), TEXT("Dot (Column)"),
	//	};
	//	TCHAR option[24];
	//	memset(&option, 0, sizeof(option));
	//	for (int i = 0; i < NUM_ELEMS; i++)
	//	{
	//		wcscpy_s(option, sizeof(option) / sizeof(TCHAR), (TCHAR*)elems[i]);
	//		SendMessage(hwndElem, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)option);
	//	}
	//	SendMessage(hwndElem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	//	hwndColor = CreateWindow(L"COMBOBOX", L"",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWN | CBS_HASSTRINGS,
	//		160, currentY + 60, 150, 300, hwndRootWindow, NULL, hInstance, NULL);

	//	const int NUM_COLORS = 12;
	//	TCHAR colors[NUM_COLORS][24] =
	//	{
	//		TEXT("Black"), TEXT("White"), TEXT("Red"), TEXT("Orange"), TEXT("Yellow"), TEXT("Green"),
	//		TEXT("Cyan"), TEXT("Blue"), TEXT("Purple"), TEXT("Magenta"), TEXT("None"), TEXT("???")
	//	};
	//	memset(&option, 0, sizeof(option));
	//	for (int i = 0; i < NUM_COLORS; i++)
	//	{
	//		wcscpy_s(option, sizeof(option) / sizeof(TCHAR), (TCHAR*)colors[i]);
	//		SendMessage(hwndColor, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)option);
	//	}
	//	SendMessage(hwndColor, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	//	CreateWindow(L"BUTTON", L"Place Symbol",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
	//		160, currentY + 90, 150, 26, hwndRootWindow, (HMENU)IDC_ADD, hInstance, NULL);
	//	CreateWindow(L"BUTTON", L"Remove Symbol",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
	//		160, currentY + 120, 150, 26, hwndRootWindow, (HMENU)IDC_REMOVE, hInstance, NULL);
	//	CreateWindow(L"BUTTON", L"Test",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
	//		160, currentY + 200, 150, 26, hwndRootWindow, (HMENU)IDC_TEST, hInstance, NULL);

	//	CreateWindow(L"STATIC", L"Shape:",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
	//		50, currentY + 20, 50, 16, hwndRootWindow, NULL, hInstance, NULL);

	//	for (int x = 0; x < 4; x++) {
	//		for (int y = 0; y < 4; y++) {
	//			CreateWindow(L"BUTTON", L"",
	//				WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
	//				50 + x * 15, currentY + 50 + y * 15, 12, 12, hwndRootWindow, (HMENU)shapePos[x + y * 4], hInstance, NULL);
	//		}
	//	}
	//	for (long long chk : defaultShape) {
	//		CheckDlgButton(hwndRootWindow, static_cast<int>(chk), TRUE);
	//		currentShape |= chk;
	//	}

	//	CreateWindow(L"BUTTON", L"",
	//		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
	//		50, currentY + 130, 12, 12, hwndRootWindow, (HMENU)IDC_ROTATED, hInstance, NULL);
	//	CreateWindow(L"STATIC", L"Rotated",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
	//		65, currentY + 130, 80, 16, hwndRootWindow, NULL, hInstance, NULL);

	//	CreateWindow(L"BUTTON", L"",
	//		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
	//		50, currentY + 150, 12, 12, hwndRootWindow, (HMENU)IDC_NEGATIVE, hInstance, NULL);
	//	CreateWindow(L"STATIC", L"Negative",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
	//		65, currentY + 150, 80, 16, hwndRootWindow, NULL, hInstance, NULL);

	//	CreateWindow(L"BUTTON", L"",
	//		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
	//		50, currentY + 190, 12, 12, hwndRootWindow, (HMENU)IDC_SYMMETRYX, hInstance, NULL);
	//	CreateWindow(L"STATIC", L"H. Symmetry",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
	//		65, currentY + 190, 90, 16, hwndRootWindow, NULL, hInstance, NULL);

	//	CreateWindow(L"BUTTON", L"",
	//		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
	//		50, currentY + 210, 12, 12, hwndRootWindow, (HMENU)IDC_SYMMETRYY, hInstance, NULL);
	//	CreateWindow(L"STATIC", L"V. Symmetry",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
	//		65, currentY + 210, 90, 16, hwndRootWindow, NULL, hInstance, NULL);

	//	CreateWindow(L"STATIC", L"Direction:",
	//		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
	//		50, currentY + 230, 90, 16, hwndRootWindow, NULL, hInstance, NULL);

	//	for (int x = 0; x < 3; x++) {
	//		for (int y = 0; y < 3; y++) {
	//			if (x == 1 && y == 1) continue;
	//			CreateWindow(L"BUTTON", L"",
	//				WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
	//				50 + x * 15, currentY + 250 + y * 15, 12, 12, hwndRootWindow, (HMENU)directions[x + y * 3], hInstance, NULL);
	//		}
	//	}

	//	currentY += 317 + lineSpacing;

	//	if (_panel->symmetry == Panel::Symmetry::Horizontal || _panel->symmetry == Panel::Symmetry::Rotational)
	//		CheckDlgButton(hwndRootWindow, IDC_SYMMETRYX, TRUE);
	//	if (_panel->symmetry == Panel::Symmetry::Vertical || _panel->symmetry == Panel::Symmetry::Rotational)
	//		CheckDlgButton(hwndRootWindow, IDC_SYMMETRYY, TRUE);
	//}

	// Restore configuration data.
	std::ifstream configFile("WRPGconfig.txt");
	if (configFile.is_open()) {
		std::map<std::string, std::string> settings;
		std::string setting, value;
		while (!configFile.eof() && configFile.good()) {
			std::getline(configFile, setting, ':');
			std::getline(configFile, value);
			settings[setting] = value;
		}

		configFile.close();
	}

	ClientWindow::get()->loadSettings();
	ClientWindow::get()->setWindowMode(ClientWindowMode::PreConnect);

	//focusEdit(hwndAddress);

	//ShowWindow(hwndRootWindow, nCmdShow);
	//UpdateWindow(hwndRootWindow);

	//ACCEL Accel[] = { { FVIRTKEY, VK_TAB, IDC_TAB}, { FVIRTKEY, VK_RETURN, IDC_RETURN} };
	//const HACCEL hAccel = CreateAcceleratorTable(Accel, sizeof(Accel) / sizeof(ACCEL));

    MSG msg;
	ClientWindow* clientWindow = ClientWindow::get();
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		//if (!TranslateAccelerator(clientWindow->getRootWindow(), hAccel, &msg)) {
		if (!IsDialogMessage(clientWindow->getRootWindow(), &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
    }

    return (int) msg.wParam;
}