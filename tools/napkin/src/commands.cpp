/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "naputils.h"
#include "commands.h"
#include "appcontext.h"

#include <nap/logger.h>

using namespace napkin;

// TODO: All commands need to get their document passed in
// so we may support multiple documents in a far future

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SetValueCommand::SetValueCommand(const PropertyPath& propPath, QVariant newValue)
		: mPath(propPath), mNewValue(newValue), QUndoCommand()
{
	setText(QString("Set value of %1 to %2").arg(QString::fromStdString(propPath.toString()),
												 newValue.toString()));
}

void SetValueCommand::undo()
{
	bool ok;
	rttr::variant variant = fromQVariant(mPath.getType(), mOldValue, &ok);
	assert(ok);
	mPath.setValue(variant);

	AppContext::get().getDocument()->propertyValueChanged(mPath);
}

void SetValueCommand::redo()
{
	// retrieve and store current value
	bool success = toQVariant(mPath.getType(), mPath.getValue(), mOldValue);
	assert(success);

	auto& ctx = AppContext::get();

	if (mPath.getProperty().get_name() == nap::rtti::sIDPropertyName)
	{
		// Deal with object names separately
		ctx.getDocument()->setObjectName(*mPath.getObject(), mNewValue.toString().toStdString());
		ctx.selectionChanged({mPath.getObject()});
	}
	else
	{
		// Any other old value
		bool ok;
		rttr::variant variant = fromQVariant(mPath.getType(), mNewValue, &ok);
		if (!ok)
		{
			nap::Logger::debug("Invalid value %s for type %s",
					mNewValue.toString().toStdString().c_str(), mPath.getType().get_name().data());
			return;
		}

		mPath.setValue(variant);

		ctx.getDocument()->propertyValueChanged(mPath);
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SetPointerValueCommand::SetPointerValueCommand(const PropertyPath& path, nap::rtti::Object* newValue)
		: mPath(path), QUndoCommand()
{
	mNewValue = newValue ? newValue->mID : "";
	setText(QString("Set pointer value at '%1' to '%2'").arg(QString::fromStdString(mPath.toString()),
															 QString::fromStdString(mNewValue)));
	auto pointee = path.getPointee();
	if (pointee != nullptr)
		mOldValue = pointee->mID;
	else
		mOldValue.clear();
}

void SetPointerValueCommand::undo()
{
	nap::rtti::ResolvedPath resolvedPath = mPath.resolve();
	assert(resolvedPath.isValid());


	auto old_object = AppContext::get().getDocument()->getObject(mOldValue);
	if (old_object == nullptr)
	{
		bool value_set = resolvedPath.setValue(nullptr);
		nap::Logger::fatal("Sorry, can't clear pointer properties");
	}
	else
	{
		bool value_set = resolvedPath.setValue(old_object);
		assert(value_set);
	}

	AppContext::get().getDocument()->propertyValueChanged(mPath);
}

void SetPointerValueCommand::redo()
{
	nap::rtti::ResolvedPath resolved_path = mPath.resolve();
	assert(resolved_path.isValid());

	nap::rtti::Object* new_object = AppContext::get().getDocument()->getObject(mNewValue);

	mPath.setPointee(new_object);

	AppContext::get().getDocument()->propertyValueChanged(mPath);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AddObjectCommand::AddObjectCommand(const rttr::type& type, nap::rtti::Object* parent)
		: mType(type), QUndoCommand()
{
	auto type_name = QString::fromUtf8(type.get_name().data());

	if (parent != nullptr)
	{
		setText(QString("Add new %1 to %2").arg(type_name, QString::fromStdString(parent->mID)));
		mParentName = parent->mID;
	}
	else
	{
		setText(QString("Add new %1").arg(type_name));
	}

}


void AddObjectCommand::redo()
{
	auto& ctx = AppContext::get();

	// Create object
	auto parent = ctx.getDocument()->getObject(mParentName);
	auto object = ctx.getDocument()->addObject(mType, parent, true);

	// Remember for undo
	mObjectName = object->mID;

	ctx.selectionChanged({object});
}

void AddObjectCommand::undo()
{
	AppContext::get().getDocument()->removeObject(mObjectName);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddComponentCommand::AddComponentCommand(nap::Entity& entity, nap::rtti::TypeInfo type)
: mEntityName(entity.mID), mType(type)
{

}

void AddComponentCommand::redo()
{
	auto doc = AppContext::get().getDocument();
	nap::Entity* entity = doc->getObject<nap::Entity>(mEntityName);
	assert(entity != nullptr);
	auto comp = AppContext::get().getDocument()->addComponent(*entity, mType);
	mComponentName = comp->mID;
}

void AddComponentCommand::undo()
{
	nap::Logger::fatal("Undo is not available...");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DeleteObjectCommand::DeleteObjectCommand(nap::rtti::Object& object) : mObjectName(object.mID), QUndoCommand()
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


AddEntityToSceneCommand::AddEntityToSceneCommand(nap::Scene& scene, nap::Entity& entity)
		: mSceneID(scene.mID), mEntityID(entity.mID), QUndoCommand()
{
	setText(QString("Add Entity '%1' to Scene '%2'").arg(QString::fromStdString(mEntityID),
														 QString::fromStdString(mSceneID)));
}

void AddEntityToSceneCommand::undo()
{
	auto doc = AppContext::get().getDocument();
	auto scene = doc->getObject<nap::Scene>(mSceneID);
	assert(scene);
	doc->removeEntityFromScene(*scene, mIndex);
}

void AddEntityToSceneCommand::redo()
{
	auto doc = AppContext::get().getDocument();
	auto scene = doc->getObject<nap::Scene>(mSceneID);
	assert(scene);
	auto entity = doc->getObject<nap::Entity>(mEntityID);
	assert(entity);

	mIndex = doc->addEntityToScene(*scene, *entity);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AddChildEntityCommand::AddChildEntityCommand(nap::Entity& parent, nap::Entity& child)
		: mParentID(parent.mID), mChildID(child.mID) {

}

void AddChildEntityCommand::redo()
{
	auto doc = AppContext::get().getDocument();
	auto child = doc->getObject<nap::Entity>(mChildID);
	auto parent = doc->getObject<nap::Entity>(mParentID);
	mIndex = doc->addChildEntity(*parent, *child);
}

void AddChildEntityCommand::undo()
{
	auto doc = AppContext::get().getDocument();
	auto parent = doc->getObject<nap::Entity>(mParentID);
	assert(parent);
	doc->removeChildEntity(*parent, mIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RemoveChildEntityCommand::RemoveChildEntityCommand(nap::Entity& parent, int index)
	: QUndoCommand(), mParentID(parent.mID), mIndex(index)
{
	setText("Remove child entity");
}

void RemoveChildEntityCommand::redo()
{
	auto doc = AppContext::get().getDocument();
	auto parent = doc->getObject<nap::Entity>(mParentID);
	doc->removeChildEntity(*parent, mIndex);
}

void RemoveChildEntityCommand::undo()
{
	assert(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RemoveCommand::RemoveCommand(const PropertyPath& path)
	: mPath(path), QUndoCommand()
{
	setText(QString("Remove Entity %1").arg(QString::fromStdString(mPath.toString())));
}

void RemoveCommand::redo()
{
	AppContext::get().getDocument()->remove(mPath);
}
void RemoveCommand::undo()
{
	nap::Logger::fatal("Undo not supported yet");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArrayAddValueCommand::ArrayAddValueCommand(const PropertyPath& prop, size_t index)
		: mPath(prop), mIndex(index), QUndoCommand()
{
	setText("Add element to: " + QString::fromStdString(prop.toString()));
}

ArrayAddValueCommand::ArrayAddValueCommand(const PropertyPath& prop) : mPath(prop), QUndoCommand()
{
	mIndex = prop.getArrayLength();
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArrayAddNewObjectCommand::ArrayAddNewObjectCommand(const PropertyPath& prop, const nap::rtti::TypeInfo& type,
												   size_t index) : mPath(prop), mType(type), mIndex(index), QUndoCommand()
{
	setText(QString("Add %1 to %2").arg(QString::fromUtf8(type.get_name().data()),
										QString::fromStdString(prop.toString())));
}

ArrayAddNewObjectCommand::ArrayAddNewObjectCommand(const PropertyPath& prop, const nap::rtti::TypeInfo& type)
		: mPath(prop), mType(type), QUndoCommand()
{
	setText(QString("Add %1 to %2").arg(QString::fromUtf8(type.get_name().data()),
										QString::fromStdString(prop.toString())));
	mIndex = prop.getArrayLength();
}


void ArrayAddNewObjectCommand::redo()
{
	AppContext::get().getDocument()->arrayAddNewObject(mPath, mType, mIndex);
}

void ArrayAddNewObjectCommand::undo()
{
	AppContext::get().getDocument()->arrayRemoveElement(mPath, mIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArrayAddExistingObjectCommand::ArrayAddExistingObjectCommand(const PropertyPath& prop, nap::rtti::Object& object,
															 size_t index)
		: mPath(prop), mObjectName(object.mID), mIndex(index), QUndoCommand()
{
	setText(QString("Add %1 to %2").arg(QString::fromStdString(object.mID),
										QString::fromStdString(prop.toString())));
}


ArrayAddExistingObjectCommand::ArrayAddExistingObjectCommand(const PropertyPath& prop, nap::rtti::Object& object)
		: mPath(prop), mObjectName(object.mID), QUndoCommand()
{
	setText(QString("Add %1 to %2").arg(QString::fromStdString(object.mID),
										QString::fromStdString(prop.toString())));
	mIndex = prop.getArrayLength();
}


void ArrayAddExistingObjectCommand::redo()
{
	nap::rtti::Object* object = AppContext::get().getDocument()->getObject(mObjectName);
	assert(object != nullptr);
	AppContext::get().getDocument()->arrayAddExistingObject(mPath, object, mIndex);
}

void ArrayAddExistingObjectCommand::undo()
{
	AppContext::get().getDocument()->arrayRemoveElement(mPath, mIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArrayRemoveElementCommand::ArrayRemoveElementCommand(const PropertyPath& array_prop, size_t index)
		: mPath(array_prop), mIndex(index), QUndoCommand()
{}

void ArrayRemoveElementCommand::redo()
{
	mValue = AppContext::get().getDocument()->arrayGetElement(mPath, mIndex);
	AppContext::get().getDocument()->arrayRemoveElement(mPath, mIndex);
}

void ArrayRemoveElementCommand::undo()
{
	// TODO: Need store on redo and be able to reinstate the original value
	nap::Logger::fatal("No undo supported");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArrayMoveElementCommand::ArrayMoveElementCommand(const PropertyPath& array_prop, size_t fromIndex, size_t toIndex)
		: mPath(array_prop), mFromIndex(fromIndex), mToIndex(toIndex), QUndoCommand()
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RemoveComponentCommand::RemoveComponentCommand(nap::Component& comp) : mComponentName(comp.mID)
{
	auto doc = AppContext::get().getDocument();
	auto owner = doc->getOwner(comp);
	assert(owner != nullptr);
	mEntityName = owner->mID;
}

void RemoveComponentCommand::redo()
{
	auto doc = AppContext::get().getDocument();
	auto owner = doc->getObject<nap::Entity>(mEntityName);
	assert(owner != nullptr);
	auto component = doc->getObject<nap::Component>(mComponentName);
	assert(owner != nullptr);
	doc->removeComponent(*component);
}

void RemoveComponentCommand::undo()
{
	QUndoCommand::undo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ReplaceEmbeddedPointerCommand::ReplaceEmbeddedPointerCommand(const PropertyPath& path, rttr::type objectType)
		: mPath(path), mType(objectType)
{

}

void ReplaceEmbeddedPointerCommand::redo()
{
	auto pointee = mPath.getPointee();
	auto doc = mPath.getDocument();
	if (pointee) // TODO: Serialize and store for undo
		doc->removeObject(*pointee);

	auto obj = doc->addObject(mType, nullptr, false);
	mCreatedObject = {*obj, *doc};
	mPath.setPointee(obj);
	doc->propertyValueChanged(mPath);
}

void ReplaceEmbeddedPointerCommand::undo()
{
	nap::Logger::fatal("Not supported");
}
