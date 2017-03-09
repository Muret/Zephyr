
#include "Texture.h"

#include "FreeImage.h"
#include "TextureLoader.h"



TextureLoader::TextureLoader(std::string filename) : filename_(filename)
{
	

}

Texture* TextureLoader::create_texture_from_file()
{
	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);
	//OpenGL's image ID to map to

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename_.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(filename_.c_str());
	//if still unkown, return failure
	if (fif == FIF_UNKNOWN)
		return nullptr;

	

	//check that the plugin has reading capabilities and load the file
	if (FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filename_.c_str());
	//if the image failed to load, return failure
	if (!dib)
		return nullptr;

	FREE_IMAGE_TYPE type = FreeImage_GetImageType(dib);

	if (type != FIT_BITMAP)
	{
		int a = 5;
		return nullptr;
	}

	unsigned red_mask, green_mask, blue_mask;
	red_mask = FreeImage_GetRedMask(dib);
	green_mask = FreeImage_GetGreenMask(dib);
	blue_mask = FreeImage_GetBlueMask(dib);

	int x = FreeImage_GetBPP(dib);

	FIBITMAP *rgba_dib = FreeImage_ConvertToType(dib, FIT_RGBAF);

	int x2 = FreeImage_GetBPP(rgba_dib);

	//retrieve the image data
	bits = FreeImage_GetBits(rgba_dib);
	//get the image width and height
	width = FreeImage_GetWidth(rgba_dib);
	height = FreeImage_GetHeight(rgba_dib);

	if (bits == nullptr)
	{
		return nullptr;
	}

	Texture *text = new Texture();
	text->create(D3DXVECTOR3(width, height, 1), bits, DXGI_FORMAT_R32G32B32A32_FLOAT, 0);

	return text;
}

