/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "standarditemsobject.h"
#include "commands.h"
#include "sceneservice.h"
#include "naputils.h"
#include "napkin-resources.h"

RTTI_DEFINE_BASE(napkin::RegularResourcesItem)
RTTI_DEFINE_BASE(napkin::EntityResourcesItem)
RTTI_DEFINE_BASE(napkin::ObjectItem)
RTTI_DEFINE_BASE(napkin::EntityItem)
RTTI_DEFINE_BASE(napkin::GroupItem)
RTTI_DEFINE_BASE(napkin::SceneItem)
RTTI_DEFINE_BASE(napkin::ComponentItem)
RTTI_DEFINE_BASE(napkin::EntityInstanceItem)
RTTI_DEFINE_BASE(napkin::RootEntityItem)
RTTI_DEFINE_BASE(napkin::ComponentInstanceItem)

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
		: mObject(o), mIsPointer(isPointer)
{
	auto& ctx = AppContext::get();
	setText(QString::fromStdString(o->mID));
	connect(&ctx, &AppContext::propertyValueChanged, this, &ObjectItem::onPropertyValueChanged);
	connect(&ctx, &AppContext::objectRemoved, this, &ObjectItem::onObjectRemoved);
	refresh();
}

const PropertyPath ObjectItem::propertyPath() const
{
	auto parentEntity = qobject_cast<EntityItem*>(parentItem());
	QStringList path;

	auto thisEntity = this;
	while (parentEntity)
	{
		auto thisID = QString::fromStdString(thisEntity->getObject()->mID);
		if (qobject_cast<const ComponentItem*>(thisEntity))
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
		parentEntity = qobject_cast<EntityItem*>(parentEntity->parentItem());
	}

	path.insert(0, QString::fromStdString(thisEntity->getObject()->mID));

	auto pathStr = "/" + path.join('/');

	return PropertyPath(absolutePath(), *(AppContext::get().getDocument()));
}

std::string ObjectItem::absolutePath() const
{
	std::vector<std::string> path;
	path.emplace(path.begin(), unambiguousName());
	auto pitem = qobject_cast<ObjectItem*>(parentItem());
	while (pitem)
	{
		path.emplace(path.begin(), pitem->unambiguousName());
		pitem = qobject_cast<ObjectItem*>(pitem->parentItem());
	}
	return "/" + nap::utility::joinString(path, "/");
}

bool ObjectItem::isPointer() const
{
	if (mIsPointer)
		return true;

	auto parentItem = qobject_cast<ObjectItem*>(this->parentItem());
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
			auto sibling = qobject_cast<ObjectItem*>(qitem_cast(parent->child(row)));
			if (!sibling)
				continue;

			// Must be of either Entity items
			if (!(qobject_cast<EntityInstanceItem*>(sibling) || qobject_cast<EntityItem*>(sibling)))
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
		auto ownerItem = qobject_cast<EntityInstanceItem*>(parentItem());
		if (ownerItem)
		{
			std::vector<std::string> namePath = {mObject->mID};
			auto parent = ownerItem;
			while (parent != nullptr && !qobject_cast<RootEntityItem*>(parent))
			{
				namePath.insert(namePath.begin(), parent->instanceName().toStdString());
				parent = qobject_cast<EntityInstanceItem*>(parent->parentItem());
			}

			return "./" + nap::utility::joinString(namePath, "/");
		}
	}
	// TODO: Merge this with the above code....
	{
		auto ownerItem = qobject_cast<EntityItem*>(parentItem());
		if (ownerItem)
		{
			std::vector<std::string> namePath = {mObject->mID};

			auto parent = ownerItem;
			while (parent)
			{
				auto parent_entity = qobject_cast<EntityItem*>(parent->parentItem());
				if (parent_entity == nullptr)
					break;

				namePath.insert(namePath.begin(), parent->instanceName().toStdString());
				parent = parent_entity;
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
		auto otherChildItem = qobject_cast<ObjectItem*>(qitem_cast(child(row, 0)));
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
	if (auto entityItem = qobject_cast<EntityItem*>(parentItem()))
	{
		return ObjectItem::unambiguousName() + ":" + std::to_string(entityItem->nameIndex(*this));
	}
	return ObjectItem::unambiguousName();
}


//////////////////////////////////////////////////////////////////////////
// Group Item
//////////////////////////////////////////////////////////////////////////

napkin::GroupItem::GroupItem(nap::IGroup& group) : ObjectItem(&group, false)
{
	// Get group members property
	PropertyPath members_path(*getGroup(), getGroup()->getMembersProperty(), *AppContext::get().getDocument());

	// Create child item for every member object
	members_path.iterateChildren([this](const PropertyPath& path)
		{
			// Append members as objects
			this->appendRow(
				{
					new ObjectItem(path.getPointee()),
					new RTTITypeItem(path.getPointee()->get_type())
				});
			return true;
		}, 0);

	// Get sub-group property
	PropertyPath children_path(*getGroup(), getGroup()->getChildrenProperty(), *AppContext::get().getDocument());

	// Create child group item for every sub-group
	children_path.iterateChildren([this](const PropertyPath& path)
		{
			assert(path.getPointee()->get_type().is_derived_from(RTTI_OF(nap::IGroup)));
			GroupItem* group_item = new GroupItem(*rtti_cast<nap::IGroup>(path.getPointee()));
			this->connect(group_item, &GroupItem::childAdded, this, &GroupItem::childAdded);
			this->appendRow( { group_item, new RTTITypeItem(path.getPointee()->get_type()) });
			return true;
		}, 0);

	// Listen to data-model changes
	connect(&AppContext::get(), &AppContext::propertyChildInserted, this, &GroupItem::onPropertyChildInserted);
	connect(&AppContext::get(), &AppContext::propertyChildRemoved, this, &GroupItem::onPropertyChildRemoved);
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


nap::IGroup* napkin::GroupItem::getGroup()
{
	return rtti_cast<nap::IGroup>(mObject);
}


void napkin::GroupItem::onPropertyChildRemoved(const PropertyPath& path, int index)
{
	// Check if this group has changed
	nap::IGroup* group = getGroup();
	if (!(path.getObject() == group))
		return;

	// Figure out actual child index, based on edited property
	// The group has 2 properties: members and children, but the group item
	// displays them as 1 long list. First the members, then the children.
	// We therefore offset the index based on the edited property.
	int child_index = index;
	if (path.getProperty() == group->getChildrenProperty())
	{
		PropertyPath array_path(*group, group->getMembersProperty(), *AppContext::get().getDocument());
		child_index += array_path.getArrayLength();
	}
	this->removeRow(child_index);
}


void napkin::GroupItem::onPropertyChildInserted(const PropertyPath& path, int index)
{
	// Check if this group has changed
	nap::IGroup* group = getGroup();
	if (!(path.getObject() == group))
		return;

	// Check if an item has been added to the members property.
	// If so, figure out the correct index to insert the child.
	// 	   
	// The NAP group has 2 properties: members and children, but the group item
	// displays them as 1 long list. First the members, then the children.
	// We therefore insert the item after the last member, but before the first child group
	if (path.getProperty() == group->getMembersProperty())
	{
		PropertyPath array_path(*group, group->getMembersProperty(), *AppContext::get().getDocument());
		int row_index = array_path.getArrayLength() - 1;
		auto member_el = path.getArrayElement(index);
		ObjectItem* new_item = new ObjectItem(member_el.getPointee());
		this->insertRow(row_index, { new_item, new RTTITypeItem(member_el.getPointee()->get_type()) });
		childAdded(*this, *new_item);
	}
	// Otherwise append it to the end
	else
	{
		// Get item from array
		assert(path.getObject()->get_type().is_derived_from(RTTI_OF(nap::IGroup)));
		auto member_el = path.getArrayElement(index);
		GroupItem* new_group = new GroupItem(*rtti_cast<nap::IGroup>(member_el.getPointee()));
		this->connect(new_group, &GroupItem::childAdded, this, &GroupItem::childAdded);
		this->appendRow( { new_group, new RTTITypeItem(member_el.getPointee()->get_type()) });
		childAdded(*this, *new_group);
	}
}


//////////////////////////////////////////////////////////////////////////
// ComponentItem
//////////////////////////////////////////////////////////////////////////

ComponentItem::ComponentItem(nap::Component& comp) : ObjectItem(&comp, false)
{ }


nap::Component& ComponentItem::getComponent()
{
	return *rtti_cast<nap::Component>(mObject);
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
	auto ownerItem = qobject_cast<EntityInstanceItem*>(parentItem());
	if (ownerItem)
	{
		std::vector<std::string> namePath = {mObject->mID};

		auto parent = ownerItem;
		while (parent != nullptr && !qobject_cast<RootEntityItem*>(parent))
		{
			namePath.insert(namePath.begin(), parent->instanceName().toStdString());
			parent = qobject_cast<EntityInstanceItem*>(parent->parentItem());
		}

		std::string path = "./" + nap::utility::joinString(namePath, "/");

		return PropertyPath(absolutePath(), *AppContext::get().getDocument());
	}
	return PropertyPath(absolutePath(), *AppContext::get().getDocument());
}


const std::string EntityInstanceItem::unambiguousName() const
{
	if (auto parentObjectItem = qobject_cast<ObjectItem*>(parentItem()))
		return ObjectItem::unambiguousName() + ":" + std::to_string(parentObjectItem->nameIndex(*this));
	return ObjectItem::unambiguousName();
}


nap::Entity& napkin::EntityInstanceItem::entity() const
{
	return *rtti_cast<nap::Entity>(mObject);
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


SceneItem* RootEntityItem::sceneItem()
{
	return qobject_cast<SceneItem*>(parentItem());
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
	return *rtti_cast<nap::Component>(mObject);
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
