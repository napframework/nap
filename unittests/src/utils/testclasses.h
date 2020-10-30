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



/**
 * Test Struct
 */
struct TestPropertiesStruct
{
	std::string 					mString = "Konnichiwa";
	std::vector<std::string> 		mStrings;

	std::string						mFilename;

	int 							mInt = 10;
	std::vector<int> 				mInts;
	std::vector<std::vector<int>>	mInts2D;

	float 							mFloat = 12.34;
	std::vector<float> 				mFloats;

	TestEnum 						mEnum = TestEnum::Undefined;
	std::vector<TestEnum> 			mEnums;

};




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


/**
 * Test ComponentInstance
 */
class TestComponentInstance : public nap::ComponentInstance
{
	RTTI_ENABLE(nap::ComponentInstance)
public:
	TestComponentInstance(nap::EntityInstance& entity, nap::Component& resource)
			: nap::ComponentInstance(entity, resource)
	{
	}


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


