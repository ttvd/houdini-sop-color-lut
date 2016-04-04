#include "SOP_ColorLUT.h"
#include "SOP_ColorLUTDefaultPalette.h"

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <GEO/GEO_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_SpareData.h>
#include <FS/FS_Info.h>
#include <CH/CH_Manager.h>
#include <IMG/IMG_File.h>
#include <PXL/PXL_Raster.h>

#define SOP_COLORLUT_MAKE_ID(A, B, C, D) ( A ) | ( B << 8 ) | ( C << 16 ) | ( D << 24 )

const unsigned int SOP_ColorLUT::s_vox_magic = SOP_COLORLUT_MAKE_ID('V', 'O', 'X', ' ');
const unsigned int SOP_ColorLUT::s_vox_main = SOP_COLORLUT_MAKE_ID('M', 'A', 'I', 'N');
const unsigned int SOP_ColorLUT::s_vox_size = SOP_COLORLUT_MAKE_ID('S', 'I', 'Z', 'E');
const unsigned int SOP_ColorLUT::s_vox_xyzi = SOP_COLORLUT_MAKE_ID('X', 'Y', 'Z', 'I');
const unsigned int SOP_ColorLUT::s_vox_rgba = SOP_COLORLUT_MAKE_ID('R', 'G', 'B', 'A');

const unsigned int SOP_ColorLUT::s_vox_version = 150u;
const unsigned int SOP_ColorLUT::s_vox_palette_size = 256u;

#define SOP_COLORLUT_USE_DEFAULT_PALETTE "lut_use_default_palette"
#define SOP_COLORLUT_CLASS "class"
#define SOP_COLORLUT_FILE "file"
#define SOP_COLORLUT_SAMPLING "lut_sampling"
#define SOP_COLORLUT_INPUT_ATTRIBUTE_NAME "lut_attributename"
#define SOP_COLORLUT_INPUT_ATTRIBUTE_NAME_DEFAULT "color_lut"
#define SOP_COLORLUT_DELETE_LUT_ATTRIBUTE "delete_lut_attribute"
#define SOP_COLORLUT_COLOR_ATTRIBUTE "Cd"

static PRM_Name s_name_use_default_palette(SOP_COLORLUT_USE_DEFAULT_PALETTE, "Use the Default 256 Color Palette");
static PRM_Name s_name_file(SOP_COLORLUT_FILE, "LUT File");
static PRM_Name s_name_class(SOP_COLORLUT_CLASS, "Class");
static PRM_Name s_name_sampling(SOP_COLORLUT_SAMPLING, "LUT Sampling");
static PRM_Name s_name_lut_attribute_name(SOP_COLORLUT_INPUT_ATTRIBUTE_NAME, "LUT Attribute Name");
static PRM_Name s_name_delete_lut_attribute(SOP_COLORLUT_DELETE_LUT_ATTRIBUTE, "Delete LUT Attribute");
static PRM_Name s_name_class_types[] =
{
    PRM_Name("point", "Point"),
    PRM_Name("vertex", "Vertex"),
    PRM_Name("primitive", "Primitive"),
    PRM_Name("detail", "Detail"),
    PRM_Name(0),
};
static PRM_Name s_name_sampling_types[] =
{
    PRM_Name("clamp", "Clamp"),
    PRM_Name("wrap", "Wrap"),
    PRM_Name(0),
};

static PRM_ChoiceList s_choicelist_class_type(PRM_CHOICELIST_SINGLE, s_name_class_types);
static PRM_ChoiceList s_choicelist_sampling_type(PRM_CHOICELIST_SINGLE, s_name_sampling_types);
static PRM_SpareData s_spare_file_picker(PRM_SpareArgs() << PRM_SpareToken(PRM_SpareData::getFileChooserModeToken(),
    PRM_SpareData::getFileChooserModeValRead()) << PRM_SpareToken(PRM_SpareData::getFileChooserPatternToken(),
    SOP_ColorLUT::fileExtensionFilterString()));

static PRM_Default s_default_use_default_palette(true);
static PRM_Default s_default_lut_attribute_name(0.0f, SOP_COLORLUT_INPUT_ATTRIBUTE_NAME_DEFAULT);
static PRM_Default s_default_delete_lut_attribute(false);


PRM_Template
SOP_ColorLUT::myTemplateList[] = {
    PRM_Template(PRM_TOGGLE, 1, &s_name_use_default_palette, &s_default_use_default_palette),
    PRM_Template(PRM_FILE, 1, &s_name_file, 0, 0, 0, 0, &s_spare_file_picker),
    PRM_Template(PRM_ORD, 1, &s_name_class, 0, &s_choicelist_class_type),
    PRM_Template(PRM_ORD, 1, &s_name_sampling, 0, &s_choicelist_sampling_type),
    PRM_Template(PRM_STRING, 1, &s_name_lut_attribute_name, &s_default_lut_attribute_name),
    PRM_Template(PRM_TOGGLE, 1, &s_name_delete_lut_attribute, &s_default_delete_lut_attribute),
    PRM_Template()
};


OP_Node*
SOP_ColorLUT::myConstructor(OP_Network* network, const char* name, OP_Operator* op)
{
    return new SOP_ColorLUT(network, name, op);
}


const char*
SOP_ColorLUT::fileExtensionFilterString()
{
    return "*.vox *.png";
}


SOP_ColorLUT::SOP_ColorLUT(OP_Network* network, const char* name, OP_Operator* op) :
    SOP_Node(network, name, op)
{

}


SOP_ColorLUT::~SOP_ColorLUT()
{

}


bool
SOP_ColorLUT::updateParmsFlags()
{
    bool changed = SOP_Node::updateParmsFlags();
    bool use_default_palette = useDefaultPalette(CHgetEvalTime());

    changed |= enableParm(SOP_COLORLUT_FILE, !use_default_palette);
    return changed;
}


OP_ERROR
SOP_ColorLUT::cookMySop(OP_Context& context)
{
    fpreal t = context.getTime();
    UT_Interrupt* boss = UTgetInterrupt();
    bool use_default_palette = useDefaultPalette(t);
    int sampling_type = getSamplingType(t);

    UT_String lut_file_name;
    UT_Array<UT_Vector3> lut_palette;

    if(lockInputs(context) >= UT_ERROR_ABORT)
    {
        return error();
    }

    duplicatePointSource(0, context);

    if(!use_default_palette)
    {
        evalString(lut_file_name, SOP_COLORLUT_FILE, 0, t);

        if(!lut_file_name || !lut_file_name.length())
        {
            UT_WorkBuffer buf;
            buf.sprintf("LUT file is not specified.");
            addError(SOP_MESSAGE, buf.buffer());

            unlockInputs();
            return error();
        }

        FS_Info file_info(lut_file_name);
        if(!file_info.fileExists())
        {
            UT_WorkBuffer buf;
            buf.sprintf("Specified LUT file does not exist.");
            addError(SOP_MESSAGE, buf.buffer());

            unlockInputs();
            return error();
        }
    }

    GA_AttributeOwner attribute_type = GA_ATTRIB_POINT;
    if(!getClassType(t, attribute_type))
    {
        UT_WorkBuffer buf;
        buf.sprintf("Unsupported attribute class type.");
        addError(SOP_MESSAGE, buf.buffer());

        unlockInputs();
        return error();
    }

    UT_String lut_input_attribute;
    evalString(lut_input_attribute, SOP_COLORLUT_INPUT_ATTRIBUTE_NAME, 0, t);
    if(!lut_input_attribute || !lut_input_attribute.length() || !lut_input_attribute.isValidVariableName())
    {
        UT_WorkBuffer buf;
        buf.sprintf("LUT input attribute is invalid.");
        addError(SOP_MESSAGE, buf.buffer());

        unlockInputs();
        return error();
    }

    GA_ROHandleI attr_input_int = GA_ROHandleI(gdp->findIntTuple(attribute_type, lut_input_attribute, 1));
    GA_ROHandleF attr_input_float = GA_ROHandleF(gdp->findFloatTuple(attribute_type, lut_input_attribute, 1));
    if(!attr_input_int.isValid() && !attr_input_float.isValid())
    {
        UT_WorkBuffer buf;
        buf.sprintf("LUT input int or float attribute not found on specified class.");
        addError(SOP_MESSAGE, buf.buffer());

        unlockInputs();
        return error();
    }

    if(use_default_palette)
    {
        getDefaultPalette(lut_palette);
    }
    else
    {
        FS_Info file_info(lut_file_name);
        UT_String lut_file_extension = file_info.getExtension();
        if(lut_file_extension == ".vox")
        {
            if(!getPaletteVox(lut_file_name, lut_palette))
            {
                lut_palette.clear();
                getDefaultPalette(lut_palette);

                UT_WorkBuffer buf;
                buf.sprintf("Error getting palette from %s LUT file. Using default instead.", (const char*) lut_file_name);
                addWarning(SOP_MESSAGE, buf.buffer());
            }

            if(!lut_palette.size())
            {
                getDefaultPalette(lut_palette);

                UT_WorkBuffer buf;
                buf.sprintf("Empty palette found in %s LUT file. Using default instead.", (const char*) lut_file_name);
                addWarning(SOP_MESSAGE, buf.buffer());
            }
        }
        else if(lut_file_extension == ".png")
        {
            if(!getPalettePng(lut_file_name, lut_palette))
            {
                UT_WorkBuffer buf;
                buf.sprintf("Error processing %s LUT file.", (const char*) lut_file_name);
                addError(SOP_MESSAGE, buf.buffer());

                unlockInputs();
                return error();
            }
        }
        else
        {
            UT_WorkBuffer buf;
            buf.sprintf("Unsupported type of LUT file.");
            addError(SOP_MESSAGE, buf.buffer());

            unlockInputs();
            return error();
        }
    }

    if(!lut_palette.size())
    {
        UT_WorkBuffer buf;
        buf.sprintf("Empty palette found, invalid.");
        addError(SOP_MESSAGE, buf.buffer());

        unlockInputs();
        return error();
    }

    GA_RWHandleV3 attr_color = GA_RWHandleV3(gdp->findFloatTuple(attribute_type, SOP_COLORLUT_COLOR_ATTRIBUTE, 3));
    if(!attr_color.isValid())
    {
        attr_color.bind(gdp->addFloatTuple(attribute_type, SOP_COLORLUT_COLOR_ATTRIBUTE, 3));

        if(!attr_color.isValid())
        {
            UT_WorkBuffer buf;
            buf.sprintf("Failed creating a color %s attribute.", SOP_COLORLUT_COLOR_ATTRIBUTE);
            addError(SOP_MESSAGE, buf.buffer());

            unlockInputs();
            return error();
        }
    }

    bool create_result = true;
    switch(attribute_type)
    {
        case GA_ATTRIB_POINT:
        {
            GA_Offset point_offset = 0;
            GA_FOR_ALL_PTOFF(gdp, point_offset)
            {
                if(boss->opInterrupt())
                {
                    break;
                }

                int lut_value = getAttributeValue(attr_input_int, attr_input_float, point_offset);
                attr_color.set(point_offset, lookupPaletteColor(sampling_type, lut_palette, lut_value));
            }

            attr_color.bumpDataId();
            break;
        }

        case GA_ATTRIB_VERTEX:
        {
            GEO_Primitive* prim = nullptr;
            GA_Offset vertex_offset = 0;
            GA_Size vertex_count = 0;

            GA_FOR_ALL_PRIMITIVES(gdp, prim)
            {
                if(boss->opInterrupt())
                {
                    break;
                }

                vertex_count = prim->getVertexCount();
                for(int vtx_idx = 0; vtx_idx < vertex_count; ++vtx_idx)
                {
                    vertex_offset = prim->getVertexOffset(vtx_idx);
                    int lut_value = getAttributeValue(attr_input_int, attr_input_float, vertex_offset);
                    attr_color.set(vertex_offset, lookupPaletteColor(sampling_type, lut_palette, lut_value));
                }
            }

            attr_color.bumpDataId();
            break;
        }

        case GA_ATTRIB_PRIMITIVE:
        {
            GEO_Primitive* prim = nullptr;
            GA_Offset prim_offset = 0;

            GA_FOR_ALL_PRIMITIVES(gdp, prim)
            {
                if(boss->opInterrupt())
                {
                    break;
                }

                prim_offset = prim->getMapOffset();

                int lut_value = getAttributeValue(attr_input_int, attr_input_float, prim_offset);
                attr_color.set(prim_offset, lookupPaletteColor(sampling_type, lut_palette, lut_value));
            }

            attr_color.bumpDataId();
            break;
        }

        case GA_ATTRIB_DETAIL:
        {
            int lut_value = getAttributeValue(attr_input_int, attr_input_float, GA_Offset(0));
            attr_color.set(GA_Offset(0), lookupPaletteColor(sampling_type, lut_palette, lut_value));

            attr_color.bumpDataId();
            break;
        }

        default:
        {
            UT_WorkBuffer buf;
            buf.sprintf("Unsupported attribute class type.");
            addError(SOP_MESSAGE, buf.buffer());

            unlockInputs();
            return error();
        }
    }

    if(deleteOriginalLookUpAttribute(t))
    {
        gdp->destroyAttribute(attribute_type, lut_input_attribute);
    }

    unlockInputs();
    return error();
}


const char*
SOP_ColorLUT::inputLabel(unsigned int idx) const
{
    return "Color LUT";
}


bool
SOP_ColorLUT::useDefaultPalette(fpreal t) const
{
    return evalInt(SOP_COLORLUT_USE_DEFAULT_PALETTE, 0, t);
}


bool
SOP_ColorLUT::deleteOriginalLookUpAttribute(fpreal t) const
{
    return evalInt(SOP_COLORLUT_DELETE_LUT_ATTRIBUTE, 0, t);
}


int
SOP_ColorLUT::getSamplingType(fpreal t) const
{
    return evalInt(SOP_COLORLUT_SAMPLING, 0, t);
}


bool
SOP_ColorLUT::getClassType(fpreal t, GA_AttributeOwner& attrib_owner) const
{
    int class_type = evalInt(SOP_COLORLUT_CLASS, 0, t);

    switch(class_type)
    {
        case 0:
        {
            attrib_owner = GA_ATTRIB_POINT;
            return true;
        }

        case 1:
        {
            attrib_owner = GA_ATTRIB_VERTEX;
            return true;
        }

        case 2:
        {
            attrib_owner = GA_ATTRIB_PRIMITIVE;
            return true;
        }

        case 3:
        {
            attrib_owner = GA_ATTRIB_DETAIL;
            return true;
        }

        default:
        {
            break;
        }
    }

    return false;
}


bool
SOP_ColorLUT::getDefaultPalette(UT_Array<UT_Vector3>& palette) const
{
    SOP_ColorLUTDefaultPalette::GetPalette(palette);
    return true;
}


bool
SOP_ColorLUT::getPaletteVox(const char* file_vox, UT_Array<UT_Vector3>& palette) const
{
    palette.clear();
    UT_IFStream stream(file_vox, UT_ISTREAM_BINARY);

    if(!stream.isError())
    {
        if(!getPaletteVoxMagic(stream))
        {
            stream.close();
            return false;
        }

        if(!getPaletteVoxVersion(stream))
        {
            stream.close();
            return false;
        }

        unsigned int main_chunk_id, main_contents_size, main_children_chunk_size;
        if(!getPaletteVoxChunk(stream, main_chunk_id, main_contents_size, main_children_chunk_size))
        {
            stream.close();
            return false;
        }

        if(SOP_ColorLUT::s_vox_main != main_chunk_id)
        {
            stream.close();
            return false;
        }

        if(!stream.seekg(main_contents_size, UT_IStream::UT_SEEK_CUR))
        {
            stream.close();
            return false;
        }

        while(!stream.isEof())
        {
            unsigned int chunk_id, chunk_contents_size, children_chunk_size;
            if(!getPaletteVoxChunk(stream, chunk_id, chunk_contents_size, children_chunk_size))
            {
                break;
            }

            if(SOP_ColorLUT::s_vox_rgba != chunk_id)
            {
                if(!stream.seekg(chunk_contents_size, UT_IStream::UT_SEEK_CUR))
                {
                    stream.close();
                    return false;
                }
            }
            else
            {
                for(unsigned int color_idx = 0u; color_idx < 256u; ++color_idx)
                {
                    unsigned char r, g, b, a;
                    if(!getPaletteVoxColor(stream, r, g, b, a))
                    {
                        stream.close();
                        return false;
                    }

                    UT_Vector3 palette_color(SYSclamp(r, 0, 255) / 255.0f, SYSclamp(g, 0, 255) / 255.0f,
                        SYSclamp(b, 0, 255) / 255.0f);
                    palette.append(palette_color);
                }
            }
        }
    }
    else
    {
        return false;
    }

    stream.close();
    return true;
}

bool
SOP_ColorLUT::getPaletteVoxMagic(UT_IFStream& stream) const
{
    unsigned int vox_magic_number = 0u;
    if(stream.bread(&vox_magic_number) != 1)
    {
        return false;
    }

    UTswap_int32(vox_magic_number, vox_magic_number);

    if(SOP_ColorLUT::s_vox_magic != vox_magic_number)
    {
        return false;
    }

    return true;
}


bool
SOP_ColorLUT::getPaletteVoxVersion(UT_IFStream& stream) const
{
    unsigned int vox_version = 0;
    if(stream.bread(&vox_version) != 1)
    {
        return false;
    }

    UTswap_int32(vox_version, vox_version);
    if(SOP_ColorLUT::s_vox_version != vox_version)
    {
        return false;
    }

    return true;
}


bool
SOP_ColorLUT::getPaletteVoxChunk(UT_IFStream& stream, unsigned int& chunk_id, unsigned int& content_size,
    unsigned int& children_chunk_size) const
{
    if(stream.bread(&chunk_id) != 1)
    {
        return false;
    }

    UTswap_int32(chunk_id, chunk_id);

    if(stream.bread(&content_size) != 1)
    {
        return false;
    }

    UTswap_int32(content_size, content_size);

    if(stream.bread(&children_chunk_size) != 1)
    {
        return false;
    }

    UTswap_int32(children_chunk_size, children_chunk_size);

    return true;
}


bool
SOP_ColorLUT::getPaletteVoxColor(UT_IFStream& stream, unsigned char& r, unsigned char& g, unsigned char& b,
    unsigned char& a) const
{
    if(stream.bread(&r) != 1)
    {
        return false;
    }

    if(stream.bread(&g) != 1)
    {
        return false;
    }

    if(stream.bread(&b) != 1)
    {
        return false;
    }

    if(stream.bread(&a) != 1)
    {
        return false;
    }

    return true;
}


bool
SOP_ColorLUT::getPalettePng(const char* file_png, UT_Array<UT_Vector3>& palette) const
{
    palette.clear();
    bool png_result = true;

    IMG_FileParms file_params;
    file_params.setDataType(IMG_FLOAT32);
    file_params.orientImage(IMG_ORIENT_LEFT_FIRST, IMG_ORIENT_TOP_FIRST);
    file_params.setColorModel(IMG_RGB);
    file_params.setInterleaved(IMG_INTERLEAVED);

    IMG_File* file = IMG_File::open(file_png, &file_params);
    if(file)
    {
        UT_ValArray<PXL_Raster*> images;
        file->readImages(images);
        if(images.entries() > 0 && images(0) && images(0)->isValid())
        {
            PXL_Raster* raster = images(0);
            int raster_width = raster->getXres();
            int raster_height = raster->getYres();
            if(raster_width > 0 && raster_height > 0)
            {
                palette.append(UT_Vector3(0.0f, 0.0f, 0.0f));

                const float* pixels = static_cast<const float*>(raster->getPixels());
                for(int pix_idx = 0; pix_idx < raster_height * raster_width; ++pix_idx)
                {
                    UT_Vector3 pixel_color(pixels[pix_idx * 3 + 0], pixels[pix_idx * 3 + 1], pixels[pix_idx * 3 + 2]);
                    palette.append(pixel_color);
                }
            }
            else
            {
                png_result = false;
            }

            delete raster;
        }
        else
        {
            png_result = false;
        }

        delete file;
    }
    else
    {
        png_result = false;
    }

    return png_result;
}


int
SOP_ColorLUT::getAttributeValue(const GA_ROHandleI& attr_input_int, const GA_ROHandleF& attr_input_float, GA_Offset offset)
{
    int lut_value = 0;

    if(attr_input_int.isValid())
    {
        lut_value = attr_input_int.get(offset);
    }
    else if(attr_input_float.isValid())
    {
        lut_value = (int) attr_input_float.get(offset);
    }

    return lut_value;
}


UT_Vector3
SOP_ColorLUT::lookupPaletteColor(int sampling_type, const UT_Array<UT_Vector3>& lut_palette, int lut_value) const
{
    int lut_value_final = 0;

    switch(sampling_type)
    {
        default:
        case 0:
        {
            lut_value_final = SYSclamp(lut_value, 0, (int) lut_palette.size() - 1);
            break;
        }

        case 1:
        {
            lut_value_final = lut_value % lut_palette.size();
            break;
        }
    }

    return lut_palette(lut_value_final);
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator("colorlut", "Color LUT", SOP_ColorLUT::myConstructor,
        SOP_ColorLUT::myTemplateList, 1, 1, 0));
}
