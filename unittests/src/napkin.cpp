#include "catch.hpp"

#include <appcontext.h>
#include <commands.h>
#include <composition.h>
#include <imagelayer.h>
#include <generic/naputils.h>
#include <QCoreApplication>
#include <firstpersoncontroller.h>
#include <transformcomponent.h>
#include <midiinputport.h>
#include <QCoreApplication>
#include <renderablemeshcomponent.h>

#include "testclasses.h"

#define TAG_NAPKIN "[napkin]"
#define ENTITY_NAME "Entity"

using namespace napkin;

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

QString getResource(const QString& filename)
{
	const QString resourceDir = "unit_tests_data";
	if (filename.isEmpty())
		return resourceDir;
	return resourceDir + "/" + filename;
}

TEST_CASE("Document Management", TAG_NAPKIN)
{
	
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
}

TEST_CASE("Document Signals", TAG_NAPKIN)
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

TEST_CASE("Array Value Elements", TAG_NAPKIN)
{
	auto doc = AppContext::get().newDocument();
	auto resource = doc->addObject<TestResource>();
	REQUIRE(resource != nullptr);

	// Check invalid nonexistent path
	PropertyPath nonExistent(*resource, "NonExistent_________");
	REQUIRE(!nonExistent.isValid());

	// Grab a valid path
	PropertyPath ints2D(*resource, "Ints2D");
	REQUIRE(ints2D.isValid());

	// Ensure empty array
	REQUIRE(ints2D.getArrayLength() == 0);

	// Add an element
	{
		auto index = doc->arrayAddValue(ints2D);
		REQUIRE(index == 0); // The index at which the new element lives
		REQUIRE(ints2D.getArrayLength() == 1);

		// Verify validity of new element
		REQUIRE(PropertyPath(*resource, "Ints2D/0").isValid());
//		 TODO: This should not happen
//		REQUIRE(!PropertyPath(*resource, "Ints2D/18").isValid());
	}

	// Add another element
	{
		auto index = doc->arrayAddValue(ints2D);
		REQUIRE(index == 1); // Index should be one
		REQUIRE(ints2D.getArrayLength() == 2);

		// Verify validity of new element
		REQUIRE(PropertyPath(*resource, "Ints2D/0").isValid());
		REQUIRE(PropertyPath(*resource, "Ints2D/1").isValid());
	}

	// Remove first element
	{
		doc->arrayRemoveElement(ints2D, 0);
		REQUIRE(ints2D.getArrayLength() == 1);
	}

	// Remove the second element
	{
		doc->arrayRemoveElement(ints2D, 0);
		REQUIRE(ints2D.getArrayLength() == 0);
	}
}


TEST_CASE("Array add string", TAG_NAPKIN)
{
	auto doc = AppContext::get().newDocument();
	auto midiinput = doc->addObject<TestResource>();
	REQUIRE(midiinput != nullptr);
	PropertyPath strings(*midiinput, "Strings");
	REQUIRE(strings.isValid());
	REQUIRE(strings.isArray());
	REQUIRE(!strings.isPointer());
	REQUIRE(strings.getArrayElementType() == RTTI_OF(std::string));

	auto index = doc->arrayAddValue(strings);
	REQUIRE(index == 0);
}

TEST_CASE("Array Modification Objects", TAG_NAPKIN)
{
	auto doc = AppContext::get().newDocument();
	auto resource = doc->addObject<TestResourceB>();
	REQUIRE(resource != nullptr);
	auto pointee = doc->addObject<TestResource>();
	REQUIRE(pointee != nullptr);

	PropertyPath respointers(*resource, "ResPointers");
	REQUIRE(respointers.isValid());
	REQUIRE(respointers.getArrayLength() == 0);

	// Add the existing layer to the composition
	{
		auto index = doc->arrayAddExistingObject(respointers, pointee);
		REQUIRE(index == 0);
		REQUIRE(respointers.getArrayLength() == 1);
	}

	// Add a new layer to the composition
	{
		auto index = doc->arrayAddNewObject(respointers, RTTI_OF(TestResource));
		REQUIRE(index == 1);
		REQUIRE(respointers.getArrayLength() == 2);
	}

}

TEST_CASE("Array Move Element", TAG_NAPKIN)
{
	auto doc = AppContext::get().newDocument();
	auto resource = doc->addObject<nap::Composition>();
	REQUIRE(resource != nullptr);

	// Set up an array with four elements
	PropertyPath layers(*resource, "Layers");
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

	size_t new_index = doc->arrayMoveElement(layers, 2, 1);
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

	doc->executeCommand(new ArrayMoveElementCommand(layers, 1, 3));
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

	doc->executeCommand(new ArrayMoveElementCommand(layers, 3, 1));
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

TEST_CASE("PropertyPath", TAG_NAPKIN)
{
	SECTION("general")
	{
		auto doc = AppContext::get().newDocument();
		auto entity = doc->addObject<nap::Entity>();
		PropertyPath nameProp(*entity, nap::rtti::sIDPropertyName);
		REQUIRE(&nameProp.getObject() == entity);
		REQUIRE(nameProp.isValid());
		std::string newName = "NewName";
		nameProp.setValue(newName);
		REQUIRE(nameProp.getValue() == newName);
		REQUIRE(entity->mID == newName);

		PropertyPath invalidPath;
		REQUIRE(!invalidPath.isValid());
	}

	TestResourceB res;
	res.mID = "MyResource";

	SECTION("enum")
	{
		PropertyPath path(res, "Enum");
		REQUIRE(path.isValid());
		REQUIRE(path.isEnum());
		REQUIRE(!path.isArray());
		REQUIRE(!path.isPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
	}

	SECTION("regular pointer")
	{
		PropertyPath path(res, "ResPointer");
		REQUIRE(path.isValid());
		REQUIRE(path.isPointer());
		REQUIRE(path.isNonEmbeddedPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isEnum());
		REQUIRE(!path.isArray());
	}

	SECTION("array of regular pointers")
	{
		PropertyPath path(res, "ResPointers");
		REQUIRE(path.isValid());
		REQUIRE(path.isArray());
		REQUIRE(path.isPointer());
		REQUIRE(path.isNonEmbeddedPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isEnum());
	}

	SECTION("embedded pointer")
	{
		PropertyPath path(res, "EmbedPointer");
		REQUIRE(path.isValid());
		REQUIRE(path.isPointer());
		REQUIRE(path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
		REQUIRE(!path.isEnum());
		REQUIRE(!path.isArray());
	}

	SECTION("array of embedded pointers")
	{
		PropertyPath path(res, "EmbedPointers");
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
		res.mStructs.emplace_back(uniform);
		PropertyPath path(res, "Structs");
		REQUIRE(path.isValid());
//		REQUIRE(!path.isArray());
		REQUIRE(!path.isPointer());
		REQUIRE(!path.isEmbeddedPointer());
		REQUIRE(!path.isNonEmbeddedPointer());
		REQUIRE(!path.isEnum());
	}

}

TEST_CASE("Commands", TAG_NAPKIN)
{
	auto& ctx = AppContext::get();
	SigCapture sigDocChanged(&ctx, &AppContext::documentChanged);
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

	PropertyPath nameProp1(*entity1, nap::rtti::sIDPropertyName);
	REQUIRE(nameProp1.isValid());
	PropertyPath nameProp2(*entity2, nap::rtti::sIDPropertyName);
	REQUIRE(nameProp2.isValid());

	ctx.executeCommand(new SetValueCommand(nameProp1, "Loco"));
	REQUIRE(sigDocChanged.count() == ++sigDocCount);
	REQUIRE(entity1->mID == "Loco");

	// Name may not be empty
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
	auto& entity = doc->addEntity();
	ctx.executeCommand(new AddComponentCommand(entity, RTTI_OF(nap::PerspCameraComponent)));
	REQUIRE(entity.hasComponent<nap::PerspCameraComponent>());
	auto component = doc->getComponent(entity, RTTI_OF(nap::PerspCameraComponent));
	REQUIRE(component != nullptr);
	ctx.executeCommand(new RemoveComponentCommand(*component));
	REQUIRE(!entity.hasComponent<nap::PerspCameraComponent>());
}

TEST_CASE("File Extensions", TAG_NAPKIN)
{
	ResourceFactory fact = AppContext::get().getResourceFactory();
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

TEST_CASE("Resource Management", TAG_NAPKIN)
{
	// Must start QApplication in order to have an event loop for signals?
	int argc = 1;
	const char* argv[] = { "arg","blop" };
	QCoreApplication app(argc, const_cast<char**>(argv));

	// Assume this test file's directory as the base path
	QString jsonFile = "objects.json";
	QString shaderFile = "shaders/debug.frag";
	QString dataDir = ".";

	// Just set the filename for reference purposes
	AppContext::get().getDocument()->setFilename(getResource(jsonFile));

	// Ensure shader file exists
	auto absJsonFilePath = QFileInfo(getResource(jsonFile)).absoluteFilePath();
	REQUIRE(QFileInfo::exists(absJsonFilePath));

	// Ensure shader file exists
	auto absShaderFilePath = QFileInfo(getResource(shaderFile)).absoluteFilePath();
	REQUIRE(QFileInfo::exists(absShaderFilePath));

	// Ensure the resource directory exists
	auto resourcedir = QFileInfo(getResource("")).absoluteFilePath();
	REQUIRE(QFileInfo::exists(resourcedir));

	// Test our local resource function with napkin's
	auto basedir = QFileInfo(getResourceReferencePath()).absoluteFilePath();
	REQUIRE(basedir.toStdString() == resourcedir.toStdString());

	// Check relative path
	auto relJsonPath = getRelativeResourcePath(absJsonFilePath);
	REQUIRE(relJsonPath.toStdString() == jsonFile.toStdString());


	// Check relative path
	auto relShaderPath = getRelativeResourcePath(absShaderFilePath);
	REQUIRE(relShaderPath.toStdString() == shaderFile.toStdString());

	// Check absolute path
	auto absShaderPath = getAbsoluteResourcePath(relShaderPath);
	REQUIRE(absShaderPath.toStdString() == absShaderFilePath.toStdString());

	// Check if dir contains path
	REQUIRE(directoryContains(resourcedir, absShaderPath));
	REQUIRE(directoryContains(resourcedir, absJsonFilePath));
	REQUIRE(directoryContains(resourcedir + "/shaders", absShaderPath));
	// or not
	REQUIRE(!directoryContains(absShaderPath, resourcedir));

}

TEST_CASE("Component to Component pointer", TAG_NAPKIN)
{
	auto& ctx = napkin::AppContext::get();
	std::string serializedData;

	auto doc = ctx.newDocument();

	nap::Entity& entity = doc->addEntity();
	doc->setObjectName(entity, ENTITY_NAME);
	REQUIRE(entity.mID == ENTITY_NAME);

	auto fpcam = doc->addComponent<nap::FirstPersonController>(entity);
	REQUIRE(fpcam != nullptr);
	REQUIRE(!fpcam->mID.empty());

	auto perspcam = doc->addComponent<nap::PerspCameraComponent>(entity);
	REQUIRE(perspcam != nullptr);
	REQUIRE(!fpcam->mID.empty());

	napkin::PropertyPath perspCamPath(*fpcam, "PerspCameraComponent");
	REQUIRE(perspCamPath.isValid());

	perspCamPath.setPointee(perspcam);

	auto pointee = perspCamPath.getPointee();
	REQUIRE(pointee == perspcam);

	serializedData = ctx.documentToString();
	nap::Logger::info(serializedData);
	REQUIRE(!serializedData.empty());

	auto loadedDoc = ctx.loadDocumentFromString(serializedData);
	REQUIRE(loadedDoc != nullptr);

	auto loadedEntity = loadedDoc->getObject(ENTITY_NAME);
	REQUIRE(loadedEntity != nullptr);
}

TEST_CASE("Pointer 'paths' 2", TAG_NAPKIN)
{
	auto doc = napkin::AppContext::get().loadDocument("unit_tests_data/entitystructure.json");

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

	napkin::AppContext::get().newDocument();
}