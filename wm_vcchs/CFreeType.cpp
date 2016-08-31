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

CachedCharInfo CFreeType::m_InfoArray[0x10000];

static FT_Library library;
static FT_Face face;

static RwRGBA *ppalette;

void CFreeType::Init()
{
	FT_Init_FreeType(&library);
	FT_New_Face(library, CFont::fontPath, 0, &face);
	FT_Set_Pixel_Sizes(face, 26, 26);

	ppalette = (RwRGBA *)std::malloc(sizeof(RwRGBA) * 256);

	for (unsigned int index = 0; index < 256; ++index)
	{
		ppalette[index] = { (RwUInt8)255, (RwUInt8)255, (RwUInt8)255, (RwUInt8)index };
	}
}

void CFreeType::Close()
{
	std::for_each(std::begin(m_InfoArray),std::end(m_InfoArray),
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

	if (FT_Load_Char(face, arg_char, FT_LOAD_NO_SCALE) == 0)
	{
		m_InfoArray[arg_char].width = face->glyph->metrics.width / 64.0f;
		m_InfoArray[arg_char].height = face->glyph->metrics.width / 64.0f;

		FT_Load_Char(face, arg_char, FT_LOAD_RENDER);

		m_InfoArray[arg_char].bitmap_width = face->glyph->bitmap.width;
		m_InfoArray[arg_char].bitmap_height = face->glyph->bitmap.rows;

		image = rwFunc::fpRwImageCreate(face->glyph->bitmap.width, face->glyph->bitmap.rows, 8);

		rwFunc::fpRwImageAllocatePixels(image);
		RwImageSetPalette(image, ppalette);

		std::memset(image->cpPixels, 0, image->height * image->stride);
		
		for (unsigned int row = 0; row < face->glyph->bitmap.rows; ++row)
		{
			for (unsigned int column = 0; column < face->glyph->bitmap.width; ++column)
			{
				image->cpPixels[row * image->stride + column] = face->glyph->bitmap.buffer[row * face->glyph->bitmap.pitch + column];
			}
		}
	}
	else
	{
		image = rwFunc::fpRwImageCreate(32, 32, 8);

		rwFunc::fpRwImageAllocatePixels(image);
		RwImageSetPalette(image, ppalette);

		std::memset(image->cpPixels, 255, image->height * image->stride);

		m_InfoArray[arg_char].width = 32;
		m_InfoArray[arg_char].height = 32;
		m_InfoArray[arg_char].bitmap_width = 32;
		m_InfoArray[arg_char].bitmap_height = 32;
	}

	rwFunc::fpRwImageFindRasterFormat(image, rwRASTERTYPETEXTURE, &width, &height, &depth, &flags);
	raster = rwFunc::fpRwRasterCreate(width, height, depth, flags);
	rwFunc::fpRwRasterSetFromImage(raster, image);
	rwFunc::fpRwImageDestroy(image);
	m_InfoArray[arg_char].sprite.SetRwTexture(rwFunc::fpRwTextureCreate(raster));
}

const CachedCharInfo &CFreeType::GetCharInfo(wchar_t arg_char)
{
	if (!(m_InfoArray[arg_char].sprite.Valid()))
	{
		InitCharInfo(arg_char);
	}

	return m_InfoArray[arg_char];
}
