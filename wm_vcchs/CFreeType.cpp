#include "CFreeType.h"
#include "rwFunc.h"
#include "CFont.h"

#include <algorithm>

#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <ft2build.h>
#include FT_FREETYPE_H 
#include <freetype/ftglyph.h>

#pragma comment (lib, "../deps/freetype-2.6.5/lib/freetype265MT.lib")

static CachedCharInfo InfoArray[0x10000];

static FT_Library library;
static FT_Face face;
static FT_Raster ftraster;

static RwRGBA *ppalette;

void CFreeType::Init()
{
	FT_Init_FreeType(&library);
	FT_New_Face(library, CFont::fontPath, 0, &face);
	FT_Set_Pixel_Sizes(face, 52, 0);

	ppalette = (RwRGBA *)std::malloc(sizeof(RwRGBA) * 256);

	for (unsigned int index = 0; index < 256; ++index)
	{
		ppalette[index] = { (RwUInt8)255, (RwUInt8)255, (RwUInt8)255, (RwUInt8)index };
	}
}

void CFreeType::Close()
{
	auto aaa = sizeof(InfoArray);
	std::for_each(std::begin(InfoArray),std::end(InfoArray),
		[](CachedCharInfo &info)
	{
		if (info.sprite.Valid())
		{
			CSprite2d::fpDelete(&info.sprite);
		}
	});

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	std::free(ppalette);
}

void CFreeType::InitCharInfo(wchar_t arg_char)
{
	RwInt32 width, height, depth, flags;

	RwImage *image;
	RwRaster *raster;

	CachedCharInfo &info = InfoArray[arg_char];

	if (FT_Load_Char(face, arg_char, FT_LOAD_NO_SCALE) == 0)
	{


		FT_Load_Char(face, arg_char, FT_LOAD_RENDER);

		FT_Bitmap &bitmap = face->glyph->bitmap;
		info.bitmap_width = bitmap.width;
		info.bitmap_height = bitmap.rows;

		//计算宽度和绘制矩形
		if (bitmap.rows < 20)
		{

		}

		image = rwFunc::fpRwImageCreate(bitmap.width, bitmap.rows, 8);

		rwFunc::fpRwImageAllocatePixels(image);
		RwImageSetPalette(image, ppalette);

		std::memset(image->cpPixels, 0, image->height * image->stride);

		for (unsigned int row = 0; row < bitmap.rows; ++row)
		{
			std::memcpy(&image->cpPixels[row * image->stride], &bitmap.buffer[row * bitmap.pitch], bitmap.width);
		}
	}
	else
	{
		image = rwFunc::fpRwImageCreate(4, 4, 8);

		rwFunc::fpRwImageAllocatePixels(image);
		RwImageSetPalette(image, ppalette);

		std::memset(image->cpPixels, 255, image->height * image->stride);

		InfoArray[arg_char].bitmap_width = 32;
		InfoArray[arg_char].bitmap_height = 32;
		InfoArray[arg_char].width = 32.0f;
	}

	rwFunc::fpRwImageFindRasterFormat(image, rwRASTERTYPETEXTURE, &width, &height, &depth, &flags);
	raster = rwFunc::fpRwRasterCreate(width, height, depth, flags);
	rwFunc::fpRwRasterSetFromImage(raster, image);
	rwFunc::fpRwImageDestroy(image);

	InfoArray[arg_char].sprite.SetRwTexture(rwFunc::fpRwTextureCreate(raster));
}

const CachedCharInfo &CFreeType::GetCharInfo(wchar_t arg_char)
{
	if (!(InfoArray[arg_char].sprite.Valid()))
	{
		InitCharInfo(arg_char);
	}

	return InfoArray[arg_char];
}

