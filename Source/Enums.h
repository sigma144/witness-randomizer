#pragma once
#include <vector>
#include "Panels.h"

enum Symbol : int {
	None = 0x0,
	Exit = 0x600001,
	Start = 0x600002,
	Stone = 0x100,
	Star = 0x200,
	Poly = 0x400,
	Eraser = 0x500,
	Triangle = 0x600,
	Triangle1 = 0x10600,
	Triangle2 = 0x20600,
	Triangle3 = 0x30600,
	Triangle4 = 0x40600,
	Empty = 0xA00, //Tiles which are not part of the grid. Using with tetris shapes might not work.
	Rotate = 0x1000,
	Negative = 0x2000,
	Gap = 0x100000,
	Gap_Row = 0x300000,
	Gap_Column = 0x500000,
	Dot = 0x20,
	Dot_Row = 0x240020,
	Dot_Column = 0x440020,
	Dot_Intersection = 0x600020,
	//Custom symbols - Don't actually set these values into the grid.
	//Instead, use values from GetValFromSymbol or GetValFromSymbolID.
	//Format: 0xVVTT700		T = Symbol Type (Must go up by 1 for each new type)		V = Symbol Variant
	Arrow = 0x000700,
	Arrow1 = 0x100700,
	Arrow2 = 0x200700,
	Arrow3 = 0x300700,
	AntiTriangle = 0x001700,
	AntiTriangle1 = 0x101700,
	AntiTriangle2 = 0x201700,
	AntiTriangle3 = 0x301700,
	AntiTriangle4 = 0x401700,
	Cave = 0x002700,
	Cave1 = 0x102700,
	Cave2 = 0x202700,
	Cave3 = 0x302700,
	Cave4 = 0x402700,
	Cave5 = 0x502700,
	Cave6 = 0x602700,
	Cave7 = 0x702700,
	Cave8 = 0x802700,
	Cave9 = 0x902700,
};

enum SymbolColor {
	NoColor = 0x0,
	Black = 0x1,
	White = 0x2,
	Red = 0x3, //Only works with WriteColors color mode
	Purple = 0x4,
	Green = 0x5,
	Cyan = 0x6,
	Magenta = 0x7,
	Yellow = 0x8,
	Blue = 0x9,
	Orange = 0xA,
	Invisible = 0xF,
};

enum Symmetry {
	NoSymmetry, Horizontal, Vertical, Rotational,
	RotateLeft, RotateRight, FlipXY, FlipNegXY, ParallelH, ParallelV, ParallelHFlip, ParallelVFlip,
	PillarParallel, PillarHorizontal, PillarVertical, PillarRotational
};

enum Style {
	SYMMETRICAL = 0x2,
	NO_BLINK = 0x4,
	HAS_DOTS = 0x8,
	IS_2COLOR = 0x10,
	HAS_STARS = 0x40,
	HAS_TRIANGLES = 0x80,
	HAS_STONES = 0x100,
	HAS_ERASERS = 0x1000,
	HAS_SHAPERS = 0x2000,
	IS_PIVOTABLE = 0x8000,
	HAS_CUSTOM = 0x80000000,
};

enum IntersectionFlags : int {
	ROW = 0x200000,
	COLUMN = 0x400000,
	INTERSECTION = 0x600000,
	ENDPOINT = 0x1,
	STARTPOINT = 0x2,
	NO_POINT = 0x8, //Points that nothing connects to
	GAP = 0x100000,
	DOT = 0x20,
	DOT_IS_BLUE = 0x100,
	DOT_IS_ORANGE = 0x200,
	DOT_IS_INVISIBLE = 0x1000,
	DOT_SMALL = 0x2000,
	DOT_MEDIUM = 0x4000,
	DOT_LARGE = 0x8000,
	//Internal randomizer use only
	OPEN = 0x3, //Full gap
	PATH = 0x4, //Path crosses this point
	OFF_GRID = INT_MIN //Point that is off the grid
};

enum PanelVar : int {
	BOAT_DELTA_SPEED = 0xD4,
	POSITION = 0x24,
	SCALE = 0x30,
	ORIENTATION = 0x34,
	BOUNDING_RADIUS = 0x44,
	MESH = 0x60, //void*
	PATH_COLOR = 0xC0, //Color
	REFLECTION_PATH_COLOR = 0xD0, //Color
	DOT_COLOR = 0xF0, //Color
	ACTIVE_COLOR = 0x100, //Color
	BACKGROUND_REGION_COLOR = 0x110, //Color
	BACKGROUND_HIDDEN_VAR = 0x11C, //float
	SUCCESS_COLOR_A = 0x120, //Color
	SUCCESS_COLOR_B = 0x130, //Color
	STROBE_COLOR_A = 0x140, //Color
	STROBE_COLOR_B = 0x150, //Color
	ERROR_COLOR = 0x160, //Color
	VIDEO_STATUS_COLOR = 0x170, //Color
	PATTERN_POINT_COLOR = 0x180, //Color
	PATTERN_POINT_COLOR_A = 0x190, //Color
	PATTERN_POINT_COLOR_B = 0x1A0, //Color
	SYMBOL_A = 0x1B0, //Color
	SYMBOL_B = 0x1C0, //Color
	SYMBOL_C = 0x1D0, //Color
	SYMBOL_D = 0x1E0, //Color
	SYMBOL_E = 0x1F0, //Color
	PUSH_SYMBOL_COLORS = 0x200, //int
	OUTER_BACKGROUND = 0x204, //Color
	OUTER_BACKGROUND_MODE = 0x214, //int
	PATTERN_NAME = 0x218,
	TRACED_EDGES = 0x228, //int
	TRACED_EDGE_DATA = 0x230, //SolutionPoint[]
	AUDIO_PREFIX = 0x270, //void*
	FLASH_MODE = 0x280,
	SOLVED = 0x298, //int
	POWER = 0x2A0, //{float, float}
	TARGET = 0x2B4,	//PanelID
	POWER_OFF_ON_FAIL = 0x2B8, //int
	IS_CYLINDER = 0x2F4, //int
	CYLINDER_Z0 = 0x2F8,
	CYLINDER_Z1 = 0x2FC,
	CYLINDER_RADIUS = 0x300,
	PATTERN_SCALE = 0x338,
	CURSOR_SPEED_SCALE = 0x350,
	NEEDS_REDRAW = 0x37C, //int
	SPECULAR_ADD = 0x38C, //float
	SPECULAR_POWER = 0x390,
	PATH_WIDTH_SCALE = 0x39C, //float
	STARTPOINT_SCALE = 0x3A0, //float
	NUM_DOTS = 0x3B4, //int
	NUM_CONNECTIONS = 0x3B8, //int
	MAX_BROADCAST_DISTANCE = 0x3BC,
	DOT_POSITIONS = 0x3C0, //float[]
	DOT_FLAGS = 0x3C8, //int[]
	DOT_CONNECTION_A = 0x3D0, //int[]
	DOT_CONNECTION_B = 0x3D8, //int[]
	RANDOMIZE_ON_POWER_ON = 0x3E0, //int
	DECORATIONS = 0x418, //int[]
	DECORATION_FLAGS = 0x420, //int[]
	DECORATION_COLORS = 0x428, //Color[]
	NUM_DECORATIONS = 0x430, //int
	REFLECTION_DATA = 0x438, //int[]
	GRID_SIZE_X = 0x440, //int
	GRID_SIZE_Y = 0x444, //int
	STYLE_FLAGS = 0x448, //int
	SEQUENCE_LEN = 0x454, //int
	SEQUENCE = 0x458, //int[]
	DOT_SEQUENCE_LEN = 0x460, //int
	DOT_SEQUENCE = 0x468, //int[]
	DOT_SEQUENCE_LEN_REFLECTION = 0x470, //int
	DOT_SEQUENCE_REFLECTION = 0x478, //int[]
	COLOR_CYCLE_INDEX = 0x480,
	NUM_COLORED_REGIONS = 0x498, //int
	COLORED_REGIONS = 0x4A0, //int[]
	PANEL_TARGET = 0x4A8, //void*
	SPECULAR_TEXTURE = 0x4D0, //void*
	CABLE_TARGET_2 = 0xD0,
	AUDIO_LOG_NAME = 0x0, //void*
	OPEN_RATE = 0xE0, //float
	METADATA = 0x13A, //short
};

enum Config {
	FullGaps, //Gaps go all the way between two vertices instead of part of the way.
	StartEdgeOnly, //Starts will only be placed on the edges of the puzzle.
	DisableWrite, //Don't write the puzzle out to the game's memory immediately. This allows for changes to be made after generation. write() must be called manually afterwards
	PreserveStructure, //Keep the same arrangement of vertices and edges that the original puzzle had. Without this, the puzzle will be expanded to a grid.
	MakeStonesUnsolvable, //Make sure a stone puzzle is unsolvable. Used for some of the eraser puzzles
	SmallShapes, //Smaller tetris shapes than normal, with an average size of about 2-3 blocks
	DisconnectShapes, //Tetris shapes may be discontiguous
	ResetColors, //Force the default symbol color scheme
	DisableCancelShapes, //Don't make solutions where positive/negative shapes cancel
	RequireCancelShapes, //Make only solutions where positive/negative shapes cancel
	OnlyCancelShapes, //Force all positive/negative shapes to cancel
	BigShapes, //Bigger tetris shapes than normal, with an average size of about 5-6 blocks
	SplitShapes, //Don't make solutions where tetris shapes combine
	RequireCombineShapes, //Make only solutions where tetris shapes combine
	TreehouseLayout, //Position starts and exits in the middle, like in the Treehouse
	AlternateColors, //Use an alternate symbol color scheme. The colors used vary by puzzle
	WriteColors, //Write out exact colors from each symbol. Sometimes needed to get a proper red, orange, or green.
	CyanAndYellowLines, //Write out line and dot colors for a blue/yellow symmetry puzzle
	CombineErasers, //Allow erasers to be together
	LongPath, //Make a longer than usual path, covering at least 7/8 of the available points
	ShortPath, //Make a path without any length restrictions
	DisableFlash, //Disable incorrect symbols from flashing
	DecorationsOnly, //Write out only the puzzle symbols to memory, preserving the original point and edge locations
	FalseParity, //Cause a full path to miss one vertex. For eraser + full dot puzzles
	DisableDotIntersection, //Dots will not be put on grid intersections
	FixDotColor, //Make dots a dark grey color. Use if dot color looks incorrect
	MatchDotColor, //Make dots the same color as the line
	LongestPath, //Make the longest path possible, covering every point. For full dot puzzles
	InvisibleSymmetryLine, //Make the symmetry path invisible
	MountainFloorH, //Only for the mountain floor puzzles on hard mode
	PowerOffOnFail, //Have the panel power off with an incorrect solution
};

enum Difficulty { Normal, Expert, Symbols };