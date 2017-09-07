#include "oscargument.h"

RTTI_DEFINE_BASE(nap::OSCArgument)
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
		memcpy(mData, data, size);
	}


	OSCBlob::~OSCBlob()
	{
		delete mData;
	}

	void* OSCBlob::getCopy()
	{
		void* dest = nullptr;
		memcpy(dest, mData, mSize);
		return dest;
	}

	bool OSCArgument::isFloat() const
	{
		return this->mValue->get_type().is_derived_from(RTTI_OF(OSCFloat));
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

	std::string OSCArgument::asString() const
	{
		assert(isString());
		return static_cast<const OSCValue<std::string>*>(this->mValue.get())->mValue;
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

	OSCArgument::OSCArgument(OSCValuePtr value) : mValue(std::move(value))
	{	}
}