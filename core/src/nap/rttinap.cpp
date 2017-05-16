#include "rttinap.h"
#include "stringutils.h"
#include "coreattributes.h"

namespace nap
{

	RTTI::TypeInfo getAttributeTypeFromValueType(const RTTI::TypeInfo& valueType)
	{
		std::string baseTypename = RTTI::TypeInfo::get<Attribute<int>>().get_name().data();
		std::string resultingTypename = replaceTemplateType(baseTypename, valueType.get_name().data());
		return RTTI::TypeInfo::get_by_name(resultingTypename.c_str());
	}


	RTTI::TypeInfo getOutpullPlugFromValueType(const RTTI::TypeInfo& valueType)
	{
		std::string baseTypename = RTTI::TypeInfo::get<OutputPullPlug<int>>().get_name().data();
		std::string resultingTypename = replaceTemplateType(baseTypename, valueType.get_name().data());
		return RTTI::TypeInfo::get_by_name(resultingTypename.c_str());
	}

    
    RTTI::TypeInfo getInputPullPlugFromValueType(const RTTI::TypeInfo& valueType)
    {
        std::string baseTypename = RTTI::TypeInfo::get<InputPullPlug<int>>().get_name().data();
        std::string resultingTypename = replaceTemplateType(baseTypename, valueType.get_name().data());
        return RTTI::TypeInfo::get_by_name(resultingTypename.c_str());
        
    }
    
}