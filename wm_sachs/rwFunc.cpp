#include "rwFunc.h"
#include "../include/injector/injector.hpp"

RwGlobals **rwFunc::m_pRwEngineInstance;
RsGlobalType *rwFunc::m_RsGlobal;

RwImage *(__cdecl *rwFunc::RtPNGImageRead)(const char *filename);
int(__cdecl *rwFunc::RwImageDestroy)(RwImage *);
RwImage *(__cdecl *rwFunc::RwImageFindRasterFormat)(RwImage *, int, int *, int *, int *, int *);
RwRaster *(__cdecl *rwFunc::RwRasterCreate)(int, int, int, int);
RwRaster *(__cdecl *rwFunc::RwRasterSetFromImage)(RwRaster *, RwImage *);
RwTexture *(__cdecl *rwFunc::RwTextureCreate)(RwRaster *);

void rwFunc::Init10U()
{
	m_pRwEngineInstance = injector::raw_ptr(0xC97B24).get();
	m_RsGlobal = injector::raw_ptr(0xC17040).get();

	RtPNGImageRead = injector::raw_ptr(0x7CF9B0).get();
	RwImageDestroy = injector::raw_ptr(0x802740).get();
	RwImageFindRasterFormat = injector::raw_ptr(0x8042C0).get();
	RwRasterCreate = injector::raw_ptr(0x7FB230).get();
	RwRasterSetFromImage = injector::raw_ptr(0x804290).get();
	RwTextureCreate = injector::raw_ptr(0x7F37C0).get();
}

void rwFunc::InitSteam()
{
	m_pRwEngineInstance = injector::aslr_ptr(0xD23664).get();
	m_RsGlobal = injector::aslr_ptr(0xCA3DAC).get();

	RtPNGImageRead = injector::aslr_ptr(0x8041C0).get();
	RwImageDestroy = injector::aslr_ptr(0x836F90).get();
	RwImageFindRasterFormat = injector::aslr_ptr(0x838B10).get();
	RwRasterCreate = injector::aslr_ptr(0x82FA80).get();
	RwRasterSetFromImage = injector::aslr_ptr(0x838AE0).get();
	RwTextureCreate = injector::aslr_ptr(0x828000).get();
}
