#include "rwFunc.h"
#include "../include/selector/AddressSelector.h"

__int32 *rwFunc::RsGlobalW;
__int32 *rwFunc::RsGlobalH;

cdecl_func_wrapper<__int32(RwRenderState state, void *value)>
rwFunc::fpRwRenderStateSet;

cdecl_func_wrapper<RwImage *(__int32 width, __int32 height, __int32 depth)>
rwFunc::fpRwImageCreate;

cdecl_func_wrapper<RwBool(RwImage *image)>
rwFunc::fpRwImageDestroy;

cdecl_func_wrapper<RwImage *(RwImage *image)>
rwFunc::fpRwImageAllocatePixels;

cdecl_func_wrapper<RwImage *(RwImage *ipImage, RwInt32 nRasterType, RwInt32 *npWidth, RwInt32 *npHeight, RwInt32 *npDepth, RwInt32 *npFormat)>
rwFunc::fpRwImageFindRasterFormat;

cdecl_func_wrapper<RwRaster *(RwInt32 width, RwInt32 height, RwInt32 depth, RwInt32 flags)>
rwFunc::fpRwRasterCreate;

cdecl_func_wrapper<RwRaster *(RwRaster *raster, RwImage *image)>
rwFunc::fpRwRasterSetFromImage;

cdecl_func_wrapper<RwTexture *(RwRaster *raster)>
rwFunc::fpRwTextureCreate;

rwFunc::rwFunc()
{
	RsGlobalW = AddressSelectorVC::SelectAddress<0x9B48E4, 0x0, 0x9B38EC, __int32>();
	RsGlobalH = RsGlobalW + 1;

	fpRwRenderStateSet = AddressSelectorVC::SelectAddress<0x649BA0, 0x0, 0x648B50>();
	fpRwImageCreate = AddressSelectorVC::SelectAddress<0x651250, 0x0, 0x0>();
	fpRwImageDestroy = AddressSelectorVC::SelectAddress<0x6512B0, 0x0, 0x0>();
	fpRwImageAllocatePixels = AddressSelectorVC::SelectAddress<0x651310, 0x0, 0x0>();
	fpRwImageFindRasterFormat = AddressSelectorVC::SelectAddress<0x6602E0, 0x0, 0x0>();
	fpRwRasterCreate = AddressSelectorVC::SelectAddress<0x655490, 0x0, 0x0>();
	fpRwRasterSetFromImage = AddressSelectorVC::SelectAddress<0x6602B0, 0x0, 0x0>();
	fpRwTextureCreate = AddressSelectorVC::SelectAddress<0x64DE60, 0x0, 0x0>();
}

static rwFunc instance;
