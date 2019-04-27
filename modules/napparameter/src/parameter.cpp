#include <parameter.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Parameter)
	RTTI_PROPERTY("Name",		&nap::Parameter::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ParameterGroup)
	RTTI_PROPERTY("Parameters",		&nap::ParameterGroup::mParameters,	nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Groups",			&nap::ParameterGroup::mChildren,	nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
	ResourcePtr<Parameter> ParameterGroup::findParameter(const std::string& name) const
	{
		for (auto& param : mParameters)
			if (param->mID == name)
				return param;

		return nullptr;
	}


	ResourcePtr<ParameterGroup> ParameterGroup::findChild(const std::string& name) const
	{
		for (auto& param : mChildren)
			if (param->mID == name)
				return param;

		return nullptr;
	}
}