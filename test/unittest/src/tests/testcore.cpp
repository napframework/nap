#include "testcore.h"
#include "../testmanager.h"

#include <nap.h>

using namespace nap;


class DummyComponent : public Component
{
	RTTI_ENABLE_DERIVED_FROM(Component)
public:
	virtual ~DummyComponent() = default;

	Attribute<float> floatAttribute = {this, "floatAttr", 0.};
	Attribute<int> intAttribute = {this, "intAttr", 0};
	Attribute<std::vector<float>> vecFloatAttribute{this, "vecFloatAttr"};
	Attribute<std::vector<int>> vecIntAttribute{this, "vecIntAttr"};
};
RTTI_DECLARE(DummyComponent)
RTTI_DEFINE(DummyComponent)


bool testCore()
{
	Core core;
	{
		std::string entityName = "MyEntity";
		Entity &e = core.getRoot().addEntity(entityName);
		core.getRoot().removeEntity(e);
		Entity *nullEntity = core.getRoot().getEntity(entityName);
		TEST_ASSERT(nullEntity == nullptr, "Entity should have been deleted");
	}

	{
		bool success = core.getRoot().removeChild("NonExistingAttribute");
		TEST_ASSERT(!success, "Removal should have failed");
	}



	{
		std::string attrName = "MyFloat";

		core.getRoot().addAttribute<float>(attrName);
		bool success = core.getRoot().removeChild(attrName);
		TEST_ASSERT(success, "Removal should have been okay");

		Attribute<float> *nullAttr = core.getRoot().getAttribute<float>(attrName);
		TEST_ASSERT(nullAttr == nullptr, "Attribute should have been deleted");
	}


	return true;
}


bool testXMLSerializer()
{
	std::string xmlString;

	{
		Core core;
		// Create an object tree
		{
			auto& root = core.getRoot();
			auto& patch = root.addComponent<PatchComponent>("patch");
			patch.setName("stijn");
			auto& rootAttr = patch.addAttribute("attrRootOfTwo", RTTI_OF(Attribute<float>));
			((Attribute<float>&)rootAttr).setValue(1.41421356237f);

			auto& dummy = root.addComponent<DummyComponent>("dummy");
			dummy.floatAttribute.setValue(1);
			dummy.intAttribute.setValue(1);
			dummy.vecFloatAttribute.setValue({1., 2., 3., 4.});
			dummy.vecIntAttribute.setValue({1, 2, 3, 4});

			auto& a = root.addEntity("A");
			auto& aComp = a.addComponent<PatchComponent>();
			auto& aAttr = aComp.addAttribute("attrPI", RTTI_OF(Attribute<float>));
			((Attribute<float>&)aAttr).setValue(3.14159265359f);
			auto& b = a.addEntity("B");
			auto& comp = b.addComponent<PatchComponent>();
			auto& myAt = comp.addAttribute("attrGoldenRatio", RTTI_OF(Attribute<float>));
			((Attribute<float>&)myAt).setValue(1.61803398875f);
		}

		// Serialize root
		{
			std::ostringstream oss;
			XMLSerializer ser(oss, core);
			ser.writeObject(core.getRoot());
			xmlString = oss.str();
		}
		//		std::cout << xmlString << std::endl;
	}


	// Deserialize again
	{
		Core core;
		{
			std::istringstream iss(xmlString);
			XMLDeserializer deser(iss, core);
			deser.readObject();
		}


		// Serialize again
		{
			std::ostringstream oss2;
			XMLSerializer ser2(oss2, core);
			ser2.writeObject(core.getRoot());
			std::string xmlString2 = oss2.str();
			//            std::cout << xmlString2 << std::endl;
			TEST_ASSERT(xmlString == xmlString2, "Second serialization gave different result:\nString1:\n" + xmlString + "\nString2:\n" + xmlString2);
		}
	}
	return true;
}

bool testObjectPath()
{

	Core core;
	Logger::debug(ObjectPath(core.getRoot()));
	TEST_ASSERT(ObjectPath(core.getRoot()) == "/", "Root path didn't resolve to '/'.");


	// Create an object tree
	//	AttributeBase* rootAttr = core.getRoot().addAttribute("Root_Attribute", RTTI_OF(Attribute<float>));
	Entity& a = core.addEntity("A");
	TEST_ASSERT(ObjectPath(a) == "/A", "Path mismatch");

	auto& aComp = a.addComponent<PatchComponent>();
	auto& aAttr = aComp.addAttribute("A_Attribute", RTTI_OF(Attribute<float>));
	auto& b = a.addEntity("B");
	auto& comp = b.addComponent<PatchComponent>();
	auto& myAt = comp.addAttribute("SomeAttribute", RTTI_OF(Attribute<float>));

	// Display paths
	std::vector<Object*> allObjects = core.getRoot().getChildren(true);
	for (auto n : allObjects) {
		ObjectPath path(n);
		Logger::debug(path);

		Object* resolvedNode = path.resolve(core.getRoot());
		Logger::debug("Resolved: %s", path.toString().c_str());
	}
	return true;
}

