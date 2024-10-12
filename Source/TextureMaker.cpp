
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

using namespace DirectX;

TextureMaker::TextureMaker(int h, int w) {
	height = h;
	width = w;
}

std::vector<uint8_t> TextureMaker::prepend_header(std::vector<uint8_t> buffer, uint16_t width, uint16_t height, uint8_t bits, const char * DXTString)
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
	uint16_t num_mipmaps = 1;
	memcpy(&header[18], &num_mipmaps, 2);
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

std::vector<uint8_t> TextureMaker::generate_desert_spec_line(std::vector<float> xpoints, std::vector<float> ypoints, float thickness, float dotthickness)
{
	std::unique_ptr<DirectX::ScratchImage> compressed_dds_image;

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
		cairo_line_to(cr, width * xpoints[index], height * ypoints[index]);
	}
	cairo_stroke(cr);

	auto pattern = cairo_pattern_create_for_surface(surface);

	auto bgcr = cairo_create(background);
	cairo_set_source(bgcr, pattern);
	cairo_paint(bgcr);


	const uint8_t* data = cairo_image_surface_get_data(background);
	int stride = cairo_image_surface_get_stride(background);


	//I really thought I'd have to transpose these pixels' bytes around.
	//after all, one is supposedly ARGB (in cairo) and the other is BGRA (in directX)
	//but it works without transposing?
	//Maybe It *isnt* working and i'm just getting lucky since this texture is grayscale?


	DirectX::Image image = {};
	image.width = width;
	image.height = height;
	image.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	image.rowPitch = width * 4;
	image.slicePitch = width * 4 * height;
	image.pixels = const_cast<uint8_t*>(data);

	DirectX::ScratchImage scratchImage;
	scratchImage.Initialize2D(DXGI_FORMAT_B8G8R8X8_UNORM, width, height, 1, 1);
	memcpy(scratchImage.GetPixels(), image.pixels, image.slicePitch);

	compressed_dds_image = std::make_unique<DirectX::ScratchImage>();
	if (0 != this->handle_errors(Compress(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DXGI_FORMAT_BC1_UNORM, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, *compressed_dds_image), "compressing image")) {
		//return -1;
	}

	auto merged_vector = std::vector<uint8_t>();

	for (int i = 0; i < compressed_dds_image.get()->GetImageCount(); i++) {
		auto raw_converted = compressed_dds_image.get()->GetImage(i, 0, 0);
		auto resulting_blob = std::make_unique<DirectX::Blob>();
		if (0 != handle_errors(DirectX::SaveToDDSMemory(*raw_converted, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, *resulting_blob), "Makin a blob")) {
			//return -1;
		}
		auto blob_pointer = reinterpret_cast<uint8_t*>(resulting_blob.get()->GetBufferPointer());
		auto blob_size = resulting_blob.get()->GetBufferSize();
		auto blob_vec = std::vector<uint8_t>(blob_pointer, blob_pointer + blob_size); //copies the data out of the blob, into a new vec.
		blob_vec.erase(blob_vec.begin(), blob_vec.begin() + 128);

		merged_vector.insert(merged_vector.end(), blob_vec.begin(), blob_vec.end());
	}
	auto withheader = TextureMaker::prepend_header(merged_vector, width, height, 0x05, "DXT1");


	cairo_pattern_destroy(pattern);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
	cairo_destroy(bgcr);
	cairo_surface_destroy(background);

	return withheader;
}
