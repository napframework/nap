#include "parametermapping.h"
#include "parameternumeric.h"

// External Include
#include <mathutils.h>

// nap::parametermapping run time class definition 
RTTI_BEGIN_CLASS(nap::ParameterMapping)
	RTTI_PROPERTY("Map",			&nap::ParameterMapping::mMap,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Parameter",		&nap::ParameterMapping::mParameter,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("InputRange",		&nap::ParameterMapping::mInputRange,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputRange",	&nap::ParameterMapping::mOutputRange,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	ParameterMapping::~ParameterMapping()			{ }


	bool ParameterMapping::init(utility::ErrorState& errorState)
	{
		Parameter* cparam = mParameter.get();
		if (!errorState.check(cparam != nullptr, "%s: no parameter specified!", mID.c_str()))
			return false;

		// We only accept float and int parameters
		bool is_int = cparam->get_type().is_derived_from(RTTI_OF(ParameterInt));
		bool is_flo = cparam->get_type().is_derived_from(RTTI_OF(ParameterFloat));
		bool is_dou = cparam->get_type().is_derived_from(RTTI_OF(ParameterDouble));
		if (!(errorState.check(is_int || is_flo || is_dou, "%s: parameter %s is not of type int, float or double")))
			return false;

		return true;
	}


	void ParameterMapping::setValue(float value)
	{
		float mapped = math::fit<float>(value, mInputRange.x, mInputRange.y, mOutputRange.x, mOutputRange.y);
		if (mParameter->get_type().is_derived_from(RTTI_OF(ParameterFloat)))
		{
			push<float>(mapped, *mParameter);
			return;
		}
		if (mParameter->get_type().is_derived_from(RTTI_OF(ParameterInt)))
		{
			push<int>(mapped, *mParameter);
			return;
		}
		if (mParameter->get_type().is_derived_from(RTTI_OF(ParameterDouble)))
		{
			push<double>(mapped, *mParameter);
			return;
		}
	}
}