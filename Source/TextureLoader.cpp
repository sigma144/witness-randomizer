#include "TextureLoader.h"
#include "Panel.h"
#include "Randomizer.h"
#include "Memory.h"
#include "TextureMaker.h"
#include <memory.h>

TextureLoader* TextureLoader::_singleton = nullptr;

void TextureLoader::generateColorBunkerTexture(int32_t panelid)
{
	Memory* memory = Memory::get();

	//I suspect this is the wrong way to get information about a panel
	//is memory->ReadPanelData better?
	//probably faster
	//TODO rewrite
	auto p = std::make_unique<Panel>(panelid);
	std::vector<Color> colorarray;

	if ((panelid == 0x0A010) || (panelid == 0x0A01B) || (panelid == 0x0A01F)) { //4x4 initial color filter panels
		std::vector<Color> colorarrayFlipped = memory->ReadArray<Color>(panelid, DECORATION_COLORS, p->get_num_grid_blocks());
		for (int i = 3; i >= 0; i--) {
			for (int j = 0; j < 4; j++) {
				colorarray.push_back(colorarrayFlipped[i * 4 + j]);
			}
		}
		
		//colorarray = memory->ReadArray<Color>(panelid, DECORATION_COLORS, p->get_num_grid_blocks());
	}
	else if (panelid == 0x17E63) {//first 3x3 ones with the door you have to open. This puzzle has a unique color scheme. It looks monochrome under the magenta light. 
		std::vector<Color> colorarrayFlipped = memory->ReadArray<Color>(panelid, DECORATION_COLORS, p->get_num_grid_blocks());
		for (int i = 2; i >= 0; i--) {
			for (int j = 0; j < 3; j++) {
				Color generated_color = colorarrayFlipped[i * 3 + j];
				if (generated_color.r == 1.0 ) {
					//must be "white"
					colorarray.push_back(Color{ 0.5, 0.0, 1.0 });
				}
				else if(generated_color.b == 1.0){
					//must be "blue"
					colorarray.push_back(Color{ 0.5, 0.65, 1.0 });

				} else {
					//must be "black"
					colorarray.push_back(Color{ 0.5, 1.0, 1.0 });
				}
			}
		}
	}
	else if (panelid == 0x17E67) { //second 3x3 one where you have to close the door to see the puzzle in magenta light. 
		std::vector<Color> colorarrayFlipped = memory->ReadArray<Color>(panelid, DECORATION_COLORS, p->get_num_grid_blocks());
		for (int i = 2; i >= 0; i--) {
			for (int j = 0; j < 3; j++) {
				colorarray.push_back(colorarrayFlipped[i * 3 + j]);
			}
		}
	}
	TextureMaker tm(1024, 1024);
	auto wtxBuffer = tm.generate_color_panel_grid(p->_grid, p->id, colorarray);

	
	//storedTextures[textureNames[id]] = wtxBuffer;
	auto texturename = textureNames[p->id];

	memory->LoadTexture(memory->GetTextureMapFromCatalog(texturename), wtxBuffer);
	memory->WritePanelData(p->id, NEEDS_REDRAW, 1);

}

void TextureLoader::generateSpecTexture(int32_t id)
{
	Memory* memory = Memory::get();
	std::vector<int> solution = memory->ReadArray<int>(id, SEQUENCE, memory->ReadPanelData<int>(id, SEQUENCE_LEN));
	std::vector<float> allPoints = memory->ReadArray<float>(id, DOT_POSITIONS, memory->ReadPanelData<int>(id, NUM_DOTS) * 2);
	std::vector<float> linePointsX;
	std::vector<float> linePointsY;
	for (int i : solution) {
		linePointsX.emplace_back(allPoints[i * 2]);
		linePointsY.emplace_back(allPoints[i * 2 + 1]);
	}
	float scale = memory->ReadPanelData<float>(id, PATH_WIDTH_SCALE);
	float dotscale = memory->ReadPanelData<float>(id, STARTPOINT_SCALE);
	int symdata = memory->ReadPanelData<int>(id, REFLECTION_DATA);

	TextureMaker tm(512, 512);
	auto wtxBuffer2 = tm.generate_desert_spec_line(linePointsX, linePointsY, scale * 35, dotscale * 35);

	//storedTextures[textureNames[id]] = wtxBuffer;
	memory->LoadTexture(memory->ReadPanelData<uint64_t>(id, SPECULAR_TEXTURE), wtxBuffer2);
	memory->WritePanelData(id, NEEDS_REDRAW, 1);
}

TextureLoader* TextureLoader::get()
{
	if (_singleton == nullptr) {
		_singleton = new TextureLoader();
	}
	return _singleton;
}

void TextureLoader::forceLoadBunkerTextures() {
	Memory* memory = Memory::get();
	memory->LoadPackage("save_58472"); //tells game to load the color bunker assets into memory, so we can edit them
}

void TextureLoader::forceLoadDesertTextures() {
	Memory* memory = Memory::get();
	memory->LoadPackage("save_58392");
	memory->LoadPackage("save_58473");
	memory->LoadPackage("save_58413");
	memory->LoadPackage("save_58440");
	memory->LoadPackage("globals");
}


void TextureLoader::loadTextures()
{
	Memory* memory = Memory::get();

	for (auto& elem : storedTextures) {
		auto texturename = elem.first;
		auto texmap = memory->GetTextureMapFromCatalog(texturename);
		memory->LoadTexture(texmap, elem.second);
		storedTexMaps[elem.first] = texmap; //store the tex map. (in case we want to unload it some day)
	}
}

void TextureLoader::generateTexture(int32_t panelid)
{
	//this was done in case a similar setup is to be used on other panels
	//can use this same generateTexture(), but handle different panels differently
	switch (panelid) {
	case 0x09F7D:
	case 0x09FDC:
	case 0x09FF7:
	case 0x09F82:
	case 0x09FF8:
	case 0x09D9F:
	case 0x09DA1:
	case 0x09DA2:
	case 0x09DAF:
	case 0x0A010:
	case 0x0A01B:
	case 0x0A01F:
	case 0x17E63:
	case 0x17E67:
	case 0x0A079:
		generateColorBunkerTexture(panelid);
	}
	
}
