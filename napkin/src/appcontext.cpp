#include "appcontext.h"
#include "napkinglobals.h"

// std
#include <fstream>

// qt
#include <QSettings>
#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>

// nap
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>

// local
#include <generic/napgeneric.h>
#include <nap/logger.h>
#include <utility/fileutils.h>

using namespace nap::rtti;
using namespace nap::utility;
using namespace napkin;

AppContext::AppContext()
{
	ErrorState err;
	if (!getCore().initializeEngine(err))
	{
		nap::Logger::fatal("Failed to initialize engine");
	}
}

AppContext::~AppContext()
{
	// Clear all objects before going down.
	mObjects.clear();
}

AppContext& AppContext::get()
{
	static AppContext inst;
	return inst;
}

void AppContext::newFile()
{
	mCurrentFilename = "";
	mObjects.clear();
	newFileCreated();
}

void AppContext::loadFile(const QString& filename)
{
	auto abspath = getAbsolutePath(filename.toStdString());
	nap::Logger::info("Loading '%s'", abspath.c_str());

	mCurrentFilename = filename;

	QSettings().setValue(settingsKey::LAST_OPENED_FILE, filename);

	ErrorState err;

	nap::rtti::RTTIDeserializeResult result;
	if (!readJSONFile(filename.toStdString(), getCore().getResourceManager()->getFactory(), result, err))
	{
		nap::Logger::fatal(err.toString());
		return;
	}

	if (!resolveLinks(result.mReadObjects, result.mUnresolvedPointers))
	{
		nap::Logger::fatal("Failed to resolve links");
		return;
	}

	// transfer
	mObjects.clear();
	for (auto& ob : result.mReadObjects)
	{
		mObjects.emplace_back(std::move(ob));
	}

	fileOpened(mCurrentFilename);
}

void AppContext::saveFile()
{
	if (mCurrentFilename.isEmpty())
	{
		nap::Logger::fatal("Cannot save file, no filename has been set.");
		return;
	}
	saveFileAs(mCurrentFilename);
}

void AppContext::saveFileAs(const QString& filename)
{
	ObjectList objects;
	for (auto& ob : mObjects)
	{
		objects.emplace_back(ob.get());
	}

	JSONWriter writer;
	ErrorState err;
	if (!serializeObjects(objects, writer, err))
	{
		nap::Logger::fatal(err.toString());
		return;
	}

	std::ofstream out(filename.toStdString());
	out << writer.GetJSON();
	out.close();

	mCurrentFilename = filename;
	nap::Logger::info("Written file: " + filename.toStdString());

	QSettings().setValue(settingsKey::LAST_OPENED_FILE, filename);

	fileSaved(mCurrentFilename);
}

void AppContext::openRecentFile()
{
	auto lastFilename = AppContext::get().getLastOpenedFilename();
	if (lastFilename.isNull())
		return;
	AppContext::get().loadFile(lastFilename);
}

const QString AppContext::getLastOpenedFilename()
{
	return QSettings().value(settingsKey::LAST_OPENED_FILE).toString();
}

nap::Entity* AppContext::getParent(const nap::Entity& child)
{
	for (const auto& o : getObjectPointers())
	{
		if (!o->get_type().is_derived_from<nap::Entity>())
			continue;

		nap::Entity* parent = rtti_cast<nap::Entity>(o);
		auto it = std::find_if(parent->mChildren.begin(), parent->mChildren.end(),
							   [&child](nap::ObjectPtr<nap::Entity> e) -> bool { return &child == e.get(); });

		if (it != parent->mChildren.end())
			return parent;
	}
	return nullptr;
}

nap::Entity* AppContext::getOwner(const nap::Component& component)
{
	for (const auto& o : getObjects())
	{
		if (!o->get_type().is_derived_from<nap::Entity>())
			continue;

		nap::Entity* owner = *rtti_cast<nap::Entity*>(o.get());
		auto it = std::find_if(
			owner->mComponents.begin(), owner->mComponents.end(),
			[&component](nap::ObjectPtr<nap::Component> comp) -> bool { return &component == comp.get(); });

		if (it != owner->mComponents.end())
			return owner;
	}
	return nullptr;
}

nap::Entity* AppContext::createEntity(nap::Entity* parent)
{
	auto e = std::make_unique<nap::Entity>();
	e->mID = getUniqueName("New Entity");
	auto ret = e.get();
	mObjects.emplace_back(std::move(e));

	if (parent != nullptr)
	{
		parent->mChildren.emplace_back(ret);
	}

	entityAdded(ret, parent);
	return ret;
}

nap::Component* AppContext::addComponent(nap::Entity& entity, rttr::type type)
{
	assert(type.can_create_instance());
	assert(type.is_derived_from<nap::Component>());

	auto compVariant = type.create();
	auto comp = compVariant.get_value<nap::Component*>();
	comp->mID = getUniqueName(type.get_name().data());
	mObjects.emplace_back(comp);
	entity.mComponents.emplace_back(comp);

	componentAdded(*comp, entity);

	return comp;
}

nap::rtti::RTTIObject* AppContext::addObject(rttr::type type)
{
	assert(type.can_create_instance());
	assert(type.is_derived_from<nap::rtti::RTTIObject>());
	rttr::variant variant = type.create();
	nap::rtti::RTTIObject* obj = variant.get_value<nap::rtti::RTTIObject*>();
	obj->mID = getUniqueName(type.get_name().data());
	mObjects.emplace_back(obj);
	objectAdded(*obj);
	return obj;
}

std::string AppContext::getUniqueName(const std::string& suggestedName)
{
	std::string newName = suggestedName;
	int i = 2;
	while (getObject(newName))
		newName = suggestedName + "_" + std::to_string(i++);
	return newName;
}

nap::rtti::RTTIObject* AppContext::getObject(const std::string& name)
{
	auto it = std::find_if(mObjects.begin(), mObjects.end(),
						   [&name](std::unique_ptr<nap::rtti::RTTIObject>& obj) { return obj->mID == name; });
	if (it == mObjects.end())
		return nullptr;
	return it->get();
}

nap::rtti::ObjectList AppContext::getObjectPointers()
{
	ObjectList ret;
	for (auto& ob : getObjects())
		ret.emplace_back(ob.get());
	return ret;
}


void AppContext::deleteObject(nap::rtti::RTTIObject& object)
{
	if (object.get_type().is_derived_from<nap::Entity>())
	{
        nap::Entity* entity = *rtti_cast<nap::Entity*>(&object);
		nap::Entity* parent = getParent(*entity);
		if (parent)
			parent->mChildren.erase(std::remove(parent->mChildren.begin(), parent->mChildren.end(), &object));
	}
	else if (object.get_type().is_derived_from<nap::Component>())
	{
        nap::Component* component = *rtti_cast<nap::Component*>(&object);
		nap::Entity* owner = getOwner(*component);
		if (owner)
			owner->mComponents.erase(std::remove(owner->mComponents.begin(), owner->mComponents.end(), &object));
	}

	mObjects.erase(
		std::remove_if(mObjects.begin(), mObjects.end(),
					   [&object](std::unique_ptr<nap::rtti::RTTIObject>& obj) { return obj.get() == &object; }),
		mObjects.end());

	objectRemoved(object);
}

void AppContext::executeCommand(QUndoCommand* cmd)
{
	mUndoStack.push(cmd);
}

void AppContext::restoreUI()
{
	// Restore theme
	const QString& recentTheme = QSettings().value(settingsKey::LAST_THEME, napkin::TXT_DEFAULT_THEME).toString();
	getThemeManager().setTheme(recentTheme);

	openRecentFile();
}
