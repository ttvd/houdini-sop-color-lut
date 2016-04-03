#pragma once

#include <UT/UT_Array.h>
#include <UT/UT_Vector3.h>

struct SOP_ColorLUTDefaultPalette
{
    public:

        static void GetPalette(UT_Array<UT_Vector3>& palette);

    protected:

        static const unsigned int s_palette[256u];
};
