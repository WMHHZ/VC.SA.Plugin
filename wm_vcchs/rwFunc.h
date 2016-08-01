#pragma once
#include "../include/func_wrapper/func_wrapper.hpp"

struct RwTexture;
struct RwRaster;

#define RWFORCEENUMSIZEINT ((__int32)((~((unsigned __int32)0))>>1))

struct RwRGBA
{
	unsigned __int8 red;
	unsigned __int8 green;
	unsigned __int8 blue;
	unsigned __int8 alpha;
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

struct RwD3D8Vertex
{
	float x;
	float y;
	float z;
	float rhw;
	unsigned __int32 emissiveColor;
	float u;
	float v;
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

enum RwPrimitiveType
{
	rwPRIMTYPENAPRIMTYPE = 0,
	rwPRIMTYPELINELIST = 1,
	rwPRIMTYPEPOLYLINE = 2,
	rwPRIMTYPETRILIST = 3,
	rwPRIMTYPETRISTRIP = 4,
	rwPRIMTYPETRIFAN = 5,
	rwPRIMTYPEPOINTLIST = 6,
	rwPRIMITIVETYPEFORCEENUMSIZEINT = RWFORCEENUMSIZEINT
};

class rwFunc
{
public:
	static __int32 *RsGlobalW;
	static __int32 *RsGlobalH;

	static cdecl_func_wrapper<__int32(RwRenderState state, void *value)>
		fpRwRenderStateSet;

	static cdecl_func_wrapper<RwImage *(__int32 width, __int32 height, __int32 depth)>
		fpRwImageCreate;

	static cdecl_func_wrapper<__int32(RwImage *image)>
		fpRwImageDestroy;

	static cdecl_func_wrapper<RwImage *(RwImage *image)>
		fpRwImageAllocatePixels;

	static cdecl_func_wrapper<RwImage *(RwImage *image)>
		fpRwImageFreePixels;

	static cdecl_func_wrapper<RwImage *(RwImage *ipImage, __int32 nRasterType, __int32 *npWidth, __int32 *npHeight, __int32 *npDepth, __int32 *npFormat)>
		fpRwImageFindRasterFormat;

	static cdecl_func_wrapper<RwRaster *(__int32 width, __int32 height, __int32 depth, __int32 flags)>
		fpRwRasterCreate;

	static cdecl_func_wrapper<RwRaster *(RwRaster *raster, RwImage *image)>
		fpRwRasterSetFromImage;

	static cdecl_func_wrapper<RwTexture *(RwRaster *raster)>
		fpRwTextureCreate;

	static RwImage *RtPNGImageRead(const char *filename);
	static RwTexture *LoadTextureFromPNG(const char *filename);

	rwFunc();
};
