#include "utils/catch.hpp"

#include "utils/RTTITestClasses.h"
#include <rtti/rtti.h>
#include <rtti/rttiutilities.h>
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <rtti/binarywriter.h>
#include <rtti/binaryreader.h>
#include <rtti/factory.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <utility/memorystream.h>
#include <utility/stringutils.h>
#include <rtti/path.h>
#include <rtti/defaultlinkresolver.h>
#include <rtti/deserializeresult.h>
#include <iostream>

using namespace nap::utility;

BaseClass* createTestHierarchy()
{
	DerivedClass* pointee = new DerivedClass();
	pointee->mID = "Pointee";
	pointee->mIntProperty = 42;
	pointee->mStringProperty = "Pointee String";
	pointee->mEnumProperty = ETestEnum::Two;
	pointee->mNestedCompound.mFloatProperty = 16.0f;
	pointee->mNestedCompound.mPointerProperty = pointee;
	pointee->mNestedCompound.mNestedEnum = DataStruct::ENestedEnum::Eight;
	pointee->mArrayOfInts.push_back(1);
	pointee->mArrayOfInts.push_back(2);
	pointee->mArrayOfInts.push_back(3);
	pointee->mArrayOfCompounds.push_back(DataStruct(4.0f, pointee));
	pointee->mArrayOfCompounds.push_back(DataStruct(5.0f));
	pointee->mArrayOfCompounds.push_back(DataStruct(6.0f));
	pointee->mArrayOfPointers.push_back(pointee);
	pointee->mEmbeddedPointer = new DerivedClass();
	pointee->mEmbeddedPointer->mID = "EmbeddedObject";
	pointee->mArrayOfEmbeddedPointers.push_back(new DerivedClass());
	pointee->mArrayOfEmbeddedPointers[0]->mID = "EmbeddedArrayObject";

	BaseClass* root = new BaseClass();
	root->mID = "Root";
	root->mIntProperty = 42 / 2;
	root->mStringProperty = "Root String";
	root->mPointerProperty = pointee;
	root->mObjectPtrProperty = pointee;

	return root;
}


template<typename T>
nap::rtti::Variant createVariant(T value)
{
	return nap::rtti::ObjectPtr<T>(value);
}

void testObjectPtr()
{
	typedef nap::rtti::ObjectPtr<BaseClass> BaseClassPtr;
	typedef nap::rtti::ObjectPtr<DerivedClass> DerivedClassPtr;
	typedef nap::rtti::ObjectPtr<DerivedClass2> DerivedClass2Ptr;

	DerivedClassPtr derived = new DerivedClass();
	BaseClassPtr base = derived;
	BaseClassPtr other_base;
	other_base = derived;
	other_base = std::move(derived);
	derived = base;

	std::vector<DerivedClassPtr> resize_test;
	resize_test.resize(10);
	for (int i = 0; i < 10; ++i)
	{
		DerivedClass* object = new DerivedClass();
		object->mIntProperty = i;
		resize_test[i] = object;

		DerivedClassPtr copy_constr = resize_test[i];
	}
	resize_test.resize(1000);

	std::vector<DerivedClassPtr> objects;
	for (int i = 0; i < 1000; ++i)
	{
		DerivedClass* object = new DerivedClass();
		object->mIntProperty = i;

		objects.push_back(object);
	}
}


TEST_CASE("Serialization", "[Serialization]")
{
	using namespace nap;

	//testObjectPtr();
	Logger::setLevel(Logger::debugLevel());

	Core core;
	nap::utility::ErrorState error;
	core.initializeEngine(error);

	// TODO: This should be unit testable, currently this fails because of a project json file we're not testing
	//	REQUIRE(core.initializeEngine(error));

	rtti::TypeInfo::get<std::vector<DataStruct>>();

	std::size_t version_datastruct = rtti::getRTTIVersion(RTTI_OF(DataStruct));
	std::size_t version_base = rtti::getRTTIVersion(RTTI_OF(BaseClass));
	std::size_t version_derived = rtti::getRTTIVersion(RTTI_OF(DerivedClass));

	// Create test hierarchy
	auto root = createTestHierarchy();

	// Create path to float property in array of nested compounds
	rtti::Path float_property_path;
	float_property_path.pushAttribute("ArrayOfCompounds");
	float_property_path.pushArrayElement(0);
	float_property_path.pushAttribute("FloatProperty");

	// Convert path to string
	std::string path_str = float_property_path.toString();

	// Convert back and verify the path is the same
	rtti::Path path_copy = rtti::Path::fromString(path_str);
	REQUIRE(path_copy == float_property_path);

	// Resolve the path and verify it succeeded
	rtti::ResolvedPath resolved_path;
	REQUIRE(float_property_path.resolve(root->mPointerProperty, resolved_path));

	// Verify setting the value works
	float old_value = resolved_path.getValue().get_value<float>();
	REQUIRE(resolved_path.setValue(8.0f));

	// Restore value so we can compare later
	resolved_path.setValue(old_value);

	rtti::Factory factory;

	{
		ErrorState error_state;

		// Write to json
		rtti::JSONWriter writer;
		REQUIRE(serializeObjects({ root }, writer, error_state));

		// Print json
		std::string json = writer.GetJSON();
		//		std::cout << json << std::endl;

		// Read json and verify it succeeds
		rtti::DeserializeResult read_result;
		REQUIRE(deserializeJSON(json, rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::AllPointerTypes, factory, read_result, error_state));

		// Sort read objects into id mapping
		std::map<std::string, rtti::Object*> objects_by_id;
		for (auto& object : read_result.mReadObjects)
			objects_by_id.insert({ object->mID, object.get() });

		// Compare root objects
		REQUIRE(rtti::areObjectsEqual(*objects_by_id["Root"], *root, read_result.mUnresolvedPointers));

		// Compare pointee-objects
		REQUIRE(rtti::areObjectsEqual(*objects_by_id["Pointee"], *root->mPointerProperty, read_result.mUnresolvedPointers));

		// Resolve links
		REQUIRE(rtti::DefaultLinkResolver::sResolveLinks(read_result.mReadObjects, read_result.mUnresolvedPointers, error_state));
	}

	{
		ErrorState error_state;

		// Write to binary
		rtti::BinaryWriter binary_writer;
		if (!serializeObjects({ root }, binary_writer, error_state))
			FAIL(error_state.toString());

		// Read binary and verify it succeeds
		MemoryStream stream(binary_writer.getBuffer().data(), binary_writer.getBuffer().size());
		rtti::DeserializeResult read_result;
		REQUIRE(deserializeBinary(stream, factory, read_result, error_state));

		// Sort read objects into id mapping
		std::map<std::string, rtti::Object*> objects_by_id;
		for (auto& object : read_result.mReadObjects)
			objects_by_id.insert({ object->mID, object.get() });

		// Compare root objects
		REQUIRE(rtti::areObjectsEqual(*objects_by_id["Root"], *root, read_result.mUnresolvedPointers));
		
		// Compare pointee-objects
		REQUIRE(rtti::areObjectsEqual(*objects_by_id["Pointee"], *root->mPointerProperty, read_result.mUnresolvedPointers));

		// Resolve links
		REQUIRE(rtti::DefaultLinkResolver::sResolveLinks(read_result.mReadObjects, read_result.mUnresolvedPointers, error_state));
	}
	delete root;
}