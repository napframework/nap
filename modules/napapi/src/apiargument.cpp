#include "apiargument.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIArgument)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::BaseAPIValue)

RTTI_BEGIN_STRUCT(nap::APIFloat)
	RTTI_VALUE_CONSTRUCTOR(const float&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APIBool)
	RTTI_VALUE_CONSTRUCTOR(const bool&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APIInt)
	RTTI_VALUE_CONSTRUCTOR(const int&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APIChar)
	RTTI_VALUE_CONSTRUCTOR(const char&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APIString)
	RTTI_VALUE_CONSTRUCTOR(const std::string&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APIFloatArray)
	RTTI_VALUE_CONSTRUCTOR(const std::vector<float>&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APIIntArray)
	RTTI_VALUE_CONSTRUCTOR(const std::vector<int>&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APICharArray)
	RTTI_VALUE_CONSTRUCTOR(const std::vector<char>&)
RTTI_END_STRUCT

RTTI_BEGIN_STRUCT(nap::APIStringArray)
	RTTI_VALUE_CONSTRUCTOR(const std::vector<std::string>&)
RTTI_END_STRUCT

namespace nap
{
	bool APIArgument::isArray() const
	{
		return mAPIValue->mRepresentedType.is_array();
	}


	const nap::rtti::TypeInfo APIArgument::getValueType() const
	{
		if (isArray())
		{
			// Create array view to determine type
			rtti::Variant var = mAPIValue->mRepresentedType.create();
			rtti::VariantArray view = var.create_array_view();
			rtti::TypeInfo type = view.get_rank_type(view.get_rank());
			return type.is_wrapper() ? type.get_wrapped_type().get_raw_type() : type.get_raw_type();
		}
		return mAPIValue->get_type().get_raw_type();
	}
}
