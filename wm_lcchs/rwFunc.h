#pragma once

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

class rwFunc
{
public:
	static __int32 *RsGlobalW;
	static __int32 *RsGlobalH;

	static __int32 RwRenderStateSet(RwRenderState state, void *value);

	static RwImage *RwImageCreate(__int32 width, __int32 height, __int32 depth);
	static __int32 RwImageDestroy(RwImage *image);
	static RwImage *RwImageAllocatePixels(RwImage *image);
	static RwImage *RwImageFreePixels(RwImage *image);

	static RwImage *RwImageFindRasterFormat(RwImage *ipImage, __int32 nRasterType, __int32 *npWidth, __int32 *npHeight, __int32 *npDepth, __int32 *npFormat);
	static RwRaster *RwRasterCreate(__int32 width, __int32 height, __int32 depth, __int32 flags);
	static RwRaster *RwRasterSetFromImage(RwRaster *raster, RwImage *image);
	static RwTexture *RwTextureCreate(RwRaster *raster);
	
	static __int32 RwIm2DRenderPrimitive(RwPrimitiveType primType, RwD3D8Vertex *vertices, __int32 numVertices);

	static RwImage *RtPNGImageRead(const char *filename);
	static RwTexture *LoadTextureFromPNG(const char *filename);

	static void GetAddresses();

private:
	static void *fpRwRenderStateSet;
	static void *fpRwImageCreate;
	static void *fpRwImageDestroy;
	static void *fpRwImageAllocatePixels;
	static void *fpRwImageFreePixels;
	static void *fpRwImageFindRasterFormat;
	static void *fpRwRasterCreate;
	static void *fpRwRasterSetFromImage;
	static void *fpRwTextureCreate;
	static void *fpRwIm2DRenderPrimitive;
};
