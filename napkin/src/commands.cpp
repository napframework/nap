#include <nap/logger.h>
#include <generic/utility.h>
#include "commands.h"
#include "appcontext.h"

using namespace napkin;

SetValueCommand::SetValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, QVariant newValue)
		: mObject(ptr), mPath(path), mNewValue(newValue)
{
	setText("Set value on: " + QString::fromStdString(path.toString()));
}

void SetValueCommand::undo()
{
	// resolve path
	nap::rtti::ResolvedRTTIPath resolvedPath;
	mPath.resolve(mObject, resolvedPath);
	assert(resolvedPath.isValid());

	// set new value
	bool ok;
	rttr::variant variant = fromQVariant(resolvedPath.getType(), mOldValue, &ok);
	assert(ok);
	resolvedPath.setValue(variant);

	AppContext::get().propertyValueChanged(*mObject, mPath);
}

void SetValueCommand::redo()
{
	// retrieve and store current value
	auto resolvedPath = resolve(*mObject, mPath);
	rttr::variant oldValueVariant = resolvedPath.getValue();
	assert(toQVariant(resolvedPath.getType(), oldValueVariant, mOldValue));

	// set new value
	bool ok;
	rttr::variant variant = fromQVariant(resolvedPath.getType(), mNewValue, &ok);
	assert(ok);
	resolvedPath.setValue(variant);

	AppContext::get().propertyValueChanged(*mObject, mPath);
}

SetPointerValueCommand::SetPointerValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, nap::rtti::RTTIObject* newValue) :
	mObject(ptr),
	mPath(path),
	mNewValue(newValue),
	mOldValue(nullptr)
{
	mOldValue = getPointee(*mObject, mPath);

	auto pointerPath = QString("%1::%1").arg(QString::fromStdString(ptr->mID),
											 QString::fromStdString(path.toString()));

	setText(QString("Set pointer value at '%1' to '%2'").arg(pointerPath, QString::fromStdString(newValue->mID)));
}

void SetPointerValueCommand::undo()
{
	nap::rtti::ResolvedRTTIPath resolvedPath = resolve(*mObject, mPath);
	assert(resolvedPath.isValid());

	resolvedPath.setValue(mOldValue);
}

void SetPointerValueCommand::redo()
{
	nap::rtti::ResolvedRTTIPath resolvedPath = resolve(*mObject, mPath);
	assert(resolvedPath.isValid());

	bool value_set = resolvedPath.setValue(mNewValue);
	assert(value_set);
}

AddObjectCommand::AddObjectCommand(const rttr::type& type) : mType(type)
{
	setText(QString("Add new %1").arg(QString::fromUtf8(type.get_name().data())));
}


void AddObjectCommand::redo()
{
	nap::Logger::warn("Adding scene to ResourceManager, but the objects are still held by AppContext");
	auto& ctx = AppContext::get();
	auto scene = ctx.getCore().getResourceManager()->getFactory().create(mType);
	scene->mID = QString::fromUtf8(mType.get_name().data()).toStdString();

	ctx.objectAdded(*scene, true);
}
void AddObjectCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}


void DeleteObjectCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}

void DeleteObjectCommand::redo()
{
}

AddEntityToSceneCommand::AddEntityToSceneCommand(nap::Scene& scene, nap::Entity& entity)
		: mSceneID(scene.mID), mEntityID(entity.mID)
{
	setText(QString("Add Entity '%1' to Scene '%1'").arg(QString::fromStdString(mEntityID),
														 QString::fromStdString(mSceneID)));
}

void AddEntityToSceneCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}

void AddEntityToSceneCommand::redo()
{
	auto scene = AppContext::get().getObjectT<nap::Scene>(mSceneID);
	assert(scene != nullptr);
	auto entity = AppContext::get().getObjectT<nap::Entity>(mEntityID);
	assert(entity != nullptr);

	nap::RootEntity rootEntity;
	rootEntity.mEntity = entity;

	scene->mEntities.emplace_back(rootEntity);

	AppContext::get().objectChanged(*scene);
}
