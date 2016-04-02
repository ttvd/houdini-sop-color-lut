#pragma once

#include <UT/UT_Array.h>
#include <UT/UT_Color.h>

struct SOP_ColorLUTDefaultPalette
{
    public:

        static void GetPalette(UT_Array<UT_Color>& palette);

    protected:

        static const unsigned int s_palette[256u];
};
