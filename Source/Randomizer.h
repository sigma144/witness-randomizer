#pragma once
#include "Memory.h"
#include <memory>
#include <set>
#include <map>

class Randomizer {
public:
	void GenerateNormal(HWND loadingHandle);
	void GenerateHard(HWND loadingHandle);

	void AdjustSpeed();

	void ClearOffsets() {_memory->ClearOffsets();}

	enum SWAP {
		NONE = 0,
		TARGETS = 1,
		LINES = 2,
		AUDIO_NAMES = 4,
		COLORS = 8,
	};

	int seed = 0;
	bool seedIsRNG = false;
	bool colorblind = false;
	bool doubleMode = false;

private:
	void RandomizeDesert();

	void Randomize(std::vector<int>& panels, int flags);
	void RandomizeRange(std::vector<int> panels, int flags, size_t startIndex, size_t endIndex);
	void RandomizeAudiologs();
	void SwapPanels(int panel1, int panel2, int flags);
	void ReassignTargets(const std::vector<int>& panels, const std::vector<int>& order, std::vector<int> targets = {});
	void SwapWithRandomPanel(int panel1, const std::vector<int>& possiblePanels, int flags);
	void ShuffleRange(std::vector<int>& order, size_t startIndex, size_t endIndex);
	void ShufflePanels(bool hard);

	std::shared_ptr<Memory> _memory = std::make_shared<Memory>("witness64_d3d11.exe");
	std::set<int> _alreadySwapped;
	std::map<int, int> _shuffleMapping;

	friend class Panel;
	friend class PuzzleList;
	friend class Special;
};

//Entity variables
#define ENTITY_MANAGER 0x18	//ptr (long long)
#define ENTITY_FLAGS 0x20 //int
#define POSITION 0x24 //float[3]
#define SCALE 0x30 //float
#define ORIENTATION 0x34 //Quaternion
#define BOUNDING_RADIUS 0x44 //float
#define BOUNDING_CENTER 0x48 //float[3]
#define GROUP_ID 0x54 //int
#define ENTITY_NAME 0x58 //char[]
#define MESH 0x60 //ptr
#define LIGHTMAP_TABLE 0x68 //ptr
#define ANIMATOR 0x70 //ptr
#define MOUNT_BONE_NAME 0x78 //char[]
#define MOUNT_PARENT_ID 0x80 //int
#define MOUNT_POSITION 0x84 //float[3]
#define MOUNT_SCALE 0x90 //float
#define MOUNT_ORIENTATION 0x94 //Quaternion
#define MOUNT_DEPTH 0xA4 //short
#define MOUNT_BONE_INDEX 0xA6 //short
#define CLUSTER_ID 0xA8 //int
#define RUNTIME_ONLY_FLAGS 0xAC //int
#define ENTITY_TREE_NODE_IDX 0xB0 //int
#define LOD_DISTANCE 0xB4 //float
#define DISPLAY_ID 0xB8 //uint
#define ENTITY_ASSETS_STATE 0xBC //byte
#define SHAPE_DESCRIPTION 0xC0 //ShapeDescription
#define MESH_NAME 0xD8 //char[]
#define TEXTURE_NAME 0xE0 //char[]
#define OPEN_RATE 0xE0 //float
#define MATERIAL_NAME 0xE8 //char[]
#define COLOR_OVERRIDE 0xF0 //int
#define COLOR 0xF4 //float[4]
#define DATA_OVERLAY_FLAGS 0x104 //int

//Panel variables
#define PATH_COLOR 0xC0 //float[4]
#define REFLECTION_PATH_COLOR 0xD0 //float[4]
#define DOT_COLOR 0xF0 //float[4]
#define ACTIVE_COLOR 0x100 //float[4]
#define BACKGROUND_REGION_COLOR 0x110 //float[4]
#define SUCCESS_COLOR_A 0x120 //float[4]
#define SUCCESS_COLOR_B 0x130 //float[4]
#define STROBE_COLOR_A 0x140 //float[4]
#define STROBE_COLOR_B 0x150 //float[4]
#define ERROR_COLOR 0x160 //float[4]
#define VIDEO_STATUS_COLOR 0x170 //float[4]
#define PATTERN_POINT_COLOR 0x180 //float[4]
#define PATTERN_POINT_COLOR_A 0x190 //float[4]
#define PATTERN_POINT_COLOR_B 0x1A0 //float[4]
#define SYMBOL_A 0x1B0 //float[4]
#define SYMBOL_B 0x1C0 //float[4]
#define SYMBOL_C 0x1D0 //float[4]
#define SYMBOL_D 0x1E0 //float[4]
#define SYMBOL_E 0x1F0 //float[4]
#define PUSH_SYMBOL_COLORS 0x200 //int
#define OUTER_BACKGROUND 0x204 //float[4]
#define OUTER_BACKGROUND_MODE 0x214 //int
#define PATTERN_NAME 0x218 //char*
#define ENTITY_PANEL_SIZE 0x220 //float
#define EXTRA_BACK_DISTANCE 0x224 //float
#define TRACED_EDGES 0x228 //short
#define TRACED_EDGE_DATA 0x230 //int[]?
#define PANEL_MESH_NAME 0x238 //char*
#define BACKING_TEXTURE_NAME 0x240 //char[]
#define OFF_TEXTURE_NAME 0x248 //char[]
#define SCANLINE_TEXTURE_NAME 0x250 //char[]
#define OVERRIDE_ENV_MAP_NAME 0x258 //char[]
#define SUCCESS_SOUND_NAME 0x260 //char[]
#define RATTLE_SOUND_NAME 0x268 //char[]
#define AUDIO_PREFIX 0x270 //char[]
#define FLASH_T 0x278 //float
#define FLASH_T_MAX 0x27C //float
#define FLASH_MODE 0x280 //int
#define DOTS_FLASHING 0x284 //int
#define DOT_FLASH_T 0x288 //float
#define DOT_FLASH_T_MAX 0x28C //float
#define SOLVED_T 0x290 //float
#define SOLVED_T_TARGET 0x294 //float
#define HAS_EVER_BEEN_SOLVED 0x298 //int
#define DON_DT 0x29C //float
#define POWER 0x2A0 //float
#define ON_T_TARGET 0x2A4 //float
#define GESTURE_FINISHED_T 0x2A8 //float
#define GESTURE_FINISHED_T_TARGET 0x2AC //float
#define GESTURE_FINISHED_TIME 0x2B0 //float
#define TARGET 0x2B4 //int
#define POWER_OFF_ON_FAIL 0x2B8 //int
#define MY_MULTIPANEL 0x2BC //int
#define MY_BRIDGE 0x2C0 //int
#define MY_LANDING_SIGNAL 0x2C8 //int
#define POWERED_BY 0x2CC //int
#define MANUAL_PREV_ID 0x2D0 //int
#define MANUAL_NEXT_ID 0x2D4 //int
#define EXTRA_ATTRACT_DISTANCE 0x2D8 //float
#define PANEL_CHECKSUM 0x2DC //uint
#define PANEL_FOCUS_POINT //float[3]
#define PANEL_NORMAL //float[3]
#define IS_CYLINDER 0x2F4 //int
#define CYLINDER_Z0 0x2F8 //float
#define CYLINDER_Z1 0x2FC //float
#define CYLINDER_RADIUS 0x300 //float
#define UV_TO_WORLD_SCALE 0x304 //float
#define ERASER_STATUS 0x308 //Eraser_Status (48 bytes)
#define PATTERN_SCALE 0x338 //float
#define SYMBOL_SCALE 0x33C //float
#define INITIAL_DOT_SIZE 0x340 //float
#define ACCEPT_ANY_HIT 0x344 //int
#define RAY_SHORTENING 0x348 //float
#define COVERT 0x34C //int
#define CURSOR_SPEED_SCALE 0x350 //float
#define SOLVABLE_FROM_BEHIND 0x354 //int
#define VOLUME_SCALE 0x358 //float
#define SCINTILLATE_STARTPOINT_ON_T 0x35C //float
#define SCINTILLATE_STARTPOINT_ON_TARGET 0x360 //float
#define SCINTILLATE_ENDPOINTS_ON_T 0x364 //float
#define SCINTILLATE_ENDPOINTS_ON_TARGET 0x368 //float
#define SCINTILLATE_TIME 0x36C //float
#define SCINTILLATE_CYCLE_COUNT 0x370 //int
#define SCINTILLATE_AUDIO_COUNT 0x374 //int
#define POINTS_OCCUPIED_BY_PRESSURE_PLATES 0x378 //ulong
#define NEEDS_REDRAW 0x37C //int
#define IGNORE_OCCLUSION 0x380 //int
#define STASHED_GIANT_FLOOR_SHAPE 0x384 //int
#define EXTRA_EMISSIVE 0x388 //float
#define SPECULAR_ADD 0x38C //float
#define SPECULAR_POWER 0x390 //float
#define FILTERED_ENV_MAP 0x394 //int
#define USE_ENV_MAP 0x398 //int
#define PATH_WIDTH_SCALE 0x39C //float
#define STARTPOINT_SCALE 0x3A0 //float
#define BACKFACE_TRACING_OFFSET 0x3A4 //float
#define VIGNETTE_INTENSITY 0x3A8 //float
#define BROKENNESS 0x3AC //float
#define ZOOM_IN_MARGIN 0x3B0 //float
#define NUM_DOTS 0x3B4 //int
#define NUM_CONNECTIONS 0x3B8 //int 
#define MAX_BROADCAST_DISTANCE 0x3BC //float
#define DOT_POSITIONS 0x3C0 //float[2][]
#define DOT_FLAGS 0x3C8 //int[]
#define DOT_CONNECTION_A 0x3D0 //int[]
#define DOT_CONNECTION_B 0x3D8 //int[]
#define RANDOMIZE_ON_POWER_ON 0x3E0 //int
#define DOT_TIME_TAGS 0x3E8 //float[]
#define DOT_TIME_TAGS_INTERPOLATED 0x3EC //float[]
#define CONTROLLED_BY_PRESSURE_PLATES 0x3F0 //int
#define UNCONSTRAINED_TRIANGLES 0x400 //? (16 bytes)
#define DOT_RESULT_FLAGS 0x410 //int[]
#define DECORATIONS 0x418 //uint[]
#define DECORATION_FLAGS 0x420 //uint[]
#define DECORATION_COLORS 0x428 //float[4][]
#define NUM_DECORATIONS 0x430 //int
#define REFLECTION_DATA 0x438 //int[]
#define GRID_SIZE_X 0x440 //int
#define GRID_SIZE_Y 0x444 //int
#define STYLE_FLAGS 0x448 //int
#define WAS_EDITED 0x44C //int
#define PATTERN_UPDATED 0x450 //int
#define SEQUENCE_LEN 0x454 //int
#define SEQUENCE 0x458 //int[]
#define DOT_SEQUENCE_LEN 0x460 //int
#define DOT_SEQUENCE 0x468 //int[]
#define DOT_SEQUENCE_LEN_REFLECTION 0x470 //int
#define DOT_SEQUENCE_REFLECTION 0x478 //int[]
#define COLOR_CYCLE_INDEX 0x480 //int
#define AUXILIARY_TRACED_EDGES 0x488 //short
#define AUXILIARY_TRACED_EDGE_DATA 0x48A //int[]?
#define NUM_COLORED_REGIONS 0x498 //int
#define COLORED_REGIONS 0x4A0 //float[3][]?
#define PANEL_TARGET 0x4A8 //ColorTarget*
#define BACKING_TEXTURE 0x4B0 //TextureMap*
#define OFF_TEXTURE 0x4B8 //TextureMap*
#define SCANLINE_TEXTURE 0x4C0 //TextureMap*
#define OVERRIDE_ENV_MAP 0x4C8 //TextureMap*
#define SPECULAR_TEXTURE 0x4D0 //TextureMap*
#define CACHED_ENV_MAP_PROBE 0x4D8 //Entity*
#define MAX_DOTS 0x4E0 //int
#define MAX_CONNECTIONS 0x4E4 //int
#define POWERED_SUBPANEL_REGIONS 0x4E8 //short
#define POWERED_SUBPANEL_REGIONS_DATA 0x4EA //float[]
#define NEEDS_JUNCTION_UDPATE 0x4F8 //bool
#define WIDER_CUTS_ON_SMALL_SCREENS 0x4F9 //bool

//Audiolog variables
#define AUDIOLOG_RECORDING_NAME 0xC0 //char[]
#define AUDIOLOG_SOURCE_NAME 0xC8 //char[]
#define AUDIOLOG_MESH_NAME 0xD0 //char[]
#define AUDIOLOG_VOLUME_SCALE 0xD8 //float
#define AUDIOLOG_UNDERGROUND 0xDC //int
#define AUDIOLOG_SIZE 0xE0 //int
#define AUDIOLOG_PLAYING 0xE4 //int
#define AUDIOLOG_PLAYED 0xE8 //int
#define MY_SOUND_ID 0xEC //int
#define NOT_ON_RADAR 0xF0 //int
#define AUDIOLOG_COLOR_OVERRIDE 0xF4 //int
#define AUDIOLOG_COLOR 0xF8 //float[4]
#define AUDIOLOG_GLOW_COLOR 0x108//float[4]
#define AUDIOLOG_GLOW_MAGNITUDE 0x118 //float
#define AUDIOLOG_DIST_MAX 0x11C //float
#define PROXY_MARKER_ID 0x120 //int
#define LAKE_Z_OVERRIDE_MARKER_ID 0x124 //int
#define RADAR_LILY_PAD_SIZE 0x128 //int

//Unknown
#define CABLE_TARGET_2 0xD0
#define METADATA 0x13A //short
#define HOTEL_EP_NAME 0x51E340

//Globals
#define DO_SCRIPTED_SOUNDS 0x61E8B0 //float
#define ID_OF_CONTROLLED_ENTITY 0x240D0D

//Functions
#define LASER_ACTIVATE 0xAF520
#define POWER_ITEM 0xC2590
#define FIND_BIRDCALL_PANEL 0x29A7FB
#define GET_PANEL_PLAYER_IS_NEAR 0x294AF0
#define GET_THE_PLAYER 0x240D00
