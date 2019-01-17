#include "apiargument.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIArgument)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::APIBaseValue)

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


	bool APIArgument::isString() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(std::string);
	}


	std::string nap::APIArgument::asString() const
	{
		assert(isString());
		return static_cast<const APIString*>(mAPIValue.get())->mValue;
	}


	bool APIArgument::isChar() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(char);
	}


	char APIArgument::asChar() const
	{
		assert(isChar());
		return static_cast<const APIChar*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isFloat() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(float);
	}


	float APIArgument::asFloat() const
	{
		assert(isFloat());
		return static_cast<const APIFloat*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isInt() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(int);
	}


	int APIArgument::asInt() const
	{
		assert(isInt());
		return static_cast<const APIInt*>(mAPIValue.get())->mValue;
	}


	bool APIArgument::isBool() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(bool);
	}


	bool nap::APIArgument::asBool() const
	{
		assert(isBool());
		return static_cast<const APIBool*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isDouble() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(double);
	}


	float nap::APIArgument::asDouble() const
	{
		assert(isDouble());
		return static_cast<const APIDouble*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isLong() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(long long);
	}


	long long nap::APIArgument::asLong() const
	{
		assert(isLong());
		return static_cast<const APILong*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isByte() const
	{
		return mAPIValue->get_type().get_raw_type() == RTTI_OF(uint8_t);
	}


	char nap::APIArgument::asByte() const
	{
		assert(isByte());
		return static_cast<const APIByte*>(mAPIValue.get())->mValue;
	}
}
