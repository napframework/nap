#include "testcore.h"
#include "../diff_match_patch.h"
#include "../testmanager.h"

#include <nap.h>
#include <nap/coreoperators.h>
#include <nap/resourcemanager.h>
#include <nap/arrayattribute.h>
#include <nap/threading.h>
#include <jsonserializer.h>
#include <thread>

using namespace nap;

class DummyComponent : public Component
{
	RTTI_ENABLE(Component)
public:
	virtual ~DummyComponent() = default;

	Attribute<float> floatAttribute = {this, "floatAttr", 0.};
	Attribute<int> intAttribute = {this, "intAttr", 0};
	Attribute<std::vector<float>> vecFloatAttribute{this, "vecFloatAttr"};
	Attribute<std::vector<int>> vecIntAttribute{this, "vecIntAttr"};
};
RTTI_DEFINE(DummyComponent)

bool testCore()
{
	Core core;
	{
		std::string entityName = "MyEntity";
		Entity& e = core.getRoot().addEntity(entityName);
		core.getRoot().removeEntity(e);
		Entity* nullEntity = core.getRoot().getEntity(entityName);
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

		Attribute<float>* nullAttr = core.getRoot().getAttribute<float>(attrName);
		TEST_ASSERT(nullAttr == nullptr, "Attribute should have been deleted");
	}

	return true;
}


std::shared_ptr<Core> createSimpleObjectTree() {
    auto core = std::make_shared<Core>();
    auto& root = core->getRoot();
    auto& a = root.addEntity("A");
//    auto& patch = root.addComponent<PatchComponent>("patchComponent");
    return core;
}
std::shared_ptr<Core> createObjectTree()
{
	auto core = std::make_shared<Core>();
	auto& root = core->getRoot();
	auto& patch = root.addComponent<PatchComponent>("patchComponent");
	patch.setName("patch");
	auto& rootAttr = patch.addAttribute("attrRootOfTwo", RTTI_OF(Attribute<float>));
	((Attribute<float>&)rootAttr).setValue(1.41421356237f);

	auto& dummy = root.addComponent<DummyComponent>("dummy");
	dummy.floatAttribute.setValue(1);
	dummy.intAttribute.setValue(1);
	dummy.vecFloatAttribute.setValue({1., 2., 3., 4.});
	dummy.vecIntAttribute.setValue({1, 2, 3, 4});

	CompoundAttribute& compAttr = dummy.addCompoundAttribute("myCompound");
	compAttr.addAttribute<float>("oneHundred", 100.0f);
	compAttr.addAttribute<int>("twoHundred", 200);

	auto& a = root.addEntity("A");
	auto& aComp = a.addComponent<PatchComponent>();
	auto& aAttr = aComp.addAttribute("attrPI", RTTI_OF(Attribute<float>));
	((Attribute<float>&)aAttr).setValue(3.14159265359f);
	auto& b = a.addEntity("B");
	auto& patch2 = b.addComponent<PatchComponent>();
	auto& myAt = patch2.addAttribute("attrGoldenRatio", RTTI_OF(Attribute<float>));
	((Attribute<float>&)myAt).setValue(1.61803398875f);


    auto& opTermA = patch2.getPatch().addOperator<FloatOperator>("TermA");
    auto& opTermB = patch2.getPatch().addOperator<FloatOperator>("TermA");
    auto& opFactorB = patch2.getPatch().addOperator<FloatOperator>("FactorB");
    auto& opResult = patch2.getPatch().addOperator<FloatOperator>("Result");
    opTermA.input.getAttribute().setValue(1);
    opTermB.input.getAttribute().setValue(2);
    opFactorB.input.getAttribute().setValue(3);

	auto& opMult = patch2.getPatch().addOperator<MultFloatOperator>("Mult");
	auto& opAdd = patch2.getPatch().addOperator<AddFloatOperator>("Add");
    
    opAdd.mTermA.connect(opTermA.output);
    opAdd.mTermB.connect(opTermB.output);
	opMult.mFactorA.connect(opAdd.sum);
    opMult.mFactorB.connect(opFactorB.output);
    opResult.input.connect(opMult.product);

    opMult.mFactorA.disconnect();
    opResult.input.disconnect();
    opResult.input.connect(opAdd.sum);

	return core;
}


nap::FloatOperator* getFloatOperator(nap::Object& root, const std::string& name)
{
    std::vector<nap::FloatOperator*> results;
    results = root.getChildrenOfType<nap::FloatOperator>(true);
    for (auto& result : results)
        if (result->getName() == name)
            return result;
    return nullptr;
}


std::string diffString(const std::string& str1, const std::string& str2)
{
	diff_match_patch<std::string> dmp;
	diff_match_patch<std::string>::Diffs diff = dmp.diff_main(str1, str2);
	dmp.diff_cleanupSemantic(diff);
	std::ostringstream stream;
	for (diff_match_patch<std::string>::Diff d : diff) {
		if (d.operation == diff_match_patch<std::string>::Operation::EQUAL)
			continue;
		if (d.operation == diff_match_patch<std::string>::Operation::DELETE)
			stream << "MISSING:" << std::endl;
		if (d.operation == diff_match_patch<std::string>::Operation::INSERT)
			stream << "ADDED:" << std::endl;
		stream << d.text << std::endl;
	}
	return stream.str();
}


bool testSerializer(const Serializer& ser)
{
    std::string xmlString1;
    {
        // Create object tree and serialize
        auto srcCore = createObjectTree();
        xmlString1 = ser.toString(srcCore->getRoot(), false);
        writeStringToFile("test.json", xmlString1);
    }



	// Deserialize
	Core dstCore;
    dstCore.initialize();
//    rtti::TypeInfo serverType = rtti::TypeInfo::getByName("nap::JsonRpcService");
//    TEST_ASSERT(serverType.isValid(), "Could not find server type");
//    auto service = dstCore.getOrCreateService(serverType);
//    TEST_ASSERT(service != nullptr, "Could not get server service");
//    service->getAttribute<bool>("threaded")->setValue(true);
//    service->getAttribute<bool>("running")->setValue(true);

	ser.fromString(xmlString1, dstCore);

	// Serialize again
	std::string xmlString2 = ser.toString(dstCore.getRoot(), false);

	TEST_ASSERT(xmlString1 == xmlString2,
				"Second serialization gave different result:\n" + diffString(xmlString1, xmlString2));


	return true;
}

bool testPatch() {
    auto srcCore = createObjectTree();
    auto resultOp = getFloatOperator(srcCore->getRoot(), "Result");
    TEST_ASSERT(resultOp, "Result operator not found in original object tree");
    float result = 0;
    resultOp->output.pull(result);
    TEST_ASSERT(result == 3, "Patch did not return proper value: 9");
    return true;
}


bool testArrayAttribute()
{
    Core core;
    auto& e = core.getRoot().addEntity("entity");
    auto& arrayA = e.addArrayAttribute<float>("floatArrayA");
    auto& arrayB = e.addArrayAttribute<float>("floatArrayB");
    arrayA.add(1);
    arrayA.add(3);
    arrayA.insert(1, 0);
    arrayA.setValue(1, 2);
    arrayA.setValue({ 1, 2, 3 });
    arrayB.setValue(arrayA);
    TEST_ASSERT(arrayB.getSize() == 3, "Copied array must have same size as original");
    
    auto i = 0;
    for (auto value : arrayB)
    {
        TEST_ASSERT(value == arrayA[i], "Elements in copied array must be the same as original");
        i++;
    }
    
    while (arrayB.getSize() != 0)
        arrayB.remove(0);
    TEST_ASSERT(arrayB.getSize() == 0, "Array must be empty after removing all elements");
    
    arrayA.clear();
    TEST_ASSERT(arrayA.getSize() == 0, "Array must be empty after clearing");
    
        
    return true;
}


bool testThreading()
{
    nap::WorkerThread myThread(false);
    
    myThread.loop.connect([](nap::WorkerThread&){
        std::cout << "this runs on myThread" << std::endl;
    });
    
    myThread.enqueue([](){
        std::cout << "this runs on myThread" << std::endl;
    });
    
    myThread.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return true;
}


bool testXMLSerializer()
{
	XMLSerializer ser;
	return testSerializer(ser);
}


bool testJSONSerializer()
{
	JSONSerializer ser;
	return testSerializer(ser);
}

bool testObjectPath()
{

	Core core;
	Logger::debug(ObjectPath(core.getRoot()));
	TEST_ASSERT(ObjectPath(core.getRoot()) == "/", "Root path didn't resolve to '/'.");

	// Create an object tree
	//	AttributeBase* rootAttr = core.getRoot().addAttribute("Root_Attribute",
	// RTTI_OF(Attribute<float>));
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

bool testFileUtils()
{
	TEST_ASSERT(getFileExtension("my.dir/myfile.tar.gz") == "gz", "Extension didn't match");

	return true;
}


bool testResourceManager() {
	rtti::TypeInfo::get<nap::JSONFileLoader>();
    Core core;
    auto resourceMan = core.getOrCreateService<ResourceManagerService>();

    std::string resource = "test.napj";

    ResourceLoader* factory = resourceMan->getLoaderFor(resource);
    TEST_ASSERT(factory != nullptr, "Unable to find factory for: " + resource);

    TEST_ASSERT(resourceMan->canLoad(resource), "Cannot load resource: " + resource);

    auto res = resourceMan->getResource(resource);
    TEST_ASSERT(res, "Resource failed to load: " + resource);

    TEST_ASSERT(res->getResourcePath() == resource, "Resulting resource path differs from original path");

    return true;
}
