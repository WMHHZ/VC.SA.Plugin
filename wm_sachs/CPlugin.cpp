#include "CPlugin.h"
#include "rwFunc.h"
#include "CFont.h"
#include "CTxdStore.h"
#include "CSprite.h"
#include "CSprite2d.h"
#include "CScriptTextDrawer.h"
#include "CCharTable.h"
#include "../include/injector/hooking.hpp"

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

bool CPlugin::CheckResourceFile(HMODULE hPlugin)
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

	return true;
}

bool CPlugin::CheckGameVersion()
{
	injector::address_manager &veref = injector::address_manager::singleton();

	if (veref.IsSA())
	{	
		if (veref.IsSteam())
		{
			rwFunc::InitSteam();
			CSprite::InitSteam();
			CSprite2d::InitSteam();
			CTxdStore::InitSteam();
			CScriptTextDrawer::InitSteam();
			CFont::InitSteam();

			PatchSteam();
		}
		else
		{
			rwFunc::Init10U();
			CSprite::Init10U();
			CSprite2d::Init10U();
			CTxdStore::Init10U();
			CScriptTextDrawer::Init10U();
			CFont::Init10U();

			Patch10U();
		}
		
		CCharTable::InitTable();
	}
	else
	{
		MessageBoxW(nullptr, L"你正在使用的游戏版本不被支持！请确保你的游戏主程序为以下之一：\n1.0美版：14383616字节\n1.0美版：5189632字节\nSteam版：5971456字节", MessageboxTitle, MB_ICONWARNING);
		return false;
	}

	return true;
}

void CPlugin::Patch10U()
{
	injector::MakeCALL(0x5BA841, LoadCHSTexture);
	injector::MakeCALL(0x7189FB, UnloadCHSTexture);

	injector::MakeCALL(0x69FD54, Hook_LoadGxt);
	injector::MakeCALL(0x6A0222, Hook_LoadGxt);
	injector::MemoryFill(0x8CFD6A, 0, 0x12, false);

	injector::MakeCALL(0x47B565, CFont::GetStringWidth);
	injector::MakeCALL(0x47B73A, CFont::GetStringWidth);
	injector::MakeCALL(0x57A49B, CFont::GetStringWidth);
	injector::MakeCALL(0x57FB52, CFont::GetStringWidth);
	injector::MakeCALL(0x57FE35, CFont::GetStringWidth);
	injector::MakeCALL(0x5814A7, CFont::GetStringWidth);
	injector::MakeCALL(0x581512, CFont::GetStringWidth);
	injector::MakeCALL(0x58BCCC, CFont::GetStringWidth);

	injector::MakeCALL(0x71A5F1, CFont::ProcessCurrentString);
	injector::MakeCALL(0x71A611, CFont::ProcessCurrentString);
	injector::MakeCALL(0x71A631, CFont::ProcessCurrentString);
	injector::MakeCALL(0x71A802, CFont::ProcessCurrentString);
	injector::MakeCALL(0x71A834, CFont::ProcessCurrentString);

	injector::MakeCALL(0x57BF70, CFont::RenderFontBuffer);
	injector::MakeCALL(0x719B5D, CFont::RenderFontBuffer);
	injector::MakeCALL(0x719F43, CFont::RenderFontBuffer);
	injector::MakeJMP(0x71A210, CFont::RenderFontBuffer);
}

void CPlugin::PatchSteam()
{
	injector::MakeCALL(injector::aslr_ptr(0x5D7CC6).get(), LoadCHSTexture);
	injector::MakeCALL(injector::aslr_ptr(0x736148).get(), UnloadCHSTexture);
	
	injector::MakeCALL(injector::aslr_ptr(0x6CD6F3).get(), Hook_LoadGxt);
	injector::MakeCALL(injector::aslr_ptr(0x6CDC1B).get(), Hook_LoadGxt);
	injector::MemoryFill(injector::aslr_ptr(0x94278A).get(), 0, 0x12, false);
	
	injector::MakeNOP(injector::aslr_ptr(0x58D7E4).get(), 5);

	injector::MakeJMP(injector::aslr_ptr(0x737150).get(), CFont::RenderFontBuffer);
	injector::MakeJMP(injector::aslr_ptr(0x737A70).get(), CFont::GetStringWidth);
	injector::MakeJMP(injector::aslr_ptr(0x737B90).get(), CFont::ProcessCurrentString);
}

void CPlugin::LoadCHSTexture()
{
	int width, height, depth, flags;

	CTxdStore::PopCurrentTxd();
	
	RwImage *image = rwFunc::RtPNGImageRead(texturePath);
	rwFunc::RwImageFindRasterFormat(image, 4, &width, &height, &depth, &flags);
	RwRaster *raster = rwFunc::RwRasterCreate(width, height, depth, flags);
	rwFunc::RwRasterSetFromImage(raster, image);
	rwFunc::RwImageDestroy(image);
	CFont::m_ChsSprite.m_pRwTexture = rwFunc::RwTextureCreate(raster);
}

void CPlugin::UnloadCHSTexture(int dummy)
{
	CTxdStore::RemoveTxdSlot(dummy);
	CFont::m_ChsSprite.Delete();
}
