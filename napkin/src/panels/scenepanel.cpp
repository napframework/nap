#include "scenepanel.h"
#include <appcontext.h>
#include <sceneservice.h>
#include <standarditemsobject.h>


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
}

void napkin::SceneModel::refresh()
{
    while (rowCount() > 0)
        removeRow(0);

	for (auto scene : getScenes())
		appendRow(new SceneItem(*scene));
}

void napkin::SceneModel::onEntityAdded(nap::Entity* newEntity, nap::Entity* parent)
{
}

void napkin::SceneModel::onComponentAdded(nap::Component& comp, nap::Entity& owner)
{
}

void napkin::SceneModel::onObjectAdded(nap::rtti::RTTIObject& obj)
{
}

void napkin::SceneModel::onObjectRemoved(nap::rtti::RTTIObject& obj)
{
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
}
