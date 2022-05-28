#include <map>
#include <vector>
#include <set>

const std::vector<float> horizontalLine = { 0.9f, 0.5f, 0.1f, 0.5f };

const std::map<int, std::vector<float>> skip_specialLine = {
	{0x0A15C, horizontalLine},
	{0x09FFF, horizontalLine},
	{0x0A15F, horizontalLine},
};

const std::set<int> skip_noLine = {
	0x0C373, // Patio Floor
	0x17C31, // Desert Final Room Transparent Reflection

	0x033EA, 0x01BE9, 0x01CD3, 0x01D3F
};

const std::set<int> skip_completelyExclude = {
	//Already simplistic
	
	0x00064, 0x00182, // Entry Panels
	0x03481, 0x0339E, 0x03535, //Vault Boxes
	0x17CC8, 0x17CA6, //Boat Spawns
	0x0360D, 0x03608, 0x03612, 
	// 0x19650, Not Shadows Laser



	//Controls, e.g. Door Handles

	0x09FA0, 0x09F86, 0x0C339, 0x09FAA, 0x0A249, 0x1C2DF, 0x1831E, 0x1C260, 0x1831C, 0x1C2F3, 0x1831D, 0x1C2B1, 0x1831B, 0x0A015,  
	0x17CC4, 0x275ED, 0x03678, 0x03679, 0x03675, 0x03676, 0x17CAC, 0x03852, 0x03858, 0x38663, 0x275FA, 0x334DB, 0x334DC, 0x09E49,



	//Needs to stay the same!

	0x03629, 0x03505, // Tutorial Gate Close

	//Complex issues arise, but could be changed

	0x0A3B5, // Tutorial Multi-Solution
	0x000B0, 0x1C349, // Symmetry Entry Doors - You can just go thru the locks for some reason


	// Doesn't seem to work at all

	0x0C335, // Tutorial Pillar
};

const std::set<int> skip_specialCase = {
	0x00A52, 0x00A57, 0x00A5B, 0x00A61, 0x00A64, 0x00A68, //Symmetry Laser Panels

	0x0A3A8, 0x0A3B9, 0x0A3BB, 0x0A3AD
};