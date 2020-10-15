/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "apiargument.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIArgument)
RTTI_END_CLASS


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
		return mAPIValue->getRepresentedType();
	}


	bool APIArgument::isString() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(std::string);
	}


	std::string nap::APIArgument::asString() const
	{
		assert(isString());
		return static_cast<const APIString*>(mAPIValue.get())->mValue;
	}


	bool APIArgument::isChar() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(char);
	}


	char APIArgument::asChar() const
	{
		assert(isChar());
		return static_cast<const APIChar*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isFloat() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(float);
	}


	float APIArgument::asFloat() const
	{
		assert(isFloat());
		return static_cast<const APIFloat*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isInt() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(int);
	}


	int APIArgument::asInt() const
	{
		assert(isInt());
		return static_cast<const APIInt*>(mAPIValue.get())->mValue;
	}


	bool APIArgument::isBool() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(bool);
	}

	const std::string& APIArgument::getName() const
	{
		return mAPIValue->mName;
	}

	bool nap::APIArgument::asBool() const
	{
		assert(isBool());
		return static_cast<const APIBool*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isDouble() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(double);
	}


	float nap::APIArgument::asDouble() const
	{
		assert(isDouble());
		return static_cast<const APIDouble*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isLong() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(int64_t);
	}


	int64_t nap::APIArgument::asLong() const
	{
		assert(isLong());
		return static_cast<const APILong*>(mAPIValue.get())->mValue;
	}


	bool nap::APIArgument::isByte() const
	{
		return mAPIValue->getRepresentedType() == RTTI_OF(uint8_t);
	}


	char nap::APIArgument::asByte() const
	{
		assert(isByte());
		return static_cast<const APIByte*>(mAPIValue.get())->mValue;
	}
}
