#pragma once

#include <rtti/objectptr.h>
#include <rtti/object.h>

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

	DataStruct(float value, nap::rtti::Object* pointer = nullptr) : 
		mFloatProperty(value), 
		mPointerProperty(pointer) 
	{
	}

	float					mFloatProperty = 0.0f;
	nap::rtti::Object*	mPointerProperty = nullptr;
	ENestedEnum				mNestedEnum = ENestedEnum::Five;
};

class BaseClass : public nap::rtti::Object
{
	RTTI_ENABLE(nap::rtti::Object)

public:
	int						mIntProperty = 0;
	std::string				mStringProperty;
	nap::rtti::Object*	mPointerProperty = nullptr;
	ETestEnum				mEnumProperty = ETestEnum::One;
	nap::rtti::ObjectPtr<nap::rtti::Object>	mObjectPtrProperty;
};

class DerivedClass : public BaseClass
{
	RTTI_ENABLE(BaseClass)

public:
	DataStruct							mNestedCompound;
	std::vector<int>					mArrayOfInts;
	std::vector<DataStruct>				mArrayOfCompounds;
	std::vector<nap::rtti::Object*>	mArrayOfPointers;
	nap::rtti::Object*				mEmbeddedPointer;
	std::vector<nap::rtti::Object*>	mArrayOfEmbeddedPointers;
};

class DerivedClass2 : public BaseClass
{

};