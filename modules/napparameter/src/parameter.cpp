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
	ResourcePtr<Parameter> ParameterGroup::findParameter(const std::string& id) const
	{
		for (const auto& param : mParameters)
			if (param->mID == id)
				return param;
		return nullptr;
	}


	nap::ResourcePtr<nap::Parameter> ParameterGroup::findParameter(const nap::ResourcePtr<Parameter>& param) const
	{
		for (const auto& param : mParameters)
			if (param == param)
				return param;
		return nullptr;
	}


	nap::ResourcePtr<nap::Parameter> ParameterGroup::findParameterRecursive(const std::string& id) const
	{
		nap::ResourcePtr<nap::Parameter> found_param = findParameter(id);
		if (found_param != nullptr)
			return found_param;

		for (const auto& group : mChildren)
		{
			found_param = group->findParameterRecursive(id);
			if (found_param == nullptr)
				continue;
			return found_param;
		}
		return nullptr;
	}


	nap::ResourcePtr<nap::Parameter> ParameterGroup::findParameterRecursive(const nap::ResourcePtr<Parameter>& param) const
	{
		nap::ResourcePtr<nap::Parameter> found_param = findParameter(param);
		if (found_param != nullptr)
			return found_param;

		for (const auto& group : mChildren)
		{
			found_param = group->findParameterRecursive(param);
			if (found_param == nullptr)
				continue;
			return found_param;
		}
		return nullptr;
	}


	ResourcePtr<ParameterGroup> ParameterGroup::findChild(const std::string& id) const
	{
		for (const auto& param : mChildren)
			if (param->mID == id)
				return param;

		return nullptr;
	}
}