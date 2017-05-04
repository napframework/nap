#include "RTTITestClasses.h"
#include <rtti/rtti.h>
#include <nap/rttiutilities.h>
#include <nap/JSonReader.h>
#include <nap/jsonwriter.h>
#include <nap/logger.h>
#include <nap/core.h>

#include <nap/resource.h>
#include <nap/stringutils.h>

#include <nap/rttipath.h>

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
		RTTI::ResolvedRTTIPath resolved_path = unresolvedPointer.mRTTIPath.Resolve(unresolvedPointer.mObject);
		if (!resolved_path.IsValid())
			return false;

		std::map<std::string, Object*>::iterator pos = objects_by_id.find(unresolvedPointer.mTargetID);
		if (pos == objects_by_id.end())
			return false;

		if (!resolved_path.SetValue(pos->second))
			return false;
	}

	return true;
}


int main(int argc, char* argv[])
{
	Logger::setLevel(Logger::debugLevel());

	Core core;
	core.initialize();

	// Create test hierarchy
	BaseClass* root = createTestHierarchy();
	 
	// Create path to float property in array of nested compounds
 	RTTI::RTTIPath float_property_path;
 	float_property_path.PushAttribute("ArrayOfCompounds");
 	float_property_path.PushArrayElement(0);
 	float_property_path.PushAttribute("FloatProperty");
 
	// Convert path to string
 	std::string path_str = float_property_path.ToString();

	// Convert back and verify the path is the same
 	RTTI::RTTIPath path_copy = RTTI::RTTIPath::FromString(path_str);
	if (path_copy != float_property_path)
		return -1;
 
	// Resolve the path and verify it succeeded
 	RTTI::ResolvedRTTIPath resolved_path = float_property_path.Resolve(root->mPointerProperty);
	if (!resolved_path.IsValid())
		return -1;

	// Verify setting the value works
	float old_value = resolved_path.GetCurrentValue().convert<float>();
	if (!resolved_path.SetValue(8.0f))
		return -1; 

	// Restore value so we can compare later
	resolved_path.SetValue(old_value);

	// Write to json
	JSONWriter writer;
	if (!serializeObjects({ root }, writer))
		return -1;

	// Print json
	std::string json = writer.GetJSON();
	std::cout << json << std::endl;

	// Read json and verify it succeeds
	ReadJSONFileResult read_result;
	InitResult init_result;
	if (!readJSON(json, read_result, init_result))
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

	return 0;
}