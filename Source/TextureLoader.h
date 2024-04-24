#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "TextureLoader.h"

class TextureLoader
{

	std::unordered_map < int32_t, std::vector<uint8_t> > storedTextures;
	std::unordered_map < int32_t, uint64_t > storedTexMaps;

	void generateColorBunkerTexture(int32_t panelid);

protected:

	static TextureLoader* _singleton;

public:

	static TextureLoader* get();
	void loadTextures();
	void generateTexture(int32_t panelid);
};

inline std::unordered_map<int32_t, std::string> textureNames = {
	{0x09F7D, "obj_panels_color_tricolor_1"},
	{0x09FDC, "obj_panels_color_tricolor_2"},
	{0x09FF7, "obj_panels_color_tricolor_3"},
	{0x09F82, "obj_panels_color_tricolor_4"},
	{0x09FF8, "obj_panels_color_tricolor_5"},
	{0x09D9F, "obj_panels_color_tricolorNew_A"},
	{0x09DA1, "obj_panels_color_tricolorNew_B"},
	{0x09DA2, "obj_panels_color_tricolorNew_C"},
	{0x09DAF, "obj_panels_color_tricolorNew_D"},
	{0x0A010, "obj_panels_color_colorfilter_1"},
	{0x0A01B, "obj_panels_color_colorfilter_2"},
	{0x0A01F, "obj_panels_color_colorfilter_3"},
	{0x17E63, "obj_panels_color_coloredlight_1"},
	{0x17E67, "obj_panels_color_coloredlight_2"},
	{0x0A079, "obj_panels_color_tricolorNew_Elevator"},
};