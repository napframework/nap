#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resource.h>
#include "nap/resourceptr.h"

namespace nap
{
	/**
	* 
	*/
	class NAPAPI Parameter : public Resource
	{
		RTTI_ENABLE(Resource)
	
	public:
		virtual void setValue(const Parameter& value) = 0;

	public:
	};

	class NAPAPI ParameterContainer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		ResourcePtr<Parameter> findParameter(const std::string& name) const;
		ResourcePtr<ParameterContainer> findChild(const std::string& name) const;

	public:
		std::vector<ResourcePtr<Parameter>>				mParameters;
		std::vector<ResourcePtr<ParameterContainer>>	mChildren;
	};
}
