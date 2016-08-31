#pragma once
#include "../deps/func_wrapper/func_wrapper.hpp"
#include <rwcore.h>

class rwFunc
{
public:
	static __int32 *RsGlobalW;
	static __int32 *RsGlobalH;

	static cdecl_func_wrapper<RwBool(RwRenderState state, void *value)>
		fpRwRenderStateSet;

	static cdecl_func_wrapper<RwImage *(RwInt32 width, RwInt32 height, RwInt32 depth)>
		fpRwImageCreate;

	static cdecl_func_wrapper<RwBool(RwImage *image)>
		fpRwImageDestroy;

	static cdecl_func_wrapper<RwImage *(RwImage *image)>
		fpRwImageAllocatePixels;

	static cdecl_func_wrapper<RwImage *(RwImage *ipImage, RwRasterType nRasterType, RwInt32 *npWidth, RwInt32 *npHeight, RwInt32 *npDepth, RwInt32 *npFormat)>
		fpRwImageFindRasterFormat;

	static cdecl_func_wrapper<RwRaster *(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags)>
		fpRwRasterCreate;

	static cdecl_func_wrapper<RwRaster *(RwRaster *raster, RwImage *image)>
		fpRwRasterSetFromImage;

	static cdecl_func_wrapper<RwTexture *(RwRaster *raster)>
		fpRwTextureCreate;

	rwFunc();
};
