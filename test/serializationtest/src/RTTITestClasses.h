#pragma once

#include <nap/objectptr.h>
#include <rtti/rttiobject.h>

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

	DataStruct(float value, nap::rtti::RTTIObject* pointer = nullptr) : 
		mFloatProperty(value), 
		mPointerProperty(pointer) 
	{
	}

	float					mFloatProperty = 0.0f;
	nap::rtti::RTTIObject*	mPointerProperty = nullptr;
	ENestedEnum				mNestedEnum = ENestedEnum::Five;
};

class BaseClass : public nap::rtti::RTTIObject
{
	RTTI_ENABLE(nap::rtti::RTTIObject)

public:
	int						mIntProperty = 0;
	std::string				mStringProperty;
	nap::rtti::RTTIObject*	mPointerProperty = nullptr;
	ETestEnum				mEnumProperty = ETestEnum::One;
	nap::ObjectPtr<nap::rtti::RTTIObject>	mObjectPtrProperty;
};

class DerivedClass : public BaseClass
{
	RTTI_ENABLE(BaseClass)

public:
	DataStruct							mNestedCompound;
	std::vector<int>					mArrayOfInts;
	std::vector<DataStruct>				mArrayOfCompounds;
	std::vector<nap::rtti::RTTIObject*>	mArrayOfPointers;
	nap::rtti::RTTIObject*				mEmbeddedPointer;
	std::vector<nap::rtti::RTTIObject*>	mArrayOfEmbeddedPointers;
};

class DerivedClass2 : public BaseClass
{

};