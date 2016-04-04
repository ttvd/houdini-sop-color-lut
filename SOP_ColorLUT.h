#pragma once

#include <SOP/SOP_API.h>
#include <SOP/SOP_Node.h>

class SOP_API SOP_ColorLUT : public SOP_Node
{
    public:

        static OP_Node* myConstructor(OP_Network* network, const char* name, OP_Operator* op);
        static PRM_Template myTemplateList[];

        static const char* fileExtensionFilterString();

    public:

        virtual bool updateParmsFlags();

    protected:

        SOP_ColorLUT(OP_Network* network, const char* name, OP_Operator* op);
        virtual ~SOP_ColorLUT();

    protected:

        virtual const char* inputLabel(unsigned int idx) const;
        virtual OP_ERROR cookMySop(OP_Context& context);

    protected:

        bool getDefaultPalette(UT_Array<UT_Vector3>& palette) const;

    protected:

        bool getPaletteVox(const char* file_vox, UT_Array<UT_Vector3>& palette) const;
        bool getPaletteVoxMagic(UT_IFStream& stream) const;
        bool getPaletteVoxVersion(UT_IFStream& stream) const;
        bool getPaletteVoxChunk(UT_IFStream& stream, unsigned int& chunk_id, unsigned int& content_size,
            unsigned int& children_chunk_size) const;
        bool getPaletteVoxColor(UT_IFStream& stream, unsigned char& r, unsigned char& g, unsigned char& b,
            unsigned char& a) const;

        //! Magic numbers used by the parser.
        static const unsigned int s_vox_magic;
        static const unsigned int s_vox_main;
        static const unsigned int s_vox_size;
        static const unsigned int s_vox_xyzi;
        static const unsigned int s_vox_rgba;
        static const unsigned int s_vox_version;
        static const unsigned int s_vox_palette_size;

    protected:

        bool getPalettePng(const char* file_png, UT_Array<UT_Vector3>& palette) const;

    protected:

        int getAttributeValue(const GA_ROHandleI& attr_input_int, const GA_ROHandleF& attr_input_float, GA_Offset offset);
        UT_Vector3 lookupPaletteColor(int sampling_type, const UT_Array<UT_Vector3>& lut_palette, int lut_value) const;

    protected:

        bool getClassType(fpreal t, GA_AttributeOwner& attrib_owner) const;
        bool useDefaultPalette(fpreal t) const;
        bool deleteOriginalLookUpAttribute(fpreal t) const;
        int getSamplingType(fpreal t) const;
};
