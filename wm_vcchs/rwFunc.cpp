#include "rwFunc.h"
#include "../depends/lpng1622/include/png.h"
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include "../include/hooking/Hooking.Patterns.h"

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng16.lib")

__int32 *rwFunc::RsGlobalW;
__int32 *rwFunc::RsGlobalH;

cdecl_func_wrapper<__int32(RwRenderState state, void *value)>
rwFunc::fpRwRenderStateSet;

cdecl_func_wrapper<RwImage *(__int32 width, __int32 height, __int32 depth)>
rwFunc::fpRwImageCreate;

cdecl_func_wrapper<__int32(RwImage *image)>
rwFunc::fpRwImageDestroy;

cdecl_func_wrapper<RwImage *(RwImage *image)>
rwFunc::fpRwImageAllocatePixels;

cdecl_func_wrapper<RwImage *(RwImage *image)>
rwFunc::fpRwImageFreePixels;

cdecl_func_wrapper<RwImage *(RwImage *ipImage, __int32 nRasterType, __int32 *npWidth, __int32 *npHeight, __int32 *npDepth, __int32 *npFormat)>
rwFunc::fpRwImageFindRasterFormat;

cdecl_func_wrapper<RwRaster *(__int32 width, __int32 height, __int32 depth, __int32 flags)>
rwFunc::fpRwRasterCreate;

cdecl_func_wrapper<RwRaster *(RwRaster *raster, RwImage *image)>
rwFunc::fpRwRasterSetFromImage;

cdecl_func_wrapper<RwTexture *(RwRaster *raster)>
rwFunc::fpRwTextureCreate;

RwTexture *rwFunc::LoadTextureFromPNG(const char *filename)
{
	__int32 width, height, depth, flags;

	RwImage *image = RtPNGImageRead(filename);
	fpRwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
	RwRaster *raster = fpRwRasterCreate(width, height, depth, flags);
	fpRwRasterSetFromImage(raster, image);
	fpRwImageDestroy(image);
	return fpRwTextureCreate(raster);
}

RwImage *rwFunc::RtPNGImageRead(const char *filename)
{
	png_uint_32 width = 0, height = 0;
	int bit_depth = 0, color_type = 0, interlace_method = 0;
	int depth = 0;
	void *buffer = nullptr;

	FILE *file = fopen(filename, "rb");
	if (!file)
	{
		return nullptr;
	}

	png_structp read_structp = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!read_structp)
	{
		fclose(file);
		return nullptr;
	}

	png_infop info_structp = png_create_info_struct(read_structp);
	if (!info_structp)
	{
		fclose(file);
		png_destroy_read_struct(&read_structp, nullptr, nullptr);
		return nullptr;
	}

	if (setjmp(png_jmpbuf(read_structp)) != 0)
	{
		free(buffer);
		fclose(file);
		png_destroy_read_struct(&read_structp, nullptr, nullptr);
		return nullptr;
	}

	png_init_io(read_structp, file);

	png_read_info(read_structp, info_structp);

	png_get_IHDR(read_structp, info_structp, &width, &height, &bit_depth, &color_type, &interlace_method, nullptr, nullptr);
	png_set_strip_16(read_structp);

	switch (color_type)
	{
	case 0:
	{
		depth = 8;
		if (bit_depth < 8)
		{
			png_set_expand_gray_1_2_4_to_8(read_structp);
		}
		break;
	}
	case 2:
	{
		depth = 32;
		if (bit_depth < 8)
		{
			png_set_filler(read_structp, 255, 1);
		}
		break;
	}
	case 3:
	{
		depth = 8;
		if (bit_depth < 8)
		{
			png_set_packing(read_structp);
			depth = 4;
		}
		break;
	}
	case 4:
	{
		depth = 32;
		png_set_gray_to_rgb(read_structp);
		break;
	}
	case 6:
	{
		depth = 32;
		break;
	}
	default:
	{
		break;
	}
	}

	RwImage *image = fpRwImageCreate(width, height, depth);
	if (!image)
	{
		fclose(file);
		return nullptr;
	}

	if (!fpRwImageAllocatePixels(image))
	{
		fpRwImageDestroy(image);
		fclose(file);
		return nullptr;
	}

	buffer = malloc(height * 4);
	if (!buffer)
	{
		fpRwImageFreePixels(image);
		fpRwImageDestroy(image);
		fclose(file);
		return nullptr;
	}

	if (height > 0)
	{
		unsigned __int8 *ecx = image->cpPixels;
		__int32 esi = image->stride;

		unsigned eax = 0;

		do
		{
			((unsigned __int8 **)buffer)[eax] = ecx;
			ecx += esi;
			++eax;
		} while (eax < height);
	}

	RwRGBA *palette = image->palette;
	if ((depth == 4 || depth == 8) && palette)
	{
		if ((1 << (unsigned __int8)depth) > 0)
		{
			unsigned __int8 al = 0;
			RwRGBA *edx = palette;

			do
			{
				edx->red = al;
				edx->green = al;
				edx->blue = al;
				edx->alpha = 255;
				++al;
				++edx;
			} while (al < (1 << (unsigned __int8)depth));
		}

		if (png_get_valid(read_structp, info_structp, 8) != 0)
		{
			png_colorp colorp;

			int num_palette;
			png_get_PLTE(read_structp, info_structp, &colorp, &num_palette);

			if (num_palette > 0)
			{
				unsigned __int8 *eax = &colorp->blue;
				unsigned __int8 *ecx = &palette->blue;
				int esi = num_palette;

				do
				{
					*(ecx - 2) = *(eax - 2);
					eax += 3;
					*(ecx - 1) = *(eax - 4);
					*ecx = *(eax - 3);
					ecx += 4;
					--esi;
				} while (esi != 0);
			}
		}

		if (png_get_valid(read_structp, info_structp, 16) != 0)
		{
			int num_trans;
			png_bytep trans_alpha;
			png_color_16p trans_color;
			png_get_tRNS(read_structp, info_structp, &trans_alpha, &num_trans, &trans_color);

			if (num_trans > 0)
			{
				unsigned __int8 *ecx = &palette->alpha;
				png_bytep edi = trans_alpha;
				int eax = 0;

				do
				{
					*ecx = edi[eax];
					++eax;
					ecx += 4;
				} while (eax < num_trans);
			}
		}
	}

	png_read_image(read_structp, (png_bytepp)buffer);
	png_read_end(read_structp, info_structp);
	png_destroy_read_struct(&read_structp, &info_structp, nullptr);
	free(buffer);
	fclose(file);
	return image;
}

rwFunc::rwFunc()
{
	RsGlobalW = *hook::pattern("8B 2D ? ? ? ? 83 EC 48").get(0).get<__int32 *>(2);
	RsGlobalH = RsGlobalW + 1;

	fpRwRenderStateSet = hook::pattern("A1 ? ? ? ? 83 EC 08 83 38 00").get(0).get();

	fpRwImageCreate = hook::pattern("8B 14 01 52 FF 90 40 01 00 00 8B F0 33 C0 83 C4 04 3B F0 75 02").get(0).get(-0xC);
	fpRwImageDestroy = hook::pattern("56 8B 74 24 08 F6 06 01").get(0).get();
	fpRwImageAllocatePixels = hook::pattern("83 EC 08 53 55 56 8B 74 24 18 57 8B 4E 0C").get(0).get();
	fpRwImageFreePixels = hook::pattern("8B 0D ? ? ? ? 56 8B 74 24 08 8B 46 14 50 FF 91 34 01 00 00").get(0).get();

	fpRwImageFindRasterFormat = hook::pattern("8B 44 24 08 8B 15 ? ? ? ? 83 EC 34").get(0).get();
	fpRwRasterCreate = hook::pattern("A1 ? ? ? ? 8B 0D ? ? ? ? 56 8B 54 01 60 52 FF 90 40 01 00 00").get(0).get();
	fpRwRasterSetFromImage = hook::pattern("A1 ? ? ? ? 56 8B 74 24 08 57 8B 7C 24 10 6A 00 57 56 FF 50 64").get(0).get();
	fpRwTextureCreate = hook::pattern("A1 ? ? ? ? 8B 0D ? ? ? ? 56 8B 54 01 08 52 FF 90 40 01 00 00").get(0).get();
}

static rwFunc instance;
