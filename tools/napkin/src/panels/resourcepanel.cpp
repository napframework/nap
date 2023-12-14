/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "resourcepanel.h"
#include "naputils.h"

#include <commands.h>
#include <napqt/filterpopup.h>
#include <napqt/qtutils.h>
#include <napkin-resources.h>
#include <QKeyEvent>

using namespace napkin;

static bool ResourceSorter(const QModelIndex& left, const QModelIndex& right, QAbstractItemModel* model)
{
	// Get model
	assert(qobject_cast<ResourceModel*>(model) != nullptr);
	ResourceModel* resource_model = static_cast<ResourceModel*>(model);

	// Get and cast to RTTI Item
	auto l_item = qitem_cast<RTTIItem*>(resource_model->itemFromIndex(left));
	auto r_item = qitem_cast<RTTIItem*>(resource_model->itemFromIndex(right));

	// Bail if we're not an rtti item
	if (l_item == nullptr || r_item == nullptr)
		return false;

	// Don't sort root (top) items
	if (qitem_cast<EntityResourcesItem*>(l_item) != nullptr &&
		qitem_cast<RootResourcesItem*>(r_item) != nullptr)
		return false;

	// Check if item is an entity
	auto le_item = qitem_cast<EntityItem*>(l_item);
	auto re_item = qitem_cast<EntityItem*>(r_item);

	// Check if item is a component
	auto lc_item = qitem_cast<ComponentItem*>(l_item);
	auto rc_item = qitem_cast<ComponentItem*>(r_item);

	// left is entity, right is component
	if (le_item != nullptr && rc_item != nullptr)
		return true;

	// right is component, left is entity
	if (lc_item != nullptr && re_item != nullptr)
		return false;

	// Don't sort items of same type of which parent is an entity -> they can be re-ordered
	if (qitem_cast<EntityItem*>(l_item->parentItem()) != nullptr &&
		qitem_cast<EntityItem*>(r_item->parentItem()) != nullptr)
		return false;

	// Prioritize groups over other items
	GroupItem* lg_item = qitem_cast<GroupItem*>(l_item);
	GroupItem* rg_item = qitem_cast<GroupItem*>(r_item);

	// Left is group, right is not
	if (lg_item != nullptr && rg_item == nullptr)
		return true;

	// Right is group, left is not
	if (rg_item != nullptr && lg_item == nullptr)
		return false;

	// Don't sort items of the same type in a group -> they can be re-ordered
	if (qitem_cast<GroupItem*>(l_item->parentItem()) != nullptr &&
		qitem_cast<GroupItem*>(r_item->parentItem()) != nullptr)
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
	connect(&AppContext::get(), &AppContext::documentOpened, this, &ResourcePanel::onFileOpened);
	connect(&AppContext::get(), &AppContext::documentClosing, this, &ResourcePanel::onFileClosing);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &ResourcePanel::onNewFile);
	connect(&AppContext::get(), &AppContext::projectLoaded, this, &ResourcePanel::onProjectLoaded);

	auto& resources_item = mModel.getRootResourcesItem();
	resources_item.setEnabled(AppContext::get().getProjectLoaded());
	connect(&resources_item, &RootResourcesItem::childAddedToGroup, this, &ResourcePanel::onChildAddedToGroup);
	connect(&resources_item, &RootResourcesItem::indexChanged, this, [this](GroupItem& parent, ObjectItem& itemA, ObjectItem& itemB)
		{
			this->onIndexChanged(parent, itemA, itemB);
		});

	auto& entities_item = mModel.getEntityResourcesItem();
	entities_item.setEnabled(AppContext::get().getProjectLoaded());
	connect(&entities_item, &EntityResourcesItem::childAddedToEntity, this, &ResourcePanel::onChildAddedToEntity);
	connect(&entities_item, &EntityResourcesItem::indexChanged, this, [this](EntityItem& parent, ObjectItem& itemA, ObjectItem& itemB)
		{ 
			this->onIndexChanged(parent, itemA, itemB);
		});

	createMenuCallbacks();
	mTreeView.setMenuHook(std::bind(&ResourcePanel::menuHook, this, std::placeholders::_1));
	mTreeView.installEventFilter(this);
}


void napkin::ResourceModel::populate()
{
	auto doc = AppContext::get().getDocument();
	assert(doc != nullptr);
	auto root_objects = topLevelObjects();
	mObjectsItem.populate(root_objects);
	mEntitiesItem.populate(root_objects);
}


void napkin::ResourceModel::clearItems()
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
	if (selected_item != nullptr)
	{
		mMenuController.populate(*selected_item, menu);
	}
}


static void addMoveAction(const PropertyPath& container, nap::rtti::Object& object, size_t index, QMenu& outMenu)
{
	// Move item up
	if (index > 0)
	{
		outMenu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_MOVE_UP),
			QString("Move '%1' up").arg(object.mID.c_str()), [container, index]()
			{
				AppContext::get().executeCommand(new ArrayMoveElementCommand(container, index, index-1));
			});
	}

	// Move item down
	if (index < container.getArrayLength() - 1)
	{
		outMenu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_MOVE_DOWN),
			QString("Move '%1' down").arg(object.mID.c_str()), [container, index]()
			{
				AppContext::get().executeCommand(new ArrayMoveElementCommand(container, index, index+1));
			});
	}
}


void napkin::ResourcePanel::createMenuCallbacks()
{
	// Move child entity up or down
	mMenuController.addOption<EntityItem>([](auto& item, auto& menu)
	{
		// Get potential parent
		auto entity_item = static_cast<EntityItem*>(&item);
		auto parent_item = qobject_cast<EntityItem*>(entity_item->parentItem());
		if (parent_item == nullptr)
			return;

		// Get component index -> can't be the row because of other possible child items
		auto entity_path = entity_item->propertyPath();
		size_t idx = entity_path.getEntityIndex();

		// Create path to component array property
		PropertyPath children_array(parent_item->getObject(),
			RTTI_OF(nap::Entity).get_property(nap::Entity::childrenPropertyName()),
			*AppContext::get().getDocument());

		// Add move up / down
		addMoveAction(children_array, entity_item->getObject(), idx, menu);
	});

	// Child Entity
	mMenuController.addOption<EntityItem>([](auto& item, auto& menu)
	{
		auto entity_item = static_cast<EntityItem*>(&item);
		if (!entity_item->isPointer())
			return;

		auto parent_item = qobject_cast<EntityItem*>(entity_item->parentItem());
		assert(parent_item != nullptr);
		menu.addAction(new RemovePathAction(&menu, entity_item->propertyPath()));
	});

	// Resource Entity
	mMenuController.addOption<EntityItem>([](auto& item, auto& menu)
	{
		auto entity_item = static_cast<EntityItem*>(&item);
		if (entity_item->isPointer())
			return;

		menu.addAction(new AddChildEntityAction(&menu, entity_item->getEntity()));
		menu.addAction(new AddComponentAction(&menu, entity_item->getEntity()));
	});

	// Component move up or down
	mMenuController.addOption<ComponentItem>([](auto& item, auto& menu)
	{
		// Get parent
		auto component_item = static_cast<ComponentItem*>(&item);
		auto parent_item = qobject_cast<EntityItem*>(component_item->parentItem());
		assert(parent_item != nullptr);

		// Get component index -> can't be the row because of other possible child items
		size_t idx = 0; bool found = false;
		const auto& components = parent_item->getEntity().getComponents();
		for (const auto& comp : components)
		{
			if (comp.get() == &component_item->getComponent())
				break;
			idx++;
		} assert(components[idx].get() == &component_item->getComponent());

		// Create path to component array property
		PropertyPath component_array(parent_item->getObject(), 
			RTTI_OF(nap::Entity).get_property(nap::Entity::componentsPropertyName()),  
			*AppContext::get().getDocument());

		// Add move up / down
		addMoveAction(component_array, component_item->getObject(), idx, menu);
	});

	// Move resource or child in group up or down
	mMenuController.addOption<ObjectItem>([](auto& item, auto& menu)
	{
		// Bail if we're not in a group
		auto object_item = static_cast<ObjectItem*>(&item);
		auto parent_item = qitem_cast<GroupItem*>(object_item->parentItem());
		if (parent_item == nullptr)
			return;

		// Get container property -> child groups or members
		nap::IGroup& group = parent_item->getGroup();
		auto container_property = object_item->getObject().get_type().is_derived_from(RTTI_OF(nap::IGroup)) ?
			group.getChildrenProperty() : group.getMembersProperty();

		// Get index in container
		PropertyPath container_path(parent_item->getObject(), container_property, *AppContext::get().getDocument());
		auto container_value = container_path.getValue(); assert(container_value.is_valid());
		auto container_view = container_value.create_array_view();
		int idx = -1;
		for (auto i = 0; i < container_view.get_size(); i++)
		{
			auto val = container_view.get_value(i); assert(val.get_type().is_wrapper());
			auto obj = val.extract_wrapped_value().get_value<nap::rtti::Object*>();
			if (obj == &object_item->getObject())
			{
				idx = i;
				break;
			}
		} assert(idx >= 0);

		// Add move up / down
		addMoveAction(container_path, object_item->getObject(), idx, menu);
	});

	// Group
	mMenuController.addOption<GroupItem>([](auto& item, auto& menu)
	{
		auto group_item = static_cast<GroupItem*>(&item);
		menu.addAction(new AddNewResourceToGroupAction(&menu, group_item->getGroup()));
		menu.addAction(new AddExistingResourceToGroupAction(&menu, group_item->getGroup()));

		// If the item is parented under a group, offer the option to remove it
		auto* item_group = getItemGroup(*group_item);
		if (item_group != nullptr)
			menu.addAction(new RemoveGroupFromGroupAction(&menu, *item_group, group_item->getGroup()));

		menu.addAction(new AddChildGroupAction(&menu, group_item->getGroup()));
		menu.addAction(new MoveGroupAction(&menu, group_item->getGroup(), item_group));
		menu.addAction(new DeleteGroupAction(&menu, group_item->getGroup()));
	});

	// Shader Resource
	mMenuController.addOption<BaseShaderItem>([](auto& item, auto& menu)
	{
		auto object_item = static_cast<BaseShaderItem*>(&item);
		menu.addAction(new LoadShaderAction(&menu, static_cast<nap::BaseShader&>(object_item->getObject())));
	});

	// Regular object that isn't a group
	mMenuController.addOption<ObjectItem>([](auto& item, auto& menu)
	{
		// Ensure it's an object and not a group
		auto object_item = static_cast<ObjectItem*>(&item);
		if (object_item->getObject().get_type().is_derived_from(RTTI_OF(nap::IGroup)))
			return;

		// Offer option to move if parented under resources
		auto obj_parent = object_item->parentItem();
		while (obj_parent != nullptr)
		{
			if (qobject_cast<RootResourcesItem*>(obj_parent) != nullptr)
			{
				// If the item is parented under a group, offer the option to remove it
				auto* item_group = getItemGroup(*object_item);
				if (item_group != nullptr)
					menu.addAction(new RemoveResourceFromGroupAction(&menu, *item_group, object_item->getObject()));

				// Add option to move resource to a new group 
				menu.addAction(new MoveResourceToGroupAction(&menu, object_item->getObject(), item_group));
				break;
			}
			obj_parent = obj_parent->parentItem();
		}

		// Allow object to be deleted if it's not a reference
		if(!object_item->isPointer())
			menu.addAction(new DeleteObjectAction(&menu, object_item->getObject()));
	});

	// Top Resource
	mMenuController.addOption<RootResourcesItem>([](auto& item, auto& menu)
	{
		auto root_item = static_cast<RootResourcesItem*>(&item);
		menu.addAction(new CreateResourceAction(&menu));
		menu.addAction(new CreateGroupAction(&menu));
	});

	// Top Entity
	mMenuController.addOption<EntityResourcesItem>([](auto& item, auto& menu)
	{
		auto root_item = static_cast<EntityResourcesItem*>(&item);
		menu.addAction(new CreateEntityAction(&menu));
	});
}


void napkin::ResourcePanel::onProjectLoaded(const nap::ProjectInfo& projectInfo)
{
	mModel.getRootResourcesItem().setEnabled(true);
	mModel.getEntityResourcesItem().setEnabled(true);
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


bool napkin::ResourcePanel::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == &mTreeView && ev->type() == QEvent::KeyPress)
	{
		// Handle deletion of object
		QKeyEvent* key_event = static_cast<QKeyEvent*>(ev);
		if (key_event->key() == Qt::Key_Delete)
		{
			// Cast to 
			auto obj_item = qitem_cast<ObjectItem*>(mTreeView.getSelectedItem());
			if (obj_item != nullptr)
			{
				DeleteObjectAction action(nullptr, obj_item->getObject());
				action.trigger();
			}
			return true;
		}
	}
	return QWidget::eventFilter(obj, ev);
}


void napkin::ResourcePanel::clear()
{
	mTreeView.getTreeView().selectionModel()->clear();
	mModel.clearItems();
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
	auto selected_it = qitem_cast<ObjectItem*>(mTreeView.getSelectedItem());
	if (selected_it != nullptr && selected_it == &entity)
		mTreeView.select(&item, true);
}


void napkin::ResourcePanel::onIndexChanged(ObjectItem& parent, ObjectItem& itemA, ObjectItem& itemB)
{
	auto selected_it = qitem_cast<ObjectItem*>(mTreeView.getSelectedItem());
	if (selected_it != nullptr && selected_it->parentItem() == &parent)
		mTreeView.select(&itemA, false);
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

