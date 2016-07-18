#pragma once
#include <cstdint>

class CRGBA
{
public:
	uint8_t red, green, blue, alpha;

	CRGBA() = default;
	CRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		red = r;
		green = g;
		blue = b;
		alpha = a;
	}
};

class CVector2D
{
public:
	float x, y;
};

class CRect
{
public:
	float x1;
	float y1;
	float x2;
	float y2;

	CRect()
	{
		x1 = 1000000.0f;
		y1 = -1000000.0f;
		x2 = -1000000.0f;
		y2 = 1000000.0f;
	}
};


