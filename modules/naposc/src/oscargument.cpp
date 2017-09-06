#include "oscargument.h"

RTTI_DEFINE_BASE(nap::OSCArgument)

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

}