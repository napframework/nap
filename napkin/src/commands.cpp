#include <nap/logger.h>
#include <generic/utility.h>
#include "commands.h"
#include "appcontext.h"

using namespace napkin;

SetValueCommand::SetValueCommand(const PropertyPath& propPath, QVariant newValue)
		: mPath(propPath), mNewValue(newValue)
{
	setText(QString("Set value of %1 to %2").arg(propPath.toString(), newValue.toString()));
}

void SetValueCommand::undo()
{
	// resolve path
	nap::rtti::ResolvedRTTIPath resolvedPath = mPath.resolve();
	assert(resolvedPath.isValid());

	// set new value
	bool ok;
	rttr::variant variant = fromQVariant(resolvedPath.getType(), mOldValue, &ok);
	assert(ok);
	resolvedPath.setValue(variant);

	AppContext::get().getDocument()->propertyValueChanged(mPath);
}

void SetValueCommand::redo()
{
	// retrieve and store current value
	auto resolvedPath = mPath.resolve();
	rttr::variant oldValueVariant = resolvedPath.getValue();
	assert(toQVariant(resolvedPath.getType(), oldValueVariant, mOldValue));

	// set new value
	bool ok;
	rttr::variant variant = fromQVariant(resolvedPath.getType(), mNewValue, &ok);
	assert(ok);
	resolvedPath.setValue(variant);

	AppContext::get().getDocument()->propertyValueChanged(mPath);
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

AddObjectCommand::AddObjectCommand(const rttr::type& type, nap::rtti::RTTIObject* parent)
		: mType(type)
{

	if (parent != nullptr) {
		setText(QString("Add new %1 to %2").arg(QString::fromUtf8(type.get_name().data()),
												QString::fromStdString(parent->mID)));
		mParentName = parent->mID;
	}
	else
	{
		setText(QString("Add new %1").arg(QString::fromUtf8(type.get_name().data())));
	}

}


void AddObjectCommand::redo()
{
	// Create object
	auto parent = AppContext::get().getDocument()->getObject(mParentName);
	auto object = AppContext::get().getDocument()->addObject(mType, parent);

	// Remember for undo
	mObjectName = object->mID;

	// Notify
	AppContext::get().getDocument()->objectAdded(*object, true);
}
void AddObjectCommand::undo()
{
	AppContext::get().getDocument()->removeObject(mObjectName);
}


DeleteObjectCommand::DeleteObjectCommand(nap::rtti::RTTIObject& object) : mObjectName(object.mID)
{
	setText(QString("Deleting Object '%1'").arg(QString::fromStdString(mObjectName)));
}

void DeleteObjectCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}

void DeleteObjectCommand::redo()
{
	AppContext::get().getDocument()->removeObject(mObjectName);
}


AddEntityToSceneCommand::AddEntityToSceneCommand(nap::Scene& scene, nap::Entity& entity)
		: mSceneID(scene.mID), mEntityID(entity.mID)
{
	setText(QString("Add Entity '%1' to Scene '%2'").arg(QString::fromStdString(mEntityID),
														 QString::fromStdString(mSceneID)));
}

void AddEntityToSceneCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}

void AddEntityToSceneCommand::redo()
{
	auto scene = AppContext::get().getDocument()->getObjectT<nap::Scene>(mSceneID);
	assert(scene != nullptr);
	auto entity = AppContext::get().getDocument()->getObjectT<nap::Entity>(mEntityID);
	assert(entity != nullptr);

	nap::RootEntity rootEntity;
	rootEntity.mEntity = entity;

	// Store index for undo
	mIndex = scene->mEntities.size();

	scene->mEntities.emplace_back(rootEntity);

	AppContext::get().getDocument()->objectChanged(*scene);
}

AddArrayElementCommand::AddArrayElementCommand(const PropertyPath& prop) : mPath(prop)
{
	setText("Add element to: " + prop.toString());
}

void AddArrayElementCommand::redo()
{
	AppContext::get().getDocument()->addArrayElement(mPath);
}

void AddArrayElementCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}
