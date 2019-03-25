#include "parameter.h"

RTTI_BEGIN_CLASS(nap::ParameterContainer)
	RTTI_PROPERTY("Parameters",	&nap::ParameterContainer::mParameters, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Children",	&nap::ParameterContainer::mChildren, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
	ResourcePtr<Parameter> ParameterContainer::findParameter(const std::string& name) const
	{
		for (auto& param : mParameters)
			if (param->mID == name)
				return param;

		return nullptr;
	}


	ResourcePtr<ParameterContainer> ParameterContainer::findChild(const std::string& name) const
	{
		for (auto& param : mChildren)
			if (param->mID == name)
				return param;

		return nullptr;
	}
}