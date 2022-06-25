/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "resourcepanel.h"
#include "naputils.h"

#include <commands.h>
#include <napqt/filterpopup.h>
#include <napqt/qtutils.h>
#include <napkin-resources.h>

using namespace napkin;

static bool ResourceSorter(const QModelIndex& left, const QModelIndex& right, QAbstractItemModel* model)
{
	// Get model
	assert(qobject_cast<ResourceModel*>(model) != nullptr);
	ResourceModel* resource_model = static_cast<ResourceModel*>(model);

	// Get item
	auto l_item = resource_model->itemFromIndex(left);
	auto r_item = resource_model->itemFromIndex(right);
	assert(l_item != nullptr && r_item != nullptr);

	// Cast to RTTI Item
	auto lr_item = qitem_cast(l_item);
	auto rr_item = qitem_cast(r_item);

	// Don't sort regular resource groups
	if (qobject_cast<EntityResourcesItem*>(lr_item) != nullptr &&
		qobject_cast<RegularResourcesItem*>(rr_item) != nullptr)
		return false;

	// Don't sort items of which parent is an entity (components)
	if (qobject_cast<EntityItem*>(lr_item) != nullptr &&
		qobject_cast<EntityItem*>(rr_item) != nullptr)
		return false;

	// Prioritize groups over other items
	GroupItem* lg_item = qobject_cast<GroupItem*>(lr_item);
	GroupItem* rg_item = qobject_cast<GroupItem*>(rr_item);

	// Left is group, right is not
	if (lg_item != nullptr && rg_item == nullptr)
		return true;

	// Right is group, left is not
	if (rg_item != nullptr && lg_item == nullptr)
		return false;

	// Otherwise sort default
	return model->data(left, Qt::ItemDataRole::DisplayRole) < model->data(right, Qt::ItemDataRole::DisplayRole);
}


napkin::ResourceModel::ResourceModel()
{
	setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_TYPE});
	appendRow(&mObjectsItem);
	appendRow(&mEntitiesItem);
}

// TODO: rename
bool shouldObjectBeVisible(const nap::rtti::Object& obj)
{
	auto doc = AppContext::get().getDocument();

	// Exclude components
	if (obj.get_type().is_derived_from<nap::Component>())
		return false;

	// Exclude embedded objects
	if (doc->isPointedToByEmbeddedPointer(obj))
		return false;

	// Non-root entities are culled
	if (obj.get_type().is_derived_from<nap::Entity>())
		if (doc->getParent(*rtti_cast<const nap::Entity>(&obj)))
			return false;

	return true;
}


void napkin::ResourceModel::populate()
{
	auto doc = AppContext::get().getDocument();
	assert(doc != nullptr);
	for (nap::rtti::Object* ob : topLevelObjects(doc->getObjectPointers()))
		addObjectItem(*ob);
}


void napkin::ResourceModel::clear()
{
	mEntitiesItem.removeRows(0, mEntitiesItem.rowCount());
	mObjectsItem.removeRows(0, mObjectsItem.rowCount());
}


ObjectItem* ResourceModel::addObjectItem(nap::rtti::Object& ob)
{
	auto typeItem = new RTTITypeItem(ob.get_type());

	// If it's an entity, add it as such
	if (ob.get_type().is_derived_from<nap::Entity>())
	{
		auto entityItem = new EntityItem(static_cast<nap::Entity&>(ob));
		mEntitiesItem.appendRow({entityItem, typeItem});
		return entityItem;
	}

	// If it's a group, add it as such
	if (ob.get_type().is_derived_from<nap::IGroup>())
	{
		auto group_item = new GroupItem(static_cast<nap::IGroup&>(ob));
		connect(group_item, &GroupItem::childAdded, this, &ResourceModel::childAddedToGroup);
		mObjectsItem.appendRow({ group_item, typeItem });
		return group_item;
	}

	// Hide items for specific types
	if (!shouldObjectBeVisible(ob))
		return nullptr;

	// ... now the rest in Objects...
	auto item = new ObjectItem(&ob, false);
	mObjectsItem.appendRow({item, typeItem});
	return item;
}


void ResourceModel::removeObjectItem(const nap::rtti::Object& object)
{
	auto item = findItemInModel<napkin::ObjectItem>(*this, object);
	if (item == nullptr)
		return;

	removeRow(item->row(), static_cast<QStandardItem*>(item)->parent()->index());
}


napkin::ResourcePanel::ResourcePanel()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.getTreeView().setColumnWidth(0, 300);
	mTreeView.enableSorting(&ResourceSorter);

	connect(&AppContext::get(), &AppContext::documentOpened, this, &ResourcePanel::onFileOpened);
	connect(&AppContext::get(), &AppContext::documentClosing, this, &ResourcePanel::onFileClosing);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &ResourcePanel::onNewFile);
	connect(mTreeView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, &ResourcePanel::onSelectionChanged);

	mTreeView.setMenuHook(std::bind(&ResourcePanel::menuHook, this, std::placeholders::_1));

	// connect(&AppContext::get(), &AppContext::dataChanged, this, &ResourcePanel::refresh);
	connect(&AppContext::get(), &AppContext::entityAdded, this, &ResourcePanel::onEntityAdded);
	connect(&AppContext::get(), &AppContext::componentAdded, this, &ResourcePanel::onComponentAdded);
	connect(&AppContext::get(), &AppContext::objectAdded, this, &ResourcePanel::onObjectAdded);
	connect(&AppContext::get(), &AppContext::objectRemoved, this, &ResourcePanel::onObjectRemoved);
	connect(&AppContext::get(), &AppContext::objectReparented, this, &ResourcePanel::onObjectReparented);
	connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &ResourcePanel::onPropertyValueChanged);

	connect(&mModel, &ResourceModel::childAddedToGroup, this, &ResourcePanel::onChildAddedToGroup);
}


/**
 * @return items group, nullptr if there is no parent or the parent isn't a group
 */
static nap::IGroup* getItemGroup(const ObjectItem& item)
{
	// Check if the parent is a group
	GroupItem* parent_item = item.parentItem() != nullptr ?
		qobject_cast<GroupItem*>(item.parentItem()) :
		nullptr;

	// Return group from parent, nullptr otherwise
	return parent_item != nullptr ? parent_item->getGroup() : nullptr;
}


void napkin::ResourcePanel::menuHook(QMenu& menu)
{
	// Get selection
	auto selected_item = qitem_cast(mTreeView.getSelectedItem());
	if (selected_item == nullptr)
		return;

	// Cast to rtti item
	if (qobject_cast<EntityItem*>(selected_item) != nullptr)
	{
		auto entity_item = static_cast<EntityItem*>(selected_item);
		menu.addAction(new AddChildEntityAction(*entity_item->getEntity()));
		menu.addAction(new AddComponentAction(*entity_item->getEntity()));

		if (entity_item->isPointer())
		{
			auto parent_item = qobject_cast<EntityItem*>(entity_item->parentItem());
			if (parent_item)
				menu.addAction(new RemovePathAction(entity_item->propertyPath()));
		}
		menu.addAction(new DeleteObjectAction(*entity_item->getObject()));
	}
	// Component
	else if (qobject_cast<ComponentItem*>(selected_item) != nullptr)
	{
		auto component_item = static_cast<ComponentItem*>(selected_item);
		menu.addAction(new DeleteObjectAction(*component_item->getObject()));
	}
	// Group
	else if (qobject_cast<GroupItem*>(selected_item) != nullptr)
	{
		// Create and add new resource
		GroupItem* group_item = static_cast<GroupItem*>(selected_item);
		menu.addAction(new AddNewResourceToGroupAction(*group_item->getGroup()));

		// Add existing resource
		menu.addAction(new AddExistingResourceToGroupAction(*group_item->getGroup()));

		// If the item is parented under a group, offer the option to remove it
		auto* item_group = getItemGroup(*group_item);
		if (item_group != nullptr)
			menu.addAction(new RemoveGroupFromGroupAction(*item_group, *group_item->getGroup()));

		// Create and add new sub group
		menu.addAction(new AddChildGroupAction(*group_item->getGroup()));

		// Add action to move group to another group
		menu.addAction(new MoveGroupAction(*group_item->getGroup(), item_group));

		// Delete group action
		menu.addAction(new DeleteGroupAction(*group_item->getGroup()));
	}
	// General Object
	else if (qobject_cast<ObjectItem*>(selected_item) != nullptr)
	{
		// Get resource
		auto object_item = static_cast<ObjectItem*>(selected_item);

		// If the item is parented under a group, offer the option to remove it
		auto* item_group = getItemGroup(*object_item);
		if (item_group != nullptr)
			menu.addAction(new RemoveResourceFromGroupAction(*item_group, *object_item->getObject()));

		// Move resource to another group
		menu.addAction(new MoveResourceToGroupAction(*object_item->getObject(), item_group));

		// Delete resource action
		menu.addAction(new DeleteObjectAction(*object_item->getObject()));
	}
	// Top Resource
	else if (qobject_cast<RegularResourcesItem*>(selected_item) != nullptr)
	{
		// Add Resource selection
		menu.addAction(new CreateResourceAction());

		// Add groups
		menu.addAction(new CreateGroupAction());
	}
	// Top Entity
	else if (qobject_cast<EntityResourcesItem*>(selected_item) != nullptr)
	{
		menu.addAction(new CreateEntityAction());
	}
}


void napkin::ResourcePanel::onNewFile()
{
	populate();
}


void napkin::ResourcePanel::onFileOpened(const QString& filename)
{
	populate();
}


void napkin::ResourcePanel::onFileClosing(const QString& filename)
{
	clear();
}


void napkin::ResourcePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	emitSelectionChanged();
}


void napkin::ResourcePanel::refresh()
{
	clear();
	populate();
}


void napkin::ResourcePanel::clear()
{
	mTreeView.getTreeView().selectionModel()->clear();
	mModel.clear();
}


void napkin::ResourcePanel::populate()
{
	mModel.populate();
	mTreeView.getTreeView().expandAll();
	emitSelectionChanged();
	mTreeView.getTreeView().sortByColumn(0, Qt::SortOrder::AscendingOrder);
}


void napkin::ResourcePanel::onEntityAdded(nap::Entity* entity, nap::Entity* parent)
{
	// TODO: Don't refresh the whole mModel
	refresh();
	mTreeView.selectAndReveal(findItemInModel<napkin::ObjectItem>(mModel, *entity));
}

void napkin::ResourcePanel::onComponentAdded(nap::Component* comp, nap::Entity* owner)
{
	// TODO: Don't refresh the whole mModel
	refresh();
	mTreeView.selectAndReveal(findItemInModel<ObjectItem>(mModel, *comp));
}

void napkin::ResourcePanel::onObjectAdded(nap::rtti::Object* obj, nap::rtti::Object* parent, bool selectNewObject)
{
	// We're only interested in root objects
	if (parent != nullptr)
		return;

	// Don't handle instance properties
	if (obj->get_type().is_derived_from(RTTI_OF(nap::InstancePropertyValue)))
		return;

	// Add item
	auto* item = mModel.addObjectItem(*obj);
	if (selectNewObject)
	{
		mTreeView.selectAndReveal(item);
	}
}


void ResourcePanel::selectObjects(const QList<nap::rtti::Object*>& obj)
{
	if (obj.size() > 0)
		mTreeView.selectAndReveal(findItemInModel<napkin::ObjectItem>(mModel, *obj[0]));
}


void napkin::ResourcePanel::onObjectRemoved(const nap::rtti::Object* object)
{
	mModel.removeObjectItem(*object);
}


void napkin::ResourcePanel::onObjectReparented(nap::rtti::Object& object, nap::IGroup* oldParent, nap::IGroup* newParent)
{
	// The item was in the root, attempt to remove it
	if (oldParent == nullptr)
	{
		mModel.removeObjectItem(object);
	}

	// The item has no new owner, add it
	if (newParent == nullptr)
	{
		auto new_item = mModel.addObjectItem(object);
	}
}


void napkin::ResourcePanel::onPropertyValueChanged(const PropertyPath& path)
{
	// Update object name?
	if (path.getProperty().get_name() == nap::rtti::sIDPropertyName)
	{
		auto obj = path.getObject();
		auto objectItem = findItemInModel<napkin::ObjectItem>(mModel, *obj);
		if (objectItem != nullptr)
		{
			objectItem->setText(QString::fromStdString(obj->mID));
		}
	}
}


void napkin::ResourcePanel::onChildAddedToGroup(GroupItem& group, ObjectItem& item)
{
	mTreeView.selectAndReveal(&item);
}


void ResourcePanel::emitSelectionChanged()
{
	// Grab selected nap objects
	QList<PropertyPath> selectedPaths;
	for (auto m : mTreeView.getSelectedItems())
	{
		auto item = qobject_cast<ObjectItem*>(qitem_cast(m));
		if (item != nullptr)
			selectedPaths << item->propertyPath();
	}
	selectionChanged(selectedPaths);
}

