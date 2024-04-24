#pragma once
#include <vector>

// Some of these (the puzzle ones) are duplicated elsewhere
inline std::vector<int> lasers = {
	0x0360D, // Symmetry
	0x03615, // Swamp
	0x09DE0, // Bunker
	0x17CA4, // Monastery
	0x032F5, // Town
	0x03613, // Treehouse
	0x0360E, // Keep Front Laser
//	0x03317, // Keep Back Laser
//	0x03608, // Desert
	0x03612, // Quarry
	0x03616, // Jungle
	0x19650, // Shadows
};

inline std::vector<int> millElevatorControlOptions = {
	0x0008D, // Glass Factory Rotational Symmetry 1
	0x00081, // Glass Factory Rotational Symmetry 2
	0x00070, // Symmetry Island Fading Lines 5
	0x01E5A, // Mill Entry Door Left
	0x28AC7, // Town Blue 1
	0x28AC8, // Town Blue 2
	0x28ACA, // Town Blue 3
	0x28ACB, // Town Blue 4
	0x28ACC, // Town Blue 5
	0x00029, // UTM Invisible Dots Symmetry 3

	0x17CC4, // Mill Elevator Control
};

inline std::vector<int> tutorialBackLeftOptions = {
	0x0008D, // Glass Factory Rotational Symmetry 1
	0x00081, // Glass Factory Rotational Symmetry 2
	0x00083, // Glass Factory Rotational Symmetry 3
	0x00084, // Glass Factory Melting 1
	0x28AC7, // Town Blue 1
	0x28AC8, // Town Blue 2
	0x28ACA, // Town Blue 3
	0x28ACB, // Town Blue 4
	0x28ACC, // Town Blue 5
	0x00029, // UTM Invisible Dots Symmetry 3
	0x00082, // Glass Factory Melting 2
	0x0343A, // Glass Factory Melting 3
	0x00022, // Symmetry Island Black Dots 1
	0x00023, // Symmetry Island Black Dots 2
	0x00024, // Symmetry Island Black Dots 3
	0x00025, // Symmetry Island Black Dots 4
	0x00026, // Symmetry Island Black Dots 5
	0x01A0D, // UTM Blue Hard Symmetry
	0x018A0, // UTM Blue Easy Symmetry

	0x0A3B5, // Tutorial Back Left
};

inline std::vector<int> tutorialBackLeftExpertOptions = {
    0x0A3B2, // Tutorial Back Right
    0x0A171, // Tutorial Optional Door 1
    0x04CA4, // Tutorial Optional Door 2
    0x17CFB, // Outside Tutorial Discard
    0x3C12B, // Glass Factory Discard
    0x01A54, // Glass Factory Entry
    0x000B0, // Symmetry Island Door 1
    0x17CE7, // Desert Discard
    0x0CC7B, // Desert Vault
    0x01E59, // Mill Entry Door Right
    0x00E0C, // Mill Lower Row 1
    0x01489, // Mill Lower Row 2
    0x0148A, // Mill Lower Row 3
    0x014D9, // Mill Lower Row 4
    0x014E7, // Mill Lower Row 5
    0x014E8, // Mill Lower Row 6
    0x00557, // Mill Upper Row 1
    0x005F1, // Mill Upper Row 2
    0x00620, // Mill Upper Row 3
    0x009F5, // Mill Upper Row 4
    0x0146C, // Mill Upper Row 5
    0x3C12D, // Mill Upper Row 6
    0x03686, // Mill Upper Row 7
    0x014E9, // Mill Upper Row 8
    0x0367C, // Mill Control Room 1
    0x17CF0, // Mill Discard
    0x021D5, // Boathouse Ramp Activation Shapers
    0x021B3, // Boathouse Erasers and Shapers 1
    0x021B4, // Boathouse Erasers and Shapers 2
    0x021B0, // Boathouse Erasers and Shapers 3
    0x021AF, // Boathouse Erasers and Shapers 4
    0x021AE, // Boathouse Erasers and Shapers 5
    0x021B5, // Boathouse Erasers and Stars 1
    0x021B6, // Boathouse Erasers and Stars 2
    0x021B7, // Boathouse Erasers and Stars 3
    0x021BB, // Boathouse Erasers and Stars 4
    0x09DB5, // Boathouse Erasers and Stars 5
    0x09DB1, // Boathouse Erasers and Stars 6
    0x3C124, // Boathouse Erasers and Stars 7
    0x09DB3, // Boathouse Erasers Shapers and Stars 1
    0x09DB4, // Boathouse Erasers Shapers and Stars 2
    0x0A3CB, // Boathouse Erasers Shapers and Stars 3
    0x0A3CC, // Boathouse Erasers Shapers and Stars 4
    0x0A3D0, // Boathouse Erasers Shapers and Stars 5
    0x09E57, // Quarry Entry Gate 1
    0x17C09, // Quarry Entry Gate 2
    0x0288C, // Treehouse Door 1
    0x02886, // Treehouse Door 2
    0x17D72, // Treehouse Yellow 1
    0x17D8F, // Treehouse Yellow 2
    0x17D74, // Treehouse Yellow 3
    0x17DAC, // Treehouse Yellow 4
    0x17D9E, // Treehouse Yellow 5
    0x17DB9, // Treehouse Yellow 6
    0x17D9C, // Treehouse Yellow 7
    0x17DC2, // Treehouse Yellow 8
    0x17DC4, // Treehouse Yellow 9
    0x0A182, // Treehouse Door 3
    0x17DC8, // Treehouse First Purple 1
    0x17DC7, // Treehouse First Purple 2
    0x17CE4, // Treehouse First Purple 3
    0x17D2D, // Treehouse First Purple 4
    0x17D6C, // Treehouse First Purple 5
    0x17D9B, // Treehouse Second Purple 1
    0x17D99, // Treehouse Second Purple 2
    0x17DAA, // Treehouse Second Purple 3
    0x17D97, // Treehouse Second Purple 4
    0x17BDF, // Treehouse Second Purple 5
    0x17D91, // Treehouse Second Purple 6
    0x17DC6, // Treehouse Second Purple 7
    0x17DB3, // Treehouse Left Orange 1
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
    0x17D88, // Treehouse Right Orange 1
    0x17DB4, // Treehouse Right Orange 2
    0x17D8C, // Treehouse Right Orange 3
    0x17DCD, // Treehouse Right Orange 5
    0x17DB2, // Treehouse Right Orange 6
    0x17DCC, // Treehouse Right Orange 7
    0x17DCA, // Treehouse Right Orange 8
    0x17D8E, // Treehouse Right Orange 9
    0x17DB1, // Treehouse Right Orange 11
    0x17DA2, // Treehouse Right Orange 12
    0x17E3C, // Treehouse Green 1
    0x17E4D, // Treehouse Green 2
    0x17E4F, // Treehouse Green 3
    0x17E5B, // Treehouse Green 5
    0x17E5F, // Treehouse Green 6
    0x17E61, // Treehouse Green 7
    0x17FA9, // Treehouse Green Bridge Discard
    0x00139, // Keep Hedges 1
    0x019DC, // Keep Hedges 2
    0x019E7, // Keep Hedges 3
    0x01A0F, // Keep Hedges 4
    0x17D28, // Shipwreck Discard
    0x2899C, // Town 25 Dots 1
    0x28A33, // Town 25 Dots 2
    0x28ABF, // Town 25 Dots 3
    0x28AC0, // Town 25 Dots 4
    0x28AC1, // Town 25 Dots 5
    0x28AC7, // Town Blue 1
    0x28A0D, // Town Church Stars
    0x28AD9, // Town Eraser
    0x28998, // Town Green Door
    0x0A0C8, // Town Orange Crate
    0x17D01, // Town Orange Crate Discard
    0x03C0C, // Town RGB Stones
    0x17C71, // Town Rooftop Discard
    0x17F89, // Theater Entrance
    0x33AB2, // Theater Corona Exit
    0x0A168, // Theater Sun Exit
    0x17C2E, // Bunker Entry Door
    0x00469, // Swamp Tutorial 1
    0x00472, // Swamp Tutorial 2
    0x00262, // Swamp Tutorial 3
    0x00474, // Swamp Tutorial 4
    0x00553, // Swamp Tutorial 5
    0x0056F, // Swamp Tutorial 6
    0x00390, // Swamp Tutorial 7
    0x010CA, // Swamp Tutorial 8
    0x00983, // Swamp Tutorial 9
    0x00984, // Swamp Tutorial 10
    0x00986, // Swamp Tutorial 11
    0x00985, // Swamp Tutorial 12
    0x00987, // Swamp Tutorial 13
    0x181A9, // Swamp Tutorial 14
    0x00982, // Swamp Red 1
    0x0097F, // Swamp Red 2
    0x0098F, // Swamp Red 3
    0x00990, // Swamp Red 4
    0x17C0D, // Swamp Red Shortcut 1
    0x17C0E, // Swamp Red Shortcut 2
    0x00999, // Swamp Discontinuous 1
    0x0099D, // Swamp Discontinuous 2
    0x009A0, // Swamp Discontinuous 3
    0x009A1, // Swamp Discontinuous 4
    0x00007, // Swamp Rotation Tutorial 1
    0x00008, // Swamp Rotation Tutorial 2
    0x00009, // Swamp Rotation Tutorial 3
    0x0000A, // Swamp Rotation Tutorial 4
    0x009AB, // Swamp Blue Underwater 1
    0x009AD, // Swamp Blue Underwater 2
    0x009AE, // Swamp Blue Underwater 3
    0x009AF, // Swamp Blue Underwater 4
    0x00006, // Swamp Blue Underwater 5
    0x003B2, // Swamp Rotation Advanced 1
    0x00A1E, // Swamp Rotation Advanced 2
    0x00C2E, // Swamp Rotation Advanced 3
    0x00E3A, // Swamp Rotation Advanced 4
    0x009A6, // Swamp Purple Tetris
    0x00002, // Swamp Teal Underwater 1
    0x00004, // Swamp Teal Underwater 2
    0x00005, // Swamp Teal Underwater 3
    0x013E6, // Swamp Teal Underwater 4
    0x00596, // Swamp Teal Underwater 5
    0x00001, // Swamp Red Underwater 1
    0x014D2, // Swamp Red Underwater 2
    0x014D4, // Swamp Red Underwater 3
    0x014D1, // Swamp Red Underwater 4
    0x17C05, // Swamp Laser Shortcut 1
    0x17C02, // Swamp Laser Shortcut 2
    0x17F9B, // Jungle Discard
    0x09FD3, // Mountain 2 Rainbow 1
    0x09FD4, // Mountain 2 Rainbow 2
    0x09FD6, // Mountain 2 Rainbow 3
    0x09FD7, // Mountain 2 Rainbow 4
    0x09FD8, // Mountain 2 Rainbow 5
    0x17F93, // Mountain 2 Discard
    0x17FA2, // Mountain 3 Secret Door
    0x17C42, // Mountainside Discard
    0x00FF8, // UTM Entrance Door
    0x0A16B, // UTM Green Dots 1
    0x0A2CE, // UTM Green Dots 2
    0x0A2D7, // UTM Green Dots 3
    0x0A2DD, // UTM Green Dots 4
    0x0008F, // UTM Invisible Dots 1
    0x0006B, // UTM Invisible Dots 2
    0x0008B, // UTM Invisible Dots 3
    0x0008C, // UTM Invisible Dots 4
    0x0008A, // UTM Invisible Dots 5
    0x00089, // UTM Invisible Dots 6
    0x0006A, // UTM Invisible Dots 7
    0x0006C, // UTM Invisible Dots 8
    0x021D7, // UTM Mountainside Shortcut
    0x00B71, // UTM Quarry
    0x01A31, // UTM Rainbow
    0x32962, // UTM Swamp
    0x32966, // UTM Treehouse
    0x17CF2, // UTM Waterfall Shortcut
    0x00A72, // UTM Blue Cave In
    0x009A4, // UTM Blue Discontinuous
    0x008B8, // UTM Blue Left 1
    0x00973, // UTM Blue Left 2
    0x0097B, // UTM Blue Left 3
    0x0097D, // UTM Blue Left 4
    0x0097E, // UTM Blue Left 5
    0x00994, // UTM Blue Right Far 1
    0x334D5, // UTM Blue Right Far 2
    0x00995, // UTM Blue Right Far 3
    0x00996, // UTM Blue Right Far 4
    0x00998, // UTM Blue Right Far 5
    0x00190, // UTM Blue Right Near 1
    0x006FE, // UTM Blue Right Near 4
    0x0A16E, // UTM Challenge Entrance
    0x039B4, // Tunnels Theater Catwalk

    0x0A3B5, // Tutorial Back Left
};

inline std::vector<int> utmElevatorControls = {
	0x335AB, // UTM In Elevator Control
	0x3369D, // UTM Lower Elevator Control
	0x335AC, // UTM Upper Elevator Control
};

inline std::vector<int> treehousePivotSet = {
	0x17DD1, // Treehouse Left Orange 9
	0x17CE3, // Treehouse Right Orange 4
	0x17DB7, // Treehouse Right Orange 10
	0x17E52, // Treehouse Green 4
};

inline std::vector<int> utmPerspectiveSet = {
	0x288EA, // UTM Perspective 1
	0x288FC, // UTM Perspective 2
	0x289E7, // UTM Perspective 3
	0x288AA, // UTM Perspective 4
};

inline std::vector<int> symmetryLaserYellows = {
	0x00A52, // Symmetry Island Laser Yellow 1
	0x00A57, // Symmetry Island Laser Yellow 2
	0x00A5B, // Symmetry Island Laser Yellow 3
};

inline std::vector<int> symmetryLaserBlues = {
	0x00A61, // Symmetry Island Laser Blue 1
	0x00A64, // Symmetry Island Laser Blue 2
	0x00A68, // Symmetry Island Laser Blue 3
};

inline std::vector<int> pillars = {
	0x0383D, // Mountain 3 Left Pillar 1
	0x0383F, // Mountain 3 Left Pillar 2
	0x03859, // Mountain 3 Left Pillar 3
	0x339BB, // Mountain 3 Left Pillar 4
	0x3C113, // Mountain 3 Left Open Door
	0x0383A, // Mountain 3 Right Pillar 1
	0x09E56, // Mountain 3 Right Pillar 2
	0x09E5A, // Mountain 3 Right Pillar 3
	0x33961, // Mountain 3 Right Pillar 4
	0x3C114, // Mountain 3 Right Open Door
//	0x09DD5, // UTM Challenge Pillar
};

inline std::vector<int> challengePanels = {
	0x0A332, // Challenge Record Start
	0x0088E, // Challenge Easy Maze
	0x00BAF, // Challenge Hard Maze
	0x00BF3, // Challenge Stones Maze
	0x00C09, // Challenge Pedestal
	0x0051F, // Challenge Column Bottom Left
	0x00524, // Challenge Column Top Right
	0x00CDB, // Challenge Column Top Left
	0x00CD4, // Challenge Column Far Panel
	0x00C80, // Challenge Triple 1 Left
	0x00CA1, // Challenge Triple 1 Center
	0x00CB9, // Challenge Triple 1 Right
	0x00C22, // Challenge Triple 2 Left
	0x00C59, // Challenge Triple 2 Center
	0x00C68, // Challenge Triple 2 Right
//	0x04CB3, // Challenge Left Timer
//	0x04CB5, // Challenge Middle Timer
//	0x04CB6, // Challenge Right Timer
	0x034EC, // Challenge Triangle
	0x034F4, // Challenge Triangle
	0x1C31A, // Challenge Left Pillar
	0x1C319, // Challenge Right Pillar
//	0x0356B, // Challenge Vault Box
};

inline std::vector<int> mountainOffset = {
    0x09E73, // Mountain 1 Orange 1
    0x09E75, // Mountain 1 Orange 2
    0x09E78, // Mountain 1 Orange 3
    0x09E79, // Mountain 1 Orange 4
    0x09E6C, // Mountain 1 Orange 5
    0x09E6F, // Mountain 1 Orange 6
    0x09E6B, // Mountain 1 Orange 7
};

inline std::vector<int> mountainMultipanel = {
	0x09FCC, // Mountain 2 Multipanel 1
	0x09FCE, // Mountain 2 Multipanel 2
	0x09FCF, // Mountain 2 Multipanel 3
	0x09FD0, // Mountain 2 Multipanel 4
	0x09FD1, // Mountain 2 Multipanel 5
	0x09FD2, // Mountain 2 Multipanel 6
};

inline std::vector<int> squarePanels = {
    0x00064, // Tutorial Straight
    0x00182, // Tutorial Bend
    0x0A3B2, // Tutorial Back Right
    0x00295, // Tutorial Center Left
    0x00293, // Tutorial Front Center
    0x002C2, // Tutorial Front Left
    0x0005D, // Outside Tutorial Dots Tutorial 1
    0x0005E, // Outside Tutorial Dots Tutorial 2
    0x0005F, // Outside Tutorial Dots Tutorial 3
    0x00060, // Outside Tutorial Dots Tutorial 4
    0x00061, // Outside Tutorial Dots Tutorial 5
    0x018AF, // Outside Tutorial Stones Tutorial 1
    0x0001B, // Outside Tutorial Stones Tutorial 2
    0x012C9, // Outside Tutorial Stones Tutorial 3
    0x0001C, // Outside Tutorial Stones Tutorial 4
    0x0001D, // Outside Tutorial Stones Tutorial 5
    0x0001E, // Outside Tutorial Stones Tutorial 6
    0x0001F, // Outside Tutorial Stones Tutorial 7
    0x00020, // Outside Tutorial Stones Tutorial 8
    0x00021, // Outside Tutorial Stones Tutorial 9
    0x033D4, // Outside Tutorial Vault
    0x0A171, // Tutorial Optional Door 1
    0x04CA4, // Tutorial Optional Door 2
    0x17CFB, // Outside Tutorial Discard
    0x3C12B, // Glass Factory Discard
    0x01A54, // Glass Factory Entry
    0x00086, // Glass Factory Vertical Symmetry 1
    0x00087, // Glass Factory Vertical Symmetry 2
    0x00059, // Glass Factory Vertical Symmetry 3
    0x00062, // Glass Factory Vertical Symmetry 4
    0x0008D, // Glass Factory Rotational Symmetry 1
    0x00081, // Glass Factory Rotational Symmetry 2
    0x00083, // Glass Factory Rotational Symmetry 3
    0x00084, // Glass Factory Melting 1
    0x00082, // Glass Factory Melting 2
    0x0343A, // Glass Factory Melting 3
    0x000B0, // Symmetry Island Door 1
    0x00022, // Symmetry Island Black Dots 1
    0x00023, // Symmetry Island Black Dots 2
    0x00024, // Symmetry Island Black Dots 3
    0x00025, // Symmetry Island Black Dots 4
    0x00026, // Symmetry Island Black Dots 5
    0x0007C, // Symmetry Island Colored Dots 1
    0x0007E, // Symmetry Island Colored Dots 2
    0x00075, // Symmetry Island Colored Dots 3
    0x00073, // Symmetry Island Colored Dots 4
    0x00077, // Symmetry Island Colored Dots 5
    0x00079, // Symmetry Island Colored Dots 6
    0x00065, // Symmetry Island Fading Lines 1
    0x0006D, // Symmetry Island Fading Lines 2
    0x00072, // Symmetry Island Fading Lines 3
    0x0006F, // Symmetry Island Fading Lines 4
    0x00070, // Symmetry Island Fading Lines 5
    0x00071, // Symmetry Island Fading Lines 6
    0x00076, // Symmetry Island Fading Lines 7
    0x17CE7, // Desert Discard
    0x0CC7B, // Desert Vault
    0x01E5A, // Mill Entry Door Left
    0x01E59, // Mill Entry Door Right
    0x00E0C, // Mill Lower Row 1
    0x01489, // Mill Lower Row 2
    0x0148A, // Mill Lower Row 3
    0x014D9, // Mill Lower Row 4
    0x014E7, // Mill Lower Row 5
    0x014E8, // Mill Lower Row 6
    0x00557, // Mill Upper Row 1
    0x005F1, // Mill Upper Row 2
    0x00620, // Mill Upper Row 3
    0x009F5, // Mill Upper Row 4
    0x0146C, // Mill Upper Row 5
    0x3C12D, // Mill Upper Row 6
    0x03686, // Mill Upper Row 7
    0x014E9, // Mill Upper Row 8
    0x0367C, // Mill Control Room 1
    0x3C125, // Mill Control Room 2
    0x03677, // Mill Stairs Control
    0x17CF0, // Mill Discard
    0x021D5, // Boathouse Ramp Activation Shapers
    0x034D4, // Boathouse Ramp Activation Stars
    0x021B3, // Boathouse Erasers and Shapers 1
    0x021B4, // Boathouse Erasers and Shapers 2
    0x021B0, // Boathouse Erasers and Shapers 3
    0x021AF, // Boathouse Erasers and Shapers 4
    0x021AE, // Boathouse Erasers and Shapers 5
    0x021B5, // Boathouse Erasers and Stars 1
    0x021B6, // Boathouse Erasers and Stars 2
    0x021B7, // Boathouse Erasers and Stars 3
    0x021BB, // Boathouse Erasers and Stars 4
    0x09DB5, // Boathouse Erasers and Stars 5
    0x09DB1, // Boathouse Erasers and Stars 6
    0x3C124, // Boathouse Erasers and Stars 7
    0x09DB3, // Boathouse Erasers Shapers and Stars 1
    0x09DB4, // Boathouse Erasers Shapers and Stars 2
    0x0A3CB, // Boathouse Erasers Shapers and Stars 3
    0x0A3CC, // Boathouse Erasers Shapers and Stars 4
    0x0A3D0, // Boathouse Erasers Shapers and Stars 5
    0x09E57, // Quarry Entry Gate 1
    0x17C09, // Quarry Entry Gate 2
    0x0288C, // Treehouse Door 1
    0x02886, // Treehouse Door 2
    0x17D72, // Treehouse Yellow 1
    0x17D8F, // Treehouse Yellow 2
    0x17D74, // Treehouse Yellow 3
    0x17DAC, // Treehouse Yellow 4
    0x17D9E, // Treehouse Yellow 5
    0x17DB9, // Treehouse Yellow 6
    0x17D9C, // Treehouse Yellow 7
    0x17DC2, // Treehouse Yellow 8
    0x17DC4, // Treehouse Yellow 9
    0x0A182, // Treehouse Door 3
    0x17DC8, // Treehouse First Purple 1
    0x17DC7, // Treehouse First Purple 2
    0x17CE4, // Treehouse First Purple 3
    0x17D2D, // Treehouse First Purple 4
    0x17D6C, // Treehouse First Purple 5
    0x17D9B, // Treehouse Second Purple 1
    0x17D99, // Treehouse Second Purple 2
    0x17DAA, // Treehouse Second Purple 3
    0x17D97, // Treehouse Second Purple 4
    0x17BDF, // Treehouse Second Purple 5
    0x17D91, // Treehouse Second Purple 6
    0x17DC6, // Treehouse Second Purple 7
    0x17DB3, // Treehouse Left Orange 1
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
    0x17D88, // Treehouse Right Orange 1
    0x17DB4, // Treehouse Right Orange 2
    0x17D8C, // Treehouse Right Orange 3
    0x17DCD, // Treehouse Right Orange 5
    0x17DB2, // Treehouse Right Orange 6
    0x17DCC, // Treehouse Right Orange 7
    0x17DCA, // Treehouse Right Orange 8
    0x17D8E, // Treehouse Right Orange 9
    0x17DB1, // Treehouse Right Orange 11
    0x17DA2, // Treehouse Right Orange 12
    0x17E3C, // Treehouse Green 1
    0x17E4D, // Treehouse Green 2
    0x17E4F, // Treehouse Green 3
    0x17E5B, // Treehouse Green 5
    0x17E5F, // Treehouse Green 6
    0x17E61, // Treehouse Green 7
    0x17FA9, // Treehouse Green Bridge Discard
    0x17FA0, // Treehouse Laser Discard
    0x00139, // Keep Hedges 1
    0x019DC, // Keep Hedges 2
    0x019E7, // Keep Hedges 3
    0x01A0F, // Keep Hedges 4
    0x0360E, // Keep Front Laser
    0x03317, // Keep Back Laser
    0x17D27, // Keep Discard
    0x17D28, // Shipwreck Discard
    0x00AFB, // Shipwreck Vault
    0x2899C, // Town 25 Dots 1
    0x28A33, // Town 25 Dots 2
    0x28ABF, // Town 25 Dots 3
    0x28AC0, // Town 25 Dots 4
    0x28AC1, // Town 25 Dots 5
    0x28938, // Town Apple Tree
    0x28AC7, // Town Blue 1
    0x28AC8, // Town Blue 2
    0x28ACA, // Town Blue 3
    0x28ACB, // Town Blue 4
    0x28ACC, // Town Blue 5
    0x28A0D, // Town Church Stars
    0x28AD9, // Town Eraser
    0x28998, // Town Green Door
    0x0A0C8, // Town Orange Crate
    0x17D01, // Town Orange Crate Discard
    0x03C08, // Town RGB Stars
    0x03C0C, // Town RGB Stones
    0x17C71, // Town Rooftop Discard
    0x17F5F, // Town Windmill Door
    0x17F89, // Theater Entrance
    0x17CF7, // Theater Discard
    0x33AB2, // Theater Corona Exit
    0x0A168, // Theater Sun Exit
    0x00B10, // Monastery Left Door
    0x00C92, // Monastery Right Door
    0x17C2E, // Bunker Entry Door
    0x00469, // Swamp Tutorial 1
    0x00472, // Swamp Tutorial 2
    0x00262, // Swamp Tutorial 3
    0x00474, // Swamp Tutorial 4
    0x00553, // Swamp Tutorial 5
    0x0056F, // Swamp Tutorial 6
    0x00390, // Swamp Tutorial 7
    0x010CA, // Swamp Tutorial 8
    0x00983, // Swamp Tutorial 9
    0x00984, // Swamp Tutorial 10
    0x00986, // Swamp Tutorial 11
    0x00985, // Swamp Tutorial 12
    0x00987, // Swamp Tutorial 13
    0x181A9, // Swamp Tutorial 14
    0x00982, // Swamp Red 1
    0x0097F, // Swamp Red 2
    0x0098F, // Swamp Red 3
    0x00990, // Swamp Red 4
    0x17C0D, // Swamp Red Shortcut 1
    0x17C0E, // Swamp Red Shortcut 2
    0x00999, // Swamp Discontinuous 1
    0x0099D, // Swamp Discontinuous 2
    0x009A0, // Swamp Discontinuous 3
    0x009A1, // Swamp Discontinuous 4
    0x00007, // Swamp Rotation Tutorial 1
    0x00008, // Swamp Rotation Tutorial 2
    0x00009, // Swamp Rotation Tutorial 3
    0x0000A, // Swamp Rotation Tutorial 4
    0x009AB, // Swamp Blue Underwater 1
    0x009AD, // Swamp Blue Underwater 2
    0x009AE, // Swamp Blue Underwater 3
    0x009AF, // Swamp Blue Underwater 4
    0x00006, // Swamp Blue Underwater 5
    0x003B2, // Swamp Rotation Advanced 1
    0x00A1E, // Swamp Rotation Advanced 2
    0x00C2E, // Swamp Rotation Advanced 3
    0x00E3A, // Swamp Rotation Advanced 4
    0x009A6, // Swamp Purple Tetris
    0x00002, // Swamp Teal Underwater 1
    0x00004, // Swamp Teal Underwater 2
    0x00005, // Swamp Teal Underwater 3
    0x013E6, // Swamp Teal Underwater 4
    0x00596, // Swamp Teal Underwater 5
    0x00001, // Swamp Red Underwater 1
    0x014D2, // Swamp Red Underwater 2
    0x014D4, // Swamp Red Underwater 3
    0x014D1, // Swamp Red Underwater 4
    0x17C05, // Swamp Laser Shortcut 1
    0x17C02, // Swamp Laser Shortcut 2
    0x17F9B, // Jungle Discard
    0x09E73, // Mountain 1 Orange 1
    0x09E75, // Mountain 1 Orange 2
    0x09E78, // Mountain 1 Orange 3
    0x09E79, // Mountain 1 Orange 4
    0x09E6C, // Mountain 1 Orange 5
    0x09E6F, // Mountain 1 Orange 6
    0x09E6B, // Mountain 1 Orange 7
    0x09EAD, // Mountain 1 Purple 1
    0x09EAF, // Mountain 1 Purple 2
    0x09E7A, // Mountain 1 Green 1
    0x09E71, // Mountain 1 Green 2
    0x09E72, // Mountain 1 Green 3
    0x09E69, // Mountain 1 Green 4
    0x09E7B, // Mountain 1 Green 5
    0x09FD3, // Mountain 2 Rainbow 1
    0x09FD4, // Mountain 2 Rainbow 2
    0x09FD6, // Mountain 2 Rainbow 3
    0x09FD7, // Mountain 2 Rainbow 4
    0x09FD8, // Mountain 2 Rainbow 5
    0x17F93, // Mountain 2 Discard
    0x17FA2, // Mountain 3 Secret Door
    0x17C42, // Mountainside Discard
    0x002A6, // Mountainside Vault
    0x0042D, // Mountaintop River
    0x00FF8, // UTM Entrance Door
    0x0A16B, // UTM Green Dots 1
    0x0A2CE, // UTM Green Dots 2
    0x0A2D7, // UTM Green Dots 3
    0x0A2DD, // UTM Green Dots 4
    0x0A2EA, // UTM Green Dots 5
    0x17FB9, // UTM Green Dots 6
    0x0008F, // UTM Invisible Dots 1
    0x0006B, // UTM Invisible Dots 2
    0x0008B, // UTM Invisible Dots 3
    0x0008C, // UTM Invisible Dots 4
    0x0008A, // UTM Invisible Dots 5
    0x00089, // UTM Invisible Dots 6
    0x0006A, // UTM Invisible Dots 7
    0x0006C, // UTM Invisible Dots 8
    0x00027, // UTM Invisible Dots Symmetry 1
    0x00028, // UTM Invisible Dots Symmetry 2
    0x00029, // UTM Invisible Dots Symmetry 3
    0x021D7, // UTM Mountainside Shortcut
    0x00B71, // UTM Quarry
    0x01A31, // UTM Rainbow
    0x32962, // UTM Swamp
    0x32966, // UTM Treehouse
    0x17CF2, // UTM Waterfall Shortcut
    0x00A72, // UTM Blue Cave In
    0x009A4, // UTM Blue Discontinuous
    0x018A0, // UTM Blue Easy Symmetry
    0x01A0D, // UTM Blue Hard Symmetry
    0x008B8, // UTM Blue Left 1
    0x00973, // UTM Blue Left 2
    0x0097B, // UTM Blue Left 3
    0x0097D, // UTM Blue Left 4
    0x0097E, // UTM Blue Left 5
    0x00994, // UTM Blue Right Far 1
    0x334D5, // UTM Blue Right Far 2
    0x00995, // UTM Blue Right Far 3
    0x00996, // UTM Blue Right Far 4
    0x00998, // UTM Blue Right Far 5
    0x00190, // UTM Blue Right Near 1
    0x00558, // UTM Blue Right Near 2
    0x00567, // UTM Blue Right Near 3
    0x006FE, // UTM Blue Right Near 4
    0x0A16E, // UTM Challenge Entrance
    0x039B4, // Tunnels Theater Catwalk
    0x09E85, // Tunnels Town Shortcut
};

inline std::vector<int> arrowPanels = {
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
    0x17CF7, // Theater Discard
    0x17F9B, // Jungle Discard
    0x17F93, // Mountain 2 Discard
    0x17FA2, // Mountain 3 Secret Door
    0x17C42, // Mountainside Discard
    0x00FF8, // UTM Entrance Door
    0x0A16B, // UTM Green Dots 1
    0x0A2CE, // UTM Green Dots 2
    0x0A2D7, // UTM Green Dots 3
    0x0A2DD, // UTM Green Dots 4
    0x0A2EA, // UTM Green Dots 5
    0x17FB9, // UTM Green Dots 6
    0x0008F, // UTM Invisible Dots 1
    0x0006B, // UTM Invisible Dots 2
    0x0008B, // UTM Invisible Dots 3
    0x0008C, // UTM Invisible Dots 4
    0x0008A, // UTM Invisible Dots 5
    0x00089, // UTM Invisible Dots 6
    0x0006A, // UTM Invisible Dots 7
    0x0006C, // UTM Invisible Dots 8
    0x00027, // UTM Invisible Dots Symmetry 1
    0x00028, // UTM Invisible Dots Symmetry 2
    0x00029, // UTM Invisible Dots Symmetry 3
    0x021D7, // UTM Mountainside Shortcut
    0x00B71, // UTM Quarry
    0x01A31, // UTM Rainbow
    0x32962, // UTM Swamp
    0x32966, // UTM Treehouse
    0x17CF2, // UTM Waterfall Shortcut
    0x00A72, // UTM Blue Cave In
    0x009A4, // UTM Blue Discontinuous
    0x018A0, // UTM Blue Easy Symmetry
    0x01A0D, // UTM Blue Hard Symmetry
    0x008B8, // UTM Blue Left 1
    0x00973, // UTM Blue Left 2
    0x0097B, // UTM Blue Left 3
    0x0097D, // UTM Blue Left 4
    0x0097E, // UTM Blue Left 5
    0x00994, // UTM Blue Right Far 1
    0x334D5, // UTM Blue Right Far 2
    0x00995, // UTM Blue Right Far 3
    0x00996, // UTM Blue Right Far 4
    0x00998, // UTM Blue Right Far 5
    0x00190, // UTM Blue Right Near 1
    0x00558, // UTM Blue Right Near 2
    0x00567, // UTM Blue Right Near 3
    0x006FE, // UTM Blue Right Near 4
    0x0A16E, // UTM Challenge Entrance
    0x039B4, // Tunnels Theater Catwalk
    0x09E85, // Tunnels Town Shortcut
};

inline std::vector<int> glassPanels = {
    0x01A54, // Glass Factory Entry
    0x00086, // Glass Factory Vertical Symmetry 1
    0x00087, // Glass Factory Vertical Symmetry 2
    0x00059, // Glass Factory Vertical Symmetry 3
    0x00062, // Glass Factory Vertical Symmetry 4
    0x0008D, // Glass Factory Rotational Symmetry 1
    0x00081, // Glass Factory Rotational Symmetry 2
    0x00083, // Glass Factory Rotational Symmetry 3
    0x00084, // Glass Factory Melting 1
    0x00082, // Glass Factory Melting 2
    0x0343A, // Glass Factory Melting 3
    0x000B0, // Symmetry Island Door 1
    0x00022, // Symmetry Island Black Dots 1
    0x00023, // Symmetry Island Black Dots 2
    0x00024, // Symmetry Island Black Dots 3
    0x00025, // Symmetry Island Black Dots 4
    0x00026, // Symmetry Island Black Dots 5
    0x0007C, // Symmetry Island Colored Dots 1
    0x0007E, // Symmetry Island Colored Dots 2
    0x00075, // Symmetry Island Colored Dots 3
    0x00073, // Symmetry Island Colored Dots 4
    0x00077, // Symmetry Island Colored Dots 5
    0x00079, // Symmetry Island Colored Dots 6
    0x00065, // Symmetry Island Fading Lines 1
    0x0006D, // Symmetry Island Fading Lines 2
    0x00072, // Symmetry Island Fading Lines 3
    0x0006F, // Symmetry Island Fading Lines 4
    0x00070, // Symmetry Island Fading Lines 5
    0x00071, // Symmetry Island Fading Lines 6
    0x00076, // Symmetry Island Fading Lines 7
    0x28998, // Town Green Door
};

inline std::vector<int> squarePanelsExpertBanned = {
    0x09E7A, // Mountain 1 Green 1
    0x09E71, // Mountain 1 Green 2
    0x09E72, // Mountain 1 Green 3
    0x09E69, // Mountain 1 Green 4
    0x09E7B, // Mountain 1 Green 5
};

inline std::vector<int> desertPanels = {
	0x00698, // Desert Surface 1
	0x0048F, // Desert Surface 2
	0x09F92, // Desert Surface 3
	0x0A036, // Desert Surface 4
	0x09DA6, // Desert Surface 5
	0x0A049, // Desert Surface 6
	0x0A053, // Desert Surface 7
	0x09F94, // Desert Surface 8
	0x00422, // Desert Light 1
	0x006E3, // Desert Light 2
	0x0A02D, // Desert Light 3
	0x00C72, // Desert Pond 1
	0x0129D, // Desert Pond 2
	0x008BB, // Desert Pond 3
	0x0078D, // Desert Pond 4
	0x18313, // Desert Pond 5
	0x04D18, // Desert Flood 1
	0x01205, // Desert Flood 2
	0x181AB, // Desert Flood 3
	0x0117A, // Desert Flood 4
	0x17ECA, // Desert Flood 5
//	0x18076, // Desert Flood Exit
//	0x0A15C, // Desert Final Left Convex
//	0x09FFF, // Desert Final Left Concave
//	0x0A15F, // Desert Final Near
	0x012D7, // Desert Final Far
};

inline std::vector<int> desertPanelsWide = {
    0x0A15C, //wide desert panel curve 1
    0x09FFF, //wide desert panel curve 2
};

inline std::vector<int> orchard{
    0x00143, // Orchard Apple Tree 1
    0x0003B, // Orchard Apple Tree 2
    0x00055, // Orchard Apple Tree 3
    0x032F7, // Orchard Apple Tree 4
    0x032FF, // Orchard Apple Tree 5
};

inline std::vector<int> transparent = {
    0x009B8, // Symmetry Island Transparent 1
    0x003E8, // Symmetry Island Transparent 2
    0x00A15, // Symmetry Island Transparent 3
    0x00B53, // Symmetry Island Transparent 4
    0x00B8D, // Symmetry Island Transparent 5
};

inline std::vector<int> shadowsPanels = {
	0x198B5, // Shadows Tutorial 1
	0x198BD, // Shadows Tutorial 2
	0x198BF, // Shadows Tutorial 3
	0x19771, // Shadows Tutorial 4
	0x0A8DC, // Shadows Tutorial 5
	0x0AC74, // Shadows Tutorial 6
	0x0AC7A, // Shadows Tutorial 7
	0x0A8E0, // Shadows Tutorial 8
	0x386FA, // Shadows Avoid 1
	0x1C33F, // Shadows Avoid 2
	0x196E2, // Shadows Avoid 3
	0x1972A, // Shadows Avoid 4
	0x19809, // Shadows Avoid 5
	0x19806, // Shadows Avoid 6
	0x196F8, // Shadows Avoid 7
	0x1972F, // Shadows Avoid 8
	0x19797, // Shadows Follow 1
	0x1979A, // Shadows Follow 2
	0x197E0, // Shadows Follow 3
	0x197E8, // Shadows Follow 4
	0x197E5, // Shadows Follow 5
	0x19650, // Shadows Laser
};

inline std::vector<int> monasteryPanels = {
	0x00B10, // Monastery Left Door
	0x00C92, // Monastery Right Door
	0x00290, // Monastery Exterior 1
	0x00038, // Monastery Exterior 2
	0x00037, // Monastery Exterior 3
//	0x09D9B, // Monastery Bonsai
	0x193A7, // Monastery Interior 1
	0x193AA, // Monastery Interior 2
	0x193AB, // Monastery Interior 3
	0x193A6, // Monastery Interior 4
//	0x03713, // Monastery Shortcut
	0x17CA4, // Monastery Laser
};

inline std::vector<int> bunkerPanels = {
	0x09F7D, // Bunker Tutorial 1
	0x09FDC, // Bunker Tutorial 2
	0x09FF7, // Bunker Tutorial 3
	0x09F82, // Bunker Tutorial 4
	0x09FF8, // Bunker Tutorial 5
	0x09D9F, // Bunker Advanced 1
	0x09DA1, // Bunker Advanced 2
	0x09DA2, // Bunker Advanced 3
	0x09DAF, // Bunker Advanced 4
//	0x0A099, // Bunker Glass Door
	0x0A010, // Bunker Glass 1
	0x0A01B, // Bunker Glass 2
	0x0A01F, // Bunker Glass 3
	0x17E63, // Bunker Ultraviolet 1
	0x17E67, // Bunker Ultraviolet 2
	0x34BC5, // Bunker Open Ultraviolet
	0x34BC6, // Bunker Close Ultraviolet
	0x0A079, // Bunker Elevator
};

inline std::vector<int> junglePanels = {
	0x002C4, // Jungle Waves 1
	0x00767, // Jungle Waves 2
	0x002C6, // Jungle Waves 3
	0x0070E, // Jungle Waves 4
	0x0070F, // Jungle Waves 5
	0x0087D, // Jungle Waves 6
	0x002C7, // Jungle Waves 7
//	0x17CAB, // Jungle Pop-up Wall
	0x0026D, // Jungle Dots 1
	0x0026E, // Jungle Dots 2
	0x0026F, // Jungle Dots 3
	0x00C3F, // Jungle Dots 4
	0x00C41, // Jungle Dots 5
	0x014B2, // Jungle Dots 6
	0x03616, // Jungle Laser
};

// There might be something to do with these, I haven't decided yet.
inline std::vector<int> nothingPanels = {
// Doors & Shortcuts & Shortcut doors & Door controls
	0x0C339, // Desert Surface Door
	0x0A249, // Desert Pond Exit Door
	0x275ED, // Mill EP Door
	0x17CAC, // Mill Stairs Shortcut Door
	0x38663, // Boathouse Shortcut
	0x09E49, // Keep Shadows Shortcut
	0x0361B, // Keep Tower Shortcut
	0x334DC, // Shadows Inner Door Control
	0x334DB, // Shadows Outer Door Control
	0x2700B, // Treehouse Exterior Door Control
	0x17CBC, // Treehouse Interior Door Control
	0x337FA, // Jungle Shortcut

// Controls (?)
	0x09FA0, // Desert Surface 3 Control
	0x09F86, // Desert Surface 8 Control
	0x1C2DF, // Desert Flood Control Lower Far Left
	0x1831E, // Desert Flood Control Lower Far Right
	0x1C260, // Desert Flood Control Lower Near Left
	0x1831C, // Desert Flood Control Lower Near Right
	0x1C2F3, // Desert Flood Control Raise Far Left
	0x1831D, // Desert Flood Control Raise Far Right
	0x1C2B1, // Desert Flood Control Raise Near Left
	0x1831B, // Desert Flood Control Raise Near Right
	0x0A015, // Desert Final Far Control
	0x03678, // Mill Lower Ramp Contol
	0x03679, // Mill Lower Lift Control
	0x03675, // Mill Upper Lift Control
	0x03676, // Mill Upper Ramp Control
	0x03852, // Boathouse Ramp Angle Control
	0x03858, // Boathouse Ramp Position Control
	0x275FA, // Boathouse Hook Control
	0x037FF, // Treehouse Drawbridge Control
	0x09F98, // Town Laser Redirect Control
	0x334D8, // Town RGB Light Control
	0x17E2B, // Swamp Flood Gate Control
	0x00609, // Swamp Surface Sliding Bridge Control
	0x18488, // Swamp Underwater Sliding Bridge Control
	0x17C0A, // Swamp Island Control 1
	0x17E07, // Swamp Island Control 2
	0x181F5, // Swamp Rotating Bridge Control

// Vault Boxes
	0x03481, // Outside Tutorial Vault Box
	0x0339E, // Desert Vault Box
	0x03535, // Shipwreck Vault Box
	0x03702, // Jungle Vault Box
	0x03542, // Mountainside Vault Box
	0x2FAF6, // Tunnels Vault Box

// Boat Summons
	0x17CC8, // Glass Factory Summon Boat
	0x17CA6, // Boathouse Summon Boat
	0x17C95, // Treehouse Summon Boat
	0x0A054, // Town Summon Boat
	0x09DB8, // Swamp Summon Boat
	0x17CDF, // Jungle Summon Boat

// Identical sets
	0x00143, // Orchard Apple Tree 1
	0x0003B, // Orchard Apple Tree 2
	0x00055, // Orchard Apple Tree 3
	0x032F7, // Orchard Apple Tree 4
	0x032FF, // Orchard Apple Tree 5
	0x009B8, // Symmetry Island Transparent 1
	0x003E8, // Symmetry Island Transparent 2
	0x00A15, // Symmetry Island Transparent 3
	0x00B53, // Symmetry Island Transparent 4
	0x00B8D, // Symmetry Island Transparent 5

// Misc
	0x03629, // Tutorial Gate Open
	0x09FAA, // Desert Lightswitch
	0x0C335, // Tutorial Pillar
	0x0C373, // Tutorial Patio floor
	0x1C349, // Symmetry Island Door 2 - Collision fails here, sadly
	0x033EA, // Keep Yellow Pressure Plates
	0x0A3A8, // Keep Yellow Reset
	0x01BE9, // Keep Purple Pressure Plates
	0x0A3B9, // Keep Purple Reset
	0x01CD3, // Keep Green Pressure Plates
	0x0A3BB, // Keep Green Reset
	0x01D3F, // Keep Blue Pressure Plates
	0x0A3AD, // Keep Blue Reset
	0x2896A, // Town Bridge
	0x28A69, // Town Lattice
	0x28A79, // Town Maze
	0x28B39, // Town Red Hexagonal
	0x034E3, // Town Soundproof Dots
	0x034E4, // Town Soundproof Waves
	0x079DF, // Town Triple
	0x00815, // Theater Video Input
	0x03553, // Theater Tutorial Video
	0x03552, // Theater Desert Video
	0x0354E, // Theater Jungle Video
	0x03549, // Theater Challenge Video
	0x0354F, // Theater Shipwreck Video
	0x03545, // Theater Mountain Video
	0x18590, // Town Transparent
	0x28AE3, // Town Wire


	0x09E39, // Mountain 1 Purple Pathway
//	0x33AF5, // Mountain 1 Blue 1
//	0x33AF7, // Mountain 1 Blue 2
//	0x09F6E, // Mountain 1 Blue 3
//	0x09FD8, // Mountain 2 Rainbow 5
	0x09E86, // Mountain 2 Blue Pathway
	0x09ED8, // Mountain 2 Orange Pathway
	0x09F8E, // Mountain 3 Near Right Floor
	0x09FC1, // Mountain 3 Near Left Floor
	0x09F01, // Mountain 3 Far Right Floor
	0x09EFF, // Mountain 3 Far Left Floor
	0x09FDA, // Mountain 3 Giant Floor
//	0x01983, // Mountain 3 Left Peekaboo
//	0x01987, // Mountain 3 Right Peekaboo
//	0x3D9A6, // Mountain 3 Left Close Door
//	0x3D9A7, // Mountain 3 Right Close Door
//	0x3D9AA, // Mountain 3 Left Activate Elevator
//	0x3D9A8, // Mountain 3 Right Activate Elevator
//	0x3D9A9, // Mountain 3 Launch Elevator
//	0x3C113, // Mountain 3 Left Open Door
//	0x3C114, // Mountain 3 Right Open Door
	0x09F7F, // Mountaintop Laser Box
	0x17C34, // Mountaintop Perspective
//	0x334E1, // UTM Secret Door Control
//	0x2773D, // Tunnels Desert Shortcut
//	0x27732, // Tunnels Theater Shortcut
//	0x0A099, // Bunker Glass Door
//	0x15ADD, // Jungle Vault
//	0x17CAA, // Jungle Courtyard Gate
	0x0005C, // Glass Factory Vertical Symmetry 5
	0x17C31, // Desert Final Transparent
	0x19650, // Shadows Laser
	0x09EEB, // Mountain 2 Elevator
};

inline std::set<int> allPanels = {
    0x00064,
0x00182,
0x00293,
0x00295,
0x002C2,
0x0A3B5,
0x0A3B2,
0x03629,
0x03505,
0x0C335,
0x0C373,
0x033D4,
0x03481,
0x0A171,
0x17CFB,
0x04CA4,
0x0005D,
0x0005E,
0x0005F,
0x00060,
0x00061,
0x018AF,
0x0001B,
0x012C9,
0x0001C,
0x0001D,
0x0001E,
0x0001F,
0x00020,
0x00021,
0x01A54,
0x3C12B,
0x00086,
0x00087,
0x00059,
0x00062,
0x0005C,
0x0008D,
0x00081,
0x00083,
0x00084,
0x00082,
0x0343A,
0x17CC8,
0x000B0,
0x00022,
0x00023,
0x00024,
0x00025,
0x00026,
0x0007C,
0x0007E,
0x00075,
0x00073,
0x00077,
0x00079,
0x00065,
0x0006D,
0x00072,
0x0006F,
0x00070,
0x00071,
0x00076,
0x009B8,
0x003E8,
0x00A15,
0x00B53,
0x00B8D,
0x1C349,
0x00A52,
0x00A57,
0x00A5B,
0x00A61,
0x00A64,
0x00A68,
0x0360D,
0x00143,
0x0003B,
0x00055,
0x032F7,
0x032FF,
0x0CC7B,
0x0339E,
0x17CE7,
0x00698,
0x0048F,
0x09F92,
0x09FA0,
0x0A036,
0x09DA6,
0x0A049,
0x0A053,
0x09F94,
0x09F86,
0x0C339,
0x09FAA,
0x00422,
0x006E3,
0x0A02D,
0x00C72,
0x0129D,
0x008BB,
0x0078D,
0x18313,
0x0A249,
0x1C2DF,
0x1831E,
0x1C260,
0x1831C,
0x1C2F3,
0x1831D,
0x1C2B1,
0x1831B,
0x04D18,
0x01205,
0x181AB,
0x0117A,
0x17ECA,
0x18076,
0x17C31,
0x012D7,
0x0A015,
0x0A15C,
0x09FFF,
0x0A15F,
0x03608,
0x09E57,
0x17C09,
0x17CC4,
0x01E5A,
0x01E59,
0x17CF0,
0x03612,
0x275ED,
0x03678,
0x00E0C,
0x01489,
0x0148A,
0x014D9,
0x014E7,
0x014E8,
0x03679,
0x03675,
0x03676,
0x00557,
0x005F1,
0x00620,
0x009F5,
0x0146C,
0x3C12D,
0x03686,
0x014E9,
0x03677,
0x3C125,
0x0367C,
0x17CAC,
0x034D4,
0x021D5,
0x03852,
0x021B3,
0x021B4,
0x021B0,
0x021AF,
0x021AE,
0x03858,
0x38663,
0x021B5,
0x021B6,
0x021B7,
0x021BB,
0x09DB5,
0x09DB1,
0x3C124,
0x09DB3,
0x09DB4,
0x275FA,
0x17CA6,
0x0A3CB,
0x0A3CC,
0x0A3D0,
0x334DB,
0x0AC74,
0x0AC7A,
0x0A8E0,
0x386FA,
0x1C33F,
0x196E2,
0x1972A,
0x19809,
0x19806,
0x196F8,
0x1972F,
0x19797,
0x1979A,
0x197E0,
0x197E8,
0x197E5,
0x19650,
0x334DC,
0x198B5,
0x198BD,
0x198BF,
0x19771,
0x0A8DC,
0x00139,
0x019DC,
0x019E7,
0x01A0F,
0x0A3A8,
0x033EA,
0x0A3B9,
0x01BE9,
0x0A3BB,
0x01CD3,
0x0A3AD,
0x01D3F,
0x17D27,
0x09E49,
0x00AFB,
0x03535,
0x17D28,
0x0361B,
0x0360E,
0x03317,
0x03713,
0x00B10,
0x00C92,
0x00290,
0x00038,
0x00037,
0x17CA4,
0x09D9B,
0x193A7,
0x193AA,
0x193AB,
0x193A6,
0x0A054,
0x0A0C8,
0x17D01,
0x09F98,
0x18590,
0x28AE3,
0x28938,
0x079DF,
0x28B39,
0x28998,
0x28A0D,
0x28A69,
0x28A79,
0x2896A,
0x17C71,
0x28AC7,
0x28AC8,
0x28ACA,
0x28ACB,
0x28ACC,
0x2899C,
0x28A33,
0x28ABF,
0x28AC0,
0x28AC1,
0x28AD9,
0x17F5F,
0x034E4,
0x034E3,
0x334D8,
0x03C0C,
0x03C08,
0x032F5,
0x17D02,
0x17F89,
0x00815,
0x03553,
0x03552,
0x0354E,
0x03549,
0x0354F,
0x03545,
0x0A168,
0x33AB2,
0x17CF7,
0x17CDF,
0x17F9B,
0x002C4,
0x00767,
0x002C6,
0x0070E,
0x0070F,
0x0087D,
0x002C7,
0x17CAB,
0x0026D,
0x0026E,
0x0026F,
0x00C3F,
0x00C41,
0x014B2,
0x03616,
0x337FA,
0x17CAA,
0x15ADD,
0x03702,
0x17C2E,
0x09DE0,
0x09F7D,
0x09FDC,
0x09FF7,
0x09F82,
0x09FF8,
0x09D9F,
0x09DA1,
0x09DA2,
0x09DAF,
0x0A099,
0x0A010,
0x0A01B,
0x0A01F,
0x34BC5,
0x34BC6,
0x17E63,
0x17E67,
0x0A079,
0x0056E,
0x00469,
0x00472,
0x00262,
0x00474,
0x00553,
0x0056F,
0x00390,
0x010CA,
0x00983,
0x00984,
0x00986,
0x00985,
0x00987,
0x181A9,
0x00609,
0x00999,
0x0099D,
0x009A0,
0x009A1,
0x00002,
0x00004,
0x00005,
0x013E6,
0x00596,
0x18488,
0x00982,
0x0097F,
0x0098F,
0x00990,
0x17C0D,
0x17C0E,
0x00007,
0x00008,
0x00009,
0x0000A,
0x00001,
0x014D2,
0x014D4,
0x014D1,
0x181F5,
0x09DB8,
0x003B2,
0x00A1E,
0x00C2E,
0x00E3A,
0x009A6,
0x009AB,
0x009AD,
0x009AE,
0x009AF,
0x00006,
0x17E2B,
0x17C0A,
0x17E07,
0x03615,
0x17C05,
0x17C02,
0x17C95,
0x0288C,
0x02886,
0x17D72,
0x17D8F,
0x17D74,
0x17DAC,
0x17D9E,
0x17DB9,
0x17D9C,
0x17DC2,
0x17DC4,
0x0A182,
0x2700B,
0x17DC8,
0x17DC7,
0x17CE4,
0x17D2D,
0x17D6C,
0x17D9B,
0x17D99,
0x17DAA,
0x17D97,
0x17BDF,
0x17D91,
0x17DC6,
0x17E3C,
0x17E4D,
0x17E4F,
0x17E52,
0x17E5B,
0x17E5F,
0x17E61,
0x17FA9,
0x17DB3,
0x17DB5,
0x17DB6,
0x17DC0,
0x17DD7,
0x17DD9,
0x17DB8,
0x17DDC,
0x17DD1,
0x17DDE,
0x17DE3,
0x17DEC,
0x17DAE,
0x17DB0,
0x17DDB,
0x17FA0,
0x17D88,
0x17DB4,
0x17D8C,
0x17CE3,
0x17DCD,
0x17DB2,
0x17DCC,
0x17DCA,
0x17D8E,
0x17DB7,
0x17DB1,
0x17DA2,
0x03613,
0x17CBC,
0x037FF,
0x0042D,
0x09F7F,
0x17C34,
0x17C42,
0x002A6,
0x03542,
0x09E39,
0x09E7A,
0x09E71,
0x09E72,
0x09E69,
0x09E7B,
0x09E73,
0x09E75,
0x09E78,
0x09E79,
0x09E6C,
0x09E6F,
0x09E6B,
0x33AF5,
0x33AF7,
0x09F6E,
0x09EAD,
0x09EAF,
0x09FD3,
0x09FD4,
0x09FD6,
0x09FD7,
0x09FD8,
0x09E86,
0x09FCC,
0x09FCE,
0x09FCF,
0x09FD0,
0x09FD1,
0x09FD2,
0x09ED8,
0x09EEB,
0x17F93,
0x09FC1,
0x09F8E,
0x09F01,
0x09EFF,
0x09FDA,
0x17FA2,
0x01983,
0x01987,
0x00FF8,
0x334E1,
0x021D7,
0x17CF2,
0x335AB,
0x335AC,
0x3369D,
0x00190,
0x00558,
0x00567,
0x006FE,
0x01A0D,
0x008B8,
0x00973,
0x0097B,
0x0097D,
0x0097E,
0x00994,
0x334D5,
0x00995,
0x00996,
0x00998,
0x009A4,
0x018A0,
0x00A72,
0x32962,
0x32966,
0x01A31,
0x00B71,
0x09DD5,
0x0A16E,
0x288EA,
0x288FC,
0x289E7,
0x288AA,
0x17FB9,
0x0A16B,
0x0A2CE,
0x0A2D7,
0x0A2DD,
0x0A2EA,
0x0008F,
0x0006B,
0x0008B,
0x0008C,
0x0008A,
0x00089,
0x0006A,
0x0006C,
0x00027,
0x00028,
0x00029,
0x0A332,
0x0088E,
0x00BAF,
0x00BF3,
0x00C09,
0x00CDB,
0x0051F,
0x00524,
0x00CD4,
0x00CB9,
0x00CA1,
0x00C80,
0x00C68,
0x00C59,
0x00C22,
0x034F4,
0x034EC,
0x1C31A,
0x1C319,
0x0356B,
0x039B4,
0x2FAF6,
0x27732,
0x2773D,
0x09E85,
0x0383A,
0x09E56,
0x09E5A,
0x33961,
0x0383D,
0x0383F,
0x03859,
0x339BB,
0x3D9A6,
0x3D9A7,
0x3C113,
0x3C114,
0x3D9AA,
0x3D9A8,
0x3D9A9,
};