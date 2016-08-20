#pragma once
#include "../include/func_wrapper/func_wrapper.hpp"

struct RwTexture;
struct RwRaster;

typedef __int32 RwBool, RwInt32;
typedef unsigned __int32 RwUInt32;

#define RWFORCEENUMSIZEINT ((__int32)((~((unsigned __int32)0))>>1))

struct RwRGBA
{
	unsigned __int8 red;
	unsigned __int8 green;
	unsigned __int8 blue;
	unsigned __int8 alpha;
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

struct RwImage
{
	__int32 flags;
	__int32 width;
	__int32 height;
	__int32 depth;
	__int32 stride;
	unsigned __int8 *cpPixels;
	RwRGBA *palette;
};

class rwFunc
{
public:
	static __int32 *RsGlobalW;
	static __int32 *RsGlobalH;

	static cdecl_func_wrapper<__int32(RwRenderState state, void *value)>
		fpRwRenderStateSet;

	static cdecl_func_wrapper<RwImage *(__int32 width, __int32 height,__int32 depth)>
		fpRwImageCreate;

	static cdecl_func_wrapper<RwBool(RwImage *image)>
		fpRwImageDestroy;

	static cdecl_func_wrapper<RwImage *(RwImage *image)>
		fpRwImageAllocatePixels;

	static cdecl_func_wrapper<RwImage *(RwImage *ipImage, RwInt32 nRasterType, RwInt32 *npWidth, RwInt32 *npHeight, RwInt32 *npDepth, RwInt32 *npFormat)>
		fpRwImageFindRasterFormat;

	static cdecl_func_wrapper<RwRaster *(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags)>
		fpRwRasterCreate;

	static cdecl_func_wrapper<RwRaster *(RwRaster *raster, RwImage *image)>
		fpRwRasterSetFromImage;

	static cdecl_func_wrapper<RwTexture *(RwRaster *raster)>
		fpRwTextureCreate;

	rwFunc();
};
