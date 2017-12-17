#include "catch.hpp"

#include <appcontext.h>
#include <commands.h>

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
//	QSignalSpy spy(&AppContext::get(), &AppContext::newFileCreated);

	doc = AppContext::get().newDocument();

//	REQUIRE(spy.count() == 1); // newFileCreated() must be emitted once

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
	PropertyPath nameProp(*entity, "mID");
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

	ctx.executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity)));
	auto entity1 = doc->getObjects()[0].get();
	ctx.executeCommand(new AddObjectCommand(RTTI_OF(nap::Entity), entity1));
	auto entity2 = doc->getObjects()[1].get();
	ctx.executeCommand(new DeleteObjectCommand(*entity2));
	PropertyPath nameProp(*entity1, "mID");
	ctx.executeCommand(new SetValueCommand(nameProp, "NewName"));
//	ctx.executeCommand(new SetPointerValueCommand())
//	ctx.executeCommand(new AddEntityToSceneCommand())
//	ctx.executeCommand(new AddArrayElementCommand());
}