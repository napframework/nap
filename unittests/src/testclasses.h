#pragma once

#include <nap/resource.h>
#include <nap/resourceptr.h>

using namespace nap;
using namespace nap::rtti;

/**
 * Test Struct
 */
struct NAPAPI TestPropertiesStruct
{
	std::string mString = "Konnichiwa";
	int mInt = 10;
	float mFloat = 12.34;

	std::vector<std::string> mStrings;
	std::vector<int> mInts;
	std::vector<float> mFloats;
};

RTTI_BEGIN_CLASS(TestPropertiesStruct)
		RTTI_PROPERTY("String", &TestPropertiesStruct::mString, EPropertyMetaData::Default)
		RTTI_PROPERTY("Int", &TestPropertiesStruct::mInt, EPropertyMetaData::Default)
		RTTI_PROPERTY("Float", &TestPropertiesStruct::mFloat, EPropertyMetaData::Default)
		RTTI_PROPERTY("Strings", &TestPropertiesStruct::mStrings, EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints", &TestPropertiesStruct::mInts, EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats", &TestPropertiesStruct::mFloats, EPropertyMetaData::Default)
RTTI_END_CLASS

/**
 * Test Resource
 */
class TestResource : public Resource
{
	RTTI_ENABLE(Resource)
public:
	TestResource() = default;

	std::string mString = "Bonjour";
	int mInt = 3;
	float mFloat = 3.14f;
	TestPropertiesStruct mStruct;

	std::vector<std::string> mStrings;
	std::vector<int> mInts;
	std::vector<std::vector<int>> mInts2D;
	std::vector<float> mFloats;
	std::vector<TestPropertiesStruct> mStructs;
};

RTTI_BEGIN_CLASS(TestResource)
		RTTI_PROPERTY("String", &TestResource::mString, EPropertyMetaData::Default)
		RTTI_PROPERTY("Int", &TestResource::mInt, EPropertyMetaData::Default)
		RTTI_PROPERTY("Float", &TestResource::mFloat, EPropertyMetaData::Default)
		RTTI_PROPERTY("Struct", &TestResource::mStruct, EPropertyMetaData::Default)

		RTTI_PROPERTY("Strings", &TestResource::mStrings, EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints", &TestResource::mInts, EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D", &TestResource::mInts2D, EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats", &TestResource::mFloats, EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs", &TestResource::mStructs, EPropertyMetaData::Default)
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

	std::string mString = "Bonjour";
	int mInt = 3;
	float mFloat = 3.14f;
	TestPropertiesStruct mStruct;
	ResourcePtr<TestResource> mResource;

	std::vector<std::string> mStrings;
	std::vector<int> mInts;
	std::vector<std::vector<int>> mInts2D;
	std::vector<float> mFloats;
	std::vector<TestPropertiesStruct> mStructs;
	std::vector<ResourcePtr<TestResource>> mResources;

	const TypeInfo getInstanceType() const override
	{
		return RTTI_OF(TestComponentInstance);
	}
};

RTTI_BEGIN_CLASS(TestComponent)
		RTTI_PROPERTY("String", &TestComponent::mString, EPropertyMetaData::Default)
		RTTI_PROPERTY("Int", &TestComponent::mInt, EPropertyMetaData::Default)
		RTTI_PROPERTY("Float", &TestComponent::mFloat, EPropertyMetaData::Default)
		RTTI_PROPERTY("Struct", &TestComponent::mStruct, EPropertyMetaData::Default)
		RTTI_PROPERTY("Resource", &TestComponent::mResource, EPropertyMetaData::Default)

		RTTI_PROPERTY("Strings", &TestComponent::mStrings, EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints", &TestComponent::mInts, EPropertyMetaData::Default)
		RTTI_PROPERTY("Ints2D", &TestComponent::mInts2D, EPropertyMetaData::Default)
		RTTI_PROPERTY("Floats", &TestComponent::mFloats, EPropertyMetaData::Default)
		RTTI_PROPERTY("Structs", &TestComponent::mStructs, EPropertyMetaData::Default)
		RTTI_PROPERTY("Resources", &TestComponent::mResources, EPropertyMetaData::Default)
RTTI_END_CLASS
