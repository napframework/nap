#include "RTTITestClasses.h"
#include <rtti/rtti.h>
#include <rtti/rttiutilities.h>
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <rtti/binarywriter.h>
#include <rtti/binaryreader.h>
#include <nap/memorystream.h>
#include <nap/logger.h>
#include <nap/core.h>

#include <nap/resource.h>
#include <nap/stringutils.h>

#include <rtti/rttipath.h>

using namespace nap;

BaseClass* createTestHierarchy()
{
	DerivedClass* pointee = new DerivedClass();
	pointee->mID								= "Pointee";
	pointee->mIntProperty						= 42;
	pointee->mStringProperty					= "Pointee String";
	pointee->mNestedCompound.mFloatProperty		= 16.0f;
	pointee->mNestedCompound.mPointerProperty	= pointee;
	pointee->mArrayOfInts.push_back(1);
	pointee->mArrayOfInts.push_back(2);
	pointee->mArrayOfInts.push_back(3);
	pointee->mArrayOfCompounds.push_back(DataStruct(4.0f, pointee));
	pointee->mArrayOfCompounds.push_back(DataStruct(5.0f));
	pointee->mArrayOfCompounds.push_back(DataStruct(6.0f));
	pointee->mArrayOfPointers.push_back(pointee);

	BaseClass* root = new BaseClass();
	root->mID									= "Root";
	root->mIntProperty							= 42 / 2;
	root->mStringProperty						= "Root String";
	root->mPointerProperty						= pointee;

	return root;
}


bool ResolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers)
{
	std::map<std::string, Object*> objects_by_id;
	for (auto& object : objects)
		objects_by_id.insert({ object->mID, object.get() });
	
	for (const UnresolvedPointer& unresolvedPointer : unresolvedPointers)
	{
		RTTI::ResolvedRTTIPath resolved_path;			
		if (!unresolvedPointer.mRTTIPath.resolve(unresolvedPointer.mObject, resolved_path))
			return false;

		std::map<std::string, Object*>::iterator pos = objects_by_id.find(unresolvedPointer.mTargetID);
		if (pos == objects_by_id.end())
			return false;

		if (!resolved_path.setValue(pos->second))
			return false;
	}

	return true;
}
 

int main(int argc, char* argv[])
{
	Logger::setLevel(Logger::debugLevel());

	Core core;
	core.initialize();

	RTTI::TypeInfo::get<std::vector<DataStruct>>();

	std::size_t version_datastruct = RTTI::getRTTIVersion(RTTI_OF(DataStruct));
	std::size_t version_base = RTTI::getRTTIVersion(RTTI_OF(BaseClass));
	std::size_t version_derived = RTTI::getRTTIVersion(RTTI_OF(DerivedClass));

	// Create test hierarchy
	BaseClass* root = createTestHierarchy();
	 
	// Create path to float property in array of nested compounds
 	RTTI::RTTIPath float_property_path;
 	float_property_path.pushAttribute("ArrayOfCompounds");
 	float_property_path.pushArrayElement(0);
 	float_property_path.pushAttribute("FloatProperty");
 
	// Convert path to string
 	std::string path_str = float_property_path.toString();

	// Convert back and verify the path is the same
 	RTTI::RTTIPath path_copy = RTTI::RTTIPath::fromString(path_str);
	if (path_copy != float_property_path)
		return -1;
 
	// Resolve the path and verify it succeeded
 	RTTI::ResolvedRTTIPath resolved_path;
	if (!float_property_path.resolve(root->mPointerProperty, resolved_path))
		return -1;

	// Verify setting the value works
	float old_value = resolved_path.getValue().convert<float>();
	if (!resolved_path.setValue(8.0f))
		return -1; 

	// Restore value so we can compare later
	resolved_path.setValue(old_value);

	{
		ErrorState error_state;

		// Write to json
		JSONWriter writer;
		if (!serializeObjects({ root }, writer, error_state))
			return -1;

		// Print json
		std::string json = writer.GetJSON();
		std::cout << json << std::endl;

		// Read json and verify it succeeds
		RTTIDeserializeResult read_result;
		if (!deserializeJSON(json, read_result, error_state))
			return -1;

		// Resolve links
		if (!ResolveLinks(read_result.mReadObjects, read_result.mUnresolvedPointers))
			return -1;

		// Sort read objects into id mapping
		std::map<std::string, Object*> objects_by_id;
		for (auto& object : read_result.mReadObjects)
			objects_by_id.insert({ object->mID, object.get() });

		// Compare root objects
		if (!RTTI::areObjectsEqual(*objects_by_id["Root"], *root, RTTI::EPointerComparisonMode::BY_ID))
			return -1;

		// Compare pointee-objects
		if (!RTTI::areObjectsEqual(*objects_by_id["Pointee"], *root->mPointerProperty, RTTI::EPointerComparisonMode::BY_ID))
			return -1;
	}

	{
		ErrorState error_state;

		// Write to binary
		BinaryWriter binary_writer;
		if (!serializeObjects({ root }, binary_writer, error_state))
			return -1;

		// Read binary and verify it succeeds
		MemoryStream stream(binary_writer.getBuffer().data(), binary_writer.getBuffer().size());
		RTTIDeserializeResult read_result;
		if (!deserializeBinary(stream, read_result, error_state))
			return -1;

		// Resolve links
		if (!ResolveLinks(read_result.mReadObjects, read_result.mUnresolvedPointers))
			return -1;

		// Sort read objects into id mapping
		std::map<std::string, Object*> objects_by_id;
		for (auto& object : read_result.mReadObjects)
			objects_by_id.insert({ object->mID, object.get() });

		// Compare root objects
		if (!RTTI::areObjectsEqual(*objects_by_id["Root"], *root, RTTI::EPointerComparisonMode::BY_ID))
			return -1;

		// Compare pointee-objects
		if (!RTTI::areObjectsEqual(*objects_by_id["Pointee"], *root->mPointerProperty, RTTI::EPointerComparisonMode::BY_ID))
			return -1;
	}

	return 0;
}