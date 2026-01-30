// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "windows.h"
#include <Richedit.h>

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
#include "SymbolData.h"

#define IDC_RANDOMIZE 0x401
#define IDC_RERANDOMIZE 0x405
#define IDC_TEST 0x406

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

#define DEBUG true

//Panel to edit
PanelID panelID = TUT_DOT_1;

HWND hwndSeed, hwndRandomize, hwndCol, hwndRow, hwndElem, hwndVariant, hwndColor, hwndLoadingText, hwndNormal, hwndExpert, hwndColorblind, hwndDoubleMode;
Panel panel;
Randomizer randomizer;
Generate generator;
Special specialCase;
SymbolsWatchdog* symbolsWatchdog;
std::vector<byte> bytes;

int ctr = 0;
TCHAR text[30];
int x, y, variant;
std::wstring str;
int symbol;
SymbolColor color;
int currentShape;
int currentDir;
std::vector<long long> shapePos = { SHAPE_11, SHAPE_12, SHAPE_13, SHAPE_14, SHAPE_21, SHAPE_22, SHAPE_23, SHAPE_24, SHAPE_31, SHAPE_32, SHAPE_33, SHAPE_34, SHAPE_41, SHAPE_42, SHAPE_43, SHAPE_44 };
std::vector<long long> defaultShape = { SHAPE_21, SHAPE_31, SHAPE_32, SHAPE_33 }; //L-shape

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Memory* memory = Memory::get();
	static bool seedIsRNG = false;

	if (message == WM_CLOSE) {
		if (MessageBox(hwnd, L"Warning: Some puzzles may be unsolvable if you close the randomizer while the game is running.\r\n\r\nExit anyway?", L"", MB_OKCANCEL) == IDOK)
			DestroyWindow(hwnd);
		return 0;
	}
	else if (message == WM_DESTROY) {
		PostQuitMessage(0);
	} else if (message == WM_COMMAND || message == WM_TIMER) {
		switch (HIWORD(wParam)) {
			// Seed contents changed
		case EN_CHANGE:
			seedIsRNG = false;
		}
		switch (LOWORD(wParam)) {

			//Test button  - general testing (debug mode only)
		case IDC_TEST:
			randomizer.StartWatchdogs();
			specialCase = Special(&specialCase.g);
			specialCase.g.seed(static_cast<unsigned int>(time(NULL)));
			//specialCase.g.seed(ctr++);
			//specialCase.g.seed(1);
			specialCase.test();
			break;

			//Difficulty selection
		case IDC_DIFFICULTY_NORMAL:
			randomizer.difficulty = Normal;
			break;
		case IDC_DIFFICULTY_EXPERT:
			randomizer.difficulty = Expert;
			break;
		case IDC_COLORBLIND:
			randomizer.colorblind = !IsDlgButtonChecked(hwnd, IDC_COLORBLIND);
			CheckDlgButton(hwnd, IDC_COLORBLIND, randomizer.colorblind);
			break;
		case IDC_DOUBLE:
			randomizer.doubleMode = !IsDlgButtonChecked(hwnd, IDC_DOUBLE);
			CheckDlgButton(hwnd, IDC_DOUBLE, randomizer.doubleMode);
			break;

			//Randomize button
		case IDC_RANDOMIZE:
		{
			bool rerandomize = false;
			if (memory->ReadPanelData<int>(TUT_ENTER_1, NUM_DOTS) > 5) {
				if (MessageBox(hwnd, L"Game is currently randomized. Are you sure you want to randomize again? (Can cause glitches)", NULL, MB_YESNO) == IDYES) {
					rerandomize = true;
					seedIsRNG = false;
				}
				else break;
			}

			//Read seed from text box
			WCHAR text[100];
			GetWindowText(hwndSeed, &text[0], 100);
			int seed = _wtoi(text);
			if (seed <= 0 || seed > 9999999) {
				if (text[0] == 0) {
					//If no seed is entered, pick random seed
					Random::seed(static_cast<int>(time(NULL)));
					seed = Random::rand() % 9999999 + 1;
					Random::seed(seed);
					randomizer.seedIsRNG = true;
				}
				else {
					MessageBox(hwnd, L"Please enter a number between 1 and 9999999.", NULL, MB_OK);
					break;
				}
			}
			else randomizer.seedIsRNG = false;

			memory->ClearOffsets();

			ShowWindow(hwndLoadingText, SW_SHOW);

			randomizer.AdjustSpeed(); //Makes certain moving objects move faster

			//If the save was previously randomized, check that seed and difficulty match with the save file
			int lastSeed = memory->ReadPanelData<int>(TUT_ENTER_1, BACKGROUND_HIDDEN_VAR);
			Difficulty lastDiff = memory->ReadPanelData<Difficulty>(TUT_ENTER_2, BACKGROUND_HIDDEN_VAR);
			if (lastSeed > 0 && !rerandomize && !DEBUG) {
				if (seed != lastSeed && !randomizer.seedIsRNG) {
					if (MessageBox(hwnd, (L"This save file was previously randomized with seed " + std::to_wstring(lastSeed) + L". Are you sure you want to use seed " + std::to_wstring(seed) + L" instead?").c_str(), NULL, MB_YESNO) == IDNO) {
						SetWindowText(hwndSeed, std::to_wstring(lastSeed).c_str());
						break;
					}
				}
				if (lastDiff == Normal && randomizer.difficulty != Normal) {
					if (MessageBox(hwnd, L"This save file was previously randomized on Normal. Are you sure you want to switch to Expert?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndNormal, BM_SETCHECK, BST_CHECKED, 1);
						SendMessage(hwndExpert, BM_SETCHECK, BST_UNCHECKED, 1);
						randomizer.difficulty = lastDiff;
						break;
					}
				}
				if (lastDiff == Expert && randomizer.difficulty != Expert) {
					if (MessageBox(hwnd, L"This save file was previously randomized on Expert. Are you sure you want to switch to Normal?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndExpert, BM_SETCHECK, BST_CHECKED, 1);
						SendMessage(hwndNormal, BM_SETCHECK, BST_UNCHECKED, 1);
						randomizer.difficulty = lastDiff;
						break;
					}
				}
				bool lastDouble = (memory->ReadPanelData<int>(TUT_2START, BACKGROUND_HIDDEN_VAR) > 0);
				if (lastDouble && !randomizer.doubleMode) {
					if (MessageBox(hwnd, L"This save file was previously randomized on Double Mode. Are you sure you want to disable it?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndDoubleMode, BM_SETCHECK, BST_CHECKED, 1);
						randomizer.doubleMode = true;
						break;
					}
				}
				if (!lastDouble && randomizer.doubleMode) {
					if (MessageBox(hwnd, L"This save file was not previously randomized on Double Mode. Are you sure you want to enable it?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndDoubleMode, BM_SETCHECK, BST_UNCHECKED, 1);
						randomizer.doubleMode = false;
						break;
					}
				}
			}

			//If the save hasn't been randomized before, make sure it is a fresh, unplayed save file
			else if (Special::hasBeenPlayed() && !rerandomize && !DEBUG) {
				if (MessageBox(hwnd, L"Warning: It is recommended that you start a new game for the randomizer, to prevent corruption of your save file. Randomize on the current save file anyway?", NULL, MB_YESNO) == IDYES) {
					randomizer.seedIsRNG = false;
				}
				else break;
			}

			EnableWindow(hwndColorblind, false);
			EnableWindow(hwndDoubleMode, false);
			if (randomizer.colorblind) {
				std::ofstream out("WRPGconfig.txt");
				out << "colorblind:true" << std::endl;
				out.close();
			}
			else {
				std::remove("WRPGconfig.txt");
			}
			SetWindowText(hwndRandomize, L"Randomizing...");
			randomizer.seed = seed;
			randomizer.Generate(hwndLoadingText);
			memory->WritePanelData(TUT_ENTER_1, BACKGROUND_HIDDEN_VAR, seed);
			memory->WritePanelData<int>(TUT_ENTER_2, BACKGROUND_HIDDEN_VAR, randomizer.difficulty);
			memory->WritePanelData<int>(TUT_2START, BACKGROUND_HIDDEN_VAR, randomizer.doubleMode);
			SetWindowText(hwndRandomize, L"Randomized!");
			SetWindowText(hwndSeed, std::to_wstring(seed).c_str());

			break;
		}

		//Add a symbol to the puzzle (debug mode only)
		case IDC_ADD:
			memset(&text, 0, sizeof(text));
			GetWindowText(hwndElem, text, 30);
			str.reserve(30);
			GetWindowText(hwndVariant, &str[0], 30);
			variant = _wtoi(str.c_str());
			if (wcscmp(text, L"Stone") == 0) symbol = Stone;
			if (wcscmp(text, L"Star") == 0) symbol = Star;
			if (wcscmp(text, L"Eraser") == 0) symbol = Eraser;
			if (wcscmp(text, L"Shape") == 0) symbol = Poly;
			if (wcscmp(text, L"Triangle") == 0) symbol = Triangle | (variant << 16);
			if (wcscmp(text, L"Arrow") == 0) symbol = SymbolData::GetValFromSymbolID(ARROW1E + variant);
			if (wcscmp(text, L"Anti-Triangle") == 0) symbol = SymbolData::GetValFromSymbolID(ANTITRIANGLE1 + variant - 1);
			if (wcscmp(text, L"Cave") == 0) symbol = SymbolData::GetValFromSymbolID(CAVE1 + variant - 1);
			if (wcscmp(text, L"Minesweeper") == 0) symbol = SymbolData::GetValFromSymbolID(MINESWEEPER0 + variant);
			if (wcscmp(text, L"Dot (Intersection)") == 0) symbol = Dot_Intersection;
			if (wcscmp(text, L"Dot (Row)") == 0) symbol = Dot_Row;
			if (wcscmp(text, L"Dot (Column)") == 0) symbol = Dot_Column;
			if (wcscmp(text, L"Gap (Row)") == 0) symbol = Gap_Row;
			if (wcscmp(text, L"Gap (Column)") == 0) symbol = Gap_Column;
			if (wcscmp(text, L"Start") == 0) symbol = Start;
			if (wcscmp(text, L"Exit") == 0) symbol = Exit;
			memset(&text, 0, sizeof(text));
			GetWindowText(hwndColor, text, 30);
			if (wcscmp(text, L"None") == 0) color = NoColor;
			if (wcscmp(text, L"Black") == 0) color = Black;
			if (wcscmp(text, L"White") == 0) color = White;
			if (wcscmp(text, L"Red") == 0) color = Red;
			if (wcscmp(text, L"Orange") == 0) color = Orange;
			if (wcscmp(text, L"Yellow") == 0) color = Yellow;
			if (wcscmp(text, L"Green") == 0) color = Green;
			if (wcscmp(text, L"Cyan") == 0) color = Cyan;
			if (wcscmp(text, L"Blue") == 0) color = Blue;
			if (wcscmp(text, L"Purple") == 0) color = Purple;
			if (wcscmp(text, L"Magenta") == 0) color = Magenta;
			if (wcscmp(text, L"Invisible") == 0) color = Invisible;
			GetWindowText(hwndCol, &str[0], 30);
			x = _wtoi(str.c_str());
			GetWindowText(hwndRow, &str[0], 30);
			y = _wtoi(str.c_str());

			panelID = memory->GetActivePanel();
			if (panelID == -1) break;
			panel.read(panelID);
			if (symbol == Poly)
				panel.setShape(x, y, currentShape, IsDlgButtonChecked(hwnd, IDC_ROTATED), IsDlgButtonChecked(hwnd, IDC_NEGATIVE), color);
			else panel.setSymbol(x, y, static_cast<Symbol>(symbol), color);
			panel.write(panelID);
			break;

			//Remove a symbol from the puzzle (debug mode only)
		case IDC_REMOVE:
			str.reserve(30);
			GetWindowText(hwndCol, &str[0], 30);
			x = _wtoi(str.c_str());
			GetWindowText(hwndRow, &str[0], 30);
			y = _wtoi(str.c_str());
			GetWindowText(hwndElem, text, 30);

			panelID = memory->GetActivePanel();
			if (panelID == -1) break;
			panel.read(panelID);
			if (wcscmp(text, L"Dot (Intersection)") == 0 ||
				wcscmp(text, L"Start") == 0 || wcscmp(text, L"Exit") == 0)
				panel.setGridSymbol(x * 2, y * 2, None, NoColor);
			else if (wcscmp(text, L"Gap (Column)") == 0 ||
				wcscmp(text, L"Dot (Column)") == 0)
				panel.setGridSymbol(x * 2, y * 2 + 1, None, NoColor);
			else if (wcscmp(text, L"Gap (Row)") == 0 ||
				wcscmp(text, L"Dot (Row)") == 0)
				panel.setGridSymbol(x * 2 + 1, y * 2, None, NoColor);
			else
				panel.setSymbol(x, y, None, NoColor);
			panel.write(panelID);
			break;

			//Debug mode checkboxes
		case IDC_ROTATED:
			CheckDlgButton(hwnd, IDC_ROTATED, !IsDlgButtonChecked(hwnd, IDC_ROTATED));
			break;

		case IDC_NEGATIVE:
			CheckDlgButton(hwnd, IDC_NEGATIVE, !IsDlgButtonChecked(hwnd, IDC_NEGATIVE));
			break;

		case IDC_SYMMETRYX:
			CheckDlgButton(hwnd, IDC_SYMMETRYX, !IsDlgButtonChecked(hwnd, IDC_SYMMETRYX));
			if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX) && IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
				panel.symmetry = Rotational;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX))
				panel.symmetry = Horizontal;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
				panel.symmetry = Vertical;
			else panel.symmetry = NoSymmetry;
			panel.write(panelID);
			break;

		case IDC_SYMMETRYY:
			CheckDlgButton(hwnd, IDC_SYMMETRYY, !IsDlgButtonChecked(hwnd, IDC_SYMMETRYY));
			if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX) && IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
				panel.symmetry = Rotational;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX))
				panel.symmetry = Horizontal;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
				panel.symmetry = Vertical;
			else panel.symmetry = NoSymmetry;
			panel.write(panelID);
			break;
		//Tetris shape editing
		case SHAPE_11: CheckDlgButton(hwnd, SHAPE_11, !IsDlgButtonChecked(hwnd, SHAPE_11)); currentShape ^= SHAPE_11; break;
		case SHAPE_12: CheckDlgButton(hwnd, SHAPE_12, !IsDlgButtonChecked(hwnd, SHAPE_12)); currentShape ^= SHAPE_12; break;
		case SHAPE_13: CheckDlgButton(hwnd, SHAPE_13, !IsDlgButtonChecked(hwnd, SHAPE_13)); currentShape ^= SHAPE_13; break;
		case SHAPE_14: CheckDlgButton(hwnd, SHAPE_14, !IsDlgButtonChecked(hwnd, SHAPE_14)); currentShape ^= SHAPE_14; break;
		case SHAPE_21: CheckDlgButton(hwnd, SHAPE_21, !IsDlgButtonChecked(hwnd, SHAPE_21)); currentShape ^= SHAPE_21; break;
		case SHAPE_22: CheckDlgButton(hwnd, SHAPE_22, !IsDlgButtonChecked(hwnd, SHAPE_22)); currentShape ^= SHAPE_22; break;
		case SHAPE_23: CheckDlgButton(hwnd, SHAPE_23, !IsDlgButtonChecked(hwnd, SHAPE_23)); currentShape ^= SHAPE_23; break;
		case SHAPE_24: CheckDlgButton(hwnd, SHAPE_24, !IsDlgButtonChecked(hwnd, SHAPE_24)); currentShape ^= SHAPE_24; break;
		case SHAPE_31: CheckDlgButton(hwnd, SHAPE_31, !IsDlgButtonChecked(hwnd, SHAPE_31)); currentShape ^= SHAPE_31; break;
		case SHAPE_32: CheckDlgButton(hwnd, SHAPE_32, !IsDlgButtonChecked(hwnd, SHAPE_32)); currentShape ^= SHAPE_32; break;
		case SHAPE_33: CheckDlgButton(hwnd, SHAPE_33, !IsDlgButtonChecked(hwnd, SHAPE_33)); currentShape ^= SHAPE_33; break;
		case SHAPE_34: CheckDlgButton(hwnd, SHAPE_34, !IsDlgButtonChecked(hwnd, SHAPE_34)); currentShape ^= SHAPE_34; break;
		case SHAPE_41: CheckDlgButton(hwnd, SHAPE_41, !IsDlgButtonChecked(hwnd, SHAPE_41)); currentShape ^= SHAPE_41; break;
		case SHAPE_42: CheckDlgButton(hwnd, SHAPE_42, !IsDlgButtonChecked(hwnd, SHAPE_42)); currentShape ^= SHAPE_42; break;
		case SHAPE_43: CheckDlgButton(hwnd, SHAPE_43, !IsDlgButtonChecked(hwnd, SHAPE_43)); currentShape ^= SHAPE_43; break;
		case SHAPE_44: CheckDlgButton(hwnd, SHAPE_44, !IsDlgButtonChecked(hwnd, SHAPE_44)); currentShape ^= SHAPE_44; break;
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	//Initialize memory globals constant depending on game version
	Memory::create();

	Memory* memory = memory->get();
	specialCase = Special(&generator);

	LoadLibrary(L"Msftedit.dll");

	WNDCLASSW wndClass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		NULL,
		LoadCursor(nullptr, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		WINDOW_CLASS,
		WINDOW_CLASS,
	};
	RegisterClassW(&wndClass);

	HWND hwnd = CreateWindow(WINDOW_CLASS, PRODUCT_NAME, WS_OVERLAPPEDWINDOW,
		650, 200, 600, DEBUG ? 700 : 320, nullptr, nullptr, hInstance, nullptr);

	memory->showMsg = true;

	//Get the seed and difficulty previously used for this save file (if applicable)
	int lastSeed = memory->ReadPanelData<int>(TUT_ENTER_1, BACKGROUND_HIDDEN_VAR);
	randomizer.difficulty = memory->ReadPanelData<Difficulty>(TUT_ENTER_2, BACKGROUND_HIDDEN_VAR);
	randomizer.doubleMode = memory->ReadPanelData<int>(TUT_2START, BACKGROUND_HIDDEN_VAR);

	//-------------------------Basic window controls---------------------------

	CreateWindow(L"STATIC", L"Version: " VERSION_STR,
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
		490, 15, 90, 16, hwnd, NULL, hInstance, NULL);

	CreateWindow(L"STATIC", L"Choose Difficuty:",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
		10, 15, 120, 16, hwnd, NULL, hInstance, NULL);
	hwndNormal = CreateWindow(L"BUTTON", L"NORMAL - Puzzles that should be reasonably challenging for most players. Puzzle mechanics are mostly identical to those in the original game.",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_MULTILINE,
		10, 35, 570, 35, hwnd, (HMENU)IDC_DIFFICULTY_NORMAL, hInstance, NULL);
	hwndExpert = CreateWindow(L"BUTTON", L"EXPERT - Very difficult puzzles with complex mechanics and mind-boggling new tricks. For brave players seeking the ultimate challenge.",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | BS_MULTILINE,
		10, 75, 570, 35, hwnd, (HMENU)IDC_DIFFICULTY_EXPERT, hInstance, NULL);
	if (randomizer.difficulty == Expert) SendMessage(hwndExpert, BM_SETCHECK, BST_CHECKED, 1);
	else SendMessage(hwndNormal, BM_SETCHECK, BST_CHECKED, 1);

	CreateWindow(L"STATIC", L"Options:",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
		10, 125, 120, 16, hwnd, NULL, hInstance, NULL);
	hwndColorblind = CreateWindow(L"BUTTON", L"Colorblind Mode - The colors on certain panels will be changed to be more accommodating to people with colorblindness. The puzzles themselves are identical to those generated without colorblind mode enabled.",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_MULTILINE,
		10, 145, 570, 50, hwnd, (HMENU)IDC_COLORBLIND, hInstance, NULL);
	hwndDoubleMode = CreateWindow(L"BUTTON", L"Double Mode - In addition to generating new puzzles, the randomizer will also shuffle the location of most puzzles. (Not recommended for first playthrough)",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX | BS_MULTILINE,
		10, 200, 570, 35, hwnd, (HMENU)IDC_DOUBLE, hInstance, NULL);
	if (randomizer.doubleMode) SendMessage(hwndDoubleMode, BM_SETCHECK, BST_CHECKED, 1);

	CreateWindow(L"STATIC", L"Enter a seed (optional):",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
		10, 250, 160, 16, hwnd, NULL, hInstance, NULL);
	hwndSeed = CreateWindow(MSFTEDIT_CLASS, lastSeed == 0 ? L"" : std::to_wstring(lastSeed).c_str(),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
		180, 245, 60, 26, hwnd, NULL, hInstance, NULL);
	SendMessage(hwndSeed, EM_SETEVENTMASK, NULL, ENM_CHANGE); // Notify on text change

	hwndRandomize = CreateWindow(L"BUTTON", L"Randomize",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		250, 245, 130, 26, hwnd, (HMENU)IDC_RANDOMIZE, hInstance, NULL);

	hwndLoadingText = CreateWindow(L"STATIC", L"",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
		400, 250, 160, 16, hwnd, NULL, hInstance, NULL);

	std::ifstream configFile("WRPGconfig.txt");
	if (configFile.is_open()) {
		std::map<std::string, std::string> settings;
		std::string setting, value;
		while (!configFile.eof() && configFile.good()) {
			std::getline(configFile, setting, ':');
			std::getline(configFile, value);
			settings[setting] = value;
		}
		if (settings.count("colorblind") && settings["colorblind"] == "true") {
			randomizer.colorblind = true;
			CheckDlgButton(hwnd, IDC_COLORBLIND, true);
		}
		configFile.close();
	}

	ShowWindow(hwndLoadingText, SW_HIDE);

	//---------------------Debug/editing controls (debug mode only)---------------------

	if (DEBUG) {
		CreateWindow(L"STATIC", L"Col/Row:",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			160, 330, 100, 26, hwnd, NULL, hInstance, NULL);
		hwndCol = CreateWindow(MSFTEDIT_CLASS, L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
			220, 330, 40, 26, hwnd, NULL, hInstance, NULL);
		hwndRow = CreateWindow(MSFTEDIT_CLASS, L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
			260, 330, 40, 26, hwnd, NULL, hInstance, NULL);
		SetWindowText(hwndCol, L"0");
		SetWindowText(hwndRow, L"0");

		hwndElem = CreateWindow(L"COMBOBOX", L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWN | CBS_HASSTRINGS,
			160, 360, 150, 300, hwnd, NULL, hInstance, NULL);

		const int NUM_ELEMS = 17;
		TCHAR elems[NUM_ELEMS][24] =
		{
			TEXT("Stone"), TEXT("Star"), TEXT("Eraser"), TEXT("Shape"), TEXT("Triangle"),
			TEXT("Arrow"), TEXT("Anti-Triangle"), TEXT("Cave"), TEXT("Minesweeper"),
			TEXT("Gap (Row)"), TEXT("Gap (Column)"), TEXT("Dot (Intersection)"), TEXT("Dot (Row)"), TEXT("Dot (Column)"),
			TEXT("Start"), TEXT("Exit"),
		};
		TCHAR option[24];
		memset(&option, 0, sizeof(option));
		for (int i = 0; i < NUM_ELEMS; i++)
		{
			wcscpy_s(option, sizeof(option) / sizeof(TCHAR), (TCHAR*)elems[i]);
			SendMessage(hwndElem, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)option);
		}
		SendMessage(hwndElem, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		hwndColor = CreateWindow(L"COMBOBOX", L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWN | CBS_HASSTRINGS,
			160, 390, 150, 300, hwnd, NULL, hInstance, NULL);

		const int NUM_COLORS = 12;
		TCHAR colors[NUM_COLORS][24] =
		{
			TEXT("None"), TEXT("Black"), TEXT("White"), TEXT("Red"), TEXT("Orange"), TEXT("Yellow"), TEXT("Green"),
			TEXT("Cyan"), TEXT("Blue"), TEXT("Purple"), TEXT("Magenta"), TEXT("Invisible"),
		};
		memset(&option, 0, sizeof(option));
		for (int i = 0; i < NUM_COLORS; i++)
		{
			wcscpy_s(option, sizeof(option) / sizeof(TCHAR), (TCHAR*)colors[i]);
			SendMessage(hwndColor, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)option);
		}
		SendMessage(hwndColor, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		CreateWindow(L"STATIC", L"Count/Variant:",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			160, 420, 100, 26, hwnd, NULL, hInstance, NULL);
		hwndVariant = CreateWindow(MSFTEDIT_CLASS, L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
			270, 420, 40, 26, hwnd, NULL, hInstance, NULL);
		SetWindowText(hwndVariant, L"0");
		CreateWindow(L"BUTTON", L"Place Symbol",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			160, 450, 150, 26, hwnd, (HMENU)IDC_ADD, hInstance, NULL);
		CreateWindow(L"BUTTON", L"Remove Symbol",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			160, 480, 150, 26, hwnd, (HMENU)IDC_REMOVE, hInstance, NULL);
		//CreateWindow(L"BUTTON", L"Randomize Active",
		//	WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		//	160, 510, 150, 26, hwnd, (HMENU)IDC_TEST, hInstance, NULL);
		CreateWindow(L"BUTTON", L"Test",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			160, 540, 150, 26, hwnd, (HMENU)IDC_TEST, hInstance, NULL);

		CreateWindow(L"STATIC", L"Shape:",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			50, 350, 50, 16, hwnd, NULL, hInstance, NULL);

		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				CreateWindow(L"BUTTON", L"",
					WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
					50 + x * 15, 380 + y * 15, 12, 12, hwnd, (HMENU)shapePos[x + y * 4], hInstance, NULL);
			}
		}
		for (long long chk : defaultShape) {
			CheckDlgButton(hwnd, static_cast<int>(chk), TRUE);
			currentShape |= chk;
		}

		CreateWindow(L"BUTTON", L"",
			WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
			50, 460, 12, 12, hwnd, (HMENU)IDC_ROTATED, hInstance, NULL);
		CreateWindow(L"STATIC", L"Rotated",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			65, 460, 80, 16, hwnd, NULL, hInstance, NULL);

		CreateWindow(L"BUTTON", L"",
			WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
			50, 480, 12, 12, hwnd, (HMENU)IDC_NEGATIVE, hInstance, NULL);
		CreateWindow(L"STATIC", L"Negative",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			65, 480, 80, 16, hwnd, NULL, hInstance, NULL);

		CreateWindow(L"BUTTON", L"",
			WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
			50, 520, 12, 12, hwnd, (HMENU)IDC_SYMMETRYX, hInstance, NULL);
		CreateWindow(L"STATIC", L"H. Symmetry",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			65, 520, 90, 16, hwnd, NULL, hInstance, NULL);

		CreateWindow(L"BUTTON", L"",
			WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
			50, 540, 12, 12, hwnd, (HMENU)IDC_SYMMETRYY, hInstance, NULL);
		CreateWindow(L"STATIC", L"V. Symmetry",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			65, 540, 90, 16, hwnd, NULL, hInstance, NULL);

		if (panel.symmetry == Horizontal || panel.symmetry == Rotational)
			CheckDlgButton(hwnd, IDC_SYMMETRYX, TRUE);
		if (panel.symmetry == Vertical || panel.symmetry == Rotational)
			CheckDlgButton(hwnd, IDC_SYMMETRYY, TRUE);
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (!GetMessage(&msg, nullptr, 0, 0) == 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}
