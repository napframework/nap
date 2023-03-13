/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "oscargument.h"

RTTI_BEGIN_CLASS(nap::OSCArgument)
    RTTI_FUNCTION("isFloat", &nap::OSCArgument::isFloat)
    RTTI_FUNCTION("asFloat", &nap::OSCArgument::asFloat)
    RTTI_FUNCTION("isInt", &nap::OSCArgument::isInt)
    RTTI_FUNCTION("asInt", &nap::OSCArgument::asInt)
    RTTI_FUNCTION("isBool", &nap::OSCArgument::isBool)
    RTTI_FUNCTION("asBool", &nap::OSCArgument::asBool)
    RTTI_FUNCTION("isString", &nap::OSCArgument::isString)
    RTTI_FUNCTION("asString", &nap::OSCArgument::asString)
    RTTI_FUNCTION("isDouble", &nap::OSCArgument::isDouble)
    RTTI_FUNCTION("asDouble", &nap::OSCArgument::asDouble)
    RTTI_FUNCTION("isChar", &nap::OSCArgument::isChar)
    RTTI_FUNCTION("asChar", &nap::OSCArgument::asChar)
    RTTI_FUNCTION("isNukt", &nap::OSCArgument::isNil)
    RTTI_FUNCTION("toString", &nap::OSCArgument::toString)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::OSCBaseValue)


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCFloat)
	RTTI_CONSTRUCTOR(const float&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCBool)
	RTTI_CONSTRUCTOR(const bool&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCInt)
	RTTI_CONSTRUCTOR(const int&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCDouble)
	RTTI_CONSTRUCTOR(const double&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCChar)
	RTTI_CONSTRUCTOR(const char&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCString)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCColor)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCTimeTag)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCBlob)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	OSCBlob::OSCBlob(const void* data, int size) : mSize(size)
	{
        mData = malloc(size);
		memcpy(mData, data, size);
	}


	OSCBlob::~OSCBlob()
	{
        free(mData);
	}

	void* OSCBlob::getCopy()
	{
        void* dest = malloc(mSize);
        memcpy(dest, mData, mSize);
		return dest;
	}


	void OSCBlob::add(osc::OutboundPacketStream& outPacket) const
	{
		outPacket << osc::Blob(mData, mSize);
	}


	bool OSCArgument::isFloat() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCFloat));
	}


	nap::rtti::TypeInfo OSCArgument::getValueType() const
	{
		return mValue->get_type().get_raw_type();
	}


	float OSCArgument::asFloat() const
	{
		assert(isFloat());
		return static_cast<const OSCValue<float>*>(this->mValue.get())->mValue;
	}


	int OSCArgument::asInt() const
	{
		assert(isInt());
		return static_cast<const OSCValue<int>*>(this->mValue.get())->mValue;
	}


	bool OSCArgument::isInt() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCInt));
	}

	bool OSCArgument::asBool() const
	{
		assert(isBool());
		return static_cast<const OSCValue<bool>*>(this->mValue.get())->mValue;
	}


	bool OSCArgument::isBool() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCBool));
	}


	const std::string& OSCArgument::asString() const
	{
		assert(isString());
		return static_cast<const OSCString*>(this->mValue.get())->mString;
	}


	bool OSCArgument::isString() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCString));
	}


	double OSCArgument::asDouble() const
	{
		assert(isDouble());
		return static_cast<const OSCValue<double>*>(this->mValue.get())->mValue;
	}


	bool OSCArgument::isDouble() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCDouble));
	}


	char OSCArgument::asChar() const
	{
		assert(isChar());
		return static_cast<const OSCValue<char>*>(this->mValue.get())->mValue;
	}


	bool OSCArgument::isChar() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCChar));
	}


	bool OSCArgument::isNil() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCNil));
	}


    std::string OSCArgument::toString()
	{
		return mValue->toString();
	}


	OSCArgument::OSCArgument(OSCValuePtr value) : mValue(std::move(value))
	{	}


	void OSCArgument::add(osc::OutboundPacketStream& outPacket) const
	{
		mValue->add(outPacket);
	}


	std::size_t OSCArgument::size() const
	{
		return mValue->size();
	}


	void OSCColor::add(osc::OutboundPacketStream& outPacket) const
	{
		outPacket << osc::RgbaColor(mColor);
	}


	std::string OSCString::toString() const
	{
		return mString;
	}


	void OSCString::add(osc::OutboundPacketStream& outPacket) const
	{
		outPacket << mString.c_str();
	}


	std::size_t OSCString::size() const
	{
		return mString.size();
	}

}
