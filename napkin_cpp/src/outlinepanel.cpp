#include <generic/utility.h>
#include "outlinepanel.h"
#include "napgeneric.h"

#include "globals.h"
#include "actions.h"

using namespace napkin;

ObjectItem::ObjectItem(nap::rtti::RTTIObject& rttiObject) : mObject(rttiObject)
{
    refresh();
}

void ObjectItem::refresh()
{
    setText(name());
}

const QString ObjectItem::name() const
{
    return QString::fromStdString(mObject.mID);
}


OutlineModel::OutlineModel()
{
    setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_TYPE});

}


void OutlineModel::refresh()
{
    while (rowCount() > 0)
        removeRow(0);

    auto objectsItem = new GroupItem(TXT_LABEL_OBJECTS);
    appendRow(objectsItem);
    auto entitiesItem = new GroupItem(TXT_LABEL_ENTITIES);
    appendRow(entitiesItem);


    for (auto& ob : AppContext::get().loadedObjects()) {

        auto typeItem = new TypeItem(ob->get_type());

        // All objects are in this flat list, filter here

        if (ob->get_type().is_derived_from<nap::Entity>()) {
            // Grab entities and stuff them in a group
            auto& e = dynamic_cast<nap::Entity&>(*ob);

            if (AppContext::get().getParent(e))
                continue; // Only add root objects

            auto entityItem = new EntityItem(e);
            entitiesItem->appendRow({entityItem, typeItem});


        } else if (!ob->get_type().is_derived_from<nap::Component>()) {
            // If it's not a Component, stick under objects group
            objectsItem->appendRow({new ObjectItem(*ob), typeItem});
        }
    }

}


OutlinePanel::OutlinePanel()
{
    setLayout(&mLayout);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
    mTreeView.tree().setColumnWidth(0, 300);
    mTreeView.tree().setSortingEnabled(true);
    connect(&AppContext::get(), &AppContext::fileOpened, this, &OutlinePanel::onFileOpened);
    connect(mTreeView.selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &OutlinePanel::onSelectionChanged);

    mTreeView.setMenuHook(std::bind(&OutlinePanel::menuHook, this, std::placeholders::_1));
//    connect(&AppContext::get(), &AppContext::dataChanged, this, &OutlinePanel::refresh);
    connect(&AppContext::get(), &AppContext::entityAdded, this, &OutlinePanel::onEntityAdded);
}

void OutlinePanel::menuHook(QMenu& menu)
{
    auto item = mTreeView.selectedItem();
    if (item == nullptr)
        return;

    auto objItem = dynamic_cast<ObjectItem*>(item);
    if (objItem != nullptr) {
        rttr::instance instance = objItem->object();
        if (instance.get_derived_type().is_derived_from(RTTI_OF(nap::Entity))) {
            // Selected item is an Entity

            menu.addAction(new AddEntityAction(instance.try_convert<nap::Entity>()));
        }
    }

    auto groupItem = dynamic_cast<GroupItem*>(item);
    if (groupItem != nullptr) {
        // No way to tell a resource/instance apart. Go refactor this and behold why this is here.
        if (groupItem->text() == TXT_LABEL_ENTITIES) {
            menu.addAction(new AddEntityAction(nullptr));
        }
    }

}

void OutlinePanel::onFileOpened(const QString& filename)
{
    refresh();
}

void OutlinePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    // Grab selected nap objects
    QList<nap::rtti::RTTIObject*> selectedObjects;
    for (auto m : mTreeView.selectedItems()) {
        auto item = dynamic_cast<ObjectItem*>(m);
        if (!item)
            continue;
        selectedObjects << &item->object();
    }

    selectionChanged(selectedObjects);
}

std::vector<rttr::instance> OutlinePanel::selectedInstances() const
{
    std::vector<rttr::instance> instances;
    for (QStandardItem* item : mTreeView.selectedItems()) {
        auto objItem = dynamic_cast<ObjectItem*>(item);
        if (objItem == nullptr)
            continue;
        instances.emplace_back(objItem->object());
    }
    return instances;
}

void OutlinePanel::refresh()
{
    mModel.refresh();
    mTreeView.tree().expandAll();
}

ObjectItem* OutlinePanel::findItem(const nap::rtti::RTTIObject& obj)
{
    ObjectItem* foundItem = nullptr;

    findItemInModel(mTreeView.filterModel(), [this, &foundItem, &obj](const QModelIndex& idx) -> bool {
        QStandardItem* item = mModel.itemFromIndex(mTreeView.filterModel().mapToSource(idx));
        if (item == nullptr)
            return false;

        auto objItem = dynamic_cast<ObjectItem*>(item);
        if (objItem == nullptr)
            return false;

        if (&objItem->object() == &obj) {
            foundItem = objItem;
            return true;
        }

        return false;
    });

    return foundItem;
}

void OutlinePanel::onEntityAdded(nap::Entity* entity, nap::Entity* parent)
{
    mModel.refresh();
    mTreeView.tree().expandAll();
    mTreeView.selectAndReveal(findItem(*entity));
}


EntityItem::EntityItem(nap::Entity& entity) : ObjectItem(entity)
{

    for (auto& child : entity.mChildren) {
        appendRow({new EntityItem(*child), new TypeItem(child->get_type())});
    }

    for (auto& comp : entity.mComponents) {
        auto compItem = new ComponentItem(*comp);
        auto compTypeItem = new TypeItem(comp->get_type());
        appendRow({compItem, compTypeItem});
    }
}
