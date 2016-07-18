#pragma once
#include <cstdint>

struct RwTexture;
struct RwRaster;

#define RWFORCEENUMSIZEINT ((__int32)((~((std::uint32_t)0))>>1))

struct RwRGBA
{
	std::uint8_t red;
	std::uint8_t green;
	std::uint8_t blue;
	std::uint8_t alpha;
};

struct RwImage
{
	__int32 flags;
	__int32 width;
	__int32 height;
	__int32 depth;
	__int32 stride;
	std::uint8_t *cpPixels;
	RwRGBA *palette;
};

enum RwRenderState
{
	rwRENDERSTATENARENDERSTATE = 0,
	rwRENDERSTATETEXTURERASTER,
	rwRENDERSTATETEXTUREADDRESS,
	rwRENDERSTATETEXTUREADDRESSU,
	rwRENDERSTATETEXTUREADDRESSV,
	rwRENDERSTATETEXTUREPERSPECTIVE,
	rwRENDERSTATEZTESTENABLE,
	rwRENDERSTATESHADEMODE,
	rwRENDERSTATEZWRITEENABLE,
	rwRENDERSTATETEXTUREFILTER,
	rwRENDERSTATESRCBLEND,
	rwRENDERSTATEDESTBLEND,
	rwRENDERSTATEVERTEXALPHAENABLE,
	rwRENDERSTATEBORDERCOLOR,
	rwRENDERSTATEFOGENABLE,
	rwRENDERSTATEFOGCOLOR,
	rwRENDERSTATEFOGTYPE,
	rwRENDERSTATEFOGDENSITY,
	rwRENDERSTATECULLMODE = 20,
	rwRENDERSTATESTENCILENABLE,
	rwRENDERSTATESTENCILFAIL,
	rwRENDERSTATESTENCILZFAIL,
	rwRENDERSTATESTENCILPASS,
	rwRENDERSTATESTENCILFUNCTION,
	rwRENDERSTATESTENCILFUNCTIONREF,
	rwRENDERSTATESTENCILFUNCTIONMASK,
	rwRENDERSTATESTENCILFUNCTIONWRITEMASK,
	rwRENDERSTATEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

namespace rwFunc
{
	extern __int32 *RsGlobalW;
	extern __int32 *RsGlobalH;

	extern __int32(__cdecl *RwRenderStateSet)(RwRenderState state, void *value);

	extern RwTexture *LoadTextureFromPNG(const char *filename);

	extern RwImage *(__cdecl *RwImageCreate)(__int32 width, __int32 height, __int32 depth);
	extern __int32(__cdecl *RwImageDestroy)(RwImage *image);
	extern RwImage *(__cdecl *RwImageAllocatePixels)(RwImage *image);
	extern RwImage *(__cdecl *RwImageFreePixels)(RwImage *image);

	extern RwImage *(__cdecl *RwImageFindRasterFormat)(RwImage *ipImage, __int32 nRasterType, __int32 *npWidth, __int32 *npHeight, __int32 *npDepth, __int32 *npFormat);
	extern RwRaster *(__cdecl *RwRasterCreate)(__int32 width, __int32 height, __int32 depth, __int32 flags);
	extern RwRaster *(__cdecl *RwRasterSetFromImage)(RwRaster *raster, RwImage *image);
	extern RwTexture *(__cdecl *RwTextureCreate)(RwRaster *raster);

	RwImage *RtPNGImageRead(const char *filename);

	void GetAddresses();
}
