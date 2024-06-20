#pragma once

class CCharTable
{
  public:
    struct CharPos
    {
        unsigned char rowIndex;
        unsigned char columnIndex;
    };

    static CharPos GetCharPos(unsigned int chr);

    static void InitTable();

  private:
    static CharPos m_Table[];
};
