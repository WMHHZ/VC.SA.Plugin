#pragma once

#include "game.h"

class CScriptTextDrawer
{
public:
	static CScriptTextDrawer *m_TextDrawers;
	static unsigned __int16 *m_CurrentTextDrawerIndex;

	CVector2D		m_LetterScale;
	CRGBA			m_Color;
	__int8			field_C;
	bool			m_bCentered;
	bool			m_bWithBackGround;
	__int8			pad_F;
	CVector2D		m_LineScale;
	CRGBA			m_BackgroundBoxColor;
	bool			m_bIsProportional;
	CRGBA			m_BackgroundColor;
	unsigned __int8 m_ShadowSize;
	__int8			m_OutlineSize;
	__int8			field_23;
	bool			m_bIsAlignRight;
	__int8			pad_25;
	__int8			pad_26;
	__int8			pad_27;
	unsigned __int32 m_nFontStyle;
	CVector2D		m_Position;
	char			m_aGxtEntry[8];
	__int32			param1;
	__int32			param2;

	static void Init10U();
	static void InitSteam();
};
static_assert(sizeof(CScriptTextDrawer) == 0x44, "Class CScriptTextDrawer is wrong.");
