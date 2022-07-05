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

	{0x01A29, {{0x01A54, PANEL, TARGET }, {0x01A54, PANEL, MOUNT_PARENT_ID }}}, // Glass Factory Entry Door
	{0x0D7ED, {{0x04AC9, CABLE, CABLE_TARGET_1 }, {0x0005A, MULTIPANEL, MOUNT_PARENT_ID}}}, // Glass Factory Back Wall
	{0x17F3E, {{0x000B0, PANEL, TARGET }, {0x000B0, PANEL, MOUNT_PARENT_ID }}}, // Symmetry Island Door to Symmetry Island Lower
	{0x18269, {{0x1C349, PANEL, TARGET }, {0x1C349, PANEL, MOUNT_PARENT_ID }}}, // Symmetry Island Door to Symmetry Island Upper

	{0x09FEE, {{0x0C339, PANEL, TARGET }}}, // Desert Door to Desert Flood Light Room
	{0x0C2C3, {{0x0A02D, PANEL, TARGET }}}, // Desert Door to Pond Room
	{0x0A24B, {{0x0A249, PANEL, TARGET }}}, // Desert Door to Water Levels Room
	{0x0C316, {{0x18076, PANEL, TARGET }}}, // Desert Door to Elevator Room

	{0x09D6F, {{0x09E57, PANEL, TARGET }, {0x09E57, PANEL, MOUNT_PARENT_ID }}}, // Quarry Door to Quarry 1
	{0x17C07, {{0x17C09, PANEL, TARGET }, {0x17C09, PANEL, MOUNT_PARENT_ID }}}, // Quarry Door to Quarry 2

	{0x02010, {{0x01D8D, MULTIPANEL, MULTIPANEL_TARGET }, {0x01D8D, MULTIPANEL, MOUNT_PARENT_ID }}}, // Quarry Door to Mill
	{0x275FF, {{0x275ED, PANEL, TARGET }}}, // Quarry Mill Ground Floor Shortcut Door
	{0x17CE8, {{0x17CAC, PANEL, TARGET }}}, // Quarry Mill Door to Outside Quarry Stairs
	{0x0368A, {{0x03677, PANEL, TARGET }}}, // Quarry Mill Stairs

	{0x19B24, {{0x0A3A9, PRESSURE_PLATE, PRESSURE_PLATE_TARGET}, {0x334DB, PANEL, MOUNT_PARENT_ID}, {0x334DC, PRESSURE_PLATE, MOUNT_PARENT_ID}, {0x334DB, PANEL, ENTITY_NAME}, {0x334DC, PRESSURE_PLATE, ENTITY_NAME}}},
};