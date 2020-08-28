#include "utils/include.h"

#include <appcontext.h>
#include <QtDebug>
#include <utility/fileutils.h>

using namespace napkin;

TEST_CASE("PropertyPath", "napkin-propertypath")
{
	napkin::AppContext::create();
    RUN_Q_APPLICATION

	auto doc = AppContext::get().newDocument();
	auto entity = doc->addObject<nap::Entity>();
	auto resB = doc->addObject<TestResourceB>();
	auto res = doc->addObject<TestResourceB>();
	res->mID = "MyResource";

	// Add a pointer to array of pointers
	doc->arrayAddExistingObject({res->mID, "ResPointers", *doc }, resB);

	SECTION("general")
	{
		PropertyPath nameProp(entity->mID, nap::rtti::sIDPropertyName, *doc);
		REQUIRE(nameProp.getType() == rttr::type::get<std::string>());
		REQUIRE(nameProp.getObject() == entity);
		REQUIRE(nameProp.isValid());
		std::string newName = "NewName";
		doc->setObjectName(*entity, newName);
		REQUIRE(nameProp.getValue() == newName);
		REQUIRE(entity->mID == newName);

		PropertyPath invalidPath;
		REQUIRE(!invalidPath.isValid());
	}

	SECTION("enum")
	{
		PropertyPath path(res->mID, "Enum", *doc);
		REQUIRE(path.getType() == rttr::type::get<TestEnum>());
		REQUIRE(path.isValid());
		REQUIRE(path.isEnum());
		REQUIRE(!path.isArray());
		REQUIRE(!path.isPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
	}

	SECTION("regular pointer")
	{
		PropertyPath path(res->mID, "ResPointer", *doc);
		REQUIRE(path.getType() == rttr::type::get<nap::ResourcePtr<TestResource>>());
		REQUIRE(path.isValid());
		REQUIRE(path.isPointer());
		REQUIRE(path.isNonEmbeddedPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isEnum());
		REQUIRE(!path.isArray());
	}

	SECTION("array of regular pointers")
	{
		PropertyPath path(res->mID, "ResPointers", *doc);
		REQUIRE(path.getType() == rttr::type::get<std::vector<nap::ResourcePtr<TestResource>>>());
		REQUIRE(path.isValid());
		REQUIRE(path.isArray());
		REQUIRE(path.isPointer());
		REQUIRE(path.isNonEmbeddedPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isEnum());
	}

	SECTION("array element: regular pointer")
	{
		PropertyPath path(res->mID, "ResPointers/0", *doc);
		REQUIRE(path.getType() == rttr::type::get<nap::ResourcePtr<TestResource>>());
		REQUIRE(path.isValid());
		REQUIRE(path.isPointer());
		REQUIRE(path.isNonEmbeddedPointer());
		REQUIRE(!path.isArray());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isEnum());
	}

	SECTION("embedded pointer")
	{
		PropertyPath path(res->mID, "EmbedPointer", *doc);
		REQUIRE(path.getType() == rttr::type::get<nap::ResourcePtr<TestResource>>());
		REQUIRE(path.isValid());
		REQUIRE(path.isPointer());
		REQUIRE(path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
		REQUIRE(!path.isEnum());
		REQUIRE(!path.isArray());
	}

	SECTION("array of embedded pointers")
	{
		PropertyPath path(res->mID, "EmbedPointers", *doc);
		REQUIRE(path.getType() == rttr::type::get<std::vector<nap::ResourcePtr<TestResource>>>());
		REQUIRE(path.isValid());
		REQUIRE(path.isArray());
		REQUIRE(path.isPointer());
		REQUIRE(path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
		REQUIRE(!path.isEnum());
	}

	SECTION("array of structs")
	{
		TestPropertiesStruct uniform;
		res->mStructs.emplace_back(uniform);
		PropertyPath path(res->mID, "Structs", *doc);
		REQUIRE(path.getType() == rttr::type::get<std::vector<TestPropertiesStruct>>());
		REQUIRE(path.isValid());
		REQUIRE(path.isArray());
		REQUIRE(!path.isPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
		REQUIRE(!path.isEnum());
	}
	napkin::AppContext::destroy();
}

TEST_CASE("InstanceProperties", "[napkinpropertypath]")
{
	/*
	QString tempFilename = "__TEMP_NAPKIN_PROP_PATH_TEST.json";
	napkin::AppContext::create();

	RUN_Q_APPLICATION
	{
		auto& ctx = AppContext::get();
		auto doc = ctx.newDocument();
		auto entity = doc->addObject<nap::Entity>();
		auto comp = doc->addComponent<TestComponent>(*entity);
		auto scene = doc->addObject<nap::Scene>();
		REQUIRE(scene);
		doc->addEntityToScene(*scene, *entity);

		REQUIRE(doc->getRootEntities(*scene, *entity).size() > 0);

		PropertyPath regularPath(comp->mID, "Float", *doc);
		REQUIRE(!regularPath.isInstanceProperty());
		REQUIRE(regularPath.isValid());

		//	// The path is an instance path if a root entity is provided
		//	PropertyPath instancePath(*rootEntity, *comp, "Float");
		//	REQUIRE(instancePath.isInstanceProperty());
		//	REQUIRE(instancePath.isValid());
		//
		//	float val1 = 123.456;
		//	regularPath.setValue(val1);
		//	REQUIRE(regularPath.getValue() == val1);
		//	REQUIRE(instancePath.getValue() == val1);
		//
		//	float val2 = 678.90;
		//	instancePath.setValue(val2);
		//	REQUIRE(instancePath.getValue() == val2);
		//	REQUIRE(regularPath.getValue() != val2);
		//
		//	PropertyPath instancePath2(*rootEntity, *comp, "Float");
		//	REQUIRE(instancePath2.getValue() == val2);
		//
		//	doc->setFilename(tempFilename);
		//	nap::Logger::info(nap::utility::getAbsolutePath(doc->getCurrentFilename().toStdString()));
		//	REQUIRE(ctx.saveDocument());
	}

	AppContext::destroy();
	*/
}


TEST_CASE("InstancePropertySerialization", "[napkinpropertypath]")
{
	/*
	std::string tempFilename("__TEMP_NAPKIN_PROP_PATH_TEST.json");
	float floatVal = 2356.7;
	int intVal = 42;
	std::string parentEntityCompName = "ParentEntityComponent";
	std::string childEntityCompName = "ChildEntityComponent";
	{
		napkin::AppContext::create();
		RUN_Q_APPLICATION

		//		 EntityA
		//		 	EntityAA
		//		 		EntityAAA
		//		 	EntityAB
		//		 		EntityABA
		//		 			ComponentABA
		//		 		EntityABB
		//		 		EntityABC
		//		 			ComponentABC
		//		 	EntityABA:0
		//				ComponentABA
		//			EntityABA:1
		//				ComponentABA

		auto& ctx = AppContext::get();
		auto doc = ctx.newDocument();

		// Root entity
		auto& entityA = doc->addEntity(nullptr, "EntityA");
		auto& entityAA = doc->addEntity(&entityA, "EntityAA");
		auto& entityAAA = doc->addEntity(&entityAA, "EntityAAA");
		auto& entityAB = doc->addEntity(&entityA, "EntityAB");
		auto& entityABA = doc->addEntity(&entityAB, "EntityABA");
		auto compABA = doc->addComponent<TestComponent>(entityABA);
		auto& entityABB = doc->addEntity(&entityAB, "EntityABB");
		auto& entityABC = doc->addEntity(&entityAB, "EntityABC");
		auto compABC = doc->addComponent<TestComponent>(entityABA);

		// Child entity
		auto comp = doc->addComponent<TestComponent>(entityAA);
		comp->mID = childEntityCompName;
		auto scene = doc->addObject<nap::Scene>();
		doc->addEntityToScene(*scene, entityA);

		REQUIRE(doc->getRootEntities(*scene, entityA).size() == 1);
//
//		PropertyPath parentInstancePath(*rootEntity, *compABA, "Int");
//		REQUIRE(parentInstancePath.isValid());
//		REQUIRE(parentInstancePath.getType().is_derived_from<int>());
//		parentInstancePath.setValue(intVal);
//
//		PropertyPath instancePath(*rootEntity, *comp, "Float");
//		REQUIRE(instancePath.isValid());
//		REQUIRE(instancePath.getType().is_derived_from<float>());
//		instancePath.setValue(floatVal);

		doc->setFilename(QString::fromStdString(tempFilename));
		REQUIRE(ctx.saveDocument());
		AppContext::destroy();
	}

	nap::Core core;

	std::string dataFile = nap::utility::getAbsolutePath(tempFilename);

	nap::utility::ErrorState err;
	if (!core.initializeEngine(err))
		FAIL(err.toString());

	if (!core.getResourceManager()->loadFile(dataFile, err))
		FAIL(err.toString());
	*/
}

TEST_CASE("PropertyIteration", "[napkinpropertypath]")
{
	napkin::AppContext::create();
	RUN_Q_APPLICATION

	auto& doc = *AppContext::get().getDocument();
	auto& res = *doc.addObject<TestResourceB>();
	res.mID = "TestResource";

	{
		auto props = PropertyPath(res, doc).getProperties();
		REQUIRE(props.size() == 16);
	}

	{
		auto props = PropertyPath(res, doc).getProperties(IterFlag::Resursive);
		REQUIRE(props.size() == 26);
	}

	{
		auto& subRes = *doc.addObject<TestResource>();
		subRes.mID = "SubRes";
		res.mResPointer = &subRes;
		REQUIRE(res.mResPointer != nullptr);

		auto& embedRes = *doc.addObject<TestResourceB>();
		embedRes.mID = "EmbedRes";
		res.mEmbedPointer = &embedRes;
		REQUIRE(res.mEmbedPointer != nullptr);

		PropertyPath p(res.mID, "ResPointer", doc);
		REQUIRE(p.isValid());
		REQUIRE(p.isPointer());


		auto props1 = PropertyPath(res, doc).getProperties(IterFlag::Resursive | IterFlag::FollowPointers);
		for (auto p : props1)
		{
			// The embedded pointee cannot be in this result, only regular pointees
			REQUIRE(p.getObject() != &embedRes);
		}
		REQUIRE(props1.size() == 38);

		auto props2 = PropertyPath(res, doc).getProperties(IterFlag::Resursive | IterFlag::FollowEmbeddedPointers);
		for (auto p : props2)
		{
			// The regular pointee subRes cannot be in this result, only embedded pointees
			REQUIRE(p.getObject() != &subRes);
		}
		REQUIRE(props2.size() == 42);

		res.mResPointer = nullptr;

	}
	napkin::AppContext::destroy();
}