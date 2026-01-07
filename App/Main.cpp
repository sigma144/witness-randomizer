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

HWND hwndSeed, hwndRandomize, hwndCol, hwndRow, hwndElem, hwndColor, hwndLoadingText, hwndNormal, hwndExpert, hwndColorblind, hwndDoubleMode;
std::shared_ptr<Panel> _panel;
std::shared_ptr<Randomizer> randomizer = std::make_shared<Randomizer>();
std::shared_ptr<Generate> generator = std::make_shared<Generate>();
std::shared_ptr<Special> specialCase = std::make_shared<Special>(generator);
std::vector<byte> bytes;

int ctr = 0;
TCHAR text[30];
int x, y;
std::wstring str;
Decoration::Shape symbol;
Decoration::Color color;
int currentShape;
int currentDir;
bool hard = false;
bool doubleMode = false;
int lastSeed;
bool lastHard;
bool colorblind;
std::vector<long long> shapePos = { SHAPE_11, SHAPE_12, SHAPE_13, SHAPE_14, SHAPE_21, SHAPE_22, SHAPE_23, SHAPE_24, SHAPE_31, SHAPE_32, SHAPE_33, SHAPE_34, SHAPE_41, SHAPE_42, SHAPE_43, SHAPE_44 };
std::vector<long long> defaultShape = { SHAPE_21, SHAPE_31, SHAPE_32, SHAPE_33 }; //L-shape
std::vector<long long> directions = { ARROW_UP_RIGHT, ARROW_UP, ARROW_UP_LEFT, ARROW_LEFT, 0, ARROW_RIGHT, ARROW_DOWN_LEFT, ARROW_DOWN, ARROW_DOWN_RIGHT }; //Order of directional check boxes
float target;

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
			generator->resetConfig();
			generator->seed(static_cast<unsigned int>(time(NULL)));
			//generator->seed(ctr++);
			//generator->seed(1);
			specialCase->test();
			break;

			//Difficulty selection
		case IDC_DIFFICULTY_NORMAL:
			hard = false;
			break;
		case IDC_DIFFICULTY_EXPERT:
			hard = true;
			break;
		case IDC_COLORBLIND:
			colorblind = !IsDlgButtonChecked(hwnd, IDC_COLORBLIND);
			CheckDlgButton(hwnd, IDC_COLORBLIND, colorblind);
			break;
		case IDC_DOUBLE:
			doubleMode = !IsDlgButtonChecked(hwnd, IDC_DOUBLE);
			CheckDlgButton(hwnd, IDC_DOUBLE, doubleMode);
			break;

			//Randomize button
		case IDC_RANDOMIZE:
		{
			bool rerandomize = false;
			if (memory->ReadPanelData<int>(0x00064, NUM_DOTS) > 5) {
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
					randomizer->seedIsRNG = true;
				}
				else {
					MessageBox(hwnd, L"Please enter a number between 1 and 9999999.", NULL, MB_OK);
					break;
				}
			}
			else randomizer->seedIsRNG = false;

			memory->ClearOffsets();

			ShowWindow(hwndLoadingText, SW_SHOW);

			randomizer->AdjustSpeed(); //Makes certain moving objects move faster

			//If the save was previously randomized, check that seed and difficulty match with the save file
			int lastSeed = memory->ReadPanelData<int>(0x00064, BACKGROUND_REGION_COLOR + 12);
			if (lastSeed > 0 && !rerandomize && !DEBUG) {
				if (seed != lastSeed && !randomizer->seedIsRNG) {
					if (MessageBox(hwnd, (L"This save file was previously randomized with seed " + std::to_wstring(lastSeed) + L". Are you sure you want to use seed " + std::to_wstring(seed) + L" instead?").c_str(), NULL, MB_YESNO) == IDNO) {
						SetWindowText(hwndSeed, std::to_wstring(lastSeed).c_str());
						break;
					}
				}
				lastHard = (memory->ReadPanelData<int>(0x00182, BACKGROUND_REGION_COLOR + 12) > 0);
				if (!lastHard && hard) {
					if (MessageBox(hwnd, L"This save file was previously randomized on Normal. Are you sure you want to switch to Expert?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndNormal, BM_SETCHECK, BST_CHECKED, 1);
						SendMessage(hwndExpert, BM_SETCHECK, BST_UNCHECKED, 1);
						hard = false;
						break;
					}
				}
				if (lastHard && !hard) {
					if (MessageBox(hwnd, L"This save file was previously randomized on Expert. Are you sure you want to switch to Normal?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndExpert, BM_SETCHECK, BST_CHECKED, 1);
						SendMessage(hwndNormal, BM_SETCHECK, BST_UNCHECKED, 1);
						hard = true;
						break;
					}
				}
				bool lastDouble = (memory->ReadPanelData<int>(0x0A3B2, BACKGROUND_REGION_COLOR + 12) > 0);
				if (lastDouble && !doubleMode) {
					if (MessageBox(hwnd, L"This save file was previously randomized on Double Mode. Are you sure you want to disable it?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndDoubleMode, BM_SETCHECK, BST_CHECKED, 1);
						doubleMode = true;
						break;
					}
				}
				if (!lastDouble && doubleMode) {
					if (MessageBox(hwnd, L"This save file was not previously randomized on Double Mode. Are you sure you want to enable it?", NULL, MB_YESNO) == IDNO) {
						SendMessage(hwndDoubleMode, BM_SETCHECK, BST_UNCHECKED, 1);
						doubleMode = false;
						break;
					}
				}
			}

			//If the save hasn't been randomized before, make sure it is a fresh, unplayed save file
			else if (Special::hasBeenPlayed() && !rerandomize && !DEBUG) {
				if (MessageBox(hwnd, L"Warning: It is recommended that you start a new game for the randomizer, to prevent corruption of your save file. Randomize on the current save file anyway?", NULL, MB_YESNO) == IDYES) {
					randomizer->seedIsRNG = false;
				}
				else break;
			}

			EnableWindow(hwndColorblind, false);
			EnableWindow(hwndDoubleMode, false);
			if (colorblind) {
				std::ofstream out("WRPGconfig.txt");
				out << "colorblind:true" << std::endl;
				out.close();
			}
			else {
				std::remove("WRPGconfig.txt");
			}
			SetWindowText(hwndRandomize, L"Randomizing...");
			randomizer->seed = seed;
			randomizer->colorblind = IsDlgButtonChecked(hwnd, IDC_COLORBLIND);
			randomizer->doubleMode = doubleMode;
			if (hard) randomizer->GenerateHard(hwndLoadingText);
			else randomizer->GenerateNormal(hwndLoadingText);
			memory->WritePanelData(0x00064, BACKGROUND_REGION_COLOR + 12, seed);
			memory->WritePanelData<int>(0x00182, BACKGROUND_REGION_COLOR + 12, hard);
			memory->WritePanelData<int>(0x0A3B2, BACKGROUND_REGION_COLOR + 12, doubleMode);
			SetWindowText(hwndRandomize, L"Randomized!");
			SetWindowText(hwndSeed, std::to_wstring(seed).c_str());

			break;
		}

		//Add a symbol to the puzzle (debug mode only)
		case IDC_ADD:
			memset(&text, 0, sizeof(text));
			GetWindowText(hwndElem, text, 30);
			if (wcscmp(text, L"Stone") == 0) symbol = Decoration::Shape::Stone;
			if (wcscmp(text, L"Star") == 0) symbol = Decoration::Shape::Star;
			if (wcscmp(text, L"Eraser") == 0) symbol = Decoration::Shape::Eraser;
			if (wcscmp(text, L"Shape") == 0) symbol = Decoration::Shape::Poly;
			if (wcscmp(text, L"Triangle 1") == 0) symbol = Decoration::Shape::Triangle1;
			if (wcscmp(text, L"Triangle 2") == 0) symbol = Decoration::Shape::Triangle2;
			if (wcscmp(text, L"Triangle 3") == 0) symbol = Decoration::Shape::Triangle3;
			if (wcscmp(text, L"Arrow 1") == 0) symbol = Decoration::Shape::Arrow1;
			if (wcscmp(text, L"Arrow 2") == 0) symbol = Decoration::Shape::Arrow2;
			if (wcscmp(text, L"Arrow 3") == 0) symbol = Decoration::Shape::Arrow3;
			if (wcscmp(text, L"Dot (Intersection)") == 0) symbol = Decoration::Shape::Dot_Intersection;
			if (wcscmp(text, L"Dot (Row)") == 0) symbol = Decoration::Shape::Dot_Row;
			if (wcscmp(text, L"Dot (Column)") == 0) symbol = Decoration::Shape::Dot_Column;
			if (wcscmp(text, L"Gap (Row)") == 0) symbol = Decoration::Shape::Gap_Row;
			if (wcscmp(text, L"Gap (Column)") == 0) symbol = Decoration::Shape::Gap_Column;
			if (wcscmp(text, L"Start") == 0) symbol = Decoration::Shape::Start;
			if (wcscmp(text, L"Exit") == 0) symbol = Decoration::Shape::Exit;
			memset(&text, 0, sizeof(text));
			GetWindowText(hwndColor, text, 30);
			if (wcscmp(text, L"Black") == 0) color = Decoration::Color::Black;
			if (wcscmp(text, L"White") == 0) color = Decoration::Color::White;
			if (wcscmp(text, L"Red") == 0) color = Decoration::Color::Red;
			if (wcscmp(text, L"Orange") == 0) color = Decoration::Color::Orange;
			if (wcscmp(text, L"Yellow") == 0) color = Decoration::Color::Yellow;
			if (wcscmp(text, L"Green") == 0) color = Decoration::Color::Green;
			if (wcscmp(text, L"Cyan") == 0) color = Decoration::Color::Cyan;
			if (wcscmp(text, L"Blue") == 0) color = Decoration::Color::Blue;
			if (wcscmp(text, L"Purple") == 0) color = Decoration::Color::Purple;
			if (wcscmp(text, L"Magenta") == 0) color = Decoration::Color::Magenta;
			if (wcscmp(text, L"None") == 0) color = Decoration::Color::None;
			if (wcscmp(text, L"???") == 0) color = Decoration::Color::X;
			str.reserve(30);
			GetWindowText(hwndCol, &str[0], 30);
			x = _wtoi(str.c_str());
			GetWindowText(hwndRow, &str[0], 30);
			y = _wtoi(str.c_str());

			_panel->Read(panel);
			if (symbol == Decoration::Shape::Poly)
				_panel->SetShape(x, y, currentShape, IsDlgButtonChecked(hwnd, IDC_ROTATED), IsDlgButtonChecked(hwnd, IDC_NEGATIVE), color);
			else if (symbol == Decoration::Shape::Arrow1 || symbol == Decoration::Shape::Arrow2 || symbol == Decoration::Shape::Arrow3)
				_panel->SetShape(x, y, symbol | currentDir, IsDlgButtonChecked(hwnd, IDC_ROTATED), IsDlgButtonChecked(hwnd, IDC_NEGATIVE), color);
			else _panel->SetSymbol(x, y, symbol, color);
			_panel->Write(panel);
			break;

			//Remove a symbol from the puzzle (debug mode only)
		case IDC_REMOVE:
			GetWindowText(hwndCol, &str[0], 30);
			x = _wtoi(str.c_str());
			GetWindowText(hwndRow, &str[0], 30);
			y = _wtoi(str.c_str());
			GetWindowText(hwndElem, text, 30);

			_panel->Read(panel);
			if (wcscmp(text, L"Dot (Intersection)") == 0 ||
				wcscmp(text, L"Start") == 0 || wcscmp(text, L"Exit") == 0)
				_panel->ClearGridSymbol(x * 2, y * 2);
			else if (wcscmp(text, L"Gap (Column)") == 0 ||
				wcscmp(text, L"Dot (Column)") == 0)
				_panel->ClearGridSymbol(x * 2, y * 2 + 1);
			else if (wcscmp(text, L"Gap (Row)") == 0 ||
				wcscmp(text, L"Dot (Row)") == 0)
				_panel->ClearGridSymbol(x * 2 + 1, y * 2);
			else
				_panel->ClearSymbol(x, y);
			_panel->Write(panel);
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
				_panel->symmetry = Panel::Symmetry::Rotational;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX))
				_panel->symmetry = Panel::Symmetry::Horizontal;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
				_panel->symmetry = Panel::Symmetry::Vertical;
			else _panel->symmetry = Panel::Symmetry::None;
			_panel->Write(panel);
			break;

		case IDC_SYMMETRYY:
			CheckDlgButton(hwnd, IDC_SYMMETRYY, !IsDlgButtonChecked(hwnd, IDC_SYMMETRYY));
			if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX) && IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
				_panel->symmetry = Panel::Symmetry::Rotational;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYX))
				_panel->symmetry = Panel::Symmetry::Horizontal;
			else if (IsDlgButtonChecked(hwnd, IDC_SYMMETRYY))
				_panel->symmetry = Panel::Symmetry::Vertical;
			else _panel->symmetry = Panel::Symmetry::None;
			_panel->Write(panel);
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
			//Arrow direction selection
		case ARROW_UP_LEFT: CheckDlgButton(hwnd, ARROW_UP_LEFT, !IsDlgButtonChecked(hwnd, ARROW_UP_LEFT)); if (IsDlgButtonChecked(hwnd, ARROW_UP_LEFT)) currentDir = ARROW_UP_LEFT; break;
		case ARROW_UP: CheckDlgButton(hwnd, ARROW_UP, !IsDlgButtonChecked(hwnd, ARROW_UP)); if (IsDlgButtonChecked(hwnd, ARROW_UP)) currentDir = ARROW_UP; break;
		case ARROW_UP_RIGHT: CheckDlgButton(hwnd, ARROW_UP_RIGHT, !IsDlgButtonChecked(hwnd, ARROW_UP_RIGHT)); if (IsDlgButtonChecked(hwnd, ARROW_UP_RIGHT)) currentDir = ARROW_UP_RIGHT; break;
		case ARROW_LEFT: CheckDlgButton(hwnd, ARROW_LEFT, !IsDlgButtonChecked(hwnd, ARROW_LEFT)); if (IsDlgButtonChecked(hwnd, ARROW_LEFT)) currentDir = ARROW_LEFT; break;
		case ARROW_RIGHT: CheckDlgButton(hwnd, ARROW_RIGHT, !IsDlgButtonChecked(hwnd, ARROW_RIGHT)); if (IsDlgButtonChecked(hwnd, ARROW_RIGHT)) currentDir = ARROW_RIGHT; break;
		case ARROW_DOWN_LEFT: CheckDlgButton(hwnd, ARROW_DOWN_LEFT, !IsDlgButtonChecked(hwnd, ARROW_DOWN_LEFT)); if (IsDlgButtonChecked(hwnd, ARROW_DOWN_LEFT)) currentDir = ARROW_DOWN_LEFT; break;
		case ARROW_DOWN: CheckDlgButton(hwnd, ARROW_DOWN, !IsDlgButtonChecked(hwnd, ARROW_DOWN)); if (IsDlgButtonChecked(hwnd, ARROW_DOWN)) currentDir = ARROW_DOWN; break;
		case ARROW_DOWN_RIGHT: CheckDlgButton(hwnd, ARROW_DOWN_RIGHT, !IsDlgButtonChecked(hwnd, ARROW_DOWN_RIGHT)); if (IsDlgButtonChecked(hwnd, ARROW_DOWN_RIGHT)) currentDir = ARROW_DOWN_RIGHT; break;
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{

	//Initialize memory globals constant depending on game version
	Memory::create();
	Memory* memory = memory->get();

	if (wcscmp(lpCmdLine, L"-nogui") == 0)
	{
		// if -nogui is passed as argument, just keep saved settings and re-randomize using them
		std::wcout << L"Running in nogui mode" << std::endl;
		// SEED
		randomizer->seed = memory->ReadPanelData<int>(0x00064, BACKGROUND_REGION_COLOR + 12);
		// HARD
		hard = (memory->ReadPanelData<int>(0x00182, BACKGROUND_REGION_COLOR + 12) > 0);
		// DOUBLE
		randomizer->doubleMode = (memory->ReadPanelData<int>(0x0A3B2, BACKGROUND_REGION_COLOR + 12) > 0);

		// COLORBLIND
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
				randomizer->colorblind = true;
			}
			configFile.close();
		}
		randomizer->seedIsRNG = false;

		std::wcout << L"Randomizing..." << std::endl;

		memory->ClearOffsets();
		randomizer->AdjustSpeed();

		if (hard) randomizer->GenerateHard(hwndLoadingText);
		else randomizer->GenerateNormal(hwndLoadingText);

		std::wcout << L"Done !" << std::endl;

		return 0;
	}

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
	int lastSeed = memory->ReadPanelData<int>(0x00064, BACKGROUND_REGION_COLOR + 12);
	hard = (memory->ReadPanelData<int>(0x00182, BACKGROUND_REGION_COLOR + 12) > 0);
	doubleMode = (memory->ReadPanelData<int>(0x0A3B2, BACKGROUND_REGION_COLOR + 12) > 0);

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
	if (hard) SendMessage(hwndExpert, BM_SETCHECK, BST_CHECKED, 1);
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
	if (doubleMode) SendMessage(hwndDoubleMode, BM_SETCHECK, BST_CHECKED, 1);

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
			colorblind = true;
			CheckDlgButton(hwnd, IDC_COLORBLIND, true);
		}
		configFile.close();
	}

	ShowWindow(hwndLoadingText, SW_HIDE);

	//---------------------Debug/editing controls (debug mode only)---------------------

	if (DEBUG) {
		_panel = std::make_shared<Panel>();
		CreateWindow(L"STATIC", L"Col/Row:",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			160, 330, 100, 26, hwnd, NULL, hInstance, NULL);
		hwndCol = CreateWindow(MSFTEDIT_CLASS, L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
			220, 330, 50, 26, hwnd, NULL, hInstance, NULL);
		hwndRow = CreateWindow(MSFTEDIT_CLASS, L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER,
			270, 330, 50, 26, hwnd, NULL, hInstance, NULL);
		SetWindowText(hwndCol, L"0");
		SetWindowText(hwndRow, L"0");

		hwndElem = CreateWindow(L"COMBOBOX", L"",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWN | CBS_HASSTRINGS,
			160, 360, 150, 300, hwnd, NULL, hInstance, NULL);

		const int NUM_ELEMS = 17;
		TCHAR elems[NUM_ELEMS][24] =
		{
			TEXT("Stone"), TEXT("Star"), TEXT("Eraser"), TEXT("Shape"),
			TEXT("Triangle 1"), TEXT("Triangle 2"), TEXT("Triangle 3"),
			TEXT("Arrow 1"), TEXT("Arrow 2"), TEXT("Arrow 3"),
			TEXT("Start"), TEXT("Exit"), TEXT("Gap (Row)"), TEXT("Gap (Column)"),
			TEXT("Dot (Intersection)"), TEXT("Dot (Row)"), TEXT("Dot (Column)"),
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
			TEXT("Black"), TEXT("White"), TEXT("Red"), TEXT("Orange"), TEXT("Yellow"), TEXT("Green"),
			TEXT("Cyan"), TEXT("Blue"), TEXT("Purple"), TEXT("Magenta"), TEXT("None"), TEXT("???")
		};
		memset(&option, 0, sizeof(option));
		for (int i = 0; i < NUM_COLORS; i++)
		{
			wcscpy_s(option, sizeof(option) / sizeof(TCHAR), (TCHAR*)colors[i]);
			SendMessage(hwndColor, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)option);
		}
		SendMessage(hwndColor, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		CreateWindow(L"BUTTON", L"Place Symbol",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			160, 420, 150, 26, hwnd, (HMENU)IDC_ADD, hInstance, NULL);
		CreateWindow(L"BUTTON", L"Remove Symbol",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			160, 450, 150, 26, hwnd, (HMENU)IDC_REMOVE, hInstance, NULL);
		CreateWindow(L"BUTTON", L"Test",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			160, 530, 150, 26, hwnd, (HMENU)IDC_TEST, hInstance, NULL);

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

		CreateWindow(L"STATIC", L"Direction:",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | SS_LEFT,
			50, 560, 90, 16, hwnd, NULL, hInstance, NULL);

		for (int x = 0; x < 3; x++) {
			for (int y = 0; y < 3; y++) {
				if (x == 1 && y == 1) continue;
				CreateWindow(L"BUTTON", L"",
					WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
					50 + x * 15, 580 + y * 15, 12, 12, hwnd, (HMENU)directions[x + y * 3], hInstance, NULL);
			}
		}

		if (_panel->symmetry == Panel::Symmetry::Horizontal || _panel->symmetry == Panel::Symmetry::Rotational)
			CheckDlgButton(hwnd, IDC_SYMMETRYX, TRUE);
		if (_panel->symmetry == Panel::Symmetry::Vertical || _panel->symmetry == Panel::Symmetry::Rotational)
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
