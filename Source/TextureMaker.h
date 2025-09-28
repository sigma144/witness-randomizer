
#pragma once
#include <comdef.h>
#include <iostream>
#include <vector>
#include <cairo/cairo.h>
#include "Panel.h"
#include <optional>

class TextureMaker
{

private:
	int width;
	int height;
	//int gridwidth;
	//int gridheight;

	std::vector<uint8_t> prepend_header(std::vector<uint8_t> buffer, uint16_t width, uint16_t height, uint16_t mipmaps, uint8_t bits, const char* DXTString);
	std::vector<uint8_t> convert_cairo_surface_to_wtx(cairo_surface_t* surface, int dxtversion, int flags);
	void draw_symbol_on_surface(cairo_surface_t* image, float x, float y, float scale, int symbolflags, std::optional<Color> customcolor, bool specular);
	void flip_image_vertically(cairo_surface_t* image);

public:
	TextureMaker(int width, int height);

	int read_spec_bg(unsigned char* dest, unsigned int len) {

	};

	int handle_errors(HRESULT result, const char* desc) {
		if (FAILED(result)) {
			_com_error err(result);
			auto errMsg = err.ErrorMessage();
			std::wcout << "failed on step " << desc << " with " << HRESULT_CODE(result) << " " << err.ErrorMessage() << std::endl; 
			return -1;
		}
		else {
			return 0;
		}
	};
	std::vector<uint8_t> generate_blank_texture();
	std::vector<uint8_t> generate_color_panel_grid(std::vector<std::vector<int>> grid, int id, std::vector<Color> colors, bool specular = false);
	std::vector<uint8_t> generate_desert_spec_line(std::vector<float> xpoints, std::vector<float> ypoints, float thickness, float dotthickness, bool symmetry);
	std::vector<uint8_t> generate_desert_spec_segments(std::vector<std::pair<float, float>> segments, std::vector<std::pair<float, float>> startpoints, float thickness, float dotthickness);
	std::vector<uint8_t> generate_shadow_line(std::vector<float> xpoints, std::vector<float> ypoints, float thickness, float dotthickness, bool symmetry);
};

