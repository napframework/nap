#include "rttinap.h"
#include "stringutils.h"
#include "coreattributes.h"

namespace nap
{

	RTTI::TypeInfo getAttributeTypeFromValueType(const RTTI::TypeInfo& valueType)
	{
		std::string baseTypename = RTTI::TypeInfo::get<Attribute<int>>().getName();
		std::string resultingTypename = replaceTemplateType(baseTypename, valueType.getName());
		return RTTI::TypeInfo::getByName(resultingTypename.c_str());
	}


	RTTI::TypeInfo getOutpullPlugFromValueType(const RTTI::TypeInfo& valueType)
	{
		std::string baseTypename = RTTI::TypeInfo::get<OutputPullPlug<int>>().getName();
		std::string resultingTypename = replaceTemplateType(baseTypename, valueType.getName());
		return RTTI::TypeInfo::getByName(resultingTypename.c_str());
	}

    
    RTTI::TypeInfo getInputPullPlugFromValueType(const RTTI::TypeInfo& valueType)
    {
        std::string baseTypename = RTTI::TypeInfo::get<InputPullPlug<int>>().getName();
        std::string resultingTypename = replaceTemplateType(baseTypename, valueType.getName());
        return RTTI::TypeInfo::getByName(resultingTypename.c_str());
        
    }
    
}