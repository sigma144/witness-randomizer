#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>

class TextureLoader
{
	// texture name -> wtx
	std::unordered_map < std::string, std::vector<uint8_t> > storedTextures;
	// wtx -> in-game texmap
	std::unordered_map < std::string, uint64_t > storedTexMaps;

	void generateColorBunkerTexture(int32_t panelid);

protected:

	static TextureLoader* _singleton;

public:

	static TextureLoader* get();
	void forceLoadBunkerTextures();
	void forceLoadDesertTextures();
	void loadTextures();
	void generateTexture(int32_t panelid);
	void generateSpecTexture(int32_t panelid);
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