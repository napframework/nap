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

	// Get and cast to RTTI Item
	auto l_item = dynamic_cast<RTTIItem*>(resource_model->itemFromIndex(left));
	auto r_item = dynamic_cast<RTTIItem*>(resource_model->itemFromIndex(right));

	// Bail if we're not an rtti item
	if (l_item == nullptr || r_item == nullptr)
		return false;

	// Don't sort regular resource groups
	if (qobject_cast<EntityResourcesItem*>(l_item) != nullptr &&
		qobject_cast<RootResourcesItem*>(r_item) != nullptr)
		return false;

	// Check if item is an entity
	auto le_item = qobject_cast<EntityItem*>(l_item);
	auto re_item = qobject_cast<EntityItem*>(r_item);

	// Check if item is a component
	auto lc_item = qobject_cast<ComponentItem*>(l_item);
	auto rc_item = qobject_cast<ComponentItem*>(r_item);

	// left is entity, right is component
	if (le_item != nullptr && rc_item != nullptr)
		return true;

	// right is component, left is entity
	if (lc_item != nullptr && re_item != nullptr)
		return false;

	// Don't sort items of which parent is an entity
	if (qobject_cast<EntityItem*>(l_item->parentItem()) != nullptr &&
		qobject_cast<EntityItem*>(r_item->parentItem()) != nullptr)
		return false;

	// Prioritize groups over other items
	GroupItem* lg_item = qobject_cast<GroupItem*>(l_item);
	GroupItem* rg_item = qobject_cast<GroupItem*>(r_item);

	// Left is group, right is not
	if (lg_item != nullptr && rg_item == nullptr)
		return true;

	// Right is group, left is not
	if (rg_item != nullptr && lg_item == nullptr)
		return false;

	// Otherwise sort default
    return l_item->text() < r_item->text();
}


napkin::ResourceModel::ResourceModel()
{
	setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_TYPE});
	appendRow(&mObjectsItem);
	appendRow(&mEntitiesItem);
	setItemPrototype(new RTTIItem());
}


napkin::ResourcePanel::ResourcePanel()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.getTreeView().setColumnWidth(0, 300);
	mTreeView.enableSorting(&ResourceSorter);

	connect(mTreeView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, &ResourcePanel::onSelectionChanged);
	mTreeView.setMenuHook(std::bind(&ResourcePanel::menuHook, this, std::placeholders::_1));

	connect(&AppContext::get(), &AppContext::documentOpened, this, &ResourcePanel::onFileOpened);
	connect(&AppContext::get(), &AppContext::documentClosing, this, &ResourcePanel::onFileClosing);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &ResourcePanel::onNewFile);

	const auto& resources_item = mModel.getRootResourcesItem();
	connect(&resources_item, &RootResourcesItem::childAddedToGroup, this, &ResourcePanel::onChildAddedToGroup);

	const auto& entities_item = mModel.getEntityResourcesItem();
	connect(&entities_item, &EntityResourcesItem::childAddedToEntity, this, &ResourcePanel::onChildAddedToEntity);
}


void napkin::ResourceModel::populate()
{
	auto doc = AppContext::get().getDocument();
	assert(doc != nullptr);
	auto root_objects = topLevelObjects();
	mObjectsItem.populate(root_objects);
	mEntitiesItem.populate(root_objects);
}


void napkin::ResourceModel::clear()
{
	mEntitiesItem.clear();
	mObjectsItem.clear();
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
	return parent_item != nullptr ? &parent_item->getGroup() : nullptr;
}


void napkin::ResourcePanel::menuHook(QMenu& menu)
{
	// Get selection
	auto selected_item = qitem_cast<RTTIItem*>(mTreeView.getSelectedItem());
	if (selected_item == nullptr)
		return;

	// Cast to rtti item
	if (qobject_cast<EntityItem*>(selected_item) != nullptr)
	{
		// If it's a child of another entity
		auto entity_item = static_cast<EntityItem*>(selected_item);
		if (entity_item->isPointer())
		{
			auto parent_item = qobject_cast<EntityItem*>(entity_item->parentItem());
			if (parent_item)
			{
				menu.addAction(new RemovePathAction(entity_item->propertyPath()));
			}
		}
		// Otherwise it's the main resource
		else
		{
			menu.addAction(new AddChildEntityAction(entity_item->getEntity()));
			menu.addAction(new AddComponentAction(entity_item->getEntity()));
			menu.addAction(new DeleteObjectAction(entity_item->getObject()));
		}
	}
	// Component
	else if (qobject_cast<ComponentItem*>(selected_item) != nullptr)
	{
		auto component_item = static_cast<ComponentItem*>(selected_item);
		menu.addAction(new DeleteObjectAction(component_item->getObject()));
	}
	// Group
	else if (qobject_cast<GroupItem*>(selected_item) != nullptr)
	{
		// Create and add new resource
		GroupItem* group_item = static_cast<GroupItem*>(selected_item);
		menu.addAction(new AddNewResourceToGroupAction(group_item->getGroup()));

		// Add existing resource
		menu.addAction(new AddExistingResourceToGroupAction(group_item->getGroup()));

		// If the item is parented under a group, offer the option to remove it
		auto* item_group = getItemGroup(*group_item);
		if (item_group != nullptr)
			menu.addAction(new RemoveGroupFromGroupAction(*item_group, group_item->getGroup()));

		// Create and add new sub group
		menu.addAction(new AddChildGroupAction(group_item->getGroup()));

		// Add action to move group to another group
		menu.addAction(new MoveGroupAction(group_item->getGroup(), item_group));

		// Delete group action
		menu.addAction(new DeleteGroupAction(group_item->getGroup()));
	}
	// General Object
	else if (qobject_cast<ObjectItem*>(selected_item) != nullptr)
	{
		// Get resource
		auto object_item = static_cast<ObjectItem*>(selected_item);

		// If the item is parented under a group, offer the option to remove it
		auto* item_group = getItemGroup(*object_item);
		if (item_group != nullptr)
			menu.addAction(new RemoveResourceFromGroupAction(*item_group, object_item->getObject()));

		// Move resource to another group
		menu.addAction(new MoveResourceToGroupAction(object_item->getObject(), item_group));

		// Delete resource action
		menu.addAction(new DeleteObjectAction(object_item->getObject()));

		// If the item is a shader, allow if to be loaded (compiled)
		// TODO: Create for more generic insertion method for object specific actions
		if (object_item->getObject().get_type().is_derived_from(RTTI_OF(nap::BaseShader)))
		{
			menu.addAction(new LoadShaderAction(static_cast<nap::BaseShader&>(object_item->getObject())));
		}
	}
	// Top Resource
	else if (qobject_cast<RootResourcesItem*>(selected_item) != nullptr)
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

void napkin::ResourcePanel::clear()
{
	mTreeView.getTreeView().selectionModel()->clear();
	mModel.clear();
}


void napkin::ResourcePanel::populate()
{
	mModel.populate();
	mTreeView.expand(mModel.getRootResourcesItem());
	mTreeView.expand(mModel.getEntityResourcesItem());
	emitSelectionChanged();
	mTreeView.getTreeView().sortByColumn(0, Qt::SortOrder::AscendingOrder);
}


void ResourcePanel::selectObjects(const QList<nap::rtti::Object*>& obj)
{
	if (obj.size() > 0)
		mTreeView.select(findItemInModel<napkin::ObjectItem>(mModel, *obj[0]), true);
}


void napkin::ResourcePanel::onChildAddedToGroup(GroupItem& group, ObjectItem& item)
{
	mTreeView.select(&item, true);
}


void napkin::ResourcePanel::onChildAddedToEntity(EntityItem& entity, ObjectItem& item)
{
	mTreeView.select(&item, true);
}


void ResourcePanel::emitSelectionChanged()
{
	// Grab selected nap objects
	QList<PropertyPath> selectedPaths;
	for (auto m : mTreeView.getSelectedItems())
	{
		auto item = qitem_cast<ObjectItem*>(m);
		if (item != nullptr)
			selectedPaths << item->propertyPath();
	}
	selectionChanged(selectedPaths);
}

