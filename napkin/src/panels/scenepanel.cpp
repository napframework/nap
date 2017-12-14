#include "scenepanel.h"
#include <appcontext.h>
#include <sceneservice.h>
#include <standarditemsobject.h>
#include <commands.h>
#include <generic/filterpopup.h>


/**
 * @return All currently loaded scenes
 */
nap::SceneService::SceneSet getScenes()
{
	return napkin::AppContext::get().getCore().getService<nap::SceneService>()->getScenes();
}


napkin::SceneModel::SceneModel() : QStandardItemModel()
{
    refresh();
    setHorizontalHeaderLabels({"Name"});

    connect(&AppContext::get(), &AppContext::fileOpened, this, &SceneModel::onFileOpened);
    connect(&AppContext::get(), &AppContext::newFileCreated, this, &SceneModel::onNewFile);
	connect(&AppContext::get(), &AppContext::objectAdded, this, &SceneModel::onObjectAdded);
}

void napkin::SceneModel::refresh()
{
    while (rowCount() > 0)
        removeRow(0);

	for (auto scene : getScenes())
		appendRow(new SceneItem(*scene));
}

void napkin::SceneModel::onObjectAdded(nap::rtti::RTTIObject& obj)
{
	// TODO: Don't refresh entire model
	refresh();
}

void napkin::SceneModel::onObjectRemoved(nap::rtti::RTTIObject& obj)
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
	layout()->addWidget(&mFilterView);
    mFilterView.setModel(&mModel);
	mFilterView.setMenuHook(std::bind(&napkin::ScenePanel::menuHook, this, std::placeholders::_1));
	mFilterView.getTreeView().expandAll();
}

void napkin::ScenePanel::menuHook(QMenu& menu)
{
	auto item = mFilterView.getSelectedItem();

	auto scene_item = dynamic_cast<SceneItem*>(item);
	if (scene_item != nullptr)
	{
		auto add_entity_action = menu.addAction("Add Entity...");
		connect(add_entity_action, &QAction::triggered, [this]()
		{
			auto entity = FilterPopup::getObject<nap::Entity>(this);
			if (entity == nullptr)
				return;
			nap::Logger::warn("NOT IMPLEMENTED: I want to add this entity: %s", entity->mID.c_str());
		});

	}

	auto add_scene_action = menu.addAction("Add Scene...");
	connect(add_scene_action, &QAction::triggered, []()
	{
		AppContext::get().executeCommand(new AddSceneCommand());
	});
}
