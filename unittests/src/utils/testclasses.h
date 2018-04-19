#pragma once

#include <componentptr.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

using namespace nap;
using namespace nap::rtti;

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
		RTTI_PROPERTY("String",  &TestPropertiesStruct::mString,  EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings", &TestPropertiesStruct::mStrings, EPropertyMetaData::Default)

		RTTI_PROPERTY("Int",     &TestPropertiesStruct::mInt,     EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",    &TestPropertiesStruct::mInts,    EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",  &TestPropertiesStruct::mInts2D,  EPropertyMetaData::Default)

		RTTI_PROPERTY("Float",   &TestPropertiesStruct::mFloat,   EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",  &TestPropertiesStruct::mFloats,  EPropertyMetaData::Default)

		RTTI_PROPERTY("Enum",    &TestPropertiesStruct::mEnum,    EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",   &TestPropertiesStruct::mEnums,   EPropertyMetaData::Default)
RTTI_END_CLASS


/**
 * Test Resource
 */
class TestResource : public Resource
{
	RTTI_ENABLE(Resource)
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
		RTTI_PROPERTY("String",   &TestResource::mString,  EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings",  &TestResource::mStrings, EPropertyMetaData::Default)

		RTTI_PROPERTY("Int",      &TestResource::mInt,     EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",     &TestResource::mInts,    EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",   &TestResource::mInts2D,  EPropertyMetaData::Default)

		RTTI_PROPERTY("Float",    &TestResource::mFloat,   EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",   &TestResource::mFloats,  EPropertyMetaData::Default)

		RTTI_PROPERTY("Struct",   &TestResource::mStruct,  EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs",  &TestResource::mStructs, EPropertyMetaData::Default)

		RTTI_PROPERTY("Enum",     &TestResource::mEnum,    EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",    &TestResource::mEnums,   EPropertyMetaData::Default)
RTTI_END_CLASS

/**
 * TestResource B (has pointers)
 */
class TestResourceB : public TestResource
{
	RTTI_ENABLE(TestResource)
public:
	ResourcePtr<TestResource>				mResPointer;
	std::vector<ResourcePtr<TestResource>>	mResPointers;
	ResourcePtr<TestResource>				mEmbedPointer;
	std::vector<ResourcePtr<TestResource>>	mEmbedPointers;
};

RTTI_BEGIN_CLASS(TestResourceB)
		RTTI_PROPERTY("ResPointer",     &TestResourceB::mResPointer,    EPropertyMetaData::Default)
		RTTI_PROPERTY("ResPointers",    &TestResourceB::mResPointers,   EPropertyMetaData::Default)
		RTTI_PROPERTY("EmbedPointer",   &TestResourceB::mEmbedPointer,  EPropertyMetaData::Embedded)
		RTTI_PROPERTY("EmbedPointers",  &TestResourceB::mEmbedPointers, EPropertyMetaData::Embedded)
RTTI_END_CLASS

/**
 * Test ComponentInstance
 */
class TestComponentInstance : public ComponentInstance
{
	RTTI_ENABLE(ComponentInstance)
};

/**
 * Test Component
 */
class TestComponent : public Component
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

	ResourcePtr<TestResource> 				mResource;
	std::vector<ResourcePtr<TestResource>> 	mResources;

	const TypeInfo getInstanceType() const override
	{
		return RTTI_OF(TestComponentInstance);
	}
};

RTTI_BEGIN_CLASS(TestComponent)
		RTTI_PROPERTY("String",    &TestComponent::mString,    EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings",   &TestComponent::mStrings,   EPropertyMetaData::Default)

		RTTI_PROPERTY("Int",       &TestComponent::mInt,       EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints",      &TestComponent::mInts,      EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D",    &TestComponent::mInts2D,    EPropertyMetaData::Default)

		RTTI_PROPERTY("Float",     &TestComponent::mFloat,     EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats",    &TestComponent::mFloats,    EPropertyMetaData::Default)

		RTTI_PROPERTY("Struct",    &TestComponent::mStruct,    EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs",   &TestComponent::mStructs,   EPropertyMetaData::Default)

		RTTI_PROPERTY("Enum",      &TestComponent::mEnum,      EPropertyMetaData::Default)
		RTTI_PROPERTY("Enums",     &TestComponent::mEnums,     EPropertyMetaData::Default)

		RTTI_PROPERTY("Resource",  &TestComponent::mResource,  EPropertyMetaData::Default)
		RTTI_PROPERTY("Resources", &TestComponent::mResources, EPropertyMetaData::Default)
RTTI_END_CLASS

/**
 * TestComponent B (contains pointers to TestComponent
 */
class TestComponentB : public TestComponent
{
RTTI_ENABLE(TestComponent)
public:
	ComponentPtr<TestComponent> 				mCompPointer;
	std::vector<ComponentPtr<TestComponent>> 	mCompPointers;
};

RTTI_BEGIN_CLASS(TestComponentB)
		RTTI_PROPERTY("CompPointer",  &TestComponentB::mCompPointer,  EPropertyMetaData::Default)
		RTTI_PROPERTY("CompPointers", &TestComponentB::mCompPointers, EPropertyMetaData::Default)
RTTI_END_CLASS
