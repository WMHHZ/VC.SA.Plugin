#pragma once
#include "game.h"

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
