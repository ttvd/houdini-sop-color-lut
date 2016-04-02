#pragma once

#include <SOP/SOP_API.h>
#include <SOP/SOP_Node.h>

class GEO_PrimVolume;

class SOP_API SOP_ColorLUT : public SOP_Node
{
    public:

        static OP_Node* myConstructor(OP_Network* network, const char* name, OP_Operator* op);
        static PRM_Template myTemplateList[];

        static const char* fileExtensionFilterString();

    protected:

        SOP_ColorLUT(OP_Network* network, const char* name, OP_Operator* op);
        virtual ~SOP_ColorLUT();

    protected:

        virtual const char* inputLabel(unsigned int idx) const;
        virtual OP_ERROR cookMySop(OP_Context& context);

    protected:

        bool getPaletteVox(const char* file_vox, UT_Array<UT_Color>& palette) const;

    protected:

        bool getClassType(fpreal t, GA_AttributeOwner& attrib_owner) const;
};
