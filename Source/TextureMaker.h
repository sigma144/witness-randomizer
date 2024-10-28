
#pragma once
#include <comdef.h>
#include <iostream>
#include <vector>
#include <cairo/cairo.h>
class TextureMaker
{

private:
	int width;
	int height;
	//int gridwidth;
	//int gridheight;

	std::vector<uint8_t> prepend_header(std::vector<uint8_t> buffer, uint16_t width, uint16_t height, uint16_t mipmaps, uint8_t bits, const char* DXTString);
	std::vector<uint8_t> convert_cairo_surface_to_wtx(cairo_surface_t* surface, int dxtversion, int flags);



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
	std::vector<uint8_t> generate_desert_spec_line(std::vector<float> xpoints, std::vector<float> ypoints, float thickness, float dotthickness);

};

