#include "utils/include.h"
#include <QString>
#include <appcontext.h>

using namespace napkin;

TEST_CASE("Document", "napkin-document")
{
	napkin::AppContext::create();
    RUN_Q_APPLICATION

	SECTION("general")
	{
		/*
		auto doc = AppContext::get().newDocument();

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

		doc = AppContext::get().newDocument();

		REQUIRE(doc->getCurrentFilename().isEmpty());
		REQUIRE(doc->getObjects().size() == 0);

		// Create entity
		auto& e = doc->addEntity();
		REQUIRE(!e.mID.empty());
		REQUIRE(doc->getObjects().size() == 1);
		REQUIRE(e.getComponents().size() == 0);

		// Add component to entity
		auto comp = doc->addComponent<TestComponent>(e);
		REQUIRE(comp != nullptr);
		REQUIRE(doc->getObjects().size() == 2);
		REQUIRE(e.getComponents().size() == 1);
		REQUIRE(doc->getOwner(*comp) == &e);

		// Add another component
		auto xfcomp = doc->addComponent<TestComponent>(e);
		REQUIRE(xfcomp != nullptr);
		REQUIRE(doc->getObjects().size() == 3);
		REQUIRE(e.getComponents().size() == 2);
		REQUIRE(doc->getOwner(*xfcomp) == &e);

		// Remove first component (from entity)
		doc->removeObject(*comp);
		REQUIRE(doc->getObjects().size() == 2);
		REQUIRE(e.getComponents().size() == 1);

		// Remove entity (should also remove component)
		doc->removeObject(e);
		REQUIRE(doc->getObjects().size() == 0);
		*/
	}

	SECTION("signals")
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

	SECTION("functions")
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

	napkin::AppContext::destroy();
}