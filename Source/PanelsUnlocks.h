#pragma once
#include <vector>

std::vector<int> doorUnlockPanels = {
	0x01A54, // Glass Factory Entry
	0x000B0, // Symmetry Island Door 1
	0x1C349, // Symmetry Island Door 2
	0x01E5A, // Mill Entry Door Left
	0x09E57, // Quarry Entry Gate 1
	0x0288C, // Treehouse Door 1
	0x0A182, // Treehouse Door 3
	0x2700B, // Treehouse Exterior Door Control
	0x334DB, // Shadows Outer Door Control
	0x00B10, // Monastery Left Door
	0x00C92, // Monastery Right Door
	0x0056E, // Swamp Entry
	0x17C2E, // Bunker Entry Door
	0x17CAB, // Jungle Pop-up Wall
	0x0C339, // Desert Surface Door
	0x0A249, // Desert Pond Exit Door
	0x28998, // Town Yellow Door
	0x28A0D, // Town Church Stars
};

//TODO: Some of these will need to be properly "unlinked" in the game.

std::vector<int> boatSummons = {
	0x17CC8, // Glass Factory Summon Boat
	0x17CA6, // Boathouse Summon Boat
	0x17C95, // Treehouse Summon Boat
	0x0A054, // Town Summon Boat
	0x09DB8, // Swamp Summon Boat
	0x17CDF, // Jungle Summon Boat
};