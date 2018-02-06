#include "catch.hpp"

#include <appcontext.h>
#include <commands.h>
#include <composition.h>
#include <ledcolorpalette.h>
#include <ledcolorpalettegrid.h>
#include <imagelayer.h>


#define TAG_NAPKIN "[napkin]"

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

TEST_CASE("Document Management", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().getDocument();

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

	doc = napkin::AppContext::get().newDocument();

	REQUIRE(doc->getCurrentFilename().isEmpty());
	REQUIRE(doc->getObjects().size() == 0);


}

TEST_CASE("Document Signals", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().newDocument();
	SigCapture sigObjectAdded(doc, &napkin::Document::objectAdded);
	SigCapture sigObjectRemoved(doc, &napkin::Document::objectRemoved);
	SigCapture sigObjectChanged(doc, &napkin::Document::objectChanged);
	SigCapture sigPropertyValueChanged(doc, &napkin::Document::propertyValueChanged);

	auto entity = doc->addObject<nap::Entity>();
	REQUIRE(sigObjectAdded.count() == 1);
	doc->removeObject(entity->mID);
	REQUIRE(sigObjectRemoved.count() == 1);

}

TEST_CASE("Array Value Elements", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().newDocument();
	auto palette = doc->addObject<nap::LedColorPalette>();
	REQUIRE(palette != nullptr);

	// Check invalid nonexistent path
	napkin::PropertyPath nonExistent(*palette, "NonExistent_________");
	REQUIRE(!nonExistent.isValid());

	// Grab a valid path
	napkin::PropertyPath ledColors(*palette, "LedColors");
	REQUIRE(ledColors.isValid());

	// Ensure empty array
	REQUIRE(ledColors.getArrayLength() == 0);

	// Add an element
	{
		auto index = doc->arrayAddValue(ledColors);
		REQUIRE(index == 0); // The index at which the new element lives
		REQUIRE(ledColors.getArrayLength() == 1);
	}

	// Add another element
	{
		auto index = doc->arrayAddValue(ledColors);
		REQUIRE(index == 1); // Index should be one
		REQUIRE(ledColors.getArrayLength() == 2);
	}

	// Remove first element
	{
		doc->arrayRemoveElement(ledColors, 0);
		REQUIRE(ledColors.getArrayLength() == 1);
	}

	// Remove the second element
	{
		doc->arrayRemoveElement(ledColors, 0);
		REQUIRE(ledColors.getArrayLength() == 0);
	}
}

TEST_CASE("Array add weekcolor", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().newDocument();
	auto* col = doc->addObject<nap::WeekColors>();
	REQUIRE(col != nullptr);

	napkin::PropertyPath variations(*col, "Variations");
	REQUIRE(variations.isValid());

	// Add an element to the array
	{
		auto index = doc->arrayAddValue(variations);
		REQUIRE(index == 0);
		REQUIRE(variations.getArrayLength() == 1);

		// Verify validity of new element
		REQUIRE(napkin::PropertyPath(*col, "Variations/0").isValid());
	}

	// Add another value
	{
		auto index = doc->arrayAddValue(variations);
		REQUIRE(index == 1);
		REQUIRE(variations.getArrayLength() == 2);

		// Verify validity of new element
		REQUIRE(napkin::PropertyPath(*col, "Variations/1").isValid());
	}
}

TEST_CASE("Array Modification Objects", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().newDocument();
	auto composition = doc->addObject<nap::Composition>();
	REQUIRE(composition != nullptr);
	auto layer = doc->addObject<nap::ImageLayer>();
	REQUIRE(layer != nullptr);

	napkin::PropertyPath layers(*composition, "Layers");
	REQUIRE(layers.isValid());
	REQUIRE(layers.getArrayLength() == 0);

	// Add the existing layer to the composition
	{
		auto index = doc->arrayAddExistingObject(layers, layer);
		REQUIRE(index == 0);
		REQUIRE(layers.getArrayLength() == 1);
	}

	// Add a new layer to the composition
	{
		auto index = doc->arrayAddNewObject(layers, RTTI_OF(nap::ImageSequenceLayer));
		REQUIRE(index == 1);
		REQUIRE(layers.getArrayLength() == 2);
	}

}

TEST_CASE("Array Move Element", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().newDocument();
	auto composition = doc->addObject<nap::Composition>();
	REQUIRE(composition != nullptr);

	// Set up an array with four elements
	napkin::PropertyPath layers(*composition, "Layers");
	REQUIRE(layers.isValid());
	doc->arrayAddNewObject(layers, RTTI_OF(nap::ImageSequenceLayer));
	doc->arrayAddNewObject(layers, RTTI_OF(nap::ImageSequenceLayer));
	doc->arrayAddNewObject(layers, RTTI_OF(nap::ImageSequenceLayer));
	doc->arrayAddNewObject(layers, RTTI_OF(nap::ImageSequenceLayer));
	REQUIRE(layers.getArrayLength() == 4);
	auto layer0 = doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0);
	auto layer1 = doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1);
	auto layer2 = doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2);
	auto layer3 = doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3);
	REQUIRE(layer0 != nullptr);
	REQUIRE(layer1 != nullptr);
	REQUIRE(layer2 != nullptr);
	REQUIRE(layer3 != nullptr);
	// State: 0, 1, 2, 3

	long new_index = doc->arrayMoveElement(layers, 2, 1);
	// State: 0, [2], 1, 3

	REQUIRE(new_index == 1);
	REQUIRE(layers.getArrayLength() == 4);
	auto variant = doc->arrayGetElement(layers, new_index);
	REQUIRE(variant.is_valid());
	auto vlayer = variant.convert<nap::ImageSequenceLayer*>();
	REQUIRE(vlayer != nullptr);

	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer0);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer1);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer3);

	doc->arrayMoveElement(layers, 0, 4);
	// State: 2, 1, 3, [0]

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer1);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer0);

	doc->executeCommand(new napkin::ArrayMoveElementCommand(layers, 1, 3));
	// State: 2, 3, [1], 0

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer1);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer0);

	doc->undo();
	// State: 2, [1], 3, 0

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer1);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer0);

	doc->redo();
	// State: 2, 3, [1], 0

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer1);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer0);

	doc->executeCommand(new napkin::ArrayMoveElementCommand(layers, 3, 1));
	// State: 2, [0], 3, 1

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer0);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer1);

	doc->undo();
	// State: 2, 3, 1, [0]

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer1);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer0);

	doc->redo();
	// State: 2, [0], 3, 1

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer0);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer1);

	doc->undo();
	// State: 2, 3, 1, [0]
	doc->undo();
	// State: 2, [1], 3, 0

	REQUIRE(layers.getArrayLength() == 4);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 0) == layer2);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 1) == layer1);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 2) == layer3);
	REQUIRE(doc->arrayGetElement<nap::ImageSequenceLayer*>(layers, 3) == layer0);

}

TEST_CASE("Document Functions", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().newDocument();

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

TEST_CASE("PropertyPath", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().newDocument();
	auto entity = doc->addObject<nap::Entity>();
	napkin::PropertyPath nameProp(*entity, nap::rtti::sIDPropertyName);
	REQUIRE(&nameProp.object() == entity);
	REQUIRE(nameProp.isValid());
	std::string newName = "NewName";
	nameProp.setValue(newName);
	REQUIRE(nameProp.getValue() == newName);
	REQUIRE(entity->mID == newName);
}

TEST_CASE("Commands", TAG_NAPKIN)
{
	auto& ctx = napkin::AppContext::get();
	SigCapture sigDocChanged(&ctx, &napkin::AppContext::documentChanged);
	int sigDocCount = 0;
	REQUIRE(sigDocChanged.count() == sigDocCount);

	auto loadeddoc = ctx.loadDocument("unit_tests_data/objects.json");
	{
		REQUIRE(loadeddoc);
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		auto object = loadeddoc->getObject("material");

	}

	auto doc = ctx.newDocument();
	REQUIRE(sigDocChanged.count() == ++sigDocCount);

	// Add an object and verify
	ctx.executeCommand(new napkin::AddObjectCommand(RTTI_OF(nap::Entity)));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(doc->getObjects().size() == 1);
	{
		doc->undo();
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(doc->getObjects().size() == 0);
		doc->redo();
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(doc->getObjects().size() == 1);
	}

	nap::rtti::RTTIObject* entity1 = doc->getObjects()[0].get();
	nap::Entity* e1 = rtti_cast<nap::Entity>(entity1);
	REQUIRE(e1 != nullptr);

	// Add another one and verify
	ctx.executeCommand(new napkin::AddObjectCommand(RTTI_OF(nap::Entity), entity1));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(doc->getObjects().size() == 2);
	{
		doc->undo();
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(doc->getObjects().size() == 1);
		doc->redo();
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(doc->getObjects().size() == 2);
	}

	nap::rtti::RTTIObject* entity2 = doc->getObjects()[1].get();
	nap::Entity* e2 = rtti_cast<nap::Entity>(entity2);
	REQUIRE(e2 != nullptr);
	REQUIRE(doc->getParent(*e2) == e1);

	napkin::PropertyPath nameProp1(*entity1, nap::rtti::sIDPropertyName);
	REQUIRE(nameProp1.isValid());
	napkin::PropertyPath nameProp2(*entity2, nap::rtti::sIDPropertyName);
	REQUIRE(nameProp2.isValid());

	ctx.executeCommand(new napkin::SetValueCommand(nameProp1, "Loco"));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(entity1->mID == "Loco");

	// Name may not be empty
	ctx.executeCommand(new napkin::SetValueCommand(nameProp1, ""));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(entity1->mID == "Loco");

	// No duplicate names
	ctx.executeCommand(new napkin::SetValueCommand(nameProp2, "Loco"));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(entity2->mID != "Loco");

	// Setting a unique name should succeed
	ctx.executeCommand(new napkin::SetValueCommand(nameProp2, "Motion"));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(entity2->mID == "Motion");

	// Remove object and verify
	ctx.executeCommand(new napkin::DeleteObjectCommand(*entity1));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(doc->getObjects().size() == 1);

	// Remove next and verify
	ctx.executeCommand(new napkin::DeleteObjectCommand(*entity2));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
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

TEST_CASE("File Extensions", TAG_NAPKIN)
{
    napkin::ResourceFactory fact = napkin::AppContext::get().getResourceFactory();
    {
        QStringList imageExtensions;
        for (const auto& e : fact.getImageExtensions())
            imageExtensions << e;
		REQUIRE(imageExtensions.size() > 0);

        auto imageExtString = "Image Extensions: " + imageExtensions.join(", ");
        REQUIRE(!imageExtString.isEmpty());

		nap::Logger::debug(imageExtString.toStdString());
    }
    {
        QStringList videoExtensions;
        for (const auto& e : fact.getVideoExtensions())
            videoExtensions << e;
		REQUIRE(videoExtensions.size() > 0);

        auto videoExtString = "Video Extensions: " + videoExtensions.join(", ");
        REQUIRE(!videoExtString.isEmpty());

        nap::Logger::debug(videoExtString.toStdString());
    }
}

//TEST_CASE("Resource Management", TAG_NAPKIN)
//{
//	auto doc = napkin::AppContext::get().loadDocument("data/kalvertoren.json");
//	REQUIRE(doc != nullptr);
//
//}