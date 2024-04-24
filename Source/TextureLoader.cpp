#include "TextureLoader.h"
#include "Panel.h"
#include "wtx_tools.h"
#include "Memory.h"

TextureLoader* TextureLoader::_singleton = nullptr;

void TextureLoader::generateColorBunkerTexture(int32_t panelid)
{
	auto p = new Panel(panelid);

	// need to flatten _grid into one contiguous array so that rust-code can read it safely. 
	std::vector<int> flattened;
	for (auto row : p->_grid) {
		for (auto element : row) {
			flattened.push_back(element);
		}
	}

	auto bg = ColorPanelBackground::Blueprint;
	switch (p->id) {
	case 0x0A010:
	case 0x0A01B:
		bg = ColorPanelBackground::LightGrey;
		break;
	case 0x0A01F:
		bg = ColorPanelBackground::DarkGrey;
		break;
	case 0x17E63:
	case 0x17E67:
		bg = ColorPanelBackground::White;
		break;
	case 0x0A079:
		bg = ColorPanelBackground::Elevator;
		break;
	}


	TextureBuffer tex = wtx_tools_generate_colorpanel_from_grid((const uint32_t*)&flattened[0], p->_width, p->_height, bg);
	// Rust will continue to hold some knowledge of that memory it allocated to return the `tex`
	// so we should copy the data to a local variable, and tell rust that we are done and it can free that memory safely.
	std::vector<uint8_t> wtxBuffer = std::vector<uint8_t>(tex.data, tex.data + tex.len);

	//let rust free the memory it allocated
	free_texbuf(tex);

	storedTextures[panelid] = wtxBuffer;
}

TextureLoader* TextureLoader::get()
{
	if (_singleton == nullptr) {
		_singleton = new TextureLoader();
	}
	return _singleton;
}

void TextureLoader::loadTextures()
{
	Memory* memory = Memory::get();
	memory->LoadPackage("save_58472"); //tells game to load the color bunker assets into memory, so we can edit them

	for (auto& elem : storedTextures) {
		auto texturename = textureNames[elem.first];
		auto texmap = memory->GetTextureMapFromCatalog(texturename);
		memory->LoadTexture(texmap, elem.second);
		storedTexMaps[elem.first] = texmap; //store the tex map. (in case we want to unload it some day)
	}
}

void TextureLoader::generateTexture(int32_t panelid)
{

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
