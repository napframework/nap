/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "standarditemsobject.h"
#include "commands.h"
#include "sceneservice.h"
#include "naputils.h"
#include "napkin-resources.h"

using namespace napkin;

//////////////////////////////////////////////////////////////////////////
// RegularResourcesItem 
//////////////////////////////////////////////////////////////////////////

napkin::RegularResourcesItem::RegularResourcesItem()
{
	setEditable(false);
	setText("Resources");
}

QVariant napkin::RegularResourcesItem::data(int role) const
{
	switch (role)
	{
	case Qt::DecorationRole:
		return AppContext::get().getResourceFactory().getIcon(QRC_ICONS_RTTIOBJECT);
	default:
		return QStandardItem::data(role);
	}
}


//////////////////////////////////////////////////////////////////////////
// EntityResourcesItem 
//////////////////////////////////////////////////////////////////////////

napkin::EntityResourcesItem::EntityResourcesItem()
{
	setEditable(false);
	setText("Entities");
}


QVariant napkin::EntityResourcesItem::data(int role) const
{
	switch (role)
	{
	case Qt::DecorationRole:
		return AppContext::get().getResourceFactory().getIcon(QRC_ICONS_ENTITY);
	default:
		return QStandardItem::data(role);
	}
}


//////////////////////////////////////////////////////////////////////////
// ObjectItem 
//////////////////////////////////////////////////////////////////////////

ObjectItem::ObjectItem(nap::rtti::Object* o, bool isPointer)
		: QObject(), mObject(o), mIsPointer(isPointer)
{
	auto& ctx = AppContext::get();
	setText(QString::fromStdString(o->mID));
	connect(&ctx, &AppContext::propertyValueChanged, this, &ObjectItem::onPropertyValueChanged);
	connect(&ctx, &AppContext::objectRemoved, this, &ObjectItem::onObjectRemoved);
	refresh();
}

const PropertyPath ObjectItem::propertyPath() const
{
	auto parentEntity = dynamic_cast<EntityItem*>(parentItem());
	QStringList path;

	auto thisEntity = this;
	while (parentEntity)
	{
		auto thisID = QString::fromStdString(thisEntity->getObject()->mID);
		if (dynamic_cast<const ComponentItem*>(thisEntity))
		{
			// TODO: Remove this case if multiple entities of the same type/name may be added to entities
			path.insert(0, thisID);
		}
		else
		{
			auto thisIndex = QString::number(parentEntity->nameIndex(*thisEntity));
			auto id = QString("%1:%2").arg(thisID, thisIndex);
			path.insert(0, id);
		}

		thisEntity = parentEntity;
		parentEntity = dynamic_cast<EntityItem*>(parentEntity->parentItem());
	}

	path.insert(0, QString::fromStdString(thisEntity->getObject()->mID));

	auto pathStr = "/" + path.join('/');

	return PropertyPath(absolutePath(), *(AppContext::get().getDocument()));
}

std::string ObjectItem::absolutePath() const
{
	std::vector<std::string> path;
	path.emplace(path.begin(), unambiguousName());

	auto pitem = dynamic_cast<ObjectItem*>(parentItem());

	while (pitem)
	{
		path.emplace(path.begin(), pitem->unambiguousName());
		pitem = dynamic_cast<ObjectItem*>(pitem->parentItem());
	}
	return "/" + nap::utility::joinString(path, "/");
}

bool ObjectItem::isPointer() const
{
	if (mIsPointer)
		return true;

	auto parentItem = dynamic_cast<ObjectItem*>(QStandardItem::parent());
	return parentItem && parentItem->isPointer();
}

void ObjectItem::refresh()
{
	QStandardItem::setText(getName());
}

const QString ObjectItem::getName() const
{
	return QString::fromStdString(mObject->mID);
}

nap::rtti::Object* ObjectItem::getObject() const
{
	return mObject;
}

void ObjectItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole)
	{
		PropertyPath prop_path(*mObject, nap::rtti::Path::fromString(nap::rtti::sIDPropertyName), *AppContext::get().getDocument());

		// Ensure filename exists
		if (nap::rtti::hasFlag(prop_path.getProperty(), nap::rtti::EPropertyMetaData::FileLink))
		{
			auto filename = getAbsoluteResourcePath(value.toString());
			if (!QFileInfo::exists(filename))
			{
				nap::Logger::fatal("File not found: %s", filename.toStdString().c_str());
				return;
			}
		}

		AppContext::get().executeCommand(new SetValueCommand(prop_path, value.toString()));
		return;
	}
	QStandardItem::setData(value, role);
}

QVariant ObjectItem::data(int role) const
{
	switch (role)
	{
	case Qt::DecorationRole:
		assert(getObject() != nullptr);
		return AppContext::get().getResourceFactory().getIcon(*getObject());
	case Qt::ForegroundRole:
	{
		return isPointer() ?
			AppContext::get().getThemeManager().getColor(theme::color::dimmedItem) :
			QStandardItem::data(role);
	}
	default:
		return QStandardItem::data(role);
	}
}

void ObjectItem::removeChildren()
{
	while (rowCount())
		removeRow(0);
}

QString ObjectItem::instanceName() const
{
	auto parent = parentItem();
	if (parent)
	{
		int i = 0;
		for (int row = 0, len = parent->rowCount(); row < len; row++)
		{
			auto sibling = dynamic_cast<ObjectItem*>(parent->child(row));
			if (!sibling)
				continue;

			// Must be of either Entity items
			if (! (dynamic_cast<EntityInstanceItem*>(sibling) || dynamic_cast<EntityItem*>(sibling)))
				continue;

			if (sibling == this)
				break;

			if (sibling->mObject->mID == mObject->mID)
				i++;
		}
		return QString("%1:%2").arg(mObject->mID.c_str(), QString::number(i));
	}

	assert(false);
	return "FAULTY";
}

std::string ObjectItem::componentPath() const
{
	{
		auto ownerItem = dynamic_cast<EntityInstanceItem*>(parentItem());
		if (ownerItem)
		{
			std::vector<std::string> namePath = {mObject->mID};

			auto parent = ownerItem;
			while (parent != nullptr && !dynamic_cast<RootEntityItem*>(parent))
			{
				namePath.insert(namePath.begin(), parent->instanceName().toStdString());
				parent = dynamic_cast<EntityInstanceItem*>(parent->parentItem());
			}

			return "./" + nap::utility::joinString(namePath, "/");
		}
	}
	// TODO: Merge this with the above code....
	{
		auto ownerItem = dynamic_cast<EntityItem*>(parentItem());
		if (ownerItem)
		{
			std::vector<std::string> namePath = {mObject->mID};

			auto parent = ownerItem;
			while (parent)
			{
				if (!dynamic_cast<EntityItem*>(parent->parentItem()))
					break;
				namePath.insert(namePath.begin(), parent->instanceName().toStdString());
				parent = dynamic_cast<EntityItem*>(parent->parentItem());
			}

			return "./" + nap::utility::joinString(namePath, "/");
		}
	}
	assert(false);
	return {"INVALID"};
}

int ObjectItem::childIndex(const ObjectItem& childItem) const
{
	int i = 0;
	for (int row = 0; row < rowCount(); row++)
	{
		auto otherChildItem = child(row, 0);
		if (otherChildItem == &childItem)
			return i;
		i++;
	}
	return -1;
}

int ObjectItem::nameIndex(const ObjectItem& childItem) const
{
	auto childName = childItem.getObject()->mID;
	int index = 0;
	for (int row = 0; row < rowCount(); row++)
	{
		auto otherChildItem = dynamic_cast<ObjectItem*>(child(row, 0));
		if (!otherChildItem)
			continue;
		if (otherChildItem == &childItem)
			return index;

		auto id = otherChildItem->getObject()->mID;
		if (otherChildItem->getObject()->mID == childName)
			++index;
	}
	return -1;
}


void ObjectItem::onPropertyValueChanged(PropertyPath path)
{
	if (path.getObject() == mObject)
		refresh();
}

void ObjectItem::onObjectRemoved(nap::rtti::Object* o)
{
	if (o == mObject)
	{
		auto parent = parentItem();
		if (parent)
			parent->removeRow(index().row());
	}
}

const std::string ObjectItem::unambiguousName() const
{
	return getObject()->mID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EntityItem::EntityItem(nap::Entity& entity, bool isPointer) : ObjectItem(&entity, isPointer)
{

	for (auto& child : entity.mChildren)
		onEntityAdded(child.get(), &entity);

	for (auto& comp : entity.mComponents)
		onComponentAdded(comp.get(), &entity);

	auto& ctx = AppContext::get();
	connect(&ctx, &AppContext::componentAdded, this, &EntityItem::onComponentAdded);
	connect(&ctx, &AppContext::entityAdded, this, &EntityItem::onEntityAdded);
	connect(&ctx, &AppContext::propertyValueChanged, this, &EntityItem::onPropertyValueChanged);
}

nap::Entity* EntityItem::getEntity()
{
	return rtti_cast<nap::Entity>(mObject);
}

void EntityItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	if (parent != mObject)
		return;

	appendRow({new EntityItem(*e, true), new RTTITypeItem(e->get_type())});
}

void EntityItem::onComponentAdded(nap::Component* comp, nap::Entity* owner)
{
	if (owner != mObject)
		return;

	auto compItem = new ComponentItem(*comp);
	auto compTypeItem = new RTTITypeItem(comp->get_type());
	appendRow({compItem, compTypeItem});
}

void EntityItem::onPropertyValueChanged(const PropertyPath& path)
{
	PropertyPath childrenPath(*getEntity(), nap::rtti::Path::fromString("Children"), *AppContext::get().getDocument());
	assert(childrenPath.isValid());
	if (path != childrenPath)
		return;

	removeChildren();
	for (auto child : getEntity()->mChildren)
		onEntityAdded(child.get(), getEntity());
}

const std::string EntityItem::unambiguousName() const
{
	if (auto entityItem = dynamic_cast<EntityItem*>(parentItem()))
	{
		return ObjectItem::unambiguousName() + ":" + std::to_string(entityItem->nameIndex(*this));
	}
	return ObjectItem::unambiguousName();
}


//////////////////////////////////////////////////////////////////////////
// Group Item
//////////////////////////////////////////////////////////////////////////

napkin::GroupItem::GroupItem(nap::Group& group) : ObjectItem(&group, false)
{
	for (auto& resource : group.mResources)
	{
		appendRow(new ObjectItem(resource.get()));
	}
}

QVariant napkin::GroupItem::data(int role) const
{
	switch (role)
	{
	case Qt::DecorationRole:
		return AppContext::get().getResourceFactory().getIcon(QRC_ICONS_GROUP);
	default:
		return QStandardItem::data(role);
	}
}

nap::Group* napkin::GroupItem::getGroup()
{
	return rtti_cast<nap::Group>(mObject);
}

//////////////////////////////////////////////////////////////////////////
// ComponentItem
//////////////////////////////////////////////////////////////////////////

ComponentItem::ComponentItem(nap::Component& comp) : ObjectItem(&comp, false)
{ }

nap::Component& ComponentItem::getComponent()
{
	auto& o = *rtti_cast<nap::Component*>(mObject);
	return *o;
}


//////////////////////////////////////////////////////////////////////////
// SceneItem
//////////////////////////////////////////////////////////////////////////

SceneItem::SceneItem(nap::Scene& scene) : ObjectItem(&scene, false)
{
	for (auto& entity : scene.getEntityResourcesRef())
		appendRow(new RootEntityItem(entity));
}


//////////////////////////////////////////////////////////////////////////
// EntityInstanceItem
//////////////////////////////////////////////////////////////////////////

EntityInstanceItem::EntityInstanceItem(nap::Entity& e, nap::RootEntity& rootEntity)
		: mRootEntity(rootEntity), ObjectItem(&e, false)
{
	assert(&mRootEntity);
	for (auto comp : e.mComponents)
		onComponentAdded(comp.get(), &entity());
	for (auto childEntity : e.mChildren)
		if (childEntity.get())
			onEntityAdded(childEntity.get(), &entity());

	auto ctx = &AppContext::get();
	connect(ctx, &AppContext::componentAdded, this, &EntityInstanceItem::onComponentAdded);
	connect(ctx, &AppContext::entityAdded, this, &EntityInstanceItem::onEntityAdded);
}

nap::RootEntity& EntityInstanceItem::rootEntity() const
{
	return mRootEntity;
}


//////////////////////////////////////////////////////////////////////////
// EntityInstanceItem
//////////////////////////////////////////////////////////////////////////

void EntityInstanceItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	if (parent != &entity())
		return;

	appendRow(new EntityInstanceItem(*e, mRootEntity));
}

void EntityInstanceItem::onComponentAdded(nap::Component* c, nap::Entity* owner)
{
	if (owner != &entity())
		return;

	appendRow(new ComponentInstanceItem(*c, mRootEntity));
}

const PropertyPath EntityInstanceItem::propertyPath() const
{
	auto ownerItem = dynamic_cast<EntityInstanceItem*>(parentItem());
	if (ownerItem)
	{
		std::vector<std::string> namePath = {mObject->mID};

		auto parent = ownerItem;
		while (parent != nullptr && !dynamic_cast<RootEntityItem*>(parent))
		{
			namePath.insert(namePath.begin(), parent->instanceName().toStdString());
			parent = dynamic_cast<EntityInstanceItem*>(parent->parentItem());
		}

		std::string path = "./" + nap::utility::joinString(namePath, "/");

		return PropertyPath(absolutePath(), *AppContext::get().getDocument());
	}
	return PropertyPath(absolutePath(), *AppContext::get().getDocument());
}

const std::string EntityInstanceItem::unambiguousName() const
{
	if (auto parentObjectItem = dynamic_cast<ObjectItem*>(parentItem()))
		return ObjectItem::unambiguousName() + ":" + std::to_string(parentObjectItem->nameIndex(*this));
	return ObjectItem::unambiguousName();
}


//////////////////////////////////////////////////////////////////////////
// RootEntityItem
//////////////////////////////////////////////////////////////////////////

RootEntityItem::RootEntityItem(nap::RootEntity& e)
		: mRootEntity(e), EntityInstanceItem(*e.mEntity.get(), e)
{
	assert(&mRootEntity);
}

const PropertyPath RootEntityItem::propertyPath() const
{
	return EntityInstanceItem::propertyPath();
}

nap::RootEntity& RootEntityItem::rootEntity() const
{
	assert(&mRootEntity);
	return mRootEntity;
}

void RootEntityItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	if (parent != mRootEntity.mEntity.get())
		return;

	appendRow(new EntityInstanceItem(*e, mRootEntity));
}

void RootEntityItem::onComponentAdded(nap::Component* c, nap::Entity* owner)
{
	if (owner != mRootEntity.mEntity.get())
		return;

	appendRow(new ComponentInstanceItem(*c, mRootEntity));
}


//////////////////////////////////////////////////////////////////////////
// ComponentInstanceItem
//////////////////////////////////////////////////////////////////////////

ComponentInstanceItem::ComponentInstanceItem(nap::Component& comp, nap::RootEntity& rootEntity)
		: ObjectItem(&comp, false), mRootEntity(rootEntity)
{
	assert(&mRootEntity);
}

const PropertyPath ComponentInstanceItem::propertyPath() const
{
	return PropertyPath(absolutePath(), *AppContext::get().getDocument());
}

nap::Component& ComponentInstanceItem::component() const
{
	return *dynamic_cast<nap::Component*>(mObject);
}

nap::RootEntity& ComponentInstanceItem::rootEntity() const
{
	return mRootEntity;
}

QVariant ComponentInstanceItem::data(int role) const
{
	if (role == Qt::TextColorRole)
	{
		if (instanceProperties())
		{
			return AppContext::get().getThemeManager().getColor(theme::color::instancePropertyOverride);
		}
	}
	return ObjectItem::data(role);
}

nap::ComponentInstanceProperties* ComponentInstanceItem::instanceProperties() const
{
	if (mInstancePropertiesResolved)
		return hasInstanceProperties() ? &mInstanceProperties : nullptr;

	for (const auto& instprops : mRootEntity.mInstanceProperties)
	{
		if (instprops.mTargetComponent.get() != &component())
			continue;
		if (!isComponentInstancePathEqual(mRootEntity, *instprops.mTargetComponent.get(),
										  instprops.mTargetComponent.toString(), componentPath()))
			continue;

		mInstanceProperties = instprops;
		break;
	}

	mInstancePropertiesResolved = true;

	return &mInstanceProperties;
}

bool ComponentInstanceItem::hasInstanceProperties() const
{
	return mInstanceProperties.mTargetComponent.get();
}
