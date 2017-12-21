#include "catch.hpp"

#include <appcontext.h>
#include <commands.h>
#include <composition.h>
#include <ledcolorpalette.h>

using namespace napkin;
using namespace nap;

class SigCapture
{
public:
	template <typename S, typename F>
	SigCapture(S* sender, F f) {
		QObject::connect(sender, f, [this]() {
			mCount++;
		});
	}

	int count() { return mCount; }

private:
	int mCount = 0;
};

TEST_CASE("Document Management", "[napkin]")
{
	auto doc = AppContext::get().getDocument();

	// Must have a default document
	REQUIRE(doc != nullptr);
	// Default filename must be empty
	REQUIRE(doc->getCurrentFilename().isEmpty());
	// Default document may not have objects
	REQUIRE(doc->getObjects().size() == 0);
	// Setting filename must be consistent
	QString testFilename("TestFilename.ext");
	doc->setFilename(testFilename);
	REQUIRE(doc->getCurrentFilename() == testFilename);

	// Track signal
	doc = AppContext::get().newDocument();

	REQUIRE(doc->getCurrentFilename().isEmpty());
	REQUIRE(doc->getObjects().size() == 0);
}

TEST_CASE("Document Signals", "[napkin]")
{
	auto doc = AppContext::get().newDocument();
	SigCapture sigObjectAdded(doc, &Document::objectAdded);
	SigCapture sigObjectRemoved(doc, &Document::objectRemoved);
	SigCapture sigObjectChanged(doc, &Document::objectChanged);
	SigCapture sigPropertyValueChanged(doc, &Document::propertyValueChanged);

	auto entity = doc->addObject<nap::Entity>();
	REQUIRE(sigObjectAdded.count() == 1);
	doc->removeObject(entity->mID);
	REQUIRE(sigObjectRemoved.count() == 1);

}

TEST_CASE("Array Value Elements", "[napkin]")
{
	auto doc = AppContext::get().newDocument();
	auto palette = doc->addObject<nap::LedColorPalette>();
	REQUIRE(palette != nullptr);

	// Check invalid nonexistent path
	PropertyPath nonExistent(*palette, "NonExistent_________");
	REQUIRE(!nonExistent.isValid());

	// Grab a valid path
	PropertyPath ledColors(*palette, "LedColors");
	REQUIRE(ledColors.isValid());

	// Check if the we have an empty array
	{
		rtti::Variant ledcolorsvalue = ledColors.resolve().getValue();
		rtti::VariantArray ledcolors_array = ledcolorsvalue.create_array_view();
		REQUIRE(ledcolors_array.get_size() == 0);
	}

	// Add an element
	{
		auto index = doc->arrayAddValue(ledColors);

		REQUIRE(index == 0); // The index at which the new element lives
		auto value = ledColors.getValue();
		auto array_view = value.create_array_view();
		REQUIRE(array_view.get_size() == 1); // Current array size
	}

	// Add another element
	{
		auto index = doc->arrayAddValue(ledColors);

		REQUIRE(index == 1); // Index should be one
		auto value = ledColors.getValue();
		auto array_view = value.create_array_view();
		REQUIRE(array_view.get_size() == 2); // Current array size
	}

	// Remove first element
	{
		doc->arrayRemoveElement(ledColors, 0);

		auto value = ledColors.getValue();
		auto array = value.create_array_view();
		REQUIRE(array.get_size() == 1);
	}

	// Remove the second element
	{
		doc->arrayRemoveElement(ledColors, 0);

		auto value = ledColors.getValue();
		auto array = value.create_array_view();
		REQUIRE(array.get_size() == 0);
	}
}

TEST_CASE("Array Modification Objects", "[napkin]")
{
	auto doc = AppContext::get().newDocument();
	auto composition = doc->addObject<nap::Composition>();
	REQUIRE(composition != nullptr);
	auto layer = doc->addObject<nap::ImageSequenceLayer>();
	REQUIRE(layer != nullptr);

	PropertyPath layers(*composition, "Layers");
	REQUIRE(layers.isValid());

 	// Check if array is empty to start with
	{
		rtti::Variant value = layers.resolve().getValue();
		rtti::VariantArray array_view = value.create_array_view();
		REQUIRE(array_view.get_size() == 0);
	}

	// Add the existing layer to the composition
	{
		auto index = doc->arrayAddExistingObject(layers, layer);
		REQUIRE(index == 0);

		rtti::Variant value = layers.resolve().getValue();
		rtti::VariantArray array_view = value.create_array_view();
		REQUIRE(array_view.get_size() == 1);
	}

	// Add a new layer to the composition
	{
		auto index = doc->arrayAddNewObject(layers, RTTI_OF(nap::ImageSequenceLayer));
		REQUIRE(index == 1);

		rtti::Variant value = layers.resolve().getValue();
		rtti::VariantArray array_view = value.create_array_view();
		REQUIRE(array_view.get_size() == 2);
	}

}

TEST_CASE("Document Functions", "[napkin]")
{
	auto doc = AppContext::get().newDocument();

	auto entity = doc->addObject<nap::Entity>();

	REQUIRE(entity != nullptr); // Entity cannot be null
	REQUIRE(!entity->mID.empty()); // Must have a name
	REQUIRE(doc->getObjects().size() == 1); // Object count must have gone up
	auto foundEntity = doc->getObject(entity->mID);
	REQUIRE(foundEntity != nullptr); // Must be able to find this entity
	REQUIRE(foundEntity == entity); // The entity must match

	auto entity2 = doc->addObject<nap::Entity>();
	REQUIRE(entity2 != nullptr); // Second entity must not be null
	REQUIRE(entity != entity2); //
	REQUIRE(entity->mID != entity2->mID); // Objects must have unique names

}

TEST_CASE("PropertyPath", "[napkin]")
{
	auto doc = AppContext::get().newDocument();
	auto entity = doc->addObject<nap::Entity>();
	PropertyPath nameProp(*entity, rtti::sIDPropertyName);
	REQUIRE(&nameProp.object() == entity);
	REQUIRE(nameProp.isValid());
	std::string newName = "NewName";
	nameProp.setValue(newName);
	REQUIRE(nameProp.getValue() == newName);
	REQUIRE(entity->mID == newName);
}

TEST_CASE("Commands", "[napkin]")
{
	auto& ctx = AppContext::get();
	auto doc = ctx.newDocument();

	// Add an object and verify
	ctx.executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity)));
	REQUIRE(doc->getObjects().size() == 1);
	{
		doc->undo();
		REQUIRE(doc->getObjects().size() == 0);
		doc->redo();
		REQUIRE(doc->getObjects().size() == 1);
	}

	nap::rtti::RTTIObject* entity1 = doc->getObjects()[0].get();
	nap::Entity* e1 = rtti_cast<nap::Entity>(entity1);
	REQUIRE(e1 != nullptr);

	// Add another one and verify
	ctx.executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity), entity1));
	REQUIRE(doc->getObjects().size() == 2);
	{
		doc->undo();
		REQUIRE(doc->getObjects().size() == 1);
		doc->redo();
		REQUIRE(doc->getObjects().size() == 2);
	}

	nap::rtti::RTTIObject* entity2 = doc->getObjects()[1].get();
	nap::Entity* e2 = rtti_cast<nap::Entity>(entity2);
	REQUIRE(e2 != nullptr);
	REQUIRE(doc->getParent(*e2) == e1);

	PropertyPath nameProp1(*entity1, rtti::sIDPropertyName);
	REQUIRE(nameProp1.isValid());
	PropertyPath nameProp2(*entity2, rtti::sIDPropertyName);
	REQUIRE(nameProp2.isValid());

	ctx.executeCommand(new SetValueCommand(nameProp1, "Loco"));
	REQUIRE(entity1->mID == "Loco");

	// Name may not be empty
	ctx.executeCommand(new SetValueCommand(nameProp1, ""));
	REQUIRE(entity1->mID == "Loco");

	// No duplicate names
	ctx.executeCommand(new SetValueCommand(nameProp2, "Loco"));
	REQUIRE(entity2->mID != "Loco");

	// Setting a unique name should succeed
	ctx.executeCommand(new SetValueCommand(nameProp2, "Motion"));
	REQUIRE(entity2->mID == "Motion");

	// Remove object and verify
	ctx.executeCommand(new DeleteObjectCommand(*entity1));
	REQUIRE(doc->getObjects().size() == 1);

	// Remove next and verify
	ctx.executeCommand(new DeleteObjectCommand(*entity2));
	REQUIRE(doc->getObjects().size() == 0);

	// TODO: Support undo for deletion
//	doc->undo();
//	REQUIRE(doc->getObjects().size() == 1);
//
//	doc->undo();
//	REQUIRE(doc->getObjects().size() == 2);

//	ctx.executeCommand(new SetPointerValueCommand())
//	ctx.executeCommand(new AddEntityToSceneCommand())
//	ctx.executeCommand(new ArrayAddValueCommand());
}

TEST_CASE("QT Specific", "[napkin]")
{

}