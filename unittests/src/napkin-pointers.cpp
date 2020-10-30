#include "utils/include.h"

#include <appcontext.h>

using namespace napkin;

TEST_CASE("Component to Component pointer", "napkin-pointers")
{
	/*
	napkin::AppContext::create();
	RUN_Q_APPLICATION
	
	{
		auto& ctx = napkin::AppContext::get();
		std::string serializedData;

		auto doc = ctx.newDocument();

		nap::Entity& entity = doc->addEntity(nullptr);
		doc->setObjectName(entity, ENTITY_NAME);
		REQUIRE(entity.mID == ENTITY_NAME);

		// Holds the pointer
		auto compB = doc->addComponent<TestComponentB>(entity);
		REQUIRE(compB != nullptr);
		REQUIRE(!compB->mID.empty());

		// Pointee
		auto comp = doc->addComponent<TestComponent>(entity);
		REQUIRE(comp != nullptr);
		REQUIRE(!compB->mID.empty());

		std::unique_ptr<napkin::PropertyPath> pointerPath = std::make_unique<napkin::PropertyPath>(compB->mID, "CompPointer", *doc);
		REQUIRE(pointerPath->isValid());

		pointerPath->setPointee(comp);
		auto pointee = pointerPath->getPointee();
		REQUIRE(pointee == comp);

		serializedData = ctx.documentToString();
		REQUIRE(!serializedData.empty());

		// Explicitly delete property path here, new doc is loaded
		pointerPath.reset(nullptr);

		auto loadedDoc = ctx.loadDocumentFromString(serializedData);
		REQUIRE(loadedDoc != nullptr);

		auto loadedEntity = loadedDoc->getObject(ENTITY_NAME);
		REQUIRE(loadedEntity != nullptr);
	}

	napkin::AppContext::destroy();
	*/
}

TEST_CASE("Pointer 'paths' 2", "napkin-pointers")
{
	/*
	napkin::AppContext::create();
    RUN_Q_APPLICATION

	auto doc = napkin::AppContext::get().loadDocument("unit_tests_data/entitystructure.json");
	REQUIRE(doc != nullptr);

	SECTION("absolute entity paths")
	{
		auto eBike = doc->getObject<nap::Entity>("Bike");
		REQUIRE(eBike != nullptr);
		REQUIRE(doc->absoluteObjectPath(*eBike) == "/Bike");

		auto eHandleBars = doc->getObject<nap::Entity>("Handlebars");
		REQUIRE(eHandleBars != nullptr);
		REQUIRE(doc->absoluteObjectPath(*eHandleBars) == "/Bike/Handlebars");

		auto eBell = doc->getObject<nap::Entity>("Bell");
		REQUIRE(eBell!= nullptr);
		REQUIRE(doc->absoluteObjectPath(*eBell) == "/Bike/Handlebars/Bell");
	}

	SECTION("absolute component paths")
	{
		auto cTransform2 = doc->getObject<nap::Component>("Transform_2");
		REQUIRE(cTransform2 != nullptr);
		REQUIRE(doc->absoluteObjectPath(*cTransform2) == "/Bike/Wheel_2/Transform_2");
	}

	SECTION("relative component path")
	{
		auto cPointerInput 	= doc->getObject<nap::Component>("PointerInput");
		auto cKeyInput 		= doc->getObject<nap::Component>("KeyInput");
		auto cTrans3 		= doc->getObject<nap::Component>("Transform_3");
		auto cOrthoCam 		= doc->getObject<nap::Component>("OrthoCam");
		auto cRotate 		= doc->getObject<nap::Component>("Rotate");

		REQUIRE(cPointerInput != nullptr);
		REQUIRE(cKeyInput     != nullptr);
		REQUIRE(cTrans3       != nullptr);
		REQUIRE(cOrthoCam     != nullptr);

		REQUIRE(doc->relativeObjectPath(*cPointerInput, *cKeyInput) == "./KeyInput");
		REQUIRE(doc->relativeObjectPath(*cPointerInput, *cTrans3)   == "../Transform_3");
		REQUIRE(doc->relativeObjectPath(*cPointerInput, *cOrthoCam) == "./Bell/OrthoCam");
		REQUIRE(doc->relativeObjectPath(*cOrthoCam,     *cRotate)   == "../../../Life/Fauna/Cats/Rotate");
	}
	napkin::AppContext::destroy();
	*/
}