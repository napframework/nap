#include "rttinap.h"
#include "utility/stringutils.h"
#include "coreattributes.h"

namespace nap
{

	rtti::TypeInfo getAttributeTypeFromValueType(const rtti::TypeInfo& valueType)
	{
		std::string baseTypename = rtti::TypeInfo::get<Attribute<int>>().get_name().data();
		std::string resultingTypename = utility::replaceTemplateType(baseTypename, valueType.get_name().data());
		return rtti::TypeInfo::get_by_name(resultingTypename.c_str());
	}


	rtti::TypeInfo getOutpullPlugFromValueType(const rtti::TypeInfo& valueType)
	{
		std::string baseTypename = rtti::TypeInfo::get<OutputPullPlug<int>>().get_name().data();
		std::string resultingTypename = utility::replaceTemplateType(baseTypename, valueType.get_name().data());
		return rtti::TypeInfo::get_by_name(resultingTypename.c_str());
	}

    
    rtti::TypeInfo getInputPullPlugFromValueType(const rtti::TypeInfo& valueType)
    {
        std::string baseTypename = rtti::TypeInfo::get<InputPullPlug<int>>().get_name().data();
        std::string resultingTypename = utility::replaceTemplateType(baseTypename, valueType.get_name().data());
        return rtti::TypeInfo::get_by_name(resultingTypename.c_str());
        
    }
    
}