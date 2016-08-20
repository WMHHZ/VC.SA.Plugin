#include "CFreeType.h"
#include "rwFunc.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <cstdlib>

#include <ft2build.h>
#include FT_FREETYPE_H 
#include <freetype/ftglyph.h>

#pragma comment (lib, "../include/freetype-2.6.5/lib/freetype265MT.lib")

CachedCharInfo CFreeType::m_Sprites[0x10000];

static FT_Library library;
static FT_Face face;

void CFreeType::Init()
{
	FT_Init_FreeType(&library);
	FT_New_Face(library, R"-(C:\Users\ClansChen\Desktop\msyhbd.ttf)-", 0, &face);
	FT_Set_Pixel_Sizes(face, 28, 28);
}

void CFreeType::Close()
{
	std::for_each(std::begin(m_Sprites),std::end(m_Sprites),
		[](CachedCharInfo &info)
	{
		if (info.sprite.Valid())
		{
			CSprite2d::fpDelete(&info.sprite);
		}
	});

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void CFreeType::InitCharSprite(wchar_t arg_char)
{
	static const RwUInt32 powsof2[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

	RwInt32 width, height, depth, flags;
	FT_Load_Char(face, arg_char + 0x20, FT_LOAD_RENDER);

	auto size = *std::upper_bound(std::begin(powsof2), std::end(powsof2), std::max(face->glyph->bitmap.rows, face->glyph->bitmap.width));

	RwImage *image = rwFunc::fpRwImageCreate(32, 32, 8);
	rwFunc::fpRwImageAllocatePixels(image);

	for (std::size_t index = 0; index < 256; ++index)
	{
		image->palette[index].red = 255;
		image->palette[index].green = 255;
		image->palette[index].blue = 255;
		image->palette[index].alpha = index;
	}

	std::memset(image->cpPixels, 0, size * size);

	for (unsigned int index = 0; index < face->glyph->bitmap.rows; ++index)
	{
		std::memcpy(&image->cpPixels[std::abs(image->stride) * index], &face->glyph->bitmap.buffer[index * std::abs(face->glyph->bitmap.pitch)], std::abs(face->glyph->bitmap.pitch));
	}

	rwFunc::fpRwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
	RwRaster *raster = rwFunc::fpRwRasterCreate(width, height, depth, flags);
	rwFunc::fpRwRasterSetFromImage(raster, image);
	rwFunc::fpRwImageDestroy(image);
	m_Sprites[arg_char + 0x20].sprite.SetRwTexture(rwFunc::fpRwTextureCreate(raster));
}

const CachedCharInfo &CFreeType::GetCharSprite(wchar_t arg_char)
{
	if (!(m_Sprites[arg_char + 0x20].sprite.Valid()))
	{
		InitCharSprite(arg_char);
	}

	return m_Sprites[arg_char + 0x20];
}
