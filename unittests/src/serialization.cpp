

#include "catch.hpp"

#include <nap/resourcemanager.h>
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>
#include <nap/logger.h>
#include <nap/core.h>
#include <rtti/defaultlinkresolver.h>

using namespace nap::rtti;
using namespace nap::utility;


void loadAndSaveFile(const std::string& filename)
{
	// Load File

	ErrorState err;

	nap::Core core;

	if (!core.initializeEngine(err)) {
		FAIL("Failed to initialize engine: " + err.toString());
	}
	auto& factory = core.getResourceManager()->getFactory();
	nap::rtti::RTTIDeserializeResult result;
	if (!readJSONFile(filename, factory, result, err)) {
		FAIL("Failed to read JSON file '" + filename + "': " + err.toString());
	}


	if (!DefaultLinkResolver::sResolveLinks(result.mReadObjects, result.mUnresolvedPointers, err)) {
		FAIL("Failed to resolve links: " + err.toString());
	}

	// Save File
	ObjectList objects;
	for (auto& ob : result.mReadObjects) {
		objects.emplace_back(ob.get());
	}

	JSONWriter writer;
	if (!serializeObjects(objects, writer, err)) {
		nap::Logger::fatal(err.toString());
		return;
	}

//	std::ofstream out(filename);
//	out << writer.GetJSON();
//	out.close();
}

TEST_CASE("Check JSON serialization", "[jsonserialize]")
{
	std::string filename ="data/tommy.json";
	loadAndSaveFile(filename);
}

