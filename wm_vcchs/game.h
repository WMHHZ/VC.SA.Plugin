#pragma once

class CVector2D
{
  public:
    float x, y;
};

class CRGBA
{
  public:
    unsigned char red, green, blue, alpha;
};

class CRect
{
  public:
    float x1, y1, x2, y2;

    CRect()
    {
        x1 = 1000000.0f;
        y1 = -1000000.0f;
        x2 = -1000000.0f;
        y2 = 1000000.0f;
    }
};
