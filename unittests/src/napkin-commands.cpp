#include "utils/include.h"

#include <appcontext.h>
#include <commands.h>

using namespace napkin;


TEST_CASE("Commands", "napkin-commands")
{
	napkin::AppContext::create();
	RUN_Q_APPLICATION

	auto& ctx = AppContext::get();
	ctx.newDocument(); // First clear, previous test might have a document

	SigCapture sigDocChanged(&ctx, &AppContext::documentChanged);
	int sigDocCount = 0;
	
	/*
	REQUIRE(sigDocChanged.count() == sigDocCount);

	auto loadeddoc = ctx.loadDocument("unit_tests_data/objects.json");
	{
		REQUIRE(loadeddoc);
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		auto object = loadeddoc->getObject("material");

	}
	*/

	auto doc = ctx.newDocument();
	REQUIRE(sigDocChanged.count() == ++sigDocCount);

	// Add an object and verify
	ctx.executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity)));
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

	nap::rtti::Object* entity1 = doc->getObjects()[0].get();
	nap::Entity* e1 = rtti_cast<nap::Entity>(entity1);
	REQUIRE(e1 != nullptr);

	// Add another one and verify
	ctx.executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity), entity1));
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

	nap::rtti::Object* entity2 = doc->getObjects()[1].get();
	nap::Entity* e2 = rtti_cast<nap::Entity>(entity2);
	REQUIRE(e2 != nullptr);
	REQUIRE(doc->getParent(*e2) == e1);

	{
		PropertyPath nameProp1(entity1->mID, nap::rtti::sIDPropertyName, *doc);
		std::string namepropType(nameProp1.getType().get_name().data());
		REQUIRE(nameProp1.getType().is_derived_from<std::string>());
		REQUIRE(nameProp1.isValid());
		PropertyPath nameProp2(entity2->mID, nap::rtti::sIDPropertyName, *doc);
		REQUIRE(nameProp2.isValid());
		REQUIRE(nameProp2.getType().is_derived_from<std::string>());

		auto nameproppath = nameProp1.toString();

		ctx.executeCommand(new SetValueCommand(nameProp1, "Loco"));
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(entity1->mID == "Loco");

		assert(nameProp1.getObject());

		// Name may not be empty, should have been reverted to previous value
		ctx.executeCommand(new SetValueCommand(nameProp1, ""));
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(entity1->mID == "Loco");

		// No duplicate names
		ctx.executeCommand(new SetValueCommand(nameProp2, "Loco"));
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(entity2->mID != "Loco");

		// Setting a unique name should succeed
		ctx.executeCommand(new SetValueCommand(nameProp2, "Motion"));
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(entity2->mID == "Motion");

		// Remove object and verify
		ctx.executeCommand(new DeleteObjectCommand(*entity1));
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(doc->getObjects().size() == 1);

		// Remove next and verify
		ctx.executeCommand(new DeleteObjectCommand(*entity2));
		REQUIRE(sigDocChanged.count() == ++sigDocCount);
		REQUIRE(doc->getObjects().size() == 0);

		// Add a component (crashes OSX?)
		auto& entity = doc->addEntity(nullptr);
		ctx.executeCommand(new AddComponentCommand(entity, RTTI_OF(TestComponent)));
		REQUIRE(entity.hasComponent<TestComponent>());
		auto component = doc->getComponent(entity, RTTI_OF(TestComponent));
		REQUIRE(component != nullptr);
		ctx.executeCommand(new RemoveComponentCommand(*component));
		REQUIRE(!entity.hasComponent<TestComponent>());

	}
	napkin::AppContext::destroy();
}