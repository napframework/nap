#include <nap/logger.h>

#include "generic/naputils.h"
#include "generic/qtutils.h"
#include "commands.h"
#include "appcontext.h"

using namespace napkin;

SetValueCommand::SetValueCommand(const PropertyPath& propPath, QVariant newValue)
		: mPath(propPath), mNewValue(newValue)
{
	setText(QString("Set value of %1 to %2").arg(QString::fromStdString(propPath.toString()),
												 newValue.toString()));
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

	if (mPath.getProperty().get_name() == nap::rtti::sIDPropertyName)
	{
		// Deal with object names separately
		AppContext::get().getDocument()->setObjectName(mPath.object(), mNewValue.toString().toStdString());
	}
	else
	{
		// Any other old value
		bool ok;
		rttr::variant variant = fromQVariant(resolvedPath.getType(), mNewValue, &ok);
		assert(ok);
		resolvedPath.setValue(variant);
		AppContext::get().getDocument()->propertyValueChanged(mPath);
	}

}

SetPointerValueCommand::SetPointerValueCommand(const PropertyPath& path, nap::rtti::RTTIObject* newValue)
		: mPath(path), mNewValue(newValue->mID), mOldValue(getPointee(path)->mID)
{
	setText(QString("Set pointer value at '%1' to '%2'").arg(QString::fromStdString(mPath.toString()),
															 QString::fromStdString(newValue->mID)));
}

void SetPointerValueCommand::undo()
{
	nap::rtti::ResolvedRTTIPath resolvedPath = mPath.resolve();
	assert(resolvedPath.isValid());

	auto old_object = AppContext::get().getDocument()->getObject(mOldValue);
	bool value_set = resolvedPath.setValue(old_object);
	assert(value_set);
	AppContext::get().getDocument()->propertyValueChanged(mPath);
}

void SetPointerValueCommand::redo()
{
	nap::rtti::ResolvedRTTIPath resolved_path = mPath.resolve();
	assert(resolved_path.isValid());

	auto new_object = AppContext::get().getDocument()->getObject(mNewValue);
	bool value_set = resolved_path.setValue(new_object);
	assert(value_set);
	AppContext::get().getDocument()->propertyValueChanged(mPath);
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

ArrayAddValueCommand::ArrayAddValueCommand(const PropertyPath& prop, long index)
		: mPath(prop), mIndex(index)
{
	setText("Add element to: " + QString::fromStdString(prop.toString()));
}

void ArrayAddValueCommand::redo()
{
	AppContext::get().getDocument()->arrayAddValue(mPath);
}

void ArrayAddValueCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}

ArrayAddNewObjectCommand::ArrayAddNewObjectCommand(const PropertyPath& prop, const nap::rtti::TypeInfo& type,
												   long index) : mPath(prop), mType(type), mIndex(index)
{}

void ArrayAddNewObjectCommand::redo()
{
	AppContext::get().getDocument()->arrayAddNewObject(mPath, mType, mIndex);
}

void ArrayAddNewObjectCommand::undo()
{
	AppContext::get().getDocument()->arrayRemoveElement(mPath, mIndex);
}

ArrayAddExistingObjectCommand::ArrayAddExistingObjectCommand(const PropertyPath& prop, nap::rtti::RTTIObject& object,
															 long index)
		: mPath(prop), mObjectName(object.mID), mIndex(index)
{}

void ArrayAddExistingObjectCommand::redo()
{
	nap::rtti::RTTIObject* object = AppContext::get().getDocument()->getObject(mObjectName);
	assert(object != nullptr);
	AppContext::get().getDocument()->arrayAddExistingObject(mPath, object, mIndex);
}

void ArrayAddExistingObjectCommand::undo()
{
	AppContext::get().getDocument()->arrayRemoveElement(mPath, mIndex);
}

ArrayRemoveElementCommand::ArrayRemoveElementCommand(const PropertyPath& array_prop, long index)
		: mPath(array_prop), mIndex(index)
{}

void ArrayRemoveElementCommand::redo()
{
	AppContext::get().getDocument()->arrayRemoveElement(mPath, mIndex);
}

void ArrayRemoveElementCommand::undo()
{
	// TODO: Need store on redo and be able to reinstate the original value
	nap::Logger::fatal("No undo supported");
}


ArrayMoveElementCommand::ArrayMoveElementCommand(const PropertyPath& array_prop, long fromIndex, long toIndex)
		: mPath(array_prop), mFromIndex(fromIndex), mToIndex(toIndex)
{
	setText(QString("Reorder '%1' from %2 to %3").arg(QString::fromStdString(array_prop.toString()),
													  QString::number(fromIndex), QString::number(toIndex)));
}

void ArrayMoveElementCommand::redo()
{
	// Also store indexes that may have shifted due to the operation so we can undo
	mOldIndex = (mFromIndex > mToIndex) ? mFromIndex + 1 : mFromIndex;
	mNewIndex = AppContext::get().getDocument()->arrayMoveElement(mPath, mFromIndex, mToIndex);
}

void ArrayMoveElementCommand::undo()
{
	AppContext::get().getDocument()->arrayMoveElement(mPath, mNewIndex, mOldIndex);
}
