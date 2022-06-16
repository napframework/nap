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
	ResourceModel* resource_model = dynamic_cast<ResourceModel*>(model);
	assert(resource_model != nullptr);

	// Get item
	auto l_item = resource_model->itemFromIndex(left);
	auto r_item = resource_model->itemFromIndex(right);
	assert(l_item != nullptr && r_item != nullptr);

	// Don't sort regular resource groups
	auto* rg_item = dynamic_cast<EntityResourcesItem*>(l_item);
	auto* lg_item = dynamic_cast<RegularResourcesItem*>(r_item);
	if (rg_item != nullptr && lg_item != nullptr)
		return false;

	// Don't sort items of which parent is an entity (components)
	EntityItem* le_item = dynamic_cast<EntityItem*>(l_item->parent());
	EntityItem* re_item = dynamic_cast<EntityItem*>(r_item->parent());
	if (le_item != nullptr && re_item != nullptr)
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
	if (ob.get_type().is_derived_from<nap::Group>())
	{
		auto group_item = new GroupItem(static_cast<nap::Group&>(ob));
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

void ResourceModel::removeEmbeddedObjects()
{
	auto doc = AppContext::get().getDocument();

	// First, gather the objects that are pointed to by embedded pointers,
	// we are going to change the model layout
	QList<const nap::rtti::Object*> removeObjects;
	for (int row=0; row < mObjectsItem.rowCount(); row++)
	{
		auto item = dynamic_cast<ObjectItem*>(mObjectsItem.child(row, 0));
		assert(item != nullptr);
		auto obj = item->getObject();
		if (doc->isPointedToByEmbeddedPointer(*obj))
			removeObjects << obj;

	}

	// Now remove
	for (const auto obj : removeObjects)
		removeObjectItem(*obj);
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
	connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &ResourcePanel::onPropertyValueChanged);
	connect(&AppContext::get(), &AppContext::propertyChildInserted, this, &ResourcePanel::onPropertyChildInserted);
	connect(&AppContext::get(), &AppContext::propertyChildRemoved, this, &ResourcePanel::onPropertyChildRemoved);
}


void napkin::ResourcePanel::menuHook(QMenu& menu)
{
	auto selectedItem = mTreeView.getSelectedItem();
	if (selectedItem == nullptr)
		return;

	if (dynamic_cast<EntityItem*>(selectedItem) != nullptr)
	{
		auto entity_item = static_cast<EntityItem*>(selectedItem);
		menu.addAction(new AddChildEntityAction(*entity_item->getEntity()));
		menu.addAction(new AddComponentAction(*entity_item->getEntity()));

		if (entity_item->isPointer())
		{
			auto parentItem = dynamic_cast<EntityItem*>(entity_item->parentItem());
			if (parentItem)
			{
				menu.addAction(new RemovePathAction(entity_item->propertyPath()));
			}
		}
		menu.addAction(new DeleteObjectAction(*entity_item->getObject()));
	}
	else if (dynamic_cast<GroupItem*>(selectedItem) != nullptr)
	{
		GroupItem* group_item = static_cast<GroupItem*>(selectedItem);
		menu.addAction(new CreateResourceGroupAction(*group_item->getGroup()));
		menu.addAction(new AddResourceToGroupAction(*group_item->getGroup()));
		menu.addAction(new DeleteObjectAction(*group_item->getObject()));
	}
	else if (dynamic_cast<ObjectItem*>(selectedItem) != nullptr)
	{
		// Cast to resource: TODO: Make the actions handle group requirements
		auto object_item = static_cast<ObjectItem*>(selectedItem);
		auto* resource = rtti_cast<nap::Resource>(object_item->getObject());
		assert(resource != nullptr);

		// Check if the parent is a group
		GroupItem* parent_item = object_item->parentItem() != nullptr ?
			dynamic_cast<GroupItem*>(object_item->parentItem()) :
			nullptr;

		// Add action to move resource to a group
		menu.addAction(new MoveResourceToGroupAction(*resource,
			parent_item != nullptr ? parent_item->getGroup() : nullptr));

		// If the item is parented under a group, offer the option to remove it
		if (parent_item != nullptr)
		{
			GroupItem* parent_item = static_cast<GroupItem*>(object_item->parentItem());
			menu.addAction(new RemoveResourceFromGroupAction(*parent_item->getGroup(), *resource));
		}
		menu.addAction(new DeleteObjectAction(*object_item->getObject()));
	}
	else if (dynamic_cast<RegularResourcesItem*>(selectedItem) != nullptr)
	{
		menu.addAction(new CreateResourceAction());
	}
	else if (dynamic_cast<EntityResourcesItem*>(selectedItem) != nullptr)
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
	// Parents handle item creation separately
	if (parent != nullptr)
		return;

	// Add item
	auto* item = mModel.addObjectItem(*obj);
	if (selectNewObject)
		mTreeView.selectAndReveal(item);
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

void napkin::ResourcePanel::onPropertyValueChanged(const PropertyPath& path)
{
	// Update object name?
	if (path.getProperty().get_name() == nap::rtti::sIDPropertyName)
	{
		auto obj = path.getObject();
		auto objectItem = findItemInModel<napkin::ObjectItem>(mModel, *obj);
		if (objectItem != nullptr)
			objectItem->setText(QString::fromStdString(obj->mID));
	}

	mModel.removeEmbeddedObjects();
}


void napkin::ResourcePanel::onPropertyChildInserted(const PropertyPath& path, int index)
{
	// If an item was moved into the group create an item
	if (path.getObject()->get_type().is_derived_from(RTTI_OF(nap::Group)))
	{
		// Find group, item in group and add
		auto* group = static_cast<nap::Group*>(path.getObject());
		GroupItem* group_item = findItemInModel<GroupItem>(mModel, *group);
		assert(group_item != nullptr);

		// Get item from array 
		auto* object_item = group_item->append(*group->mResources[index].get());
		mTreeView.selectAndReveal(object_item);
	}
}


void ResourcePanel::onPropertyChildRemoved(const PropertyPath& path, int index)
{
	if (path.getObject()->get_type().is_derived_from(RTTI_OF(nap::Group)))
	{
		// Find group, item in group and add
		auto* group = static_cast<nap::Group*>(path.getObject());
		GroupItem* group_item = findItemInModel<GroupItem>(mModel, *group);
		assert(group_item != nullptr);
		group_item->removeRow(index);
	}
}


void ResourcePanel::emitSelectionChanged()
{
	// Grab selected nap objects
	QList<PropertyPath> selectedPaths;
	for (auto m : mTreeView.getSelectedItems())
	{
		auto item = dynamic_cast<ObjectItem*>(m);
		if (item != nullptr)
			selectedPaths << item->propertyPath();
	}

	selectionChanged(selectedPaths);
}

