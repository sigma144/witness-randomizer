#pragma once
#include "Panels.h"

std::vector<PanelID> squarePanels = {
    TUT_ENTER_1, // Tutorial Straight
    TUT_ENTER_2, // Tutorial Bend
    TUT_MAZE_1, // Tutorial Front Center
    TUT_MAZE_2, // Tutorial Center Left
    TUT_MAZE_3, // Tutorial Front Left
    TUT_2START, // Tutorial Back Right
    TUT_OUTER_1, // Tutorial Optional Door 1
    TUT_OUTER_2, // Tutorial Optional Door 2
    TUT_DISCARD, // Outside Tutorial Discard
    TUT_DOT_1, // Outside Tutorial Dots Tutorial 1
    TUT_DOT_2, // Outside Tutorial Dots Tutorial 2
    TUT_DOT_3 , // Outside Tutorial Dots Tutorial 3
    TUT_DOT_4, // Outside Tutorial Dots Tutorial 4
    TUT_DOT_5, // Outside Tutorial Dots Tutorial 5
    TUT_STONE_1 , // Outside Tutorial Stones Tutorial 1
    TUT_STONE_2, // Outside Tutorial Stones Tutorial 2
    TUT_STONE_3, // Outside Tutorial Stones Tutorial 3
    TUT_STONE_4, // Outside Tutorial Stones Tutorial 4
    TUT_STONE_5, // Outside Tutorial Stones Tutorial 5
    TUT_STONE_6, // Outside Tutorial Stones Tutorial 6
    TUT_STONE_7, // Outside Tutorial Stones Tutorial 7
    TUT_STONE_8, // Outside Tutorial Stones Tutorial 8
    TUT_STONE_9, // Outside Tutorial Stones Tutorial 9
    TUT_VAULT, // Outside Tutorial Vault

    SYM_DISCARD, // Glass Factory Discard
    SYM_MAZE_V1, // Glass Factory Vertical Symmetry 1
    SYM_MAZE_V2, // Glass Factory Vertical Symmetry 2
    SYM_MAZE_V3, // Glass Factory Vertical Symmetry 3
    SYM_MAZE_V4, // Glass Factory Vertical Symmetry 4
    SYM_MAZE_R1, // Glass Factory Rotational Symmetry 1
    SYM_MAZE_R2, // Glass Factory Rotational Symmetry 2
    SYM_MAZE_R3, // Glass Factory Rotational Symmetry 3
    SYM_MAZE_M1, // Glass Factory Melting 1
    SYM_MAZE_M2, // Glass Factory Melting 2
    SYM_MAZE_M3, // Glass Factory Melting 3
    SYM_DOT_ENTER, // Symmetry Island Door 1
    SYM_DOT_1, // Symmetry Island Black Dots 1
    SYM_DOT_2, // Symmetry Island Black Dots 2
    SYM_DOT_3, // Symmetry Island Black Dots 3
    SYM_DOT_4, // Symmetry Island Black Dots 4
    SYM_DOT_5, // Symmetry Island Black Dots 5
    SYM_COLOR_1, // Symmetry Island Colored Dots 1
    SYM_COLOR_2, // Symmetry Island Colored Dots 2
    SYM_COLOR_3, // Symmetry Island Colored Dots 3
    SYM_COLOR_4, // Symmetry Island Colored Dots 4
    SYM_COLOR_5, // Symmetry Island Colored Dots 5
    SYM_COLOR_6, // Symmetry Island Colored Dots 6
    SYM_INVISIBLE_1, // Symmetry Island Fading Lines 1
    SYM_INVISIBLE_2, // Symmetry Island Fading Lines 2
    SYM_INVISIBLE_3, // Symmetry Island Fading Lines 3
    SYM_INVISIBLE_4, // Symmetry Island Fading Lines 4
    SYM_INVISIBLE_5, // Symmetry Island Fading Lines 5
    SYM_INVISIBLE_6, // Symmetry Island Fading Lines 6
    SYM_INVISIBLE_7, // Symmetry Island Fading Lines 7

    DESERT_DISCARD, // Desert Discard
    DESERT_VAULT, // Desert Vault

    QUARRY_ENTER_1, // Quarry Entry Gate 1
    QUARRY_ENTER_2, // Quarry Entry Gate 2
    MILL_ENTER_L, // Mill Entry Door Left
    MILL_ENTER_R, // Mill Entry Door Right
    MILL_DOT_1, // Mill Lower Row 1
    MILL_DOT_2, // Mill Lower Row 2
    MILL_DOT_3, // Mill Lower Row 3
    MILL_DOT_4, // Mill Lower Row 4
    MILL_DOT_5, // Mill Lower Row 5
    MILL_DOT_6, // Mill Lower Row 6
    MILL_STONE_1, // Mill Upper Row 1
    MILL_STONE_2, // Mill Upper Row 2
    MILL_STONE_3, // Mill Upper Row 3
    MILL_STONE_4, // Mill Upper Row 4
    MILL_STONE_5, // Mill Upper Row 5
    MILL_STONE_6, // Mill Upper Row 6
    MILL_STONE_7, // Mill Upper Row 7
    MILL_STONE_8, // Mill Upper Row 8
    MILL_FINAL, // Mill Control Room 1
    MILL_OPTIONAL, // Mill Control Room 2
    MILL_STAIRS, // Mill Stairs Control
    MILL_DISCARD, // Mill Discard
    BOATHOUSE_BEGIN_L, // Boathouse Ramp Activation Stars
    BOATHOUSE_BEGIN_R, // Boathouse Ramp Activation Shapers
    BOATHOUSE_SHAPE_1, // Boathouse Erasers and Shapers 1
    BOATHOUSE_SHAPE_2, // Boathouse Erasers and Shapers 2
    BOATHOUSE_SHAPE_3, // Boathouse Erasers and Shapers 3
    BOATHOUSE_SHAPE_4, // Boathouse Erasers and Shapers 4
    BOATHOUSE_SHAPE_5, // Boathouse Erasers and Shapers 5
    BOATHOUSE_STAR_1, // Boathouse Erasers and Stars 1
    BOATHOUSE_STAR_2, // Boathouse Erasers and Stars 2
    BOATHOUSE_STAR_3, // Boathouse Erasers and Stars 3
    BOATHOUSE_STAR_4 , // Boathouse Erasers and Stars 4
    BOATHOUSE_STAR_5, // Boathouse Erasers and Stars 5
    BOATHOUSE_STAR_6, // Boathouse Erasers and Stars 6
    BOATHOUSE_STAR_7, // Boathouse Erasers and Stars 7
    BOATHOUSE_STAR_8, // Boathouse Erasers Shapers and Stars 1
    BOATHOUSE_STAR_9, // Boathouse Erasers Shapers and Stars 2
    BOATHOUSE_FINAL_1, // Boathouse Erasers Shapers and Stars 3
    BOATHOUSE_FINAL_2, // Boathouse Erasers Shapers and Stars 4
    BOATHOUSE_FINAL_3, // Boathouse Erasers Shapers and Stars 5

    TREEHOUSE_ENTER_1, // Treehouse Door 1
    TREEHOUSE_ENTER_2, // Treehouse Door 2
    TREEHOUSE_YELLOW_1, // Treehouse Yellow 1
    TREEHOUSE_YELLOW_2, // Treehouse Yellow 2
    TREEHOUSE_YELLOW_3, // Treehouse Yellow 3
    TREEHOUSE_YELLOW_4, // Treehouse Yellow 4
    TREEHOUSE_YELLOW_5, // Treehouse Yellow 5
    TREEHOUSE_YELLOW_6, // Treehouse Yellow 6
    TREEHOUSE_YELLOW_7, // Treehouse Yellow 7
    TREEHOUSE_YELLOW_8, // Treehouse Yellow 8
    TREEHOUSE_YELLOW_9, // Treehouse Yellow 9
    TREEHOUSE_YELLOW_DOOR, // Treehouse Door 3
    TREEHOUSE_PURPLE_A1, // Treehouse First Purple 1
    TREEHOUSE_PURPLE_A2, // Treehouse First Purple 2
    TREEHOUSE_PURPLE_A3, // Treehouse First Purple 3
    TREEHOUSE_PURPLE_A4, // Treehouse First Purple 4
    TREEHOUSE_PURPLE_A5, // Treehouse First Purple 5
    TREEHOUSE_PURPLE_B1, // Treehouse Second Purple 1
    TREEHOUSE_PURPLE_B2, // Treehouse Second Purple 2
    TREEHOUSE_PURPLE_B3, // Treehouse Second Purple 3
    TREEHOUSE_PURPLE_B4, // Treehouse Second Purple 4
    TREEHOUSE_PURPLE_B5, // Treehouse Second Purple 5
    TREEHOUSE_PURPLE_B6, // Treehouse Second Purple 6
    TREEHOUSE_PURPLE_B7, // Treehouse Second Purple 7
    TREEHOUSE_ORANGE_L1, // Treehouse Left Orange 1
    TREEHOUSE_ORANGE_L2, // Treehouse Left Orange 2
    TREEHOUSE_ORANGE_L3, // Treehouse Left Orange 3
    TREEHOUSE_ORANGE_L4, // Treehouse Left Orange 4
    TREEHOUSE_ORANGE_L5, // Treehouse Left Orange 5
    TREEHOUSE_ORANGE_L6, // Treehouse Left Orange 6
    TREEHOUSE_ORANGE_L7, // Treehouse Left Orange 7
    TREEHOUSE_ORANGE_L8, // Treehouse Left Orange 8
    TREEHOUSE_ORANGE_L10, // Treehouse Left Orange 10
    TREEHOUSE_ORANGE_L11, // Treehouse Left Orange 11
    TREEHOUSE_ORANGE_L12, // Treehouse Left Orange 12
    TREEHOUSE_ORANGE_L13, // Treehouse Left Orange 13
    TREEHOUSE_ORANGE_L14, // Treehouse Left Orange 14
    TREEHOUSE_ORANGE_L15, // Treehouse Left Orange 15
    TREEHOUSE_ORANGE_DISCARD, // Treehouse Laser Discard
    TREEHOUSE_ORANGE_R1, // Treehouse Right Orange 1
    TREEHOUSE_ORANGE_R2, // Treehouse Right Orange 2
    TREEHOUSE_ORANGE_R3, // Treehouse Right Orange 3
    TREEHOUSE_ORANGE_R5, // Treehouse Right Orange 5
    TREEHOUSE_ORANGE_R6, // Treehouse Right Orange 6
    TREEHOUSE_ORANGE_R7, // Treehouse Right Orange 7
    TREEHOUSE_ORANGE_R8, // Treehouse Right Orange 8
    TREEHOUSE_ORANGE_R9, // Treehouse Right Orange 9
    TREEHOUSE_ORANGE_R11, // Treehouse Right Orange 11
    TREEHOUSE_ORANGE_R12, // Treehouse Right Orange 12
    TREEHOUSE_GREEN_1, // Treehouse Green 1
    TREEHOUSE_GREEN_2, // Treehouse Green 2
    TREEHOUSE_GREEN_3, // Treehouse Green 3
    TREEHOUSE_GREEN_5, // Treehouse Green 5
    TREEHOUSE_GREEN_6, // Treehouse Green 6
    TREEHOUSE_GREEN_7, // Treehouse Green 7
    TREEHOUSE_GREEN_DISCARD, // Treehouse Green Bridge Discard

    KEEP_PRESSURE_LASER, // Keep Back Laser
    KEEP_DISCARD, // Keep Discard
    SHIPWRECK_DISCARD, // Shipwreck Discard
    SHIPWRECK_VAULT, // Shipwreck Vault

    TOWN_BLUE_1, // Town Blue 1
    TOWN_BLUE_2, // Town Blue 2
    TOWN_BLUE_3, // Town Blue 3
    TOWN_BLUE_4, // Town Blue 4
    TOWN_BLUE_5, // Town Blue 5
    TOWN_ROOF_DISCARD, // Town Rooftop Discard
    TOWN_DOT_1, // Town 25 Dots 1
    TOWN_DOT_2, // Town 25 Dots 2
    TOWN_DOT_3, // Town 25 Dots 3
    TOWN_DOT_4, // Town 25 Dots 4
    TOWN_DOT_5, // Town 25 Dots 5
    TOWN_DOT_FINAL, // Town 25 Dots + Eraser
    TOWN_GLASS, // Town Green Door
    TOWN_RGB_L, // Town RGB Stones
    TOWN_RGB_R, // Town RGB Stars
    TOWN_CRATE, // Town Orange Crate
    TOWN_CRATE_DISCARD, // Town Orange Crate Discard
    TOWN_WINDMILL_ENTER, // Town Windmill Door
    THEATER_ENTER, // Theater Entrance
    THEATER_DISCARD, // Theater Discard
    THEATER_EXIT_L, // Theater Sun Exit
    THEATER_EXIT_R, // Theater Corona Exit

    SWAMP_ENTER,
    SWAMP_TUT_A1, // Swamp Tutorial 1
    SWAMP_TUT_A2, // Swamp Tutorial 2
    SWAMP_TUT_A3, // Swamp Tutorial 3
    SWAMP_TUT_A4, // Swamp Tutorial 4
    SWAMP_TUT_A5, // Swamp Tutorial 5
    SWAMP_TUT_A6, // Swamp Tutorial 6
    SWAMP_TUT_B1, // Swamp Tutorial 7
    SWAMP_TUT_B2, // Swamp Tutorial 8
    SWAMP_TUT_B3, // Swamp Tutorial 9
    SWAMP_TUT_B4, // Swamp Tutorial 10
    SWAMP_TUT_B5, // Swamp Tutorial 11
    SWAMP_TUT_B6, // Swamp Tutorial 12
    SWAMP_TUT_B7, // Swamp Tutorial 13
    SWAMP_TUT_B8, // Swamp Tutorial 14
    SWAMP_RED_1, // Swamp Red 1
    SWAMP_RED_2, // Swamp Red 2
    SWAMP_RED_3, // Swamp Red 3
    SWAMP_RED_4, // Swamp Red 4
    SWAMP_RED_SHORTCUT_1, // Swamp Red Shortcut 1
    SWAMP_RED_SHORTCUT_2, // Swamp Red Shortcut 2
    SWAMP_DISJOINT_1, // Swamp Discontinuous 1
    SWAMP_DISJOINT_2, // Swamp Discontinuous 2
    SWAMP_DISJOINT_3, // Swamp Discontinuous 3
    SWAMP_DISJOINT_4, // Swamp Discontinuous 4
    SWAMP_ROTATE_A1, // Swamp Rotation Tutorial 1
    SWAMP_ROTATE_A2, // Swamp Rotation Tutorial 2
    SWAMP_ROTATE_A3, // Swamp Rotation Tutorial 3
    SWAMP_ROTATE_A4, // Swamp Rotation Tutorial 4
    SWAMP_ROTATE_B1, // Swamp Rotation Advanced 1
    SWAMP_ROTATE_B2, // Swamp Rotation Advanced 2
    SWAMP_ROTATE_B3, // Swamp Rotation Advanced 3
    SWAMP_ROTATE_B4, // Swamp Rotation Advanced 4
    SWAMP_PURPLE, // Swamp Purple Tetris
    SWAMP_NEG_A1, // Swamp Blue Underwater 1
    SWAMP_NEG_A2, // Swamp Blue Underwater 2
    SWAMP_NEG_A3, // Swamp Blue Underwater 3
    SWAMP_NEG_A4, // Swamp Blue Underwater 4
    SWAMP_NEG_A5, // Swamp Blue Underwater 5
    SWAMP_NEG_B1, // Swamp Teal Underwater 1
    SWAMP_NEG_B2, // Swamp Teal Underwater 2
    SWAMP_NEG_B3, // Swamp Teal Underwater 3
    SWAMP_NEG_B4, // Swamp Teal Underwater 4
    SWAMP_NEG_B5, // Swamp Teal Underwater 5
    SWAMP_NEG_C1, // Swamp Red Underwater 1
    SWAMP_NEG_C2, // Swamp Red Underwater 2
    SWAMP_NEG_C3, // Swamp Red Underwater 3
    SWAMP_NEG_C4, // Swamp Red Underwater 4
    SWAMP_EXIT_1, // Swamp Laser Shortcut 1
    SWAMP_EXIT_2, // Swamp Laser Shortcut 2

    MOUNTAIN_OUTSIDE_DISCARD, // Mountainside Discard
    MOUNTAIN_VAULT, // Mountainside Vault
    MOUNTAIN_ORANGE_1, // Mountain 1 Orange 1
    MOUNTAIN_ORANGE_2, // Mountain 1 Orange 2
    MOUNTAIN_ORANGE_3, // Mountain 1 Orange 3
    MOUNTAIN_ORANGE_4, // Mountain 1 Orange 4
    MOUNTAIN_ORANGE_5, // Mountain 1 Orange 5
    MOUNTAIN_ORANGE_6, // Mountain 1 Orange 6
    MOUNTAIN_ORANGE_7, // Mountain 1 Orange 7
    MOUNTAIN_PURPLE_1, // Mountain 1 Purple 1
    MOUNTAIN_PURPLE_2, // Mountain 1 Purple 2
    MOUNTAIN_GREEN_1, // Mountain 1 Green 1
    MOUNTAIN_GREEN_2, // Mountain 1 Green 2
    MOUNTAIN_GREEN_3, // Mountain 1 Green 3
    MOUNTAIN_GREEN_4, // Mountain 1 Green 4
    MOUNTAIN_GREEN_5, // Mountain 1 Green 5
    MOUNTAIN_RAINBOW_1, // Mountain 2 Rainbow 1
    MOUNTAIN_RAINBOW_2, // Mountain 2 Rainbow 2
    MOUNTAIN_RAINBOW_3, // Mountain 2 Rainbow 3
    MOUNTAIN_RAINBOW_4, // Mountain 2 Rainbow 4
    MOUNTAIN_RAINBOW_5, // Mountain 2 Rainbow 5
    MOUNTAIN_SECRET_DOOR, // Mountain 3 Secret Door

    CAVES_ENTER, // UTM Entrance Door
    CAVES_SHAPE_L1, // UTM Blue Left 1
    CAVES_SHAPE_L2, // UTM Blue Left 2
    CAVES_SHAPE_L3, // UTM Blue Left 3
    CAVES_SHAPE_L4, // UTM Blue Left 4
    CAVES_SHAPE_L5, // UTM Blue Left 5
    CAVES_SHAPE_R1, // UTM Blue Right Near 1
    CAVES_SHAPE_R2, // UTM Blue Right Near 2
    CAVES_SHAPE_R3, // UTM Blue Right Near 3
    CAVES_SHAPE_R4, // UTM Blue Right Near 4
    CAVES_SHAPE_B1, // UTM Blue Right Far 1
    CAVES_SHAPE_B2, // UTM Blue Right Far 2
    CAVES_SHAPE_B3, // UTM Blue Right Far 3
    CAVES_SHAPE_B4, // UTM Blue Right Far 4
    CAVES_SHAPE_B5, // UTM Blue Right Far 5
    CAVES_SHAPE_SYM1, // UTM Blue Hard Symmetry
    CAVES_SHAPE_SYM2, // UTM Blue Easy Symmetry
    CAVES_SHAPE_NEG, // UTM Blue Cave In
    CAVES_SHAPE_DISJOINT, // UTM Blue Discontinuous
    CAVES_QUARRY, // UTM Quarry
    CAVES_RAINBOW, // UTM Rainbow
    CAVES_TREEHOUSE, // UTM Treehouse
    CAVES_SWAMP, // UTM Swamp
    CAVES_DOT_1, // UTM Green Dots 1
    CAVES_DOT_2, // UTM Green Dots 2
    CAVES_DOT_3, // UTM Green Dots 3
    CAVES_DOT_4, // UTM Green Dots 4
    CAVES_DOT_5, // UTM Green Dots 5
    CAVES_DOT_6, // UTM Green Dots 6
    CAVES_INVISIBLE_1, // UTM Invisible Dots 1
    CAVES_INVISIBLE_2, // UTM Invisible Dots 2
    CAVES_INVISIBLE_3, // UTM Invisible Dots 3
    CAVES_INVISIBLE_4, // UTM Invisible Dots 4
    CAVES_INVISIBLE_5, // UTM Invisible Dots 5
    CAVES_INVISIBLE_6, // UTM Invisible Dots 6
    CAVES_INVISIBLE_7, // UTM Invisible Dots 7
    CAVES_INVISIBLE_8, // UTM Invisible Dots 8
    CAVES_INVISIBLE_SYM1, // UTM Invisible Dots Symmetry 1
    CAVES_INVISIBLE_SYM2, // UTM Invisible Dots Symmetry 2
    CAVES_INVISIBLE_SYM3, // UTM Invisible Dots Symmetry 3
    CAVES_TRIANGLE_EXIT, // UTM Waterfall Shortcut
    CAVES_STAR_EXIT, // UTM Mountainside Shortcut
    CAVES_CHALLENGE_ENTER, // UTM Challenge Entrance
    CAVES_THEATER_WALK, // Tunnels Theater Catwalk
    CAVES_TOWN_EXIT, // Tunnels Town Shortcut
};

std::vector<PanelID> elevatorControls = {
    QUARRY_ELEVATOR,
    MOUNTAIN_ELEVATOR,
    CAVES_ELEVATOR_IN,
    CAVES_ELEVATOR_LOWER,
    CAVES_ELEVATOR_UPPER
};

std::vector<PanelID> treehousePivotSet = {
    TREEHOUSE_ORANGE_L9,
    TREEHOUSE_ORANGE_R4,
    TREEHOUSE_ORANGE_R10,
    TREEHOUSE_GREEN_4
};

std::vector<PanelID> utmPerspectiveSet = {
    CAVES_PERSPECTIVE_1,
    CAVES_PERSPECTIVE_2,
    CAVES_PERSPECTIVE_3,
    CAVES_PERSPECTIVE_4,
};

std::vector<PanelID> symmetryLaserYellows = {
    SYM_YELLOW_1,
    SYM_YELLOW_2,
    SYM_YELLOW_3,
};

std::vector<PanelID> symmetryLaserBlues = {
    SYM_CYAN_1,
    SYM_CYAN_2,
    SYM_CYAN_3,
};

std::vector<PanelID> pillars = {
    MOUNTAIN_PILLAR_L1,
    MOUNTAIN_PILLAR_L2,
    MOUNTAIN_PILLAR_L3,
    MOUNTAIN_PILLAR_L4,
    MOUNTAIN_PILLAR_R1,
    MOUNTAIN_PILLAR_R2,
    MOUNTAIN_PILLAR_R3,
    MOUNTAIN_PILLAR_R4,
    CAVES_PILLAR,
};

std::vector<PanelID> mountainMultipanel = {
    MOUNTAIN_MULTI_1,
    MOUNTAIN_MULTI_2,
    MOUNTAIN_MULTI_3,
    MOUNTAIN_MULTI_4,
    MOUNTAIN_MULTI_5,
    MOUNTAIN_MULTI_6,
};

std::vector<PanelID> squarePanelsExpertBanned = {
    MOUNTAIN_GREEN_1,
    MOUNTAIN_GREEN_2,
    MOUNTAIN_GREEN_3,
    MOUNTAIN_GREEN_4,
    MOUNTAIN_GREEN_5,
};

std::vector<PanelID> desertPanels = {
    DESERT_SUN_1,
    DESERT_SUN_2,
    DESERT_SUN_3,
    DESERT_SUN_4,
    DESERT_SUN_5,
    DESERT_SUN_6,
    DESERT_SUN_7,
    DESERT_SUN_8,
    DESERT_LIGHT_L,
    DESERT_LIGHT_R,
    DESERT_LIGHT_FLOOR,
    DESERT_POND_1,
    DESERT_POND_2,
    DESERT_POND_3,
    DESERT_POND_4,
    DESERT_POND_5,
    DESERT_FLOOD_1,
    DESERT_FLOOD_2,
    DESERT_FLOOD_3,
    DESERT_FLOOD_4,
    DESERT_FLOOD_5,
    DESERT_TILT,
};

std::vector<PanelID> desertPanelsWide = {
    DESERT_CURVED_1,
    DESERT_CURVED_2,
};

std::set<PanelID> desertValidSun2 = {
    DESERT_SUN_1,
    DESERT_SUN_2,
    DESERT_SUN_3,
    DESERT_SUN_5,
    DESERT_POND_4,
    DESERT_FLOOD_1,
    DESERT_FLOOD_4,
    DESERT_FLOOD_5,
    DESERT_LIGHT_FLOOR,
};

std::set<PanelID> desertValidSun3 = {
    DESERT_SUN_1,
    DESERT_SUN_2,
    DESERT_SUN_3,
    DESERT_SUN_6,
    DESERT_LIGHT_R,
    DESERT_POND_3,
    DESERT_POND_4,
    DESERT_FLOOD_2,
    DESERT_FLOOD_5,
    DESERT_TILT,
    DESERT_LIGHT_FLOOR
};

std::set<PanelID> desertValidSun7 = {
    DESERT_SUN_1,
    DESERT_SUN_2,
    DESERT_SUN_3,
    DESERT_SUN_4,
    DESERT_SUN_6,
    DESERT_SUN_7,
    DESERT_LIGHT_L,
    DESERT_LIGHT_R,
    DESERT_LIGHT_FLOOR,
    DESERT_POND_1,
    DESERT_POND_2,
    DESERT_POND_3,
    DESERT_POND_4,
    DESERT_POND_5,
    DESERT_FLOOD_1,
    DESERT_FLOOD_2,
    DESERT_FLOOD_3,
    DESERT_FLOOD_5,
    DESERT_TILT
};

std::set<PanelID> desertValidLightR = {
    DESERT_SUN_1,
    DESERT_SUN_2,
    DESERT_SUN_3,
    DESERT_SUN_4,
    DESERT_SUN_6,
    DESERT_SUN_7,
    DESERT_LIGHT_L,
    DESERT_LIGHT_R,
    DESERT_POND_1,
    DESERT_POND_3,
    DESERT_POND_4,
    DESERT_FLOOD_1,
    DESERT_FLOOD_2,
    DESERT_FLOOD_3,
    DESERT_FLOOD_5,
    DESERT_TILT,
    DESERT_LIGHT_FLOOR
};

std::set<PanelID> desertValidLightFloor = {
    DESERT_SUN_1,
    DESERT_SUN_2,
    DESERT_SUN_3,
    DESERT_SUN_4,
    DESERT_SUN_6,
    DESERT_LIGHT_L,
    DESERT_POND_3,
    DESERT_POND_4,
    DESERT_POND_5,
    DESERT_FLOOD_2
};

