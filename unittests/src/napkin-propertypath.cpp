#include "utils/include.h"

#include <appcontext.h>
#include <QtDebug>

using namespace napkin;

TEST_CASE("PropertyPath", "napkin-propertypath")
{
    RUN_Q_APPLICATION

	auto doc = AppContext::get().newDocument();
	auto entity = doc->addObject<nap::Entity>();
	auto resB = doc->addObject<TestResourceB>();
	auto res = doc->addObject<TestResourceB>();
	res->mID = "MyResource";

	// Add a pointer to array of pointers
	doc->arrayAddExistingObject({*res, "ResPointers"}, resB);

	SECTION("general")
	{
		PropertyPath nameProp(*entity, nap::rtti::sIDPropertyName);
		REQUIRE(nameProp.getType() == rttr::type::get<std::string>());
		REQUIRE(&nameProp.getObject() == entity);
		REQUIRE(nameProp.isValid());
		std::string newName = "NewName";
		nameProp.setValue(newName);
		REQUIRE(nameProp.getValue() == newName);
		REQUIRE(entity->mID == newName);

		PropertyPath invalidPath;
		REQUIRE(!invalidPath.isValid());
	}

	SECTION("enum")
	{
		PropertyPath path(*res, "Enum");
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
		PropertyPath path(*res, "ResPointer");
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
		PropertyPath path(*res, "ResPointers");
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
		PropertyPath path(*res, "ResPointers/0");
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
		PropertyPath path(*res, "EmbedPointer");
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
		PropertyPath path(*res, "EmbedPointers");
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
		PropertyPath path(*res, "Structs");
		REQUIRE(path.getType() == rttr::type::get<std::vector<TestPropertiesStruct>>());
		REQUIRE(path.isValid());
		REQUIRE(path.isArray());
		REQUIRE(!path.isPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
		REQUIRE(!path.isEnum());
	}

}

TEST_CASE("PropertyIteration", "[napkinpropertypath]")
{
	RUN_Q_APPLICATION

	TestResourceB res;
	res.mID = "TestResource";

	{
		auto props = PropertyPath::getProperties(res);
		REQUIRE(props.size() == 16);
	}

	{
		auto props = PropertyPath::getProperties(res, IterFlag::Resursive);
		REQUIRE(props.size() == 26);
	}

	{
		TestResource subRes;
		subRes.mID = "SubRes";
		res.mResPointer = &subRes;
		REQUIRE(res.mResPointer != nullptr);

		TestResourceB embedRes;
		embedRes.mID = "EmbedRes";
		res.mEmbedPointer = &embedRes;
		REQUIRE(res.mEmbedPointer != nullptr);

		PropertyPath p(res, "ResPointer");
		REQUIRE(p.isValid());
		REQUIRE(p.isPointer());

		auto props1 = PropertyPath::getProperties(res, IterFlag::Resursive | IterFlag::FollowPointers);
		for (auto p : props1)
		{
			// The embedded pointee cannot be in this result, only regular pointees
			REQUIRE(&p.getObject() != &embedRes);
		}
		REQUIRE(props1.size() == 38);

		auto props2 = PropertyPath::getProperties(res, IterFlag::Resursive | IterFlag::FollowEmbeddedPointers);
		for (auto p : props2)
		{
			// The regular pointee subRes cannot be in this result, only embedded pointees
			REQUIRE(&p.getObject() != &subRes);
		}
		REQUIRE(props2.size() == 42);

		res.mResPointer = nullptr;

	}
}