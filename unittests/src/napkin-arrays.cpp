#include "utils/include.h"

#include <appcontext.h>
#include <commands.h>

using namespace napkin;

TEST_CASE("Arrays", "napkin-arrays")
{
	napkin::AppContext::create();
	RUN_Q_APPLICATION

	SECTION("Array Value Elements")
	{
		auto doc = AppContext::get().newDocument();
		auto resource = doc->addObject<TestResource>();
		REQUIRE(resource != nullptr);

		// Check invalid nonexistent path
		PropertyPath nonExistent(resource->mID, "NonExistent_________", *doc);
		REQUIRE(!nonExistent.isValid());

		// Grab a valid path
		PropertyPath ints2D(resource->mID, "Ints2D", *doc);
		REQUIRE(ints2D.isValid());

		// Ensure empty array
		REQUIRE(ints2D.getArrayLength() == 0);

		// Add an element
		{
			auto index = doc->arrayAddValue(ints2D);
			REQUIRE(index == 0); // The index at which the new element lives
			REQUIRE(ints2D.getArrayLength() == 1);

			// Verify validity of new element
			REQUIRE(PropertyPath(resource->mID, "Ints2D/0", *doc).isValid());
//		 TODO: This should not happen
//		REQUIRE(!PropertyPath(*resource, "Ints2D/18").isValid());
		}

		// Add another element
		{
			auto index = doc->arrayAddValue(ints2D);
			REQUIRE(index == 1); // Index should be one
			REQUIRE(ints2D.getArrayLength() == 2);

			// Verify validity of new element
			REQUIRE(PropertyPath(resource->mID, "Ints2D/0", *doc).isValid());
			REQUIRE(PropertyPath(resource->mID, "Ints2D/1", *doc).isValid());
		}

		// Remove first element
		{
			doc->arrayRemoveElement(ints2D, 0);
			REQUIRE(ints2D.getArrayLength() == 1);
		}

		// Remove the second element
		{
			doc->arrayRemoveElement(ints2D, 0);
			REQUIRE(ints2D.getArrayLength() == 0);
		}
	}

	SECTION("Array add string")
	{
		auto doc = AppContext::get().newDocument();
		auto midiinput = doc->addObject<TestResource>();
		REQUIRE(midiinput != nullptr);
		PropertyPath strings(midiinput->mID, "Strings", *doc);
		REQUIRE(strings.isValid());
		REQUIRE(strings.isArray());
		REQUIRE(!strings.isPointer());
		REQUIRE(strings.getArrayElementType() == RTTI_OF(std::string));

		auto index = doc->arrayAddValue(strings);
		REQUIRE(index == 0);
	}

	SECTION("Array Modification Objects")
	{
		auto doc = AppContext::get().newDocument();
		auto resource = doc->addObject<TestResourceB>();
		REQUIRE(resource != nullptr);
		auto pointee = doc->addObject<TestResource>();
		REQUIRE(pointee != nullptr);

		PropertyPath respointers(resource->mID, "ResPointers", *doc);
		REQUIRE(respointers.isValid());
		REQUIRE(respointers.getArrayLength() == 0);

		// Add the existing layer to the composition
		{
			auto index = doc->arrayAddExistingObject(respointers, pointee);
			REQUIRE(index == 0);
			REQUIRE(respointers.getArrayLength() == 1);
		}

		// Add a new layer to the composition
		{
			auto index = doc->arrayAddNewObject(respointers, RTTI_OF(TestResource));
			REQUIRE(index == 1);
			REQUIRE(respointers.getArrayLength() == 2);
		}

	}

	SECTION("Array Move Element")
	{
		auto doc = AppContext::get().newDocument();
		auto resource = doc->addObject<TestResourceB>();
		REQUIRE(resource != nullptr);

		// Set up an array with four elements
		PropertyPath pointers(resource->mID, "ResPointers", *doc);
		REQUIRE(pointers.isValid());
		doc->arrayAddNewObject(pointers, RTTI_OF(TestResource));
		doc->arrayAddNewObject(pointers, RTTI_OF(TestResource));
		doc->arrayAddNewObject(pointers, RTTI_OF(TestResource));
		doc->arrayAddNewObject(pointers, RTTI_OF(TestResource));
		REQUIRE(pointers.getArrayLength() == 4);
		auto layer0 = doc->arrayGetElement<TestResource*>(pointers, 0);
		auto layer1 = doc->arrayGetElement<TestResource*>(pointers, 1);
		auto layer2 = doc->arrayGetElement<TestResource*>(pointers, 2);
		auto layer3 = doc->arrayGetElement<TestResource*>(pointers, 3);
		REQUIRE(layer0 != nullptr);
		REQUIRE(layer1 != nullptr);
		REQUIRE(layer2 != nullptr);
		REQUIRE(layer3 != nullptr);
		// State: 0, 1, 2, 3

		size_t new_index = doc->arrayMoveElement(pointers, 2, 1);
		// State: 0, [2], 1, 3

		REQUIRE(new_index == 1);
		REQUIRE(pointers.getArrayLength() == 4);
		auto variant = doc->arrayGetElement(pointers, new_index);
		REQUIRE(variant.is_valid());
		auto vlayer = variant.convert<TestResource*>();
		REQUIRE(vlayer != nullptr);


		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer0);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer1);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer3);

		doc->arrayMoveElement(pointers, 0, 4);
		// State: 2, 1, 3, [0]

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer1);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer0);

		doc->executeCommand(new ArrayMoveElementCommand(pointers, 1, 3));
		// State: 2, 3, [1], 0

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer1);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer0);

		doc->undo();
		// State: 2, [1], 3, 0

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer1);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer0);

		doc->redo();
		// State: 2, 3, [1], 0

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer1);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer0);

		doc->executeCommand(new ArrayMoveElementCommand(pointers, 3, 1));
		// State: 2, [0], 3, 1

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer0);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer1);

		doc->undo();
		// State: 2, 3, 1, [0]

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer1);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer0);

		doc->redo();
		// State: 2, [0], 3, 1

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer0);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer1);

		doc->undo();
		// State: 2, 3, 1, [0]
		doc->undo();
		// State: 2, [1], 3, 0

		REQUIRE(pointers.getArrayLength() == 4);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 0) == layer2);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 1) == layer1);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 2) == layer3);
		REQUIRE(doc->arrayGetElement<TestResource*>(pointers, 3) == layer0);

	}
	napkin::AppContext::destroy();
}