#include <map>
#include <vector>
#include <set>

inline const std::vector<float> horizontalLine = { 0.9f, 0.5f, 0.1f, 0.5f };
inline const std::vector<float> verticalLine = { 0.5f, 0.9f, 0.5f, 0.1f };
inline const std::vector<float> pillarLine = { 0.9f, 0.5f, 0.7f, 0.5f, 0.5f, 0.5f, 0.3f, 0.5f, 0.1f, 0.5f };
inline const std::vector<float> offsetline = { 0.2f, 0.9f, 0.2f, 0.1f };
inline const std::vector<float> shortLineTop = { 0.5f, 0.9f, 0.5f, 0.7f };

inline const std::map<int, std::vector<float>> skip_specialLine = {
	{0x18076, verticalLine},
	{0x28B39, verticalLine},
	{0x193A7, verticalLine},
	{0x193AA, verticalLine},
	{0x193AB, verticalLine},
	{0x193A6, verticalLine},
	{0x0A15C, horizontalLine},
	{0x09FFF, horizontalLine},
	{0x0A15F, horizontalLine},
	{0x33Af5, offsetline},
	{0x33AF7, offsetline},
	{0x09F6E, shortLineTop},
	{0x0383A, pillarLine},
	{0x09E56, pillarLine},
	{0x09E5A, pillarLine},
	{0x33961, pillarLine},
	{0x0383D, pillarLine},
	{0x0383F, pillarLine},
	{0x03859, pillarLine},
	{0x339BB, pillarLine},
	{0x09DD5, pillarLine},
};

inline const std::set<int> dont_power = {
	0x01983,
	0x01987, // Peekaboo

	0x17D8F, // Treehouse Yellow 2
	0x17D74, // Treehouse Yellow 3
	0x17DAC, // Treehouse Yellow 4
	0x17D9E, // Treehouse Yellow 5
	0x17DB9, // Treehouse Yellow 6
	0x17D9C, // Treehouse Yellow 7
	0x17DC2, // Treehouse Yellow 8
	0x17DC4, // Treehouse Yellow 9
	0x17DC7, // Treehouse First Purple 2
	0x17CE4, // Treehouse First Purple 3
	0x17D2D, // Treehouse First Purple 4
	0x17D6C, // Treehouse First Purple 5
	0x17D99, // Treehouse Second Purple 2
	0x17DAA, // Treehouse Second Purple 3
	0x17D97, // Treehouse Second Purple 4
	0x17BDF, // Treehouse Second Purple 5
	0x17D91, // Treehouse Second Purple 6
	0x17DC6, // Treehouse Second Purple 7
	0x17DB5, // Treehouse Left Orange 2
	0x17DB6, // Treehouse Left Orange 3
	0x17DC0, // Treehouse Left Orange 4
	0x17DD7, // Treehouse Left Orange 5
	0x17DD9, // Treehouse Left Orange 6
	0x17DB8, // Treehouse Left Orange 7
	0x17DDC, // Treehouse Left Orange 8
	0x17DDE, // Treehouse Left Orange 10
	0x17DE3, // Treehouse Left Orange 11
	0x17DEC, // Treehouse Left Orange 12
	0x17DAE, // Treehouse Left Orange 13
	0x17DB0, // Treehouse Left Orange 14
	0x17DDB, // Treehouse Left Orange 15
	0x17DB4, // Treehouse Right Orange 2
	0x17D8C, // Treehouse Right Orange 3
	0x17DCD, // Treehouse Right Orange 5
	0x17DB2, // Treehouse Right Orange 6
	0x17DCC, // Treehouse Right Orange 7
	0x17DCA, // Treehouse Right Orange 8
	0x17D8E, // Treehouse Right Orange 9
	0x17DB1, // Treehouse Right Orange 11
	0x17DA2, // Treehouse Right Orange 12
	0x17E4D, // Treehouse Green 2
	0x17E4F, // Treehouse Green 3
	0x17E5B, // Treehouse Green 5
	0x17E5F, // Treehouse Green 6
	0x17E61, // Treehouse Green 7
	0x17DD1, // Treehouse Left Orange 9
	0x17CE3, // Treehouse Right Orange 4
	0x17DB7, // Treehouse Right Orange 10
	0x17E52, // Treehouse Green 4
};

inline const std::set<int> skip_noLine = {
	0x03629, // Tutorial Gate Open

	0x0C373, // Patio Floor
	0x17C31, // Desert Final Room Transparent Reflection
	0x28A69, // Town Lattice

	0x15ADD, // River Vault

	0x00290, 0x00038, 0x00037, //Monastery Lattice

	0x033EA, 0x01BE9, 0x01CD3, 0x01D3F, //Keep PPs

	0x09F7D, 0x09FDC, 0x09FF7, 0x09F82, 0x09FF8, 0x09D9F, 0x09DA1, 0x09DA2, 0x09DAF, 0x0A010, 0x0A01B, 0x0A01F, 0x17E63, 0x17E67,

	0x09FCC, 0x09FCE, 0x09FCF, 0x09FD0, 0x09FD1, 0x09FD2, // Same solution set

	0x17E52, 0x17DD1, 0x17CE3, 0x17DB7, // Treehouse Directional Bridge Panels

	0x17C34, // Mountaintop Perspective 3-way

	0x09E39, 0x09E86, 0x09ED8, // Light Bridge Controls

	0x17CAA, // River to Monastery shortcut lattice
};

inline const std::set<int> cutoutPanels = {
	0x28a69, 0x15ADD, 0x00290, 0x00038, 0x00037, 0x17caa, 0x09F7D, 0x09FDC, 0x09FF7, 0x09F82, 0x09D9F, 0x09FF8, 0x09DA1, 0x09DA2, 0x09DAF, 0x0A010, 0x0A01B, 0x0A01F, 0x17E63, 0x17E67,
};

inline const std::set<int> dont_touch_panel_at_all = {
	0x0088E, 0x00BAF, 0x00BF3, 0x00C09, 0x00CDB, 0x0051F, 0x00524, 0x00CD4, 0x00CB9, 0x00CA1, 0x00C80, 0x00C68, 0x00C59, 0x00C22, 0x034F4, 0x034EC, 0x1C31A, 0x1C319,

	0x01983, 0x01987,
};

inline const std::set<int> skip_completelyExclude = {
	0x0c373, // Patio Floor (EP)

	0x00064, 0x00182, // Entry Panels
	0x03481, 0x0339E, 0x03535, 0x03702, 0x03542, 0x0356B, 0x2FAF6, //Vault Boxes
	0x17CC8, 0x17CA6, 0x0A054, 0x17CDF, 0x09DB8, 0x17C95, //Boat Spawns
	0x34D96, 0x34C7F, //Boat Map & Speed
	0x0360D, 0x03608, 0x032F5, 0x17CA4, 0x03616, 0x09DE0, 0x03615, 0x03613, //Lasers except Shadows & Keep

	0x0042D, // Mountaintop River Shape

	//Controls, e.g. Door Handles

	0x09FA0, 0x09F86, 0x0C339, 0x09FAA, 0x0A249, 0x1C2DF, 0x1831E, 0x1C260, 0x1831C, 0x1C2F3, 0x1831D, 0x1C2B1, 0x1831B, 0x0A015,  
	0x17CC4, 0x275ED, 0x03678, 0x03679, 0x03675, 0x03676, 0x17CAC, 0x03852, 0x03858, 0x38663, 0x275FA, 0x334DB, 0x334DC, 0x09E49,
	0x0361B, 0x334D8, 0x2896A, 0x17D02, 0x03713, 0x00B10, 0x00C92, 0x09D9B, 0x17CAB, 0x337FA, 0x0A099, 0x34BC5, 0x34BC6, 0x17CBC,
	0x2700B, 0x09EEB, 0x334E1, 0x27732, 0x2773D, 0x3D9A6, 0x3D9A7, 0x3C113, 0x3C114, 0x3D9AA, 0x3D9A8, 0x3D9A9,

	//Video Inputs
	0x00815, 0x03553, 0x03552, 0x0354E, 0x03549, 0x0354F, 0x03545,

	//Needs to stay the same!

	0x03505, // Tutorial Gate Close
	0x09F98, // Town Laser Redirect
	0x0A079, // Bunker Elevator
	0x00609, 0x17E2B, 0x18488, // Swamp Bridge Controls
	0x17C04, 0x17E07, // Swamp Maze Controls
	0x037FF, // Treehouse Bridge to Keep
	0x09F7F, // Mountaintop Box Open
	0x335AB, 0x335AC, 0x3369D, // Elevators in Caves 

	//Complex issues arise, but could be changed
	0x034E4, 0x034E3, //Sound room
	0x0A3B5, // Tutorial Multi-Solution


	// Doesn't seem to work at all
	// Town Triple Environmental
	0x0C335, // Tutorial Pillar

	// Challenge Puzzles crash the game

	0x0A332, 0x0088E, 0x00BAF, 0x00BF3, 0x00C09, 0x00CDB, 0x0051F, 0x00524, 0x00CD4, 0x00CB9, 0x00CA1, 0x00C80, 0x00C68, 0x00C59, 0x00C22, 0x034F4, 0x034EC, 0x1C31A, 0x1C319,

	0x01983, 0x01987, // Peekaboo

};

// Puzzles that don't make sense to skip on Normal but are useful to skip on Hard.
inline const std::set<int> skip_excludeOnNormal = {
	0x181F5, // Swamp Rotating Bridge
	0x334D8, // Town RGB Control
};

// Puzzles that don't make sense to skip on Hard but are useful to skip on Normal.
inline const std::set<int> skip_excludeOnHard = {
};

// Non-trivial puzzles that have multiple gameplay-relevant solutions and should continue to be skippable even after the puzzle has been solved.
//   Note that there are puzzles in here that are currently excluded or special-cased in some manner; they're here for completeness's sake so
//   that if we ever decide to un-exclude them we'll already have them represented here and (hopefully) not run into any issues there.
inline const std::set<int> skip_multisolvePuzzles = {
	// Bunker
	0x0A079, // Bunker Elevator

	// Caves
	0x17FA2, // Entry "discard" triangle puzzle
	0x335AB, 0x335AC, 0x3369D, // Elevators

	// Keep
	0x0A3A8, 0x0A3B9, 0x0A3BB, 0x0A3AD, // Pressure plate resets (needed for EPs)

	// Mountain
	0x09FC1, 0x09F8E, 0x09F01, 0x09EFF, // Metapuzzle small panels <-- TODO: These are fully "solved" once the big puzzle has been solved.
	0x09E39, 0x09E86, 0x09ED8, // Light Bridge Controls
	0x09FCC, 0x09FCE, 0x09FCF, 0x09FD0, 0x09FD1, 0x09FD2, // Same solution set

	// Quarry
	0x03678, 0x03675, 0x03679, 0x03676, // Mill ramp/lift controls
	0x03852, 0x03858, // Boathouse ramp controls
	0x275FA, // Hook

	// Swamp
	0x00609, 0x17E2B, 0x18488, // Swamp Bridge Controls
	0x17C04, 0x17E07, // Swamp Maze Controls
	0x181F5, // Swamp Rotating Bridge

	// Theater
	0x00815, // Video input

	// Town
	0x334D8, // RGB Control
	0x079DF, // Shadow/Perspective/Reflection Triple Solve <-- TODO: This is fully solved once all three of its solutions have been entered.
	0x2896A, // Rooftop bridge

	// Treehouse
	0x17E52, 0x17DD1, 0x17CE3, 0x17DB7, // Treehouse Directional Bridge Panels

	// Tutorial
	0x0A3B5, // Tutorial back left
	0x03629, // Tutorial gate
};

inline const std::set<int> skip_specialCase = {
	0x09E86, 0x09ED8,

	0x00A52, 0x00A57, 0x00A5B, 0x00A61, 0x00A64, 0x00A68, //Symmetry Laser Panels

	0x0A3A8, 0x0A3B9, 0x0A3BB, 0x0A3AD, // Pressure plate reset lines

	0x09FC1, 0x09F8E, 0x09F01, 0x09EFF, // Metapuzzle

	0x181F5, // Swamp Rotating Bridge
	0x334D8, // Town RGB Control
};

inline const std::set<int> thin_panels = {
	0x09FA0, 0x09F86, 0x0C339, 0x09FAA, 0x0A249, 0x1C2DF, 0x1831E, 0x1C260, 0x1831C, 0x1C2F3, 0x1831D, 0x1C2B1, 0x1831B, 0x0A015,
	0x17CC4, 0x275ED, 0x03678, 0x03679, 0x03675, 0x03676, 0x17CAC, 0x03852, 0x03858, 0x38663, 0x275FA, 0x334DB, 0x334DC, 0x09E49,
	0x0361B, 0x334D8, 0x2896A, 0x03713, 0x00B10, 0x00C92, 0x09D9B, 0x17CAB, 0x337FA, 0x0A099, 0x34BC5, 0x34BC6, 0x17CBC,
	0x2700B, 0x09EEB, 0x334E1, 0x27732, 0x2773D, 0x3D9A6, 0x3D9A7, 0x3C113, 0x3C114, 0x3D9AA, 0x3D9A8, 0x3D9A9,

	0x0360D, 0x03608, 0x032F5, 0x17CA4, 0x03616, 0x09DE0, 0x03615, 0x03613,
};

inline const std::set<int> wide_panels = {
	0x17CC8, 0x17CA6, 0x0A054, 0x17CDF, 0x09DB8, 0x17C95, //Boat Spawns
};