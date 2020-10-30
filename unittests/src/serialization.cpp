#include "utils/catch.hpp"

#include <fstream>
#include <nap/core.h>
#include <nap/logger.h>
#include <rtti/defaultlinkresolver.h>
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <utility/fileutils.h>

using namespace nap::rtti;
using namespace nap::utility;


std::string readJSONData(const std::string& filename)
{
	ErrorState err;

	std::string jsonData;
	if (!readFileToString(filename, jsonData, err))
		FAIL("Failed to load file: " + filename + ": " + err.toString());

	return jsonData;
}

void deserialize(nap::Core& core, const std::string& jsonData, OwnedObjectList& outObjects) {
	ErrorState err;

	auto& factory = core.getResourceManager()->getFactory();

	nap::rtti::DeserializeResult result;

	if (!deserializeJSON(jsonData, EPropertyValidationMode::DisallowMissingProperties, EPointerPropertyMode::AllPointerTypes, factory, result, err))
		FAIL("Failed to deserialize json data: " + err.toString());

	if (!DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, err))
		FAIL("Failed to resolve links: " + err.toString());

	// Transfer ownership to given vector
	for (auto& ob : result.mReadObjects)
		outObjects.emplace_back(std::move(ob));
}

std::string serialize(const OwnedObjectList& objects) {
	// Got raw pointers?
	ObjectList objPointers;
	for (auto& ob : objects)
		objPointers.emplace_back(ob.get());

	// Dump objects into tree
	ErrorState err;
	JSONWriter writer;
	if (!serializeObjects(objPointers, writer, err))
		FAIL("Failed to serialize objects: " + err.toString());
	return writer.GetJSON();
}

void serializeDeserializeTest(const std::string& filename)
{
	// Initialize core engine
	nap::Core core;
	{
		ErrorState err;
		if (!core.initializeEngine(err))
			FAIL("Failed to initialize engine: " + err.toString());
	}

	// Read our test data
	std::string jsonTestData = readJSONData(filename);
	REQUIRE(!jsonTestData.empty());

	// A. deserialize the loaded test data
	OwnedObjectList readObjectsA;
	deserialize(core, jsonTestData, readObjectsA);

	// Now serialize again
	std::string engineSerializedA = serialize(readObjectsA);

	// B. deserialize again so we can compare strings later
	OwnedObjectList readObjectsB;
	deserialize(core, engineSerializedA, readObjectsB);

	// TODO: Compare both object lists here

	// Serialize for the second time and compare
	std::string engineSerializedB = serialize(readObjectsB);

	// Serializing the same data twice in a row must yield the same result
	REQUIRE(engineSerializedA == engineSerializedB);
}

// TODO: Add more fine-grained tests to spot problems quicker
// (The ones below are fairly large for testing purposes)

//TEST_CASE("JSON serialization (Tommy)", "[serialization]")
//{
//	Logger::setLevel(Logger::infoLevel()); //< Hide some debug data
//	serializeDeserializeTest("objects.json");
//}
//
//TEST_CASE("JSON serialization (Kalvertoren)", "[serialization]")
//{
//	Logger::setLevel(Logger::infoLevel()); //< Hide some debug data
//	serializeDeserializeTest("kalvertoren.json");
//}
