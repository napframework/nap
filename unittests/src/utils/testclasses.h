#pragma once

#include <componentptr.h>
#include <nap/resourceptr.h>

enum class TestEnum : int
{
	Undefined = 0,
	Alpha,
	Beta,
	Gamma,
};

RTTI_BEGIN_ENUM(TestEnum)
	RTTI_ENUM_VALUE(TestEnum::Undefined, "Undefined"),
	RTTI_ENUM_VALUE(TestEnum::Alpha,     "Alpha"),
	RTTI_ENUM_VALUE(TestEnum::Beta,      "Beta"),
	RTTI_ENUM_VALUE(TestEnum::Gamma,     "Gamma")
RTTI_END_ENUM

/**
 * Test Struct
 */
struct NAPAPI TestPropertiesStruct
{
	std::string 					mString = "Konnichiwa";
	std::vector<std::string> 		mStrings;

	int 							mInt = 10;
	std::vector<int> 				mInts;
	std::vector<std::vector<int>>	mInts2D;

	float 							mFloat = 12.34;
	std::vector<float> 				mFloats;

	TestEnum 						mEnum = TestEnum::Undefined;
	std::vector<TestEnum> 			mEnums;

};

RTTI_BEGIN_CLASS(TestPropertiesStruct)
		RTTI_PROPERTY("String",  &TestPropertiesStruct::mString,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings", &TestPropertiesStruct::mStrings, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Int",     &TestPropertiesStruct::mInt,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",    &TestPropertiesStruct::mInts,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",  &TestPropertiesStruct::mInts2D,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Float",   &TestPropertiesStruct::mFloat,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",  &TestPropertiesStruct::mFloats,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enum",    &TestPropertiesStruct::mEnum,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",   &TestPropertiesStruct::mEnums,   nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


/**
 * Test Resource
 */
class TestResource : public nap::Resource
{
	RTTI_ENABLE(nap::Resource)
public:
	TestResource() = default;

	std::string 							mString = "Bonjour";
	std::vector<std::string> 				mStrings;

	int 									mInt = 3;
	std::vector<int> 						mInts;
	std::vector<std::vector<int>> 			mInts2D;

	float 									mFloat = 3.14f;
	std::vector<float> 						mFloats;

	TestPropertiesStruct 					mStruct;
	std::vector<TestPropertiesStruct> 		mStructs;

	TestEnum 								mEnum = TestEnum::Undefined;
	std::vector<TestEnum> 					mEnums;
};

RTTI_BEGIN_CLASS(TestResource)
		RTTI_PROPERTY("String",   &TestResource::mString,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings",  &TestResource::mStrings, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Int",      &TestResource::mInt,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",     &TestResource::mInts,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",   &TestResource::mInts2D,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Float",    &TestResource::mFloat,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",   &TestResource::mFloats,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Struct",   &TestResource::mStruct,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs",  &TestResource::mStructs, nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enum",     &TestResource::mEnum,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",    &TestResource::mEnums,   nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

/**
 * TestResource B (has pointers)
 */
class TestResourceB : public TestResource
{
	RTTI_ENABLE(TestResource)
public:
	nap::ResourcePtr<TestResource>				mResPointer;
	std::vector<nap::ResourcePtr<TestResource>>	mResPointers;
	nap::ResourcePtr<TestResource>				mEmbedPointer;
	std::vector<nap::ResourcePtr<TestResource>>	mEmbedPointers;
};

RTTI_BEGIN_CLASS(TestResourceB)
		RTTI_PROPERTY("ResPointer",     &TestResourceB::mResPointer,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("ResPointers",    &TestResourceB::mResPointers,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("EmbedPointer",   &TestResourceB::mEmbedPointer,  nap::rtti::EPropertyMetaData::Embedded)
		RTTI_PROPERTY("EmbedPointers",  &TestResourceB::mEmbedPointers, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

/**
 * Test ComponentInstance
 */
class TestComponentInstance : public nap::ComponentInstance
{
	RTTI_ENABLE(nap::ComponentInstance)
};

/**
 * Test Component
 */
class TestComponent : public nap::Component
{
RTTI_ENABLE(Component)
public:
	TestComponent() = default;

	std::string 							mString = "Bonjour";
	std::vector<std::string> 				mStrings;

	int 									mInt = 3;
	std::vector<int> 						mInts;
	std::vector<std::vector<int>> 			mInts2D;

	float 									mFloat = 3.14f;
	std::vector<float> 						mFloats;

	TestPropertiesStruct 					mStruct;
	std::vector<TestPropertiesStruct> 		mStructs;

	TestEnum 								mEnum = TestEnum::Undefined;
	std::vector<TestEnum> 					mEnums;

	nap::ResourcePtr<TestResource> 				mResource;
	std::vector<nap::ResourcePtr<TestResource>> 	mResources;

	const nap::rtti::TypeInfo getInstanceType() const override
	{
		return RTTI_OF(TestComponentInstance);
	}
};

RTTI_BEGIN_CLASS(TestComponent)
		RTTI_PROPERTY("String",    &TestComponent::mString,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings",   &TestComponent::mStrings,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Int",       &TestComponent::mInt,       nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",      &TestComponent::mInts,      nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",    &TestComponent::mInts2D,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Float",     &TestComponent::mFloat,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",    &TestComponent::mFloats,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Struct",    &TestComponent::mStruct,    nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs",   &TestComponent::mStructs,   nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enum",      &TestComponent::mEnum,      nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",     &TestComponent::mEnums,     nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Resource",  &TestComponent::mResource,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("Resources", &TestComponent::mResources, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

/**
 * TestComponent B (contains pointers to TestComponent
 */
class TestComponentB : public TestComponent
{
RTTI_ENABLE(TestComponent)
public:
	nap::ComponentPtr<TestComponent> 				mCompPointer;
	std::vector<nap::ComponentPtr<TestComponent>> 	mCompPointers;
};

RTTI_BEGIN_CLASS(TestComponentB)
		RTTI_PROPERTY("CompPointer",  &TestComponentB::mCompPointer,  nap::rtti::EPropertyMetaData::Default)
		RTTI_PROPERTY("CompPointers", &TestComponentB::mCompPointers, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS
