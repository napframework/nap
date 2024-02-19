	/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "standarditemsobject.h"
#include "commands.h"
#include "sceneservice.h"
#include "naputils.h"
#include "napkin-resources.h"
#include "napkinutils.h"

// External Includes
#include <nap/assert.h>

RTTI_DEFINE_BASE(napkin::RootResourcesItem)
RTTI_DEFINE_BASE(napkin::EntityResourcesItem)
RTTI_DEFINE_BASE(napkin::EntityItem)
RTTI_DEFINE_BASE(napkin::GroupItem)
RTTI_DEFINE_BASE(napkin::SceneItem)
RTTI_DEFINE_BASE(napkin::ComponentItem)
RTTI_DEFINE_BASE(napkin::EntityInstanceItem)
RTTI_DEFINE_BASE(napkin::RootEntityItem)
RTTI_DEFINE_BASE(napkin::ComponentInstanceItem)
RTTI_DEFINE_BASE(napkin::BaseShaderItem)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::ShaderItem)
	RTTI_CONSTRUCTOR(nap::rtti::Object&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::ShaderFromFileItem)
	RTTI_CONSTRUCTOR(nap::rtti::Object&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(napkin::ComputeShaderFromFileItem)
	RTTI_CONSTRUCTOR(nap::rtti::Object&)
RTTI_END_CLASS

using namespace napkin;

//////////////////////////////////////////////////////////////////////////
// Static functions
//////////////////////////////////////////////////////////////////////////

/**
 * Helper function to create an item for the given object.
 * @param object to create object item for
 */
static ObjectItem* createObjectItem(nap::rtti::Object& object)
{
	struct Binding
	{
		nap::rtti::TypeInfo mSource = nap::rtti::TypeInfo::empty();
		nap::rtti::TypeInfo mTarget = nap::rtti::TypeInfo::empty();
	};

	// Object to item map -> order sensitive!
	const static std::vector<Binding> mMap =
	{
		{ RTTI_OF(nap::ShaderFromFile),			RTTI_OF(napkin::ShaderFromFileItem) },
		{ RTTI_OF(nap::ComputeShaderFromFile),	RTTI_OF(napkin::ComputeShaderFromFileItem) },
		{ RTTI_OF(nap::Shader),					RTTI_OF(napkin::ShaderItem)			}
	};

	// Check if there is an item overload for the given object
	for(const auto& item : mMap)
	{
		if (object.get_type().is_derived_from(item.mSource))
		{
			auto* new_obj = item.mTarget.create<napkin::ObjectItem>({ object });
			assert(new_obj != nullptr);
			return new_obj;
		}
	}

	// Default
	return new ObjectItem(object);
};


/**
 * Helper function that finds and swaps items based on the given property index change.
 * @param path the array property that holds the items that were swapped
 * @param oldIndex original index
 * @param newIndex the new index
 * @param parent parent item
 * @return swapped child indices (previous, new)
 */
static std::array<size_t, 2> swapItems(const PropertyPath& path, size_t oldIndex, size_t newIndex, ObjectItem& parent)
{
	// Get modified objects
	auto old_val = path.getArrayElement(oldIndex).getValue(); assert(old_val.is_valid());
	auto old_ptr = old_val.extract_wrapped_value().get_value<nap::rtti::Object*>();

	auto new_val = path.getArrayElement(newIndex).getValue(); assert(new_val.is_valid());
	auto new_ptr = new_val.extract_wrapped_value().get_value<nap::rtti::Object*>();

	// Map component indices to child items
	int a_idx = -1; int b_idx = -1;
	for (auto row = 0; row < parent.rowCount(); row++)
	{
		auto* it = qitem_cast<ObjectItem*>(parent.child(row));
		if (it != nullptr)
		{
			a_idx = &(it->getObject()) == new_ptr ? row : a_idx;
			b_idx = &(it->getObject()) == old_ptr ? row : b_idx;
		}
	} assert(a_idx >= 0 && b_idx >= 0);

	auto child_a = parent.takeChild(a_idx);
	auto child_b = parent.takeChild(b_idx);
	parent.setChild(a_idx, child_b);
	parent.setChild(b_idx, child_a);
	return { static_cast<size_t>(a_idx), static_cast<size_t>(b_idx) };
}


//////////////////////////////////////////////////////////////////////////
// RegularResourcesItem 
//////////////////////////////////////////////////////////////////////////

napkin::RootResourcesItem::RootResourcesItem()
{
	setEditable(false);
	setText("Resources");

	connect(&AppContext::get(), &AppContext::objectAdded, this, &RootResourcesItem::onObjectAdded);
	connect(&AppContext::get(), &AppContext::objectReparented, this, &RootResourcesItem::onObjectReparented);
}


QVariant napkin::RootResourcesItem::data(int role) const
{
	switch (role)
	{
	case Qt::DecorationRole:
		return AppContext::get().getResourceFactory().getIcon(QRC_ICONS_RTTIOBJECT);
	default:
		return QStandardItem::data(role);
	}
}


void napkin::RootResourcesItem::clear()
{
	this->removeRows(0, rowCount());
}


void napkin::RootResourcesItem::populate(nap::rtti::ObjectSet& objects)
{
	clear();
	for (auto& obj : objects)
	{
		onObjectAdded(obj, nullptr);
	}
}


void napkin::RootResourcesItem::onObjectAdded(nap::rtti::Object* obj, nap::rtti::Object* parent)
{
	// only add objects that have no parent
	if (parent != nullptr)
		return;

	// Skip items associated with entity resources
	if (obj->get_type().is_derived_from(RTTI_OF(nap::Entity)) ||
		obj->get_type().is_derived_from(RTTI_OF(nap::Component)) ||
		obj->get_type().is_derived_from(RTTI_OF(nap::InstancePropertyValue)))
		return;

	// If it's a group, add it as such
	if (obj->get_type().is_derived_from<nap::IGroup>())
	{
		auto group_item = new GroupItem(static_cast<nap::IGroup&>(*obj));
		this->appendRow({ group_item, new RTTITypeItem(obj->get_type()) });
		connect(group_item, &GroupItem::childAdded, this, &RootResourcesItem::childAddedToGroup);
		connect(group_item, &GroupItem::indexChanged, this, &RootResourcesItem::indexChanged);
	}
	else
	{
		// Add as regular item
		this->appendRow({ createObjectItem(*obj), new RTTITypeItem(obj->get_type()) });
	}
}


void napkin::RootResourcesItem::onObjectReparented(nap::rtti::Object& object, PropertyPath oldParent, PropertyPath newParent)
{
	// Only add if new parent is invalid (ie: part of root)
	if (!newParent.isValid())
	{
		onObjectAdded(&object, nullptr);
	}
}


//////////////////////////////////////////////////////////////////////////
// EntityResourcesItem 
//////////////////////////////////////////////////////////////////////////

napkin::EntityResourcesItem::EntityResourcesItem()
{
	setEditable(false);
	setText("Entities");
	connect(&AppContext::get(), &AppContext::objectAdded, this, &EntityResourcesItem::onObjectAdded);
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


void napkin::EntityResourcesItem::clear()
{
	this->removeRows(0, rowCount());
}


void napkin::EntityResourcesItem::populate(nap::rtti::ObjectSet& objects)
{
	clear();
	for (auto& obj : objects)
	{
		onObjectAdded(obj, nullptr);
	}
}


void napkin::EntityResourcesItem::onObjectAdded(nap::rtti::Object* obj, nap::rtti::Object* parent)
{
	// Only consider entities
	if (!obj->get_type().is_derived_from(RTTI_OF(nap::Entity)))
		return;

	// Only consider root objects
	if (parent != nullptr)
		return;

	// Add as regular item
	auto entity_item = new EntityItem(*static_cast<nap::Entity*>(obj), false);
	this->appendRow(
		{
			entity_item,
			new RTTITypeItem(obj->get_type())
		});

	// Listen to changes
	entity_item->connect(entity_item, &EntityItem::childAdded, this, &EntityResourcesItem::childAddedToEntity);
	entity_item->connect(entity_item, &EntityItem::indexChanged, this, &EntityResourcesItem::indexChanged);
}


const napkin::EntityItem* napkin::EntityResourcesItem::findEntityItem(const nap::Entity& entity) const
{
	for (int i = 0; i < rowCount(); i++)
	{
		auto entity_item = qitem_cast<EntityItem*>(this->child(i));
		if (entity_item != nullptr && &entity_item->getObject() == &entity)
		{
			return entity_item;
		}
	}
	return nullptr;
}


//////////////////////////////////////////////////////////////////////////
// ObjectItem 
//////////////////////////////////////////////////////////////////////////

ObjectItem::ObjectItem(nap::rtti::Object& o) : ObjectItem(o, false)
{ }


ObjectItem::ObjectItem(nap::rtti::Object& o, bool isPointer) :
	mObject(&o), mIsPointer(isPointer)
{
	auto& ctx = AppContext::get();
	setText(QString::fromStdString(o.mID));
	connect(&ctx, &AppContext::propertyValueChanged, this, &ObjectItem::onPropertyValueChanged);
	connect(&ctx, &AppContext::removingObject, this, &ObjectItem::onObjectRemoved);
	connect(&ctx, &AppContext::objectReparenting, this, &ObjectItem::onObjectReparenting);
	refresh();
}


const PropertyPath ObjectItem::propertyPath() const
{
	return PropertyPath(absolutePath(), *(AppContext::get().getDocument()));
}


std::string ObjectItem::absolutePath() const
{
	std::vector<std::string> path = { unambiguousName() };
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

nap::rtti::Object& ObjectItem::getObject() const
{
	return *mObject;
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
		return AppContext::get().getResourceFactory().getIcon(getObject());
	case Qt::ForegroundRole:
	{
		return isPointer() ?
			AppContext::get().getThemeManager().getColor(theme::color::dimmedItem) :
			QStandardItem::data(role);
	}
	case Qt::ToolTipRole:
	{
		const char* obj_desc = nap::rtti::getDescription(getObject().get_type());
		return obj_desc != nullptr ? QString(obj_desc) : QStandardItem::data(role);
	}
	default:
		return QStandardItem::data(role);
	}
}

void ObjectItem::removeChildren()
{
	removeRows(0, rowCount());
}


QString ObjectItem::instanceName() const
{
	auto parent = parentItem();
	if (parent)
	{
		int i = 0;
		for (int row = 0, len = parent->rowCount(); row < len; row++)
		{
			auto sibling = qitem_cast<ObjectItem*>(parent->child(row));
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
	return "";
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
	auto childName = childItem.getObject().mID;
	int index = 0;
	for (int row = 0; row < rowCount(); row++)
	{
		auto otherChildItem = qitem_cast<ObjectItem*>(child(row, 0));
		if (!otherChildItem)
			continue;

		if (otherChildItem == &childItem)
			return index;

		auto id = otherChildItem->getObject().mID;
		if (otherChildItem->getObject().mID == childName)
			++index;
	}
	return -1;
}


void ObjectItem::onPropertyValueChanged(PropertyPath path)
{
	if (path.getObject() == mObject)
		refresh();
}


void ObjectItem::onObjectRemoved(nap::rtti::Object* object)
{
	// Remove item if object that is deleted is referenced by this item
	// Note that if the item has no parent it won't be able to delete itself
	// In that case the model that manages the items must delete it.
	if (object == mObject && parentItem() != nullptr)
		parentItem()->removeRow(this->row());
}


const std::string ObjectItem::unambiguousName() const
{
	return getObject().mID;
}


void napkin::ObjectItem::onObjectReparenting(nap::rtti::Object& object, PropertyPath oldParent, PropertyPath newParent)
{
	onObjectRemoved(&object);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EntityItem::EntityItem(nap::Entity& entity, bool isPointer) : ObjectItem(entity, isPointer)
{
	// Populate item
	populate();

	// Handle data model changes
	auto& ctx = AppContext::get();
	connect(&ctx, &AppContext::componentAdded, this, &EntityItem::onComponentAdded);
	connect(&ctx, &AppContext::childEntityAdded, this, &EntityItem::onEntityAdded);
	connect(&ctx, &AppContext::propertyValueChanged, this, &EntityItem::onPropertyValueChanged);
	connect(&ctx, &AppContext::arrayIndexSwapped, this, &EntityItem::onIndexSwapped);
}


nap::Entity& EntityItem::getEntity()
{
	return static_cast<nap::Entity&>(getObject());
}


void EntityItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	// Bail if we're not handling this object
	if (parent != mObject)
		return;

	// Create and add new item
	auto* new_item = new EntityItem(*e, true);
	new_item->connect(new_item, &EntityItem::childAdded, this, &EntityItem::childAdded);
	insertRow(parent->mChildren.size()-1, {new_item, new RTTITypeItem(e->get_type())});

	// Notify listeners
	childAdded(*this, *new_item);
}


void EntityItem::onComponentAdded(nap::Component* comp, nap::Entity* owner)
{
	// Bail if we're not handling this object
	if (owner != mObject)
		return;

	// Create and add new component
	auto comp_item = new ComponentItem(*comp);
	appendRow({ comp_item, new RTTITypeItem(comp->get_type()) });

	// Notify listeners
	childAdded(*this, *comp_item);
}


void napkin::EntityItem::onIndexSwapped(const PropertyPath& path, size_t oldIndex, size_t newIndex)
{
	// Bail if we're not handling this object
	if (path.getObject() != mObject)
		return;

	// Only handle component index changes because sub-tree is rebuild when entity order changes
	// TODO: properly handle child entity changes -> implementation below is optimized for it
	auto comp_path = PropertyPath(mObject->mID, nap::Entity::componentsPropertyName(), *AppContext::get().getDocument());
	if (path != comp_path)
		return;

	// Swap child items and notify
	auto idx = swapItems(path, oldIndex, newIndex, *this);
	indexChanged(*this, *qitem_cast<ObjectItem*>(child(idx[1])));
}


void EntityItem::onPropertyValueChanged(const PropertyPath& path)
{
	// Bail if we're not handling this object
	if (mObject != path.getObject())
		return;

	// Check if the children property was edited
	auto child_path = PropertyPath(mObject->mID, nap::Entity::childrenPropertyName(), *AppContext::get().getDocument());
	if (path == child_path)
	{
		populate();
	}
}


const std::string EntityItem::unambiguousName() const
{
	if (auto parent_entity = qobject_cast<EntityItem*>(parentItem()))
	{
		return ObjectItem::unambiguousName() + ":" + std::to_string(parent_entity->nameIndex(*this));
	}
	return ObjectItem::unambiguousName();
}


void napkin::EntityItem::populate()
{
	removeChildren();

	// Create and add entities
	auto& entity = getEntity();
	for (auto& child : entity.mChildren)
	{
		auto* child_entity = new EntityItem(*child, true);
		child_entity->connect(child_entity, &EntityItem::childAdded, this, &EntityItem::childAdded);
		child_entity->connect(child_entity, &EntityItem::indexChanged, this, &EntityItem::indexChanged);
		appendRow({ child_entity, new RTTITypeItem(child->get_type()) });
	}

	// Create and add component items
	for (auto& comp : entity.mComponents)
	{
		appendRow({ new ComponentItem(*comp), new RTTITypeItem(comp->get_type()) });
	}
}

//////////////////////////////////////////////////////////////////////////
// Group Item
//////////////////////////////////////////////////////////////////////////

napkin::GroupItem::GroupItem(nap::IGroup& group) : ObjectItem(group, false)
{
	// Get group members property
	PropertyPath members_path(getGroup(), getGroup().getMembersProperty(), *AppContext::get().getDocument());

	// Create child item for every member object
	members_path.iterateChildren([this](const PropertyPath& path)
		{
			// Append members as objects
			this->appendRow(
				{
					createObjectItem(*path.getPointee()),
					new RTTITypeItem(path.getPointee()->get_type())
				});
			return true;
		}, 0);

	// Get sub-group property
	PropertyPath children_path(getGroup(), getGroup().getChildrenProperty(), *AppContext::get().getDocument());

	// Create child group item for every sub-group
	children_path.iterateChildren([this](const PropertyPath& path)
		{
			assert(path.getPointee()->get_type().is_derived_from(RTTI_OF(nap::IGroup)));
			GroupItem* group_item = new GroupItem(*rtti_cast<nap::IGroup>(path.getPointee()));
			this->connect(group_item, &GroupItem::childAdded, this, &GroupItem::childAdded);
			this->connect(group_item, &GroupItem::indexChanged, this, &GroupItem::indexChanged);
			this->appendRow( { group_item, new RTTITypeItem(path.getPointee()->get_type()) });
			return true;
		}, 0);

	// Listen to data-model changes
	connect(&AppContext::get(), &AppContext::propertyChildInserted, this, &GroupItem::onPropertyChildInserted);
	connect(&AppContext::get(), &AppContext::arrayIndexSwapped, this, &GroupItem::onIndexSwapped);
}


QVariant napkin::GroupItem::data(int role) const
{
	switch (role)
	{
	case Qt::DecorationRole:
		return AppContext::get().getResourceFactory().getIcon(QRC_ICONS_GROUP);
	default:
		return ObjectItem::data(role);
	}
}


nap::IGroup& napkin::GroupItem::getGroup()
{
	return static_cast<nap::IGroup&>(getObject());
}


void napkin::GroupItem::onPropertyChildInserted(const PropertyPath& path, int index)
{
	// Check if this group has changed
	nap::IGroup& group = getGroup();
	if (!(path.getObject() == &group))
		return;

	// Check if an item has been added to the members property.
	// If so, figure out the correct index to insert the child.
	// The NAP group has 2 properties: members and children, but the group item
	// displays them as 1 long list. First the members, then the children.
	if (path.getProperty() == group.getMembersProperty())
	{
		PropertyPath array_path(group, group.getMembersProperty(), *AppContext::get().getDocument());
		auto member_el = path.getArrayElement(index);
		ObjectItem* new_item = new ObjectItem(*member_el.getPointee());
		this->insertRow(index, { new_item, new RTTITypeItem(member_el.getPointee()->get_type()) });
		childAdded(*this, *new_item);
	}
	// Otherwise, insert it at the end
	else
	{
		// Create item
		assert(path.getObject()->get_type().is_derived_from(RTTI_OF(nap::IGroup)));
		auto child_el = path.getArrayElement(index);
		GroupItem* new_group = new GroupItem(*rtti_cast<nap::IGroup>(child_el.getPointee()));
		this->connect(new_group, &GroupItem::childAdded, this, &GroupItem::childAdded);

		// Figure out where to insert
		PropertyPath members_path(group, group.getMembersProperty(), *AppContext::get().getDocument());
		int child_index = index + members_path.getArrayLength();

		// Insert
		this->insertRow(child_index, { new_group, new RTTITypeItem(child_el.getPointee()->get_type()) });
		childAdded(*this, *new_group);
	}
}


void napkin::GroupItem::onIndexSwapped(const PropertyPath& path, size_t oldIndex, size_t newIndex)
{
	// Check if this group has changed
	nap::IGroup& group = getGroup();
	if (!(path.getObject() == &group))
		return;

	// Swap child indices and notify
	auto idx = swapItems(path, oldIndex, newIndex, *this);
	indexChanged(*this, *qitem_cast<ObjectItem*>(child(idx[1])));
}


//////////////////////////////////////////////////////////////////////////
// Shader Item
//////////////////////////////////////////////////////////////////////////

napkin::BaseShaderItem::BaseShaderItem(nap::rtti::Object& object) : ObjectItem(object)
{
	mShader = rtti_cast<nap::BaseShader>(&object);
	assert(mShader != nullptr);
}


void napkin::BaseShaderItem::init()
{
	nap::utility::ErrorState error;
	if (!loadShader(getShader(), AppContext::get().getCore(), error))
	{
		nap::Logger::error("Failed to load '%s'", getShader().mID.c_str());
		nap::Logger::error(error.toString());
	}
}


//////////////////////////////////////////////////////////////////////////
// ShaderFromFile Item
//////////////////////////////////////////////////////////////////////////

napkin::ShaderFromFileItem::ShaderFromFileItem(nap::rtti::Object& object) : BaseShaderItem(object)
{
	auto* sff = rtti_cast<nap::ShaderFromFile>(&object);
	assert(sff != nullptr);
	if (!sff->mFragPath.empty() && !sff->mVertPath.empty())
	{
		init();
	}
}


//////////////////////////////////////////////////////////////////////////
// ComputeShaderFromFile Item
//////////////////////////////////////////////////////////////////////////

napkin::ComputeShaderFromFileItem::ComputeShaderFromFileItem(nap::rtti::Object& object) : BaseShaderItem(object)
{
	auto* csff = rtti_cast<nap::ComputeShaderFromFile>(&object);
	assert(csff != nullptr);
	if (!csff->mComputePath.empty())
	{
		init();
	}
}



//////////////////////////////////////////////////////////////////////////
// Shader Item
//////////////////////////////////////////////////////////////////////////

napkin::ShaderItem::ShaderItem(nap::rtti::Object& object) : BaseShaderItem(object)
{
	init();
}


//////////////////////////////////////////////////////////////////////////
// ComponentItem
//////////////////////////////////////////////////////////////////////////

ComponentItem::ComponentItem(nap::Component& comp) : ObjectItem(comp, false)
{ }


nap::Component& ComponentItem::getComponent()
{
	return *rtti_cast<nap::Component>(mObject);
}


//////////////////////////////////////////////////////////////////////////
// SceneItem
//////////////////////////////////////////////////////////////////////////

SceneItem::SceneItem(nap::Scene& scene) : ObjectItem(scene, false)
{
	for (auto& entity : scene.getEntityResourcesRef())
		appendRow(new RootEntityItem(entity));
}


nap::Scene& napkin::SceneItem::getScene()
{
	return static_cast<nap::Scene&>(getObject());
}

//////////////////////////////////////////////////////////////////////////
// EntityInstanceItem
//////////////////////////////////////////////////////////////////////////

EntityInstanceItem::EntityInstanceItem(nap::Entity& entity, nap::RootEntity& rootEntity)
		: mRootEntity(rootEntity), ObjectItem(entity, false)
{
	setEditable(false);
	for (auto& childEntity : entity.mChildren)
		appendRow(new EntityInstanceItem(*childEntity, mRootEntity));

	for (auto& comp : entity.mComponents)
		appendRow(new ComponentInstanceItem(*comp, mRootEntity));

	auto ctx = &AppContext::get();
	connect(ctx, &AppContext::componentAdded, this, &EntityInstanceItem::onComponentAdded);
	connect(ctx, &AppContext::childEntityAdded, this, &EntityInstanceItem::onEntityAdded);
}


nap::RootEntity& EntityInstanceItem::rootEntity() const
{
	return mRootEntity;
}


void EntityInstanceItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	if (parent != &entity())
		return;

	insertRow(nap::math::max<int>(parent->mChildren.size()-1, 0), new EntityInstanceItem(*e, mRootEntity));
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

RootEntityItem::RootEntityItem(nap::RootEntity& rootEntity) : EntityInstanceItem(*rootEntity.mEntity.get(), rootEntity)
{ }


SceneItem* RootEntityItem::sceneItem()
{
	return qobject_cast<SceneItem*>(parentItem());
}


//////////////////////////////////////////////////////////////////////////
// ComponentInstanceItem
//////////////////////////////////////////////////////////////////////////

ComponentInstanceItem::ComponentInstanceItem(nap::Component& comp, nap::RootEntity& rootEntity)
		: ObjectItem(comp, false), mRootEntity(rootEntity)
{
	setEditable(false);
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
		for (const auto& overrides : mRootEntity.mInstanceProperties)
		{
			// See if component matches
			if (overrides.mTargetComponent.get() != &component())
				continue;

			// Create relative component path for current instance.
			// We do this so we can compare if the override of this component targets the correct instance.
			// TODO: Remove from gui code and move to path utilities.
			auto* parent = qitem_cast<EntityInstanceItem*>(parentItem());
			std::vector<std::string> target_path = { mObject->mID };
			while (qitem_cast<RootEntityItem*>(parent) == nullptr)
			{
				target_path.insert(target_path.begin(), parent->instanceName().toStdString());
				parent = qitem_cast<EntityInstanceItem*>(parent->parentItem());
			} 

			// If instance path matches return override colour
			std::string instance_path = "./" + nap::utility::joinString(target_path, "/");
			if (overrides.mTargetComponent.toString() == instance_path)
			{
				return AppContext::get().getThemeManager().getColor(theme::color::instancePropertyOverride);
			}
		}
	}
	return ObjectItem::data(role);
}

