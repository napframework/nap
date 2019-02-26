#include "scenepanel.h"
#include <appcontext.h>
#include <sceneservice.h>
#include <standarditemsobject.h>
#include <commands.h>
#include <napqt/filterpopup.h>
#include <naputils.h>


/**
 * @return All currently loaded scenes
 */
nap::SceneService::SceneSet getScenes()
{
	auto sceneService = napkin::AppContext::get().getCore().getService<nap::SceneService>();
	if (sceneService)
		return sceneService->getScenes();
	return {};
}


napkin::SceneModel::SceneModel() : QStandardItemModel()
{
    setHorizontalHeaderLabels({"Name"});

    connect(&AppContext::get(), &AppContext::documentOpened, this, &SceneModel::onFileOpened);
    connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &SceneModel::onNewFile);
	connect(&AppContext::get(), &AppContext::objectAdded, this, &SceneModel::onObjectAdded);
	connect(&AppContext::get(), &AppContext::objectChanged, this, &SceneModel::onObjectChanged);
}

void napkin::SceneModel::refresh()
{
    while (rowCount() > 0)
        removeRow(0);

	for (auto scene : getScenes())
		appendRow(new SceneItem(*scene));

}

void napkin::SceneModel::onObjectAdded(nap::rtti::Object* obj)
{
	// TODO: Don't refresh entire model
	refresh();
}

void napkin::SceneModel::onObjectRemoved(nap::rtti::Object* obj)
{
	// TODO: Don't refresh entire model
	refresh();
}

void napkin::SceneModel::onObjectChanged(nap::rtti::Object* obj)
{
	// TODO: Don't refresh entire model
	refresh();
}

void napkin::SceneModel::onNewFile()
{
    refresh();
}

void napkin::SceneModel::onFileOpened(const QString& filename)
{
    refresh();
}

napkin::ScenePanel::ScenePanel() : QWidget()
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(&mFilterView);
    mFilterView.setModel(&mModel);
	mFilterView.setMenuHook(std::bind(&napkin::ScenePanel::menuHook, this, std::placeholders::_1));
	mFilterView.getTreeView().expandAll();

	connect(mFilterView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, &ScenePanel::onSelectionChanged);
	connect(&mModel, &QAbstractItemModel::rowsInserted, [this](const QModelIndex &parent, int first, int last) {
		mFilterView.getTreeView().expandAll();
	});

}

void napkin::ScenePanel::menuHook(QMenu& menu)
{
	auto item = mFilterView.getSelectedItem();

	{
		auto sceneItem = dynamic_cast<SceneItem*>(item);
		if (sceneItem)
		{
			auto scene = rtti_cast<nap::Scene>(sceneItem->getObject());
			assert(scene->get_type().is_derived_from<nap::Scene>());

			auto addEntityAction = menu.addAction("Add Entity...");
			connect(addEntityAction, &QAction::triggered, [this, sceneItem, scene]()
			{
				auto entities = AppContext::get().getDocument()->getObjects(RTTI_OF(nap::Entity));
				auto entity = dynamic_cast<nap::Entity*>(napkin::showObjectSelector(this, entities));
				if (!entity)
					return;

				AppContext::get().executeCommand(new AddEntityToSceneCommand(*scene, *entity));
			});


		}
	}

	auto rootEntityItem = dynamic_cast<RootEntityItem*>(item);
	if (rootEntityItem)
	{
		auto sceneItem = rootEntityItem->sceneItem();

		if (sceneItem)
		{
			auto scene = rtti_cast<nap::Scene>(sceneItem->getObject());
			assert(scene);
			auto rootEntity = &rootEntityItem->rootEntity();
			assert(rootEntity);

			auto removeEntityAction = menu.addAction("Delete Instance");
			connect(removeEntityAction, &QAction::triggered, [scene, rootEntity]
			{
				AppContext::get().executeCommand(new RemoveEntityFromSceneCommand(*scene, *rootEntity));
			});
		}
	}
}

void napkin::ScenePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	// Grab selected nap objects
	QList<PropertyPath> selectedObjects;
	for (auto m : mFilterView.getSelectedItems())
	{
		auto eItem = dynamic_cast<EntityInstanceItem*>(m);
		if (eItem)
			selectedObjects << eItem->propertyPath();

		auto cItem = dynamic_cast<ComponentInstanceItem*>(m);
		if (cItem)
			selectedObjects << cItem->propertyPath();
	}

	selectionChanged(selectedObjects);
}
