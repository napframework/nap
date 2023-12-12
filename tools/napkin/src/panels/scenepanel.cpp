/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "scenepanel.h"

#include <appcontext.h>
#include <sceneservice.h>
#include <standarditemsobject.h>
#include <commands.h>
#include <napqt/filterpopup.h>
#include <naputils.h>
#include <QKeyEvent>


napkin::SceneModel::SceneModel() : QStandardItemModel()
{
	setHorizontalHeaderLabels({"Name"});
	connect(&AppContext::get(), &AppContext::documentOpened, this, &SceneModel::onFileOpened);
	connect(&AppContext::get(), &AppContext::documentClosing, this, &SceneModel::onFileClosing);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &SceneModel::onNewFile);
	connect(&AppContext::get(), &AppContext::objectAdded, this, &SceneModel::onObjectAdded);
	connect(&AppContext::get(), &AppContext::objectChanged, this, &SceneModel::onObjectChanged);
	connect(&AppContext::get(), &AppContext::objectRemoved, this, &SceneModel::onObjectRemoved);
	connect(&AppContext::get(), &AppContext::propertyIndexChanged, this, &SceneModel::onIndexChanged);
}


napkin::RootEntityItem* napkin::SceneModel::rootEntityItem(nap::RootEntity& rootEntity) const
{
	for (auto i = 0; i < rowCount(); i++)
	{
		auto sceneItem = item(i, 0);
		for (auto j = 0; j < sceneItem->rowCount(); j++)
		{
			auto reItem = qitem_cast<RootEntityItem*>(sceneItem->child(j, 0));
			assert(reItem);
			if (&reItem->rootEntity() == &rootEntity)
				return reItem;
		}
	}
	return nullptr;
}


void napkin::SceneModel::clear()
{
	removeRows(0, rowCount());
}


void napkin::SceneModel::populate()
{
	this->clear();
	mSceneItems.clear();

	auto doc = napkin::AppContext::get().getDocument();
	assert(doc != nullptr);

	// Create items for scene & children in scene
	auto scenes = doc->getObjects<nap::Scene>();
	for (const auto& scene : scenes)
	{
		auto scene_item = new SceneItem(*scene);
		appendRow(scene_item);
		mSceneItems.emplace_back(scene_item);
	}

	// Notify listeners
	populated(mSceneItems);
}


static bool refresh(nap::rtti::Object* obj)
{
	// TODO: Check if the component or entity is part of the scene.
	return	obj->get_type().is_derived_from(RTTI_OF(nap::Scene))	||
		obj->get_type().is_derived_from(RTTI_OF(nap::Entity))		||
		obj->get_type().is_derived_from(RTTI_OF(nap::Component));
}


void napkin::SceneModel::onObjectAdded(nap::rtti::Object* obj)
{
	if (refresh(obj))
	{
		populate();
	}
}


void napkin::SceneModel::onObjectChanged(nap::rtti::Object* obj)
{
	if (refresh(obj))
	{
		populate();
	}
}


void napkin::SceneModel::onObjectRemoved(nap::rtti::Object* obj)
{
	if (refresh(obj))
	{
		populate();
	}
}


void napkin::SceneModel::onIndexChanged(const PropertyPath& path, size_t oldIndex, size_t newIndex)
{
	if (refresh(path.getObject()))
	{
		populate();
	}
}


void napkin::SceneModel::onNewFile()
{
	populate();
}


void napkin::SceneModel::onFileOpened(const QString& filename)
{
	populate();
}


void napkin::SceneModel::onFileClosing(const QString& filename)
{
	clear();
}


napkin::ScenePanel::ScenePanel() : QWidget()
{
	// Setup widget
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(&mFilterView);

	// Add model
	mFilterView.setModel(&mModel);
	mFilterView.setMenuHook(std::bind(&napkin::ScenePanel::menuHook, this, std::placeholders::_1));
	mFilterView.disableSorting();

	// Listen to changes
	connect(mFilterView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, &ScenePanel::onSelectionChanged);
	connect(&mModel, &SceneModel::populated, this, &ScenePanel::onModelPopulated);

	// Filter events
	mFilterView.installEventFilter(this);
}


bool napkin::ScenePanel::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == &mFilterView && ev->type() == QEvent::KeyPress)
	{
		// Handle deletion of object
		QKeyEvent* key_event = static_cast<QKeyEvent*>(ev);
		if (key_event->key() == Qt::Key_Delete)
		{
			// Cast to root entity item
			auto entity_item = qitem_cast<RootEntityItem*>(mFilterView.getSelectedItem());
			if (entity_item != nullptr)
			{
				AppContext::get().executeCommand(
					new RemoveCommand(entity_item->propertyPath())
				);
			}
			return true;
		}
	}
	return QWidget::eventFilter(obj, ev);
}


void napkin::ScenePanel::menuHook(QMenu& menu)
{
	auto item (mFilterView.getSelectedItem());

	auto scene_item = qitem_cast<SceneItem*>(item);
	if (scene_item != nullptr)
	{
		auto& scene = scene_item->getScene();
		auto add_entity_action = menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_ENTITY), "Add Entity...");
		connect(add_entity_action, &QAction::triggered, [this, &scene]()
			{
				auto entities = AppContext::get().getDocument()->getObjects(RTTI_OF(nap::Entity));
				auto entity = rtti_cast<nap::Entity>(napkin::showObjectSelector(this, entities));
				if (entity != nullptr)
				{
					AppContext::get().executeCommand(new AddEntityToSceneCommand(scene, *entity));
				}
			});
	}

	auto root_entity_item = qitem_cast<RootEntityItem*>(item);
	if (root_entity_item != nullptr)
	{
		auto scene_item = root_entity_item->sceneItem();
		if (scene_item)
		{
			auto removeEntityAction = menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_DELETE), "Delete Instance");
			connect(removeEntityAction, &QAction::triggered, [root_entity_item]
			{
				AppContext::get().executeCommand(new RemoveCommand(root_entity_item->propertyPath()));
			});
		}
	}
}


nap::qt::FilterTreeView& napkin::ScenePanel::treeView()
{
	return mFilterView;
}


void napkin::ScenePanel::select(nap::RootEntity* rootEntity, const QString& path)
{
	auto item = resolveItem(rootEntity, path);
	mFilterView.select(item, true);
}


void napkin::ScenePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	// Grab selected nap objects
	QList<PropertyPath> selectedPaths;
	for (auto m : mFilterView.getSelectedItems())
	{
		auto eItem = qitem_cast<EntityInstanceItem*>(m);
		if (eItem != nullptr)
		{
			selectedPaths << eItem->propertyPath();
			continue;
		}

		auto cItem = qitem_cast<ComponentInstanceItem*>(m);
		if (cItem != nullptr)
		{
			selectedPaths << cItem->propertyPath();
			continue;
		}
	}
	selectionChanged(selectedPaths);
}


void napkin::ScenePanel::onModelPopulated(const std::vector<SceneItem*>& scenes)
{
	for (const auto& item : scenes)
	{
		mFilterView.expand(*item);
	}
}


napkin::ComponentInstanceItem* napkin::ScenePanel::resolveItem(nap::RootEntity* rootEntity, const QString& path)
{
	EntityInstanceItem* rootEntityItem = mModel.rootEntityItem(*rootEntity);
	assert(rootEntityItem);
	EntityInstanceItem* current_parent = rootEntityItem;

	auto splitstring = nap::utility::splitString(path.toStdString(), '/');
	for (int i = 0; i < splitstring.size(); i++)
	{
		// entity
		auto part = splitstring[i];
		if (part == ".")
			continue;

		if (i == splitstring.size() - 1)
		{
			// component
			for (int row = 0; row < current_parent->rowCount(); row++)
			{
				auto child = qitem_cast<ComponentInstanceItem*>(current_parent->child(row));
				if (child->component().mID == part)
					return child;
			}
		}
		else
		{
			std::string name;
			int index = 0;
			if (!nameAndIndex(part, name, index))
				index = 0;

			int foundIndex = 0;
			for (int row = 0; row < current_parent->rowCount(); row++)
			{
				auto child = qitem_cast<EntityInstanceItem*>(current_parent->child(row));
				if (!child)
					continue;

				if (child->entity().mID == name)
				{
					if (foundIndex == index)
					{
						current_parent = child;
						break;
					}
					foundIndex++;
				}
			}
		}
	}
	return nullptr;
}

