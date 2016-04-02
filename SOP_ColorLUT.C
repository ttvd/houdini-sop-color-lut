#include "SOP_ColorLUT.h"

#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <PRM/PRM_Include.h>
#include <GEO/GEO_Detail.h>
#include <GEO/GEO_PrimPoly.h>
#include <GU/GU_Detail.h>
#include <PRM/PRM_SpareData.h>

#define SOP_COLORLUT_CLASS "class"
#define SOP_COLORLUT_FILE "file"

static PRM_Name s_name_file(SOP_COLORLUT_FILE, "LUT File");
static PRM_Name s_name_class(SOP_COLORLUT_CLASS, "Class");
static PRM_Name s_name_class_types[] =
{
    PRM_Name("point", "Point"),
    PRM_Name("vertex", "Vertex"),
    PRM_Name("primitive", "Primitive"),
    PRM_Name("detail", "Detail"),
    PRM_Name(0),
};

static PRM_ChoiceList s_choicelist_class_type(PRM_CHOICELIST_SINGLE, s_name_class_types);


PRM_Template
SOP_ColorLUT::myTemplateList[] = {
    PRM_Template(PRM_FILE, 1, &s_name_file, 0, 0, 0, 0, &PRM_SpareData::fileChooserModeRead),
    PRM_Template(PRM_ORD, 1, &s_name_class, 0, &s_choicelist_class_type),
    PRM_Template()
};


OP_Node*
SOP_ColorLUT::myConstructor(OP_Network* network, const char* name, OP_Operator* op)
{
    return new SOP_ColorLUT(network, name, op);
}


SOP_ColorLUT::SOP_ColorLUT(OP_Network* network, const char* name, OP_Operator* op) :
    SOP_Node(network, name, op)
{

}


SOP_ColorLUT::~SOP_ColorLUT()
{

}


OP_ERROR
SOP_ColorLUT::cookMySop(OP_Context& context)
{
    fpreal t = context.getTime();
    UT_Interrupt* boss = UTgetInterrupt();

    if(lockInputs(context) >= UT_ERROR_ABORT)
    {
        return error();
    }

    duplicatePointSource(0, context);

    /*
    const GU_Detail* input_gdp = inputGeo(0);
    const GA_PrimitiveGroup* prim_group = nullptr;
    GEO_Primitive* prim = nullptr;

    GA_FOR_ALL_OPT_GROUP_PRIMITIVES(input_gdp, prim_group, prim)
    {
        if(boss->opInterrupt())
        {
            break;
        }

        if(prim->getTypeId() == GEO_PRIMVOLUME)
        {
            GEO_PrimVolume* volume = (GEO_PrimVolume *) prim;
            volumes.append(volume);
        }
    }
    */

    unlockInputs();
    return error();
}


const char*
SOP_ColorLUT::inputLabel(unsigned int idx) const
{
    return "Color LUT";
}


void
newSopOperator(OP_OperatorTable* table)
{
    table->addOperator(new OP_Operator("colorlut", "Color LUT", SOP_ColorLUT::myConstructor,
        SOP_ColorLUT::myTemplateList, 1, 1, 0));
}
