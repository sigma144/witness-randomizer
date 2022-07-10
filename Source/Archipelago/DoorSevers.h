#pragma once

#include <map>
#include <vector>

#define TARGET 0x2B4

#define PANEL 0
#define CABLE 1
#define MULTIPANEL 2
#define PRESSURE_PLATE 3

#define CABLE_TARGET_0 0xCC
#define CABLE_TARGET_1 0xD0
#define CABLE_TARGET_2 0xD4

#define CABLE_COLOR_R 0x158
#define CABLE_COLOR_G 0x15C
#define CABLE_COLOR_B 0x160

#define MULTIPANEL_TARGET 0xD4
#define MOUNT_PARENT_ID 0x80

#define PRESSURE_PLATE_TARGET 0x170

#define ENTITY_NAME 0x58

struct Connection
{
	int id;
	int entity_type;
	int target_no;
};

inline std::map<int, std::vector<Connection>> severTargetsById = {
	//{0x03BA2, {{0x03BA7, CABLE, 0 }}}, // Tutorial Back Left -> Door to Outpost 1

	{0x03BA2, {{0x03BA7, CABLE, ENTITY_NAME }}}, // Tutorial Back Left -> Door to Outpost 1
	{0x0A170, {{0x0A171, PANEL, TARGET }}}, // Outside Tutorial Door to Outpost
	{0x04CA3, {{0x04CA4, PANEL, TARGET }}}, // Outside Tutorial Exit Door from Outpost

	{0x01A29, {{0x01A54, PANEL, TARGET }, /*{0x01A54, PANEL, MOUNT_PARENT_ID}*/}}, // Glass Factory Entry Door
	{0x0D7ED, {{0x04AC9, CABLE, CABLE_TARGET_1 }, /*{0x0005A, MULTIPANEL, MOUNT_PARENT_ID}*/}}, // Glass Factory Back Wall
	{0x17F3E, {{0x000B0, PANEL, TARGET }, /*{0x000B0, PANEL, MOUNT_PARENT_ID }*/}}, // Symmetry Island Door to Symmetry Island Lower
	{0x18269, {{0x1C349, PANEL, TARGET }, /*{0x1C349, PANEL, MOUNT_PARENT_ID }*/}}, // Symmetry Island Door to Symmetry Island Upper

	{0x09FEE, {{0x0C339, PANEL, TARGET }}}, // Desert Door to Desert Flood Light Room
	{0x0C2C3, {{0x0A02D, PANEL, TARGET }}}, // Desert Door to Pond Room
	{0x0A24B, {{0x0A249, PANEL, TARGET }}}, // Desert Door to Water Levels Room
	{0x0C316, {{0x18076, PANEL, TARGET }}}, // Desert Door to Elevator Room

	{0x09D6F, {{0x09E57, PANEL, TARGET }, /*{0x09E57, PANEL, MOUNT_PARENT_ID }*/ }}, // Quarry Door to Quarry 1
	{0x17C07, {{0x17C09, PANEL, TARGET }, /*{0x17C09, PANEL, MOUNT_PARENT_ID }*/ }}, // Quarry Door to Quarry 2

	{0x02010, {{0x01D8D, MULTIPANEL, MULTIPANEL_TARGET }, {0x01D8D, MULTIPANEL, MOUNT_PARENT_ID }}}, // Quarry Door to Mill
	{0x275FF, {{0x275ED, PANEL, TARGET }}}, // Quarry Mill Ground Floor Shortcut Door
	{0x17CE8, {{0x17CAC, PANEL, TARGET }}}, // Quarry Mill Door to Outside Quarry Stairs
	{0x0368A, {{0x03677, PANEL, TARGET }}}, // Quarry Mill Stairs

	{0x2769B, {{0x04D70, CABLE, CABLE_TARGET_0 }}}, // Quarry Boathouse Boat Staircase
	{0x17C50, {{0x021AE, PANEL, TARGET }}}, // Quarry Boathouse First Barrier
	{0x3865F, {{0x38663, PANEL, TARGET }}}, // Quarry Boathouse Second Barrier

	{0x19B24, {{0x0A3A9, PRESSURE_PLATE, PRESSURE_PLATE_TARGET}, /*{0x334DB, PANEL, MOUNT_PARENT_ID}, {0x334DC, PANEL, MOUNT_PARENT_ID},*/ {0x334DB, PANEL, ENTITY_NAME}, {0x334DC, PANEL, ENTITY_NAME}}}, // Shadows Timed Door
	{0x194B2, {{0x1972F, PANEL, TARGET }}}, // Shadows Laser Right Door
	{0x19665, {{0x197E5, PANEL, TARGET }}}, // Shadows Laser Left Door

	{0x19865, {{0x319AC, CABLE, CABLE_TARGET_1 }, {0x0ABEA, CABLE, CABLE_TARGET_1 }}}, // Shadows Barrier to Quarry
	{0x1855B, {{0x319FA, CABLE, CABLE_TARGET_0 }, /*{0x319F9, CABLE, CABLE_TARGET_1}, {0x319F9, CABLE, CABLE_TARGET_2},*/ {0x1C347, CABLE, CABLE_TARGET_1}, {0x1C347, CABLE, CABLE_TARGET_2}}}, // Shadows Ledge Barrier

	{0x01954, {{0x00139, PANEL, TARGET }, /*{0x00139, PANEL, MOUNT_PARENT_ID}*/ }}, // Keep Hedge Maze 1 Exit
	{0x018CE, {{0x00521, PANEL, TARGET }, /*{0x00139, PANEL, MOUNT_PARENT_ID}*/ }}, // Shortcut
	{0x019D8, {{0x019DC, PANEL, TARGET }, /*{0x019DC, PANEL, MOUNT_PARENT_ID}*/ }}, // Keep Hedge Maze 2 Exit
	{0x019B5, {{0x019DC, PANEL, TARGET }, /*{0x019DC, PANEL, MOUNT_PARENT_ID}*/ }}, // Shortcut
	{0x019E6, {{0x019E7, PANEL, TARGET }, /*{0x019E7, PANEL, MOUNT_PARENT_ID}*/ }}, // Keep Hedge Maze 3 Exit
	{0x0199A, {{0x019E7, PANEL, TARGET }, /*{0x019E7, PANEL, MOUNT_PARENT_ID}*/ }}, // Shortcut
	{0x01A0E, {}}, // Keep Hedge Maze 4 Exit

	{0x01BEC, {{0x033EA, PANEL, TARGET }}}, // Keep Pressure Plates 1
	{0x01BEA, {{0x01BE9, PANEL, TARGET }}}, // Keep Pressure Plates 2
	{0x01CD5, {{0x01CD3, PANEL, TARGET }}}, // Keep Pressure Plates 3
	{0x01D40, {{0x01D3F, PANEL, TARGET }}}, // Keep Pressure Plates 4

	{0x04F8F, {{0x0361B, PANEL, TARGET }}}, // Keep Tower Shortcut

	{0x09E3D, {{0x09E49, PANEL, TARGET }}}, // Keep Shortcut to Shadows

	{0x0364E, {{0x03713, PANEL, TARGET }}}, // Monastery Shortcut
	{0x0C128, {{0x00CAE, CABLE, CABLE_TARGET_0 }}}, // Monastery Left Door
	{0x0C153, {{0x00C92, PANEL, TARGET }}}, // Monastery Right Door
	{0x03750, {{0x38F22, CABLE, CABLE_TARGET_1 }}}, // Monastery Door to Garden

	{0x034F5, {{0x0351C, CABLE, CABLE_TARGET_0 }}}, // Town Wooden Roof Staircase
	{0x28A61, {{0x28998, PANEL, TARGET }}}, // Town Tinted Door to RGB House
	{0x2897B, {{0x034EB, CABLE, ENTITY_NAME }}}, // Town Tinted Door to RGB House
	{0x03BB0, {{0x28A0D, PANEL, TARGET }, {0x28A0D, PANEL, MOUNT_PARENT_ID }}}, // Town Door to Church
	{0x28AA2, {{0x28A79, PANEL, TARGET }}}, // Maze Staircase
	{0x1845B, {{0x17F5F, PANEL, TARGET }, /*{0x17F5F, PANEL, MOUNT_PARENT_ID }*/}}, // Windmill Door

	{0x27798, {{0x28ACC, PANEL, TARGET}}}, // Blue Panels Door
	{0x27799, {{0x28A69, PANEL, TARGET}}}, // Square Avoid Door
	{0x2779A, {{0x28B39, PANEL, TARGET}}}, // Environmental Set Door
	{0x2779C, {{0x28AD9, PANEL, TARGET}}}, // Eraser Set Door

	{0x17F88, {{0x17F89, PANEL, TARGET}}}, // Door to Front of Theater
	{0x0A16D, {{0x0A168, PANEL, TARGET}}}, // Door to Cargo Box Left)
	{0x3CCDF, {{0x33AB2, PANEL, TARGET}}}, // Door to Cargo Box Right) 

	{0x3873B, {{0x337FA, PANEL, TARGET}}}, // Jungle Shortcut to River
	{0x0CF2A, {{0x17CAA, PANEL, TARGET}, {0x17CAA, PANEL, MOUNT_PARENT_ID}}}, // Jungle Shortcut to Monastery Garden
	{0x1475B, {{0x17CAB, PANEL, TARGET}}}, // Jungle Popup Wall

	{0x0C2A4, {{0x17C2E, PANEL, TARGET}}}, // Bunker Entry
	{0x17C79, {{0x0A099, PANEL, TARGET}}}, // Bunker Tinted Door
	{0x0C2A3, {{0x0A01F, PANEL, TARGET}}}, // Door to UV
	{0x0A08D, {{0x17E67, PANEL, TARGET}}}, // Door to Elevator

	{0x00C1C, {{0x0056E, PANEL, TARGET}}}, // Swamp Entry
	{0x184B7, {{0x00BFC, CABLE, CABLE_TARGET_1}}}, // Door to Broken Shapers
	{0x38AE6, {{0x17C0E, PANEL, TARGET}}}, // Platform Shortcut
	{0x04B7F, {{0x00006, PANEL, TARGET}}}, // Cyan Pump

	{0x18507, {{0x00bc9, CABLE, CABLE_TARGET_0}}}, // Door to Broken Shapers
	{0x183F2, {{0x00596, PANEL, TARGET}}}, // Red Pump
	{0x305D5, {{0x014D1, PANEL, TARGET}}}, // Red Exit

	{0x18482, {{0x00E3A, PANEL, TARGET}}}, // Blue Pump
	{0x0A1D6, {{0x00E3A, PANEL, TARGET}}}, // Purple Pump

	{0x2D880, {{0x17C02, PANEL, TARGET}}}, //Near Laser Shortcut


	{0x0C309, {{0x0288C, PANEL, TARGET}}}, // Treehouse Entry Door 1
	{0x0C310, {{0x02886, PANEL, TARGET}, {0x02886, PANEL, MOUNT_PARENT_ID}}}, // Treehouse Entry Door 2
	{0x0A181, {{0x0A182, PANEL, TARGET}}}, // Treehouse Entry Door 3
	{0x0C323, {{0x034FC, PRESSURE_PLATE, PRESSURE_PLATE_TARGET}, /*{0x334DB, PANEL, MOUNT_PARENT_ID}, {0x334DC, PANEL, MOUNT_PARENT_ID},*/ {0x2700B, PANEL, ENTITY_NAME}, {0x17CBC, PANEL, ENTITY_NAME}} }, // Treehouse Timed Door
	{0x0C32D, {{0x037FF, PANEL, TARGET}}}, // Treehouse Draw Bridge

	{0x09E54, {{0x09EAF, PANEL, TARGET}, {0x09F6E, PANEL, TARGET}, {0x09E6B, PANEL, TARGET}, {0x09E7B, PANEL, TARGET}} }, // Mountain Door to 2nd Layer

	{0x09FFB, {{0x09FD8, PANEL, TARGET}} }, // Mountain 2nd layer staircase near
	{0x09E07, {{0x09FCD, CABLE, CABLE_TARGET_0}, {0x09FCD, CABLE, CABLE_COLOR_R}, {0x09FCD, CABLE, CABLE_COLOR_G}, {0x09FCD, CABLE, CABLE_COLOR_B}} }, // Mountain 2nd layer staircase far

	{ 0x09EDD, {{0x09ED8, PANEL, TARGET}, {0x09E86, PANEL, TARGET}} }, // Door to Elevator

	{ 0x09F89, {{0x09FDA, PANEL, TARGET}} }, // Metapuzzle Glass Doors

	{ 0x0C141, {{0x01983, PANEL, TARGET}, {0x01987, PANEL, TARGET}} }, // Final Room Entry Door

	{ 0x17F33, {{0x17FA2, PANEL, TARGET}, {0x0A3AF, PRESSURE_PLATE, PRESSURE_PLATE_TARGET}} }, // Rock

	{ 0x2D77D, {{0x00FF8, PANEL, TARGET}} }, // Door to Caves
	{ 0x019A5, {{0x09DD5, PANEL, TARGET}} }, // Pillar Door
	{ 0x2D73F, {{0x021D7, PANEL, TARGET}, {0x021D7, PANEL, MOUNT_PARENT_ID}} }, // Caves Mountain Shortcut
	{ 0x2D859, {{0x17CF2, PANEL, TARGET}} }, // Caves Swamp Shortcut
	{ 0x0A19A, {{0x0A16E, PANEL, TARGET}} }, // Door to Challenge

	{ 0x0348A, {{0x039B4, PANEL, TARGET}} }, // Challenge to Theater Walkway

	{ 0x27739, {{0x27732, PANEL, TARGET}} }, // Walkway to Theater
	{ 0x27263, {{0x2773D, PANEL, TARGET}} }, // Walkway to Desert
	{ 0x09E87, {{0x09E85, PANEL, TARGET}} }, // Walkway to Town
};