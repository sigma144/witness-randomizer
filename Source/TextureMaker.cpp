
/*#undef IN
#undef OUT
#undef BINDING*/
//#include <cairoconfig.h>
#include <cairo/cairo.h>
//#include <cairomm/cairomm.h>
//#include <cairomm/context.h>
#include "TextureMaker.h"
#include <DirectXTex.h>
#include <comdef.h>
#include <algorithm>
#include <vector>
#include <random>
#include "Panel.h"
#include <optional>

using namespace DirectX;

TextureMaker::TextureMaker(int h, int w) {
	height = h;
	width = w;
}

std::vector<uint8_t> TextureMaker::prepend_header(std::vector<uint8_t> buffer, uint16_t width, uint16_t height, uint16_t mipmaps,  uint8_t bits, const char * DXTString)
{
	auto header = std::vector<uint8_t>({
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, //header always constant
		0x00, 0x00, 0x00, 0x00, //these 4 = length of rest of data
		0x00, 0x00, //width 
		0x00, 0x00, //height
		0x01, 0x00, //"depth" always (0x01, 0x00) as these are 2d images
		0x00, 0x00, //number of mipmaps. 
		bits, //weird bitmask thing
		0x00, 0x00, 0x00, //these 3 always zero (part of bitmask, which is really 4 bytes)
		0x00,0x00,0x00,0x00, //float R // these floats give the average color across the image, in each channel.
		0x00,0x00,0x00,0x00, //float G // as far as i know, nothing in-game actually ends up reading these
		0x00,0x00,0x00,0x00, //float B // so i dont think we actually need to calcuate them.
		0x00,0x00,0x00,0x00, //float A
		0x00,0x00,0x00,0x00, //image format, ascii string. "DXT1" for desert spec panels. 
		});
	uint32_t len = buffer.size() + 32;
	memcpy(&header[8], &len, 4);
	memcpy(&header[12], &width, 2);
	memcpy(&header[14], &height, 2);
	memcpy(&header[18], &mipmaps, 2);
	float color = 0.0;
	float alpha = 1.0;
	memcpy(&header[24], &color, 4);
	memcpy(&header[28], &color, 4);
	memcpy(&header[32], &color, 4);
	memcpy(&header[36], &alpha, 4);
	memcpy(&header[40], DXTString, 4);

	header.insert(header.end(), buffer.begin(), buffer.end());

	return header;
}

//dxtVersion is either 1 or 5, for "DXT1" or "DXT5". this also sets compression level to BC1/BC3.
//"flags" is some hardcoded bit flags. Not sure how much they matter. spec textures use 0x05. most diffuse textures use 0x01.
std::vector<uint8_t> TextureMaker::convert_cairo_surface_to_wtx(cairo_surface_t* surface, int dxtVersion, int flags) {
	int width = cairo_image_surface_get_width(surface);
	int height = cairo_image_surface_get_height(surface);
	int stride = cairo_image_surface_get_stride(surface);

	const uint8_t* data = cairo_image_surface_get_data(surface);

	DirectX::Image image = {};
	image.width = width;
	image.height = height;
	image.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	image.rowPitch = width * 4;
	image.slicePitch = width * 4 * height;
	image.pixels = const_cast<uint8_t*>(data);

	DirectX::ScratchImage scratchImage;
	scratchImage.Initialize2D(DXGI_FORMAT_B8G8R8A8_UNORM, width, height, 1, 1);
	memcpy(scratchImage.GetPixels(), image.pixels, image.slicePitch);
	std::unique_ptr<DirectX::ScratchImage> compressed_dds_image;

	const char* dxtString = "DXT5";
	DXGI_FORMAT dxtFormat = DXGI_FORMAT_BC3_UNORM;

	switch (dxtVersion) {
	case 1:
		dxtString = "DXT1";
		dxtFormat = DXGI_FORMAT_BC1_UNORM;
		break;
	case 5:
		dxtString = "DXT5";
		dxtFormat = DXGI_FORMAT_BC3_UNORM;
		break;
	}

	auto scratchtexture_with_mipmaps = std::make_unique<DirectX::ScratchImage>();
	if (0 != this->handle_errors(GenerateMipMaps(*scratchImage.GetImage(0,0,0), TEX_FILTER_CUBIC | TEX_FILTER_SEPARATE_ALPHA , 0, *scratchtexture_with_mipmaps), "making mipmaps")) {
		//
	}
	

	compressed_dds_image = std::make_unique<DirectX::ScratchImage>();
	if (0 != this->handle_errors(Compress(scratchtexture_with_mipmaps->GetImages(), scratchtexture_with_mipmaps->GetImageCount(), scratchtexture_with_mipmaps->GetMetadata(), dxtFormat, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, *compressed_dds_image), "compressing image")) {
		//return -1;
	}

	auto merged_vector = std::vector<uint8_t>();

	auto raw_converted = compressed_dds_image.get()->GetImages();
	auto resulting_blob = std::make_unique<DirectX::Blob>();
	if (0 != handle_errors(DirectX::SaveToDDSMemory(raw_converted, compressed_dds_image.get()->GetImageCount(), compressed_dds_image.get()->GetMetadata(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, *resulting_blob), "Makin a blob")) {
		//return -1;
	}
	if (0 != handle_errors(DirectX::SaveToDDSFile(raw_converted, compressed_dds_image.get()->GetImageCount(), compressed_dds_image.get()->GetMetadata(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, L"D:\\mipmaps.dds"), "Makin a blob")) {
		//return -1;
	}
	auto blob_pointer = reinterpret_cast<uint8_t*>(resulting_blob.get()->GetBufferPointer());
	auto blob_size = resulting_blob.get()->GetBufferSize();
	auto blob_vec = std::vector<uint8_t>(blob_pointer, blob_pointer + blob_size); //copies the data out of the blob, into a new vec.
	blob_vec.erase(blob_vec.begin(), blob_vec.begin() + 128);

	merged_vector.insert(merged_vector.end(), blob_vec.begin(), blob_vec.end());
	
	auto mipmaps = compressed_dds_image.get()->GetImageCount();
	auto withheader = TextureMaker::prepend_header(merged_vector, width, height, mipmaps, flags, dxtString);
	return withheader;
}

std::vector<uint8_t> TextureMaker::generate_desert_spec_line(std::vector<float> xpoints, std::vector<float> ypoints, float thickness, float dotthickness, bool symmetry)
{
	auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	auto background = cairo_image_surface_create_from_png("./images/desertspecpanel_square_bg.png");
	//auto status = surface->get_status();

	auto cr = cairo_create(surface);


	//fill background (idk if this is necessary)
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_paint(cr);

	//draw starting circle
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1);
	cairo_arc(cr, width * xpoints[0], height * ypoints[0], dotthickness, 0.0, 360.0);
	cairo_fill(cr);

	//draw line
	cairo_set_line_width(cr, thickness);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
	cairo_move_to(cr, width * xpoints[0], height * ypoints[0]);
	for (int index = 0; index < xpoints.size(); index++) {
		if (symmetry && index == xpoints.size() / 2) { //Draw start of second (symmetry) line
			cairo_stroke(cr);
			cairo_move_to(cr, width * xpoints[index], height * ypoints[index]);
			cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1);
			cairo_arc(cr, width * xpoints[index], height * ypoints[index], dotthickness, 0.0, 360.0);
			cairo_fill(cr);
		}
		cairo_line_to(cr, width * xpoints[index], height * ypoints[index]);
	}
	cairo_stroke(cr);

	auto pattern = cairo_pattern_create_for_surface(surface);

	auto bgcr = cairo_create(background);
	cairo_set_source(bgcr, pattern);
	cairo_paint(bgcr);

	auto finalTexture = convert_cairo_surface_to_wtx(background, 1, 0x05);
	cairo_pattern_destroy(pattern);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	cairo_destroy(bgcr);
	cairo_surface_destroy(background);
	return finalTexture;
}

void TextureMaker::draw_symbol_on_surface(cairo_surface_t* image, float x, float y, float scale, int symbolflags, std::optional<Color> customcolor, bool specular) {
	if (customcolor.has_value() && customcolor.value().a == 0)
		return;
	if (symbolflags == 0)
		return;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(0, 3);
	std::string filepath;
	if ((symbolflags & 0x700) == Decoration::Stone) {
		int stone_number = dis(gen);
		filepath = "./images/symbols/brush_stone_0.png";
		switch (stone_number) {
		case 0:
			filepath = "./images/symbols/brush_stone_0.png";
			break;
		case 1:
			filepath = "./images/symbols/brush_stone_1.png";
			break;
		case 2:
			filepath = "./images/symbols/brush_stone_2.png";
			break;
		case 3:
			filepath = "./images/symbols/brush_stone_3.png";
			break;
		}
		if (specular) filepath = "./images/symbols/spec_stone.png";
	}
	else if ((symbolflags & 0x700) == Decoration::Star) {
		filepath = "./images/symbols/spec_star.png";
	}
	else if ((symbolflags & 0x700) == Decoration::Triangle) {
		filepath = "./images/symbols/spec_tri1.png";
		if ((symbolflags & 0xF0000) == 0x20000)
			filepath = "./images/symbols/spec_tri2.png";
		if ((symbolflags & 0xF0000) == 0x30000)
			filepath = "./images/symbols/spec_tri3.png";
	}
	else if ((symbolflags & 0x700) == Decoration::Poly) {
		filepath = "./images/symbols/spec_poly.png";
	}
	else {
		filepath = "./images/symbols/spec_stone.png";  //Undefined symbol
	}

	auto* cr_image = cairo_create(image);

	std::tuple<float, float, float> color = { 0.0, 0.0, 0.0 };
	switch (symbolflags & 0xF) {
	case 0x2:
		color = { 0xff / 255.0, 0xff / 255.0, 0xff / 255.0 };
		break;
	case 0x4:
		color = { 0xa5 / 255.0, 0x51 / 255.0, 0xff / 255.0 };
		break;
	case 0x5:
		color = { 0x6e / 255.0, 0xab / 255.0, 0x5d / 255.0 };
		break;
	case 0x7:
		color = { 0xa4 / 255.0, 0x37 / 255.0, 0xf0 / 255.0 };
		break;
	case 0x8:
		color = { 0xf9 / 255.0, 0xf8 / 255.0, 0x45 / 255.0 };
		break;
	case 0x9:
		color = { 0x00 / 255.0, 0xa8 / 255.0, 0xe9 / 255.0 };
		break;
	}
	if (customcolor.has_value()) {
		color = { customcolor.value().r,customcolor.value().g,customcolor.value().b };
	}

	auto mask_image = cairo_image_surface_create_from_png(filepath.c_str());
	int maskwidth = cairo_image_surface_get_width(mask_image);
	int maskheight = cairo_image_surface_get_height(mask_image);
	auto scaled_stone_mask = cairo_surface_create_similar(mask_image, CAIRO_CONTENT_ALPHA, maskwidth * scale, maskheight * scale);
	auto cr_scaled_mask = cairo_create(scaled_stone_mask);
	cairo_scale(cr_scaled_mask, scale, scale);
	cairo_set_source_surface(cr_scaled_mask, mask_image, 0, 0);
	cairo_paint(cr_scaled_mask);
	cairo_destroy(cr_scaled_mask);

	auto [r, g, b] = color;
	cairo_set_source_rgb(cr_image, r, g, b);
	cairo_mask_surface(cr_image, scaled_stone_mask, x - (scale * maskwidth * .5), y - (scale * maskheight * .5));

	cairo_fill(cr_image);
	cairo_destroy(cr_image);
}

void TextureMaker::flip_image_vertically(cairo_surface_t* image)
{
	auto flipped_surface = cairo_surface_create_similar(image, CAIRO_CONTENT_COLOR_ALPHA, width, height);
	auto cr = cairo_create(flipped_surface);
	cairo_matrix_t matrix;
	cairo_matrix_init(&matrix, 1.0, 0.0, 0.0, -1.0, 0.0, height);
	cairo_transform(cr, &matrix);

	cairo_set_source_surface(cr, image, 0, 0);
	cairo_paint(cr);
	//flipped surface should now be painted with flipped image
	//now revert the flip, so we can copy contents back to original (could just return new surface? but this works too)
	cairo_transform(cr, &matrix);

	auto originalcr = cairo_create(image);
	cairo_set_source_surface(originalcr, flipped_surface, 0, 0);
	cairo_paint(originalcr);
	cairo_destroy(cr);
	cairo_destroy(originalcr);
	cairo_surface_destroy(flipped_surface);
}


// blank white texture. patches blue squares on mountain rainbow 5
std::vector<uint8_t> TextureMaker::generate_blank_texture() {
	auto background = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
	//flip_image_vertically(background); //doesn't *really* matter, but texture loads upside down to what game displays.

	auto cr = cairo_create(background);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_paint(cr);
	cairo_surface_flush(background);
	auto finalTexture = convert_cairo_surface_to_wtx(background, 1, 0x00);
	cairo_destroy(cr);
	cairo_surface_destroy(background);
	return finalTexture;
}

std::vector<uint8_t> TextureMaker::generate_color_panel_grid(std::vector<std::vector<int>> grid, int id, std::vector<Color> colors, bool specular)
{
	auto gridheight = grid.size();
	auto gridwidth = grid[0].size();
	std::vector<int> only_symbols;
	for (int i = gridwidth - 1; i >= 0; --i) {
		for (int j = 0; j < gridheight; ++j) {
			if ((i % 2 != 0) && (j % 2 != 0)) { // Check if it's between two lines
				int cell = grid[j][i];
				only_symbols.push_back(cell);
			}
		}
	}

	//while its theoretically better to actually calculate these for arbitrary sizes
	//until we can change the puzzle meshes in the bunker, there are only ever a few puzzle sizes
	//3x3, 4x4, and 4x5
	//we can simply hard code the positions of the puzzle elements and their scales
	float puzzle_element_scale;
	std::vector<std::tuple<double, double>> symbol_coordinates;

	float minx = 0.1f; float miny = 0.11f; float maxx = 0.92f; float maxy = 0.922f;
	float unitWidth = (maxx - minx) / (gridwidth - 1);
	float unitHeight = (maxy - miny) / (gridheight - 1);
	for (int y = 1; y < gridheight; y += 2) {
		for (int x = 1; x < gridwidth; x += 2) {
			symbol_coordinates.push_back({ (minx + x * unitWidth) * 500, (miny + y * unitHeight) * 500 });
		}
	}
	puzzle_element_scale = min(unitWidth, unitHeight) * 2.7f;

	auto bg_path = "./images/color_bunker_blueprint_bg.png";
	switch (id) {
	case 0x0A010:
	case 0x0A01B:
		bg_path = "./images/color_bunker_greyred_light.png";
		break;
	case 0x0A01F:
		bg_path = "./images/color_bunker_greyred_dark.png";
		break;
	case 0x17E63:
	case 0x17E67:
		bg_path = "./images/color_bunker_whitepaper.png";
		break;
	case 0x0A079:
		bg_path = "./images/color_bunker_blueprint_elevator.png";
		break;
	}
	if (specular) bg_path = "./images/desertspecpanel_square_bg.png";

	//auto surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	auto background = cairo_image_surface_create_from_png(bg_path);
	//flip_image_vertically(background); //doesn't *really* matter, but texture loads upside down to what game displays.
	//auto status = surface->get_status();

	auto cr = cairo_create(background);


	////fill background (idk if this is necessary)
	//cairo_set_source_rgba(cr, 0, 0, 0, 0);
	//cairo_paint(cr);

	//iterate over both (puzzle elements, coordinates) by index
	for (int i = 0; i < only_symbols.size(); i++) {
		auto element = only_symbols[i];
		auto [x, y] = symbol_coordinates[i];
		if ((element & 0x700) == Decoration::Poly) {
			int shape = element >> 16;
			int maxx = 0, maxy = 0;
			for (int x2 = 0; x2 < 4; x2++) {
				for (int y2 = 0; y2 < 4; y2++) {
					if (((shape >> (y2 * 4 + x2)) & 1) == 1) {
						maxx = max(x2, maxx);
						maxy = max(y2, maxy);
					}
				}
			}
			for (int x2 = 0; x2 < 4; x2++) {
				for (int y2 = 0; y2 < 4; y2++) {
					if (((shape >> (y2 * 4 + x2)) & 1) == 1) {
						if (colors.size() > 0) {
							draw_symbol_on_surface(background, x + (x2 * 2 - maxx) * 33.3f * puzzle_element_scale + 1.5f, y + (y2 * 2 - maxy) * 33.3f * puzzle_element_scale,
								puzzle_element_scale, element, colors[i], specular);
						}
						else {
							draw_symbol_on_surface(background, x + (x2 * 2 - maxx) * 33.3f * puzzle_element_scale + 1.5f, y + (y2 * 2 - maxy) * 33.3f * puzzle_element_scale,
								puzzle_element_scale, element, std::nullopt, specular);
						}
					}
				}
			}
		}
		else {
			if (colors.size() > 0) {
				draw_symbol_on_surface(background, x, y, puzzle_element_scale, element, colors[i], specular);
			}
			else {
				draw_symbol_on_surface(background, x, y, puzzle_element_scale, element, std::nullopt, specular);
			}
		}
	}
	flip_image_vertically(background);
	//zero alpha channel
	//TODO which panels do we need to do this with, exactly?
	cairo_surface_flush(background);
	if (!specular) {
		uint8_t* data = cairo_image_surface_get_data(background);
		for (int i = 3; i < (width * height * 4); i += 4) {
			data[i] = 0;
		}
		cairo_surface_mark_dirty(background); //tell cairo we tampered with the raw pixel data.
	}	
	auto finalTexture = convert_cairo_surface_to_wtx(background, 5, specular ? 0x05 : 0x01);
	//cairo_pattern_destroy(pattern);
	cairo_destroy(cr);
	//cairo_surface_destroy(/*surface*/);
	//cairo_destroy(bgcr);
	cairo_surface_destroy(background);

	return finalTexture;
}
