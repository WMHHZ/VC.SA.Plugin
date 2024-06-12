#include "CPlugin.h"
#include <game_sa/RenderWare.h>
#include <game_sa/CTxdStore.h>
#include "CFontPatch.h"
#include "CCharTable.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

char CPlugin::texturePath[MAX_PATH];
char CPlugin::textPath[MAX_PATH];

const wchar_t *CPlugin::MessageboxTitle = L"《侠盗猎车手：圣安地列斯》汉化补丁 Kokoro(2.2) Build160728 by 无名汉化组";

char aRb[] = "rb";
__declspec(naked) void Hook_LoadGxt()
{
	__asm
	{
		pop eax;
		inc eax;
		push offset aRb;
		push offset CPlugin::textPath;
		jmp eax;
	}
}

bool CPlugin::Init(HMODULE hPlugin)
{
	char pluginPath[MAX_PATH];

	GetModuleFileNameA(hPlugin, pluginPath, MAX_PATH);
	strcpy(texturePath, pluginPath);
	strcpy(textPath, pluginPath);
	strcpy(strrchr(texturePath, '.'), "\\wm_sachs.png");
	strcpy(strrchr(textPath, '.'), "\\wm_sachs.gxt");

	if (!PathFileExistsA(texturePath) || !PathFileExistsA(textPath))
	{
		MessageBoxW(nullptr, L"找不到资源文件，请确认是否带上了wm_sachs文件夹！", MessageboxTitle, MB_ICONWARNING);
		return false;
	}

    CFontPatch::Init10U();
	Patch10U();
	CCharTable::InitTable();

	return true;
}

void CPlugin::Patch10U()
{
	injector::MakeCALL(0x5BA841, LoadCHSTexture);
	injector::MakeCALL(0x7189FB, UnloadCHSTexture);

	injector::MakeCALL(0x69FD54, Hook_LoadGxt);
	injector::MakeCALL(0x6A0222, Hook_LoadGxt);
	injector::MemoryFill(0x8CFD6A, 0, 0x12, false);

	injector::MakeCALL(0x47B565, CFontPatch::GetStringWidth);
	injector::MakeCALL(0x47B73A, CFontPatch::GetStringWidth);
	injector::MakeCALL(0x57A49B, CFontPatch::GetStringWidth);
	injector::MakeCALL(0x57FB52, CFontPatch::GetStringWidth);
	injector::MakeCALL(0x57FE35, CFontPatch::GetStringWidth);
	injector::MakeCALL(0x5814A7, CFontPatch::GetStringWidth);
	injector::MakeCALL(0x581512, CFontPatch::GetStringWidth);
	injector::MakeCALL(0x58BCCC, CFontPatch::GetStringWidth);

	injector::MakeCALL(0x71A5F1, CFontPatch::ProcessCurrentString);
	injector::MakeCALL(0x71A611, CFontPatch::ProcessCurrentString);
	injector::MakeCALL(0x71A631, CFontPatch::ProcessCurrentString);
	injector::MakeCALL(0x71A802, CFontPatch::ProcessCurrentString);
	injector::MakeCALL(0x71A834, CFontPatch::ProcessCurrentString);

	injector::MakeCALL(0x57BF70, CFontPatch::RenderFontBuffer);
	injector::MakeCALL(0x719B5D, CFontPatch::RenderFontBuffer);
	injector::MakeCALL(0x719F43, CFontPatch::RenderFontBuffer);
	injector::MakeJMP(0x71A210, CFontPatch::RenderFontBuffer);
}

void CPlugin::LoadCHSTexture()
{
	int width, height, depth, flags;

	CTxdStore::PopCurrentTxd();
	
	RwImage *image = RtPNGImageRead(texturePath);
	RwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
	RwRaster *raster = RwRasterCreate(width, height, depth, flags);
	RwRasterSetFromImage(raster, image);
	RwImageDestroy(image);
	CFontPatch::m_ChsSprite.m_pTexture = RwTextureCreate(raster);
}

void CPlugin::UnloadCHSTexture(int dummy)
{
	CTxdStore::RemoveTxdSlot(dummy);
	CFontPatch::m_ChsSprite.Delete();
}
