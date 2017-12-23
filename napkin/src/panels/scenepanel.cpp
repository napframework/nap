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
	connect(&AppContext::get(), &AppContext::objectChanged, this, &SceneModel::onObjectChanged);
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

void napkin::SceneModel::onObjectChanged(nap::rtti::RTTIObject& obj)
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

	connect(&mModel, &QAbstractItemModel::rowsInserted, [this](const QModelIndex &parent, int first, int last) {
		mFilterView.getTreeView().expandAll();
	});
}

void napkin::ScenePanel::menuHook(QMenu& menu)
{
	auto item = mFilterView.getSelectedItem();

	auto scene_item = dynamic_cast<SceneItem*>(item);
	if (scene_item != nullptr)
	{
		auto scene = rtti_cast<nap::Scene>(scene_item->getObject());
		assert(scene->get_type().is_derived_from<nap::Scene>());

		auto add_entity_action = menu.addAction("Add Entity...");
		connect(add_entity_action, &QAction::triggered, [this, scene_item, scene]()
		{
			auto entity = FilterPopup::getObject<nap::Entity>(this);
			if (entity == nullptr)
				return;

			AppContext::get().executeCommand(new AddEntityToSceneCommand(*scene, *entity));
		});

	}

	auto add_scene_action = menu.addAction("Add Scene...");
	connect(add_scene_action, &QAction::triggered, []()
	{
		AppContext::get().executeCommand(new AddObjectCommand(RTTI_OF(nap::Scene)));
	});
}
