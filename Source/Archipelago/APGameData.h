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

#define CABLE_POWER 0xC4

#define LASER_TARGET 0xD0

#define CABLE_COLOR_R 0x158
#define CABLE_COLOR_G 0x15C
#define CABLE_COLOR_B 0x160

#define DOOR_OPEN 0x1E4

#define MULTIPANEL_TARGET 0xD4
#define MOUNT_PARENT_ID 0x80

#define PRESSURE_PLATE_TARGET 0x170

#define ENTITY_NAME 0x58

#define AUDIO_LOG_IS_PLAYING 0xE4

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

	{0x03307, {{0x0330F, CABLE, CABLE_TARGET_0}}}, // Orchard Mid Gate
	{0x03313, {{0x032FF, PANEL, TARGET}}}, // Orchard Mid Gate

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

	{0x19B24, {{0x0A3A9, PRESSURE_PLATE, PRESSURE_PLATE_TARGET}, {0x334DB, PANEL, TARGET}, {0x334DC, PANEL, TARGET}, {0x334DB, PANEL, ENTITY_NAME}, {0x334DC, PANEL, ENTITY_NAME}}}, // Shadows Timed Door
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
	{0x01D40, {{0x09E11, CABLE, CABLE_TARGET_0 }}}, // Keep Pressure Plates 4

	{0x04F8F, {{0x0361B, PANEL, TARGET }}}, // Keep Tower Shortcut

	{0x09E3D, {{0x09E49, PANEL, TARGET }}}, // Keep Shortcut to Shadows

	{0x0364E, {{0x03713, PANEL, TARGET }}}, // Monastery Shortcut
	{0x0C128, {{0x00CAE, CABLE, CABLE_TARGET_0 }}}, // Monastery Left Door
	{0x0C153, {{0x00C92, PANEL, TARGET }}}, // Monastery Right Door
	{0x03750, {{0x38F22, CABLE, CABLE_TARGET_1 }}}, // Monastery Door to Garden

	{0x0A0C9, {{0x0A0C8, PANEL, TARGET }}}, // Cargo Box
	{0x034F5, {{0x0351C, CABLE, CABLE_TARGET_0 }}}, // Town Wooden Roof Staircase
	{0x28A61, {{0x28998, PANEL, TARGET }}}, // Town Tinted Door to RGB House
	{0x2897B, {{0x034EB, CABLE, CABLE_TARGET_0}, {0x034EB, CABLE, CABLE_COLOR_R}, {0x034EB, CABLE, CABLE_COLOR_G}, {0x034EB, CABLE, CABLE_COLOR_B}} }, // Town RGB Staircase
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
	{0x305D5, {{0x00BED, CABLE, CABLE_TARGET_0}, {0x00BED, CABLE, CABLE_COLOR_R}, {0x00BED, CABLE, CABLE_COLOR_G}, {0x00BED, CABLE, CABLE_COLOR_B}} }, // Red Exit

	{0x18482, {{0x00E3A, PANEL, TARGET}}}, // Blue Pump
	{0x0A1D6, {{0x00E3A, PANEL, TARGET}}}, // Purple Pump

	{0x2D880, {{0x17C02, PANEL, TARGET}}}, //Near Laser Shortcut


	{0x0C309, {{0x0288C, PANEL, TARGET}}}, // Treehouse Entry Door 1
	{0x0C310, {{0x02886, PANEL, TARGET}, {0x02886, PANEL, MOUNT_PARENT_ID}}}, // Treehouse Entry Door 2
	{0x0A181, {{0x0A182, PANEL, TARGET}}}, // Treehouse Entry Door 3
	{0x0C323, {{0x034FC, PRESSURE_PLATE, PRESSURE_PLATE_TARGET}, {0x2700B, PANEL, TARGET}, {0x17CBC, PANEL, TARGET}, {0x2700B, PANEL, ENTITY_NAME}, {0x17CBC, PANEL, ENTITY_NAME}} }, // Treehouse Timed Door
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




	{ 0x01A54 , {} }, // Glass Factory Entry Door (Panel)
	{ 0x000B0 , {{0x0343A, PANEL, TARGET}} }, // Symmetry Island Lower (Panel)
	{ 0x1C349, {}}, // Symmetry Island Upper (Panel)

	{ 0x0C339 , {{0x09F94, PANEL, TARGET}} }, // Desert Pond Room (Panel)
	{ 0x1C2DF , {} }, // Desert Flood Room Controls
	{ 0x1831E , {} }, // Desert Flood Room Controls
	{ 0x1C260 , {} }, // Desert Flood Room Controls
	{ 0x1831C , {} }, // Desert Flood Room Controls
	{ 0x1C2F3 , {} }, // Desert Flood Room Controls
	{ 0x1831D , {} }, // Desert Flood Room Controls
	{ 0x1C2B1 , {} }, // Desert Flood Room Controls
	{ 0x1831B , {} }, // Desert Flood Room Controls

		//CONTROL PANEL RANDO
		
	{ 0x01E5A, {} }, // Mill Gate Left
	{ 0x01E59 , {} }, // Mill Gate Right
	{ 0x03678 ,{} }, // Quarry Mill Lower Ramp Control
	{ 0x03675 , {} }, // Quarry Mill Upper Ramp Control
	{ 0x03679, {{0x014E8, PANEL, TARGET}} }, // Quarry Mill Lower Elevator Control
	{ 0x03676, {} }, // Quarry Mill Upper Elevator Control
	
	{ 0x03852, { {0x021C1, MULTIPANEL, MULTIPANEL_TARGET}} }, // Quarry Boathouse Ramp Height
	{ 0x03858, {} }, // Quarry Boathouse Ramp Horizontal	 

	{ 0x334DB, {}}, // Shadows Timer
	{ 0x334DC, {}}, // Shadows Timer
		
	{ 0x00B10, {{0x00CAE, CABLE, CABLE_TARGET_1 }} }, // Monastery Left
	{ 0x00C92, {} }, // Monastery Right

	{ 0x28998, {} }, // Town RGB House
	{ 0x28A0D, {} }, // Town Church
	{ 0x28A79, {} }, // Town Maze Panel
	{ 0x17F5F, {} }, // Windmill

	{ 0x0288C, {} }, // Treehouse First Door
	{ 0x02886, {} }, // Treehouse Second Door
	{ 0x0A182, {} }, // Treehouse Third Door
	{ 0x2700B, {} }, // Treehouse Timer 1
	{ 0x334DC, {} }, // Treehouse Timer 2
	{ 0x17CBC, {} }, // Treehouse Drop-Down

	{ 0x17CAB, {{0x002C7, PANEL, TARGET}} }, // Jungle Popup Wall
	{ 0x17C2E, {} }, // Bunker Entry Door
	{ 0x0A099, {{0x09DAF, PANEL, TARGET}} }, // Bunker Tinted Door
	{ 0x0A079, {} }, // Bunker Elevator
	{ 0x0056E, {} }, // Swamp Entry 

	{ 0x00609, {} }, // Swamp Slide Bridge
	{ 0x18488, {} }, // Swamp Slide Bridge
	{ 0x181F5, {} }, // Swamp Rotating Bridge
	{ 0x17C0A, {} }, // Swamp Maze Control
	{ 0x17E07, {} }, // Swamp Maze Control

	{ 0x17CDF, {} }, // Boat
	{ 0x17CC8, {} }, // Boat
	{ 0x17CA6, {} }, // Boat
	{ 0x09DB8, {} }, // Boat
	{ 0x17C95, {} }, // Boat
	{ 0x0A054, {} }, // Boat


	// Lasers
	{ 0x00509, {{0x0360D, PANEL, TARGET }} }, // Symmetry
	{ 0x012FB, {{0x03608, PANEL, TARGET }} }, // Desert
	{ 0x01539, {{0x03612, PANEL, TARGET }} }, // Quarry
	{ 0x181B3, {{0x19650, PANEL, TARGET }} }, // Shadows
	{ 0x014BB, {{0x0360E, PANEL, TARGET },{0x03317, PANEL, TARGET }} }, // Keep
	{ 0x17C65, {{0x17CA4, PANEL, TARGET }} }, // Monastery
	{ 0x032F9, {{0x032F5, PANEL, TARGET }} }, // Town
	{ 0x00274, {{0x03616, PANEL, TARGET }} }, // Jungle
	{ 0x0C2B2, {{0x09DE0, PANEL, TARGET }} }, // Bunker
	{ 0x00BF6, {{0x03615, PANEL, TARGET }} }, // Swamp
	{ 0x028A4, {{0x03613, PANEL, TARGET }} }, // Treehouse
};

inline std::map<int, std::vector<int>> doorCollisions = {
	{ 0x0, { 0x271b, 0x182, 0x302ba, }},
	{ 0x85, { 0x2a6, 0xbff2,} },
	{ 0x5a2, { 0x38756, 0x181f5, 0x2d874, 0x2d873, 0xb8e,} },
	{ 0x61a, { 0x609, 0x2d774, 0x61b,} },
	{ 0xb0c, { 0x37a3,} },
	{ 0xc1c, { 0x56e, 0x2677a,} },
	{ 0x12c8, { 0x12d7,} },
	{ 0x1300, {} },
	{ 0x1317, { 0x3608, 0x277ab, 0x277a8, 0x277a6, 0x277aa, 0x277a9, 0x277a7,} },
	{ 0x18b3, {} },
	{ 0x18ce, { 0x1aaeb,} },
	{ 0x1954, { 0x31ccb, 0x139, 0x1418d,} },
	{ 0x199a, { 0x1aaed,} },
	{ 0x19a5, { 0x33782,} },
	{ 0x19b5, { 0x1aaec,} },
	{ 0x19d8, { 0x3345e, 0x19dc, 0x1418c,} },
	{ 0x19e6, { 0x335d7, 0x19e7, 0x14473,} },
	{ 0x1a0e, { 0x335da, 0x1a0f, 0x14470,} },
	{ 0x1a29, { 0x1a54, 0x1fd5f, 0x2fa23, 0x3937c,} },
	{ 0x1bea, { 0x1abb2,} },
	{ 0x1bec, { 0x1abb6,} },
	{ 0x1cd5, { 0x1abb4,} },
	{ 0x1d40, { 0x1447a,} },
	{ 0x2010, { 0x1e59, 0x1e5a, 0x1d8d, 0x18169,} },
	{ 0x21ba, { 0x4016,} },
	{ 0x270f, {} },
	{ 0x3307, { 0x330a,} },
	{ 0x3313, { 0x274cf,} },
	{ 0x33ac, { 0x339e,} },
	{ 0x33d0, { 0x1febc, 0x33d4, 0x17d11,} },
	{ 0x3444, { 0xcc7b, 0x17da1,} },
	{ 0x348a, { 0x31eca, 0x392d1, 0x39b4, 0x39df,} },
	{ 0x34f5, { 0x2dd70, 0x3cd4d,} },
	{ 0x3506, { 0x3481,} },
	{ 0x3538, { 0x3535,} },
	{ 0x354a, { 0x3542,} },
	{ 0x3614, { 0x356b,} },
	{ 0x364e, { 0xcf31, 0x364f,} },
	{ 0x368a, { 0x368f,} },
	{ 0x3703, { 0x3702,} },
	{ 0x3750, { 0xcf2c, 0x3758,} },
	{ 0x3839, { 0x3838,} },
	{ 0x3ba2, { 0x3ba3,} },
	{ 0x3bb0, { 0x28a0d, 0x35435,} },
	{ 0x4b7f, { 0xa2c4, 0xa2c1,}},
	{ 0x4ca3, { 0x3381e, 0x4ca4, 0x4ca6,} },
	{ 0x4d37, {} },
	{ 0x4d75, { 0x4d7a,} },
	{ 0x4f8f, { 0x1abb5,} },
	{ 0x9d6f, { 0x9e57, 0xc471,} },
	{ 0x9da0, {} },
	{ 0x9e02, {} },
	{ 0x9e03, {} },
	{ 0x9e07, { 0xa06d,} },
	{ 0x9e15, {} },
	{ 0x9e16, {} },
	{ 0x9e1d, {} },
	{ 0x9e1e, { 0x3387c,} },
	{ 0x9e26, {} },
	{ 0x9e3d, { 0x1418f,} },
	{ 0x9e54, {} },
	{ 0x9e87, { 0x33881, 0x9e85, 0xa0f1,} },
	{ 0x9e98, {} },
	{ 0x9ed4, {} },
	{ 0x9edd, {} },
	{ 0x9ee3, {} },
	{ 0x9eec, { 0x3375a, 0x9eeb,} },
	{ 0x9f10, {} },
	{ 0x9f11, {} },
	{ 0x9f21, {} },
	{ 0x9f2a, {} },
	{ 0x9f2b, {} },
	{ 0x9f3c, {} },
	{ 0x9f3d, {} },
	{ 0x9f3e, {} },
	{ 0x9f89, { 0x38aa6,} },
	{ 0x9f8a, { 0x38aa5,} },
	{ 0x9f95, { 0x9f94,} },
	{ 0x9fa1, { 0x38c79, 0x9f92,} },
	{ 0x9fa2, { 0x1c344, 0x1c343, 0x335aa,} },
	{ 0x9fee, { 0xc31b,} },
	{ 0x9ff9, {} },
	{ 0x9ffb, { 0xa176,} },
	{ 0x9ffc, {} },
	{ 0xa001, {} },
	{ 0xa003, {} },
	{ 0xa00b, {} },
	{ 0xa00f, {} },
	{ 0xa01a, {} },
	{ 0xa01e, {} },
	{ 0xa069, {} },
	{ 0xa06a, {} },
	{ 0xa06b, {} },
	{ 0xa06c, {} },
	{ 0xa070, {} },
	{ 0xa071, {} },
	{ 0xa072, {} },
	{ 0xa073, {} },
	{ 0xa074, {} },
	{ 0xa075, {} },
	{ 0xa076, {} },
	{ 0xa077, {} },
	{ 0xa08d, { 0xa092,} },
	{ 0xa0c9, { 0x39a3, 0xa0c8, 0x1414b, 0x14149,} },
	{ 0xa16d, { 0xa168, 0x277ee,} },
	{ 0xa170, { 0x3381c, 0xa171, 0xa17a,} },
	{ 0xa181, { 0xa182, 0xa187,} },
	{ 0xa19a, { 0xa16e, 0x27074,} },
	{ 0xa1d6, { 0x26a4b, 0x4c71,} },
	{ 0xa24b, { 0xc347,} },
	{ 0xa2df, { 0x32ed,} },
	{ 0xc128, { 0x17df7, 0x17d17,} },
	{ 0xc141, { 0x1983, 0x1987, 0x305d4, 0x305d3,} },
	{ 0xc153, { 0x17d98, 0x17d9a,} },
	{ 0xc160, {} },
	{ 0xc161, {} },
	{ 0xc164, {} },
	{ 0xc179, {} },
	{ 0xc19d, {} },
	{ 0xc29d, { 0x34bc5, 0x17e63, 0x17e67, 0x34bc6, 0xa08c,} },
	{ 0xc29e, {} },
	{ 0xc2a3, { 0xa093,} },
	{ 0xc2a4, { 0x33484, 0x17c2e, 0x17c5d,} },
	{ 0xc2bc, {} },
	{ 0xc2c3, { 0xc340,} },
	{ 0xc309, { 0x288c, 0xc30f, 0x2219,} },
	{ 0xc310, { 0x335f4, 0x2886, 0xc314, 0x360b, 0x360c, 0x35c4,} },
	{ 0xc316, { 0xbfb0,} },
	{ 0xc323, { 0xc31f, 0x362d, 0x3631,} },
	{ 0xc32d, { 0xc330, 0x3886d,} },
	{ 0xcf2a, { 0x31a4f, 0x17caa, 0xc3e5,} },
	{ 0xd7ed, { 0x62, 0x5c, 0x59, 0x87, 0x86, 0x5a, 0x4d04,} },
	{ 0x1467c, {} },
	{ 0x14685, {} },
	{ 0x14724, {} },
	{ 0x14733, {} },
	{ 0x14734, {} },
	{ 0x1475b, { 0xc41, 0xc3f, 0x26d, 0x26e, 0x26f, 0x14b2, 0x26c, 0xd610, 0xd612,} },
	{ 0x15287, { 0x15add, 0x18025,} },
	{ 0x17bb4, { 0xafb, 0x3557, 0x353e, 0x34fb, 0x34dc,} },
	{ 0x17c07, { 0x17c09, 0x17c92,} },
	{ 0x17c50, { 0x2724a,} },
	{ 0x17c6a, {} },
	{ 0x17c79, { 0x3798,} },
	{ 0x17cc1, { 0x2bf42, 0x395b6, 0x396b4, 0x17cc4, 0x17d2b,} },
	{ 0x17ce8, { 0x17d86,} },
	{ 0x17e1d, {} },
	{ 0x17e57, {} },
	{ 0x17e66, {} },
	{ 0x17e70, {} },
	{ 0x17e74, { 0x17f86, 0x2d875,} },
	{ 0x17ec0, {} },
	{ 0x17ec5, {} },
	{ 0x17ec6, {} },
	{ 0x17ecd, {} },
	{ 0x17f02, {} },
	{ 0x17f33, { 0xc197,} },
	{ 0x17f3e, { 0xb0, 0x18272, 0x18271,} },
	{ 0x17f88, { 0x17f89, 0x17f9e, 0x2787a,} },
	{ 0x17f90, {} },
	{ 0x1802c, { 0x180a9, 0x2d878,} },
	{ 0x18269, { 0x1c349, 0x28959, 0x1826a,} },
	{ 0x183f2, { 0x26a4a, 0x18400,} },
	{ 0x1845b, { 0x17f5f, 0x2662d,} },
	{ 0x18482, { 0x3863, 0x1847a,} },
	{ 0x184b1, {} },
	{ 0x184b7, { 0x2674d,} },
	{ 0x184ca, {} },
	{ 0x184e3, {} },
	{ 0x184fa, {} },
	{ 0x18507, { 0x26762,} },
	{ 0x18511, {} },
	{ 0x1852e, {} },
	{ 0x18545, {} },
	{ 0x1855b, { 0x3302,} },
	{ 0x1855c, {} },
	{ 0x1856d, {} },
	{ 0x1859b, {} },
	{ 0x194b2, { 0x3398b, 0x3399a, 0x3399b, 0x33988, 0x3399c, 0x18460, 0x33985, 0x3399d, 0x3399e, 0x3398a, 0x339a0, 0x3399f, 0x33989, 0x3398e,} },
	{ 0x19665, { 0x3398c, 0x33999, 0x33990, 0x33998, 0x3398f, 0x33997, 0x1845a, 0x33991, 0x33996, 0x33995, 0x33992, 0x33994, 0x3398d, 0x33993,} },
	{ 0x19865, {} },
	{ 0x19ade, {} },
	{ 0x19b24, { 0x334dc, 0x334db, 0x19800,} },
	{ 0x27163, { 0x27326,} },
	{ 0x27263, { 0x2773d, 0x27294,} },
	{ 0x272dd, { 0x3852, 0x338cf, 0x338ce, 0x35c0, 0x338d0,} },
	{ 0x2733d, {} },
	{ 0x2733e, {} },
	{ 0x2733f, {} },
	{ 0x27340, {} },
	{ 0x27341, {} },
	{ 0x27342, {} },
	{ 0x275ff, { 0x27636,} },
	{ 0x2762c, { 0x17e45,} },
	{ 0x2769b, { 0x229f,} },
	{ 0x27739, { 0x27732, 0x2773a,} },
	{ 0x27798, { 0x2782f,} },
	{ 0x27799, { 0x2782e,} },
	{ 0x2779a, { 0x27830,} },
	{ 0x2779c, { 0x27831,} },
	{ 0x288e8, { 0x3657b,} },
	{ 0x288e9, {} },
	{ 0x288f3, { 0x3657d,} },
	{ 0x288f9, {} },
	{ 0x28907, {} },
	{ 0x28942, { 0x3657c,} },
	{ 0x2897b, { 0x289df,} },
	{ 0x2897c, { 0x61e0,} },
	{ 0x289a1, { 0x3362f,} },
	{ 0x289a2, {} },
	{ 0x289ae, { 0x33630,} },
	{ 0x289b0, {} },
	{ 0x289c4, {} },
	{ 0x289ca, {} },
	{ 0x289cd, {} },
	{ 0x289d7, {} },
	{ 0x289da, {} },
	{ 0x289dc, {} },
	{ 0x289dd, {} },
	{ 0x28a61, { 0x28998,} },
	{ 0x28aa2, { 0x28aa4, 0x3397e,} },
	{ 0x28ad4, {} },
	{ 0x28ade, {} },
	{ 0x28ae2, {} },
	{ 0x28ae8, {} },
	{ 0x28aed, {} },
	{ 0x2d73f, { 0x21d7, 0x2d743, 0x2d741, 0x2d742, 0x2d740,} },
	{ 0x2d77d, { 0x336cc, 0x392ce, 0xff8, 0x2d781,} },
	{ 0x2d859, { 0x17cf2, 0x2d85b,} },
	{ 0x2d880, { 0x17c02, 0x17c05, 0x3a36, 0x2d881,} },
	{ 0x2facf, {} },
	{ 0x2fad4, {} },
	{ 0x2fad6, {} },
	{ 0x2fad7, {} },
	{ 0x2fada, {} },
	{ 0x2fadb, {} },
	{ 0x2fafb, { 0x2faf6,} },
	{ 0x2fd5e, {} },
	{ 0x2fd76, {} },
	{ 0x2fdae, {} },
	{ 0x2fdaf, {} },
	{ 0x2fdb0, {} },
	{ 0x2fe2d, {} },
	{ 0x2fe94, {} },
	{ 0x2fe95, {} },
	{ 0x305d5, { 0x305d6,} },
	{ 0x307d7, {} },
	{ 0x307d8, {} },
	{ 0x31486, { 0x3930d,} },
	{ 0x31487, { 0x3930f,} },
	{ 0x31488, { 0x39310,} },
	{ 0x3148a, { 0x3930c,} },
	{ 0x314b0, { 0x39324,} },
	{ 0x314e1, { 0x3d2e,} },
	{ 0x314e2, { 0x3d2f,} },
	{ 0x314e3, { 0x3d30,} },
	{ 0x314e4, { 0x3d31,} },
	{ 0x314e5, { 0x3d32,} },
	{ 0x314e6, { 0x3d33,} },
	{ 0x314e7, { 0x3d34,} },
	{ 0x314e8, { 0x3d35,} },
	{ 0x314e9, { 0x3d36,} },
	{ 0x314ea, { 0x3d38,} },
	{ 0x314eb, { 0x3d3a,} },
	{ 0x314ec, { 0x39336,} },
	{ 0x314ed, { 0x39335,} },
	{ 0x314ee, { 0x39334,} },
	{ 0x314ef, { 0x3d40,} },
	{ 0x314f0, { 0x3d41,} },
	{ 0x314f1, { 0x3d42,} },
	{ 0x314f2, { 0x3933e,} },
	{ 0x314f3, { 0x3d92,} },
	{ 0x314f4, { 0x3934c,} },
	{ 0x314f5, { 0x3934b,} },
	{ 0x31511, { 0x3934a,} },
	{ 0x31512, { 0x3930e,} },
	{ 0x31514, { 0x39349,} },
	{ 0x31515, { 0x39348,} },
	{ 0x317fd, { 0x3490, 0x34b2, 0x348e, 0x349c, 0x3498, 0x652b, 0x347e, 0x38bed, 0x6523, 0x6539, 0x347d, 0x6525, 0x65ac, 0x38bee, 0x6524, 0x38bec, 0x6526, 0x6602, 0x6527, 0x65fb, 0x65da, 0x38bef, 0x38bdb, 0x6603, 0x6604, 0x33cd, 0x65ff, 0x62d4, 0x6608, 0x62a5, 0x38bda, 0x6605, 0x650f, 0x6606, 0x6682, 0x347c, 0x629c, 0x652a, 0x6607, 0x668f, 0x38bf0, 0x6687, 0x667f, 0x6231, 0x668e, 0x668d, 0x6529, 0x38bf1, 0x665f, 0x6685, 0x6684, 0x347f, 0x6683, 0x631a, 0x6688, 0x6686, 0x651f, 0x64eb, 0x38bf2, 0x66b2, 0x348b, 0x66aa, 0x6694, 0x66a9, 0x668b, 0x6690, 0x668a, 0x6689, 0x6691, 0x6693, 0x62f4, 0x6692, 0x6520, 0x630e, 0x62d3, 0x6313, 0x66cb, 0x66cc,} },
	{ 0x318a2, { 0x6922, 0x692f, 0x6921, 0x6923, 0x692e, 0x6929, 0x692c, 0x694d, 0x6934, 0x6935, 0x692d, 0x694a, 0x6949, 0x691f, 0x6948, 0x694c,} },
	{ 0x31903, { 0x391a1, 0x68ad, 0x68ae, 0x68d0, 0x68b8, 0x68cf, 0x68c9, 0x68cc, 0x68b7, 0x68ca, 0x68b6, 0x6890, 0x68ac, 0x6891, 0x687d, 0x689f, 0x6892, 0x687e, 0x68a0, 0x6893, 0x687f, 0x68ab, 0x68a1, 0x68a7, 0x68aa, 0x6880, 0x6896, 0x6894, 0x68a2, 0x68a6, 0x6881, 0x6898, 0x68a3, 0x6882, 0x6895, 0x68a8, 0x68a9, 0x68a5, 0x68a4, 0x6883, 0x6899, 0x6884, 0x6897, 0x6887, 0x689a, 0x6886, 0x689b, 0x6888, 0x6889, 0x688c, 0x688b, 0x688d, 0x688a, 0x688e, 0x688f, 0x68d1, 0x68b0,} },
	{ 0x32969, {} },
	{ 0x3296f, {} },
	{ 0x32975, {} },
	{ 0x3297b, {} },
	{ 0x32981, {} },
	{ 0x334ca, {} },
	{ 0x335d6, { 0x3001b, 0x3001e, 0x331d, 0x3533c, 0x302d8, 0xd69d, 0x3036b, 0x3318, 0x30041,} },
	{ 0x33780, {} },
	{ 0x33794, { 0x337cf,} },
	{ 0x33795, { 0x337d1,} },
	{ 0x33796, { 0x337d0,} },
	{ 0x337a0, { 0x337d5,} },
	{ 0x337a1, { 0x337d4,} },
	{ 0x337a4, { 0x337d3,} },
	{ 0x337c1, { 0x337d2,} },
	{ 0x338d4, {} },
	{ 0x338d5, {} },
	{ 0x338d6, {} },
	{ 0x33982, {} },
	{ 0x33aa9, {} },
	{ 0x33aea, {} },
	{ 0x33b38, {} },
	{ 0x34eda, { 0x64, 0x14481,} },
	{ 0x34f46, { 0x337ce,} },
	{ 0x35306, {} },
	{ 0x35308, {} },
	{ 0x36343, {} },
	{ 0x36360, { 0x3c134,} },
	{ 0x3636d, {} },
	{ 0x36373, { 0x3c133,} },
	{ 0x363d5, { 0x3c116,} },
	{ 0x3645a, { 0x2c399,} },
	{ 0x3645b, { 0x2c39c,} },
	{ 0x3865f, { 0x33762,} },
	{ 0x386b7, {} },
	{ 0x3873b, { 0x3873f,} },
	{ 0x38acb, { 0x335ab, 0x3ce36, 0x38a9e, 0x3ce34,} },
	{ 0x38ae6, { 0x17c0d, 0x17c0e, 0x3381, 0x38ae7,} },
	{ 0x38c10, { 0x33958, 0x33959, 0x27837, 0x3395a,} },
	{ 0x391a5, {} },
	{ 0x391de, { 0x33aeb,} },
	{ 0x3ccdf, { 0x33ab2, 0x2787c,} },
	{ 0x3d9ec, {} },
	{ 0x3da76, {} },
	{ 0x3da78, {} },
};

inline std::set<int> allLasers = {
	0x00509,
	0x012FB,
	0x01539,
	0x181B3,
	0x014BB,
	0x17C65,
	0x032F9,
	0x00274,
	0x0C2B2,
	0x00BF6,
	0x028A4,
};

inline std::set<int> knownIrrelevantCollisions = { // We will still update these collisions once for safety.
	0x01e59,
	0x01e5a,
	0x01d8d,
	0x28a0d,
	0x17caa,
	0x21d7,
	0x2d740,
	0x2d741,
	0x2d743,
	0x0360b,
	0x02886,
	0x035c4,
	0x0360c,
};

inline std::set<int> audioLogs = {
	0x3C0F7,0x3C0FD,0x32A00,0x3C0FE,0x338B0,0x338B7,0x338AD,0x338A5,0x338AE,0x338AF,0x338A7,0x338A3,0x338A4,0x3C108,0x338EF,0x336E5,0x338A6,0x3C100,0x3C0F4,0x3C102,0x3C10D,0x3C10E,0x3C10B,0x0074F,0x012C7,0x329FF,0x3C106,0x33AFF,0x011F9,0x00763,0x32A08,0x3C101,0x3C0FF,0x3C103,0x00A0F,0x339A9,0x015C0,0x33B36,0x3C10C,0x32A0E,0x329FE,0x32A07,0x00761,0x3C109,0x33B37,0x3C107,0x3C0F3,0x015B7,0x3C10A,0x32A0A,0x015C1,0x3C12A,0x3C104,0x3C105,0x339A8,0x0050A,0x338BD,0x3C135,0x338C9,0x338D7,0x338C1,0x338CA
};

inline std::map<int, std::string> laserNames = {
	{0x028A4, "Treehouse"},
	{0x00274, "Jungle"},
	{0x032F9, "Town"},
	{0x01539, "Quarry"},
	{0x181B3, "Shadows"},
	{0x0C2B2, "Bunker"},
	{0x00509, "Symmetry"},
	{0x00BF6, "Swamp"},
	{0x014BB, "Keep"},
	{0x012FB, "Desert"},
	{0x17C65, "Monastery"},
};

const inline int AllPuzzles[]{
		0x00293, 0x00295, 0x002C2, 0x0A3B5, 0x0A3B2, 0x03629, // Tutorial
		0x0C373, // Tutorial Patio Floor
		0x0C335, // Tutorial Pillar??
		0x0A171, 0x04CA4, // Tutorial Secret back area
		0x0005D, 0x0005E, 0x0005F, 0x00060, 0x00061, // Tutorial Dots Tutorial
		0x018AF, 0x0001B, 0x012C9, 0x0001C, 0x0001D, 0x0001E, 0x0001F, 0x00020, 0x00021,	// Tutorial Stones Tutorial
		0x00086, 0x00087, 0x00059, 0x00062, 0x0005C, // Symmetry Vertical Symmetry Mazes
		0x0008D, 0x00081, 0x00083, 0x00084, 0x00082, 0x0343A, // Symmetry Rotational Symmetry Mazes
		0x00022, 0x00023, 0x00024, 0x00025, 0x00026, // Symmetry Black Dots
		0x0007C, 0x0007E, 0x00075, 0x00073, 0x00077, 0x00079, // Symmetry Colored Dots
		0x00065, 0x0006D, 0x00072, 0x0006F, 0x00070, 0x00071, 0x00076, // Symmetry Fading Lines
		0x009B8, 0x009B8, 0x003E8, 0x00A15, 0x00B8D, // Symmetry Environmental Set
		0x00A52, 0x00A61, 0x00A57, 0x00A64, 0x00A5B, 0x00A68, // Symmetry Dot Reflection Dual Panels (before laser)
		0x17C09, // Quarry Entry Gates
		0x01E59, // Quarry Mill Entry Door
		0x00E0C, 0x01489, 0x0148A, 0x014D9, 0x014E7, 0x014E8, // Quarry Dots
		0x00557, 0x005F1, 0x00620, 0x009F5, 0x0146C, 0x3C12D,0x03686, 0x014E9, // Quarry Stones
		0x0367C, 0x3C125, // Quarry Dots + Stones
		0x034D4, 0x021D5, // Quarry Boathouse Ramp Activation
		0x03852, // Quarry Ramp Height Control
		0x03858, // Quarry Boathouse Horizontal Ramp Control
		0x275FA, // Ramp Hook Control
		0x021B3, 0x021B4, 0x021B0, 0x021AF, 0x021AE, // Quarry Eraser + Shapes
		0x021B5, 0x021B6, 0x021B7, 0x021BB, 0x09DB5, 0x09DB1, 0x3C124, // Quarry + Stars
		0x09DB3, 0x09DB4, 0x0A3CB, 0x0A3CC, 0x0A3D0, // Quarry Eraser + Stars + Shapes
		0x00469, 0x00472, 0x00262, 0x00474, 0x00553, 0x0056F, // Swamp First row
		0x00390, 0x010CA, 0x00983, 0x00984, 0x00986, 0x00985, 0x00987, 0x181A9, // Swamp Second Row
		0x00609, // Swamp Bridge controll
		0x00982, 0x0097F, 0x0098F, 0x00990, 0x17C0D, 0x17C0E, // Swamp Red Panels
		0x00999, 0x0099D, 0x009A0, 0x009A1, // Swamp Disconnected Shapes
		0x00007, 0x00008, 0x00009, 0x0000A, 0x003B2, 0x00A1E, 0x00C2E, 0x00E3A, // Swamp Rotating Shapes
		0x009A6, // Swamp Optional Tetris
		0x009AB, 0x009AD, 0x009AE, 0x009AF, 0x00006, // Swamp Negative Shapes 1
		0x00002, 0x00004, 0x00005, 0x013E6, 0x00596, // Swamp Negative Shapes 2
		0x00001, 0x014D2, 0x014D4, 0x014D1, // Swamp Negative Shapes 3
		0x17C05, 0x17C02, // Swamp Exit Shortcut
		0x02886, // Treehouse Entry door
		0x17D72, 0x17D8F, 0x17D74, 0x17DAC, 0x17D9E, 0x17DB9, 0x17D9C, 0x17DC2, 0x17DC4, // Treehouse Yellow Bridge
		0x17DC8, 0x17DC7, 0x17CE4, 0x17D2D, 0x17D6C, // Treehouse Pink Bridge 1
		0x17D9B, 0x17D99, 0x17DAA, 0x17D97, 0x17BDF, 0x17D91, 0x17DC6, // Treehouse Pink Bridge 2
		0x17DB3, 0x17DB5, 0x17DB6, 0x17DC0, 0x17DD7, 0x17DD9, 0x17DB8, 0x17DDC, 0x17DD1, 0x17DDE, 0x17DE3, 0x17DEC, 0x17DAE, 0x17DB0, 0x17DDB, // Treehouse Orange Bridge 1
		0x17D88, 0x17DB4, 0x17D8C, 0x17CE3, 0x17DCD, 0x17DB2, 0x17DCC, 0x17DCA, 0x17D8E, 0x17DB7, 0x17DB1, 0x17DA2, // Treehouse Orange Bridge 2
		0x17E3C, 0x17E4D, 0x17E4F, 0x17E52, 0x17E5B, 0x17E5F, 0x17E61, // Treehouse Green Bridge
		0x037FF, // Treehouse Shortcut Bridge to Keep
		0x2899C, 0x28A33, 0x28ABF, 0x28AC0, 0x28AC1, 0x28AD9, // Town Full Dots + Shapes
		0x2896A, // Town Rooftop Bridge
		0x28AC7, 0x28AC8, 0x28ACA, 0x28ACB, 0x28ACC, // Town Blue Symmetry
		0x18590, // Town Symmetry start
		0x28A69, // Town Church Star Door
		0x034E3, // Town Soundproof Room
		0x334D8, 0x03C0C, 0x03C08, // Town 3-color Room
		0x0A0C8, // Town Orange
		0x17F89, 0x0A168, 0x33AB2, //Windmill Puzzles
		0x033D4, 0x0CC7B, 0x002A6, 0x00AFB, 0x15ADD, // Vaults
		0x17C34, // Mountain
		0x09E73, 0x09E75, 0x09E78, 0x09E79, 0x09E6C, 0x09E6F, 0x09E6B, // Mountain Orange Row
		0x09E7A, 0x09E71, 0x09E72, 0x09E69, 0x09E7B, // Mountain Green Row
		0x09EAD, 0x09EAF, 0x33AF5, 0x33AF7, 0x09F6E, // Mountain Purple Panels
		0x09FD3, 0x09FD4, 0x09FD6, 0x09FD7, 0x09FD8, // Mountain Rainbow Row
		0x09FCC, 0x09FCE, 0x09FCF, 0x09FD0, 0x09FD1, 0x09FD2, // Mountain multi puzzle
		0x09E86, 0x09E39, 0x09ED8, // Mountain double bridge //might need to be disable if we cant fix the scaling
		0x09EEB, // Mountain Elevator to Giant Puzzle
		0x09EFF, 0x09F01, 0x09FC1, 0x09F8E, 0x09FDA, // Mountain floor
		0x0383D, 0x0383A, 0x0383F, 0x09E56, 0x03859, 0x09E5A, 0x339BB, 0x33961, // Mountain pillar puzzles

		0x17FA2, 0x00FF8, // Caves
		0x01A0D, 0x018A0, 0x009A4, 0x00A72, // Caves Blue Symmetry/Tetris
		0x00190, 0x00558, 0x00567, 0x006FE, 0x008B8, 0x00973, 0x0097B, 0x0097D, 0x0097E, 0x00994, 0x334D5, 0x00995, 0x00996, 0x00998, // Caves riangle Puzzles
		0x32962, 0x32966, 0x01A31, 0x00B71, // Caves First alcove
		0x288EA, 0x288FC, 0x289E7, 0x288AA, // Caves Perspective
		0x0A16B, 0x0A2CE, 0x0A2D7, 0x0A2DD, 0x0A2EA, 0x17FB9, // Caves Full Dots
		0x0008F, 0x0006B, 0x0008B, 0x0008C, 0x0008A, 0x00089, 0x0006A, 0x0006C, 0x00027, 0x00028, 0x00029,	// Caves Invisible Dots
		0x17CF2, 0x021D7, // Caves Exit
		0x09DD5, 0x0A16E, 0x039B4, 0x09E85, // Caves Deep challenge caves
		0x335AB, 0x335AC, 0x3369D, // Caves Elevator

		//0x032FF, //Why a single apple tree?
		/*
		0x00698,	0x0048F, 0x09F92,	0x0A036,	0x09DA6,	0x0A049, 0x0A053, 0x09F94, // Desert Surface
		0x00422, 0x006E3, 0x0A02D, // Desert Light
		0x00C72, 0x0129D,	0x008BB, 0x0078D, 0x18313, // Desert Pond
		0x04D18,	0x01205, 0x181AB, 0x0117A, 0x17ECA, // Desert Flood
		0x18076, 0x0A15C, 0x09FFF, 0x0A15F, 0x012D7 // Desert Final and exit
		*/
		0x033EA, 0x01BE9, 0x01CD3, 0x01D3F, 0x03317, 0x0360E, // Keep
		//0x17F9B,	0x002C4,	0x00767,	0x002C6,	0x0070E,	0x0070F,	0x0087D,	0x002C7, // Jungle Sound waves //Does not contain dots so isnt locked
		0x0026D,	0x0026E,	0x0026F,	0x00C3F,	0x00C41,	0x014B2, // Jungle Dots
		//Doors
		0x01A54, // Glass Factory Entry
		0x000B0, // Symmetry Island Door 1
		// 0x1C349, // Symmetry Island Door 2. Doesn't work properly :(
		0x01E5A, // Mill Entry Door Left
		0x09E57, // Quarry Entry Gate 1

		0x03678, 0x03679, 0x03675, 0x03676, 0x03677, // Mill Controls

		0x0288C, // Treehouse Door 1&2
		0x0A182, // Treehouse Door 3
		0x2700B, // Treehouse Exterior Door Control
		0x334DB, // Shadows Outer Door Control
		0x00B10, // Monastery Left Door
		0x00C92, // Monastery Right Door
		0x09D9B, // Monastery Overhead Door Control
		0x0056E, // Swamp Entry
		0x17C2E, // Bunker Entry Door
		0x09F7D, 0x0A010, 0x17E63, 0x0A079, // Bunker
		0x17CAB, // Jungle Pop-up Wall
		0x0C339, // Desert Surface Door
		0x0A249, // Desert Pond Exit Door
		0x28998, // Town Yellow Door
		0x28A0D, // Town Church Stars
		0x17F5F, // Windmill door
		0x17D02, // Windmill Turn Control
		//Discards
		0x17F93, // Mountain 2 Discard
		0x17CFB, // Outside Tutorial Discard
		0x3C12B, // Glass Factory Discard
		0x17CE7, // Desert Discard
		0x17CF0, // Mill Discard
		0x17FA9, // Treehouse Green Bridge Discard
		0x17FA0, // Treehouse Laser Discard
		0x17D27, // Keep Discard
		0x17D28, // Shipwreck Discard
		0x17D01, // Town Orange Crate Discard
		0x17C71, // Town Rooftop Discard
		0x17F9B, // Jungle Discard
		0x17C42, // Mountainside Discard
		0x17CF7, // Theater Discard
		0x386FA,	0x1C33F,	0x196E2,	0x1972A,	0x19809,	0x19806,	0x196F8,	0x1972F, //shadows avoid
		0x19797,	0x1979A,	0x197E0,	0x197E8,	0x197E5, //Shadow follow
		0x09F7F, // Quary lazer panel
		0x181F5, 0x18488, 0x17E2B, 0x17C0A, 0x17E07, // Swamp bridge controlls

		0x0A332, // Challenge Timer
		0x3D9A9,
};

class CollisionCube
{
public:
	CollisionCube(float x1, float y1, float z1, float x2, float y2, float z2) {
		this->x1 = std::min(x1,x2);
		this->y1 = std::min(y1,y2);
		this->z1 = std::min(z1,z2);
		this->x2 = std::max(x1, x2);
		this->y2 = std::max(y1, y2);
		this->z2 = std::max(z1, z2);
	}

	CollisionCube(std::vector<float> pos1, std::vector<float> pos2) {
		CollisionCube(pos1[0], pos1[1], pos1[2], pos2[0], pos2[1], pos2[2]);
	}

	bool containsPoint(std::vector<float> playerPos) {
		return containsPoint(playerPos[0], playerPos[1], playerPos[2]);
	}

	bool containsPoint(float playerPosX, float playerPosY, float playerPosZ) {
		return playerPosX > x1 && playerPosX < x2 && playerPosY > y1 && playerPosY < y2 && playerPosZ > z1 && playerPosZ < z2;
	}

private:
	float x1;
	float y1;
	float z1;
	float x2;
	float y2;
	float z2;
};