#pragma once
#include <map>

std::map<int, long> doorUnlockPanels = {
	{ 0x01A54, 158000 }, // Glass Factory Entry
	{ 0x000B0, 158001 }, // Symmetry Island Door 1
	{ 0x1C349, 158002 }, // Symmetry Island Door 2
	{ 0x01E5A, 158003 }, // Mill Entry Door Left
	{ 0x09E57, 158004 }, // Quarry Entry Gate 1
	{ 0x0288C, 158005 }, // Treehouse Door 1&2
	{ 0x0A182, 158006 }, // Treehouse Door 3
	{ 0x2700B, 158007 }, // Treehouse Exterior Door Control
	{ 0x334DB, 158008 }, // Shadows Outer Door Control
	{ 0x00B10, 158009 }, // Monastery Left Door
	{ 0x00C92, 158010 }, // Monastery Right Door
	{ 0x0056E, 158011 }, // Swamp Entry
	{ 0x17C2E, 158012 }, // Bunker Entry Door
	{ 0x17CAB, 158013 }, // Jungle Pop-up Wall
	//{ 0x00000, 158014 }, // Boat Access
	{ 0x0C339, 158015 }, // Desert Surface Door
	{ 0x0A249, 158016 }, // Desert Pond Exit Door
	{ 0x28998, 158017 }, // Town Yellow Door
	{ 0x28A0D, 158018 }, // Town Church Stars
};

//TODO: Some of these will need to be properly "unlinked" in the game.

std::map<int, long> boatSummons = {
	{ 0x17CC8, 158019 }, // Glass Factory Summon Boat
	{ 0x17CA6, 158020 }, // Boathouse Summon Boat
	{ 0x17C95, 158021 }, // Treehouse Summon Boat
	{ 0x0A054, 158022 }, // Town Summon Boat
	{ 0x09DB8, 158023 }, // Swamp Summon Boat
	{ 0x17CDF, 158024 }, // Jungle Summon Boat
};

//other items: 158500
//maybe traps