#pragma once

#include <nap/object.h>
#include <nap/objectptr.h>

enum class ETestEnum
{
	One,
	Two,
	Three,
	Four
};

struct DataStruct
{
	enum class ENestedEnum
	{
		Five,
		Six,
		Seven,
		Eight
	};

	DataStruct()
	{
	}

	DataStruct(float value, nap::Object* pointer = nullptr) : 
		mFloatProperty(value), 
		mPointerProperty(pointer) 
	{
	}

	float			mFloatProperty = 0.0f;
	nap::Object*	mPointerProperty = nullptr;
	ENestedEnum		mNestedEnum = ENestedEnum::Five;
};

class BaseClass : public nap::Object
{
	RTTI_ENABLE(nap::Object)

public:
	int				mIntProperty = 0;
	std::string		mStringProperty;
	nap::Object*	mPointerProperty = nullptr;
	ETestEnum		mEnumProperty = ETestEnum::One;
	nap::ObjectPtr<nap::Object>	mObjectPtrProperty;
};

class DerivedClass : public BaseClass
{
	RTTI_ENABLE(BaseClass)

public:
	DataStruct					mNestedCompound;
	std::vector<int>			mArrayOfInts;
	std::vector<DataStruct>		mArrayOfCompounds;
	std::vector<nap::Object*>	mArrayOfPointers;
	nap::Object*				mEmbeddedPointer;
	std::vector<nap::Object*>	mArrayOfEmbeddedPointers;
};

class DerivedClass2 : public BaseClass
{

};