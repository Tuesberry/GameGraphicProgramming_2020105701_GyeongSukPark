#include "Game/Game.h"

namespace library
{
    /*--------------------------------------------------------------------
      Function definitions dddd
    --------------------------------------------------------------------*/
    void PrintHi()
    {
        OutputDebugString(L"hi\n");
        MessageBox(nullptr, L"hi", L"Game Graphics Programming", MB_OK);
    }
}