#include <generic/utility.h>
#include "napkinglobals.h"

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


    for (auto& ob : AppContext::get().objects()) {

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
    layout()->setContentsMargins(0, 0, 0, 0);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
    mTreeView.tree().setColumnWidth(0, 300);
    mTreeView.tree().setSortingEnabled(true);

    connect(&AppContext::get(), &AppContext::fileOpened, this, &OutlinePanel::onFileOpened);
    connect(&AppContext::get(), &AppContext::newFileCreated, this, &OutlinePanel::onNewFile);

    connect(mTreeView.selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &OutlinePanel::onSelectionChanged);

    mTreeView.setMenuHook(std::bind(&OutlinePanel::menuHook, this, std::placeholders::_1));
//    connect(&AppContext::get(), &AppContext::dataChanged, this, &OutlinePanel::refresh);
    connect(&AppContext::get(), &AppContext::entityAdded, this, &OutlinePanel::onEntityAdded);
    connect(&AppContext::get(), &AppContext::componentAdded, this, &OutlinePanel::onComponentAdded);
    connect(&AppContext::get(), &AppContext::objectAdded, this, &OutlinePanel::onObjectAdded);
    connect(&AppContext::get(), &AppContext::objectRemoved, this, &OutlinePanel::onObjectRemoved);
}

void OutlinePanel::menuHook(QMenu& menu)
{
    auto item = mTreeView.selectedItem();
    if (item == nullptr)
        return;

    auto objItem = dynamic_cast<ObjectItem*>(item);
    if (objItem != nullptr) {
        auto entityItem = dynamic_cast<EntityItem*>(item);

        if (entityItem != nullptr) {
            // Selected item is an Entity
            menu.addAction(new AddEntityAction(&entityItem->entity()));

            // Components
            auto addComponentMenu = menu.addMenu("Add Component");
            for (const auto& type : getComponentTypes()) {
                addComponentMenu->addAction(new AddComponentAction(entityItem->entity(), type));
            }
        }

        menu.addAction(new DeleteObjectAction(objItem->object()));
    }

    auto groupItem = dynamic_cast<GroupItem*>(item);
    if (groupItem != nullptr) {
        // TODO: Use anything other than string comparison to filter this shit
        // (necessary type chain is not usable at the time of writing)
        if (groupItem->text() == TXT_LABEL_ENTITIES) {
            menu.addAction(new AddEntityAction(nullptr));
        } else if (groupItem->text() == TXT_LABEL_OBJECTS) {

            // Resources
            auto addObjectMenu = menu.addMenu("Add Object");
            for (const auto& type : getResourceTypes()) {
                addObjectMenu->addAction(new AddObjectAction(type));
            }

        }
    }

}

void OutlinePanel::onNewFile()
{
    refresh();
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
    // TODO: Don't refresh the whole mModel
    mModel.refresh();
    mTreeView.tree().expandAll();
    mTreeView.selectAndReveal(findItem(*entity));
}

void OutlinePanel::onComponentAdded(nap::Component& comp, nap::Entity& owner)
{
    // TODO: Don't refresh the whole mModel
    mModel.refresh();
    mTreeView.tree().expandAll();
    mTreeView.selectAndReveal(findItem(comp));
}

void OutlinePanel::onObjectAdded(nap::rtti::RTTIObject& obj)
{
    // TODO: Don't refresh the whole mModel
    mModel.refresh();
    mTreeView.tree().expandAll();
    mTreeView.selectAndReveal(findItem(obj));
}


void OutlinePanel::onObjectRemoved(nap::rtti::RTTIObject& object)
{
    // TODO: Don't refresh the whole mModel
    mModel.refresh();
    mTreeView.tree().expandAll();
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
