#include "outlinepanel.h"
#include "napgeneric.h"

#include "globals.h"
using namespace napkin;

ObjectItem::ObjectItem(nap::rtti::RTTIObject& rttiObject) : mObject(rttiObject) {
    refresh();
}

void ObjectItem::refresh() {
    setText(name());
}

const QString ObjectItem::name() const {
    return QString::fromStdString(mObject.mID);
}


OutlineModel::OutlineModel() {
    setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_TYPE});
}


void OutlineModel::refresh() {
    while (rowCount() > 0)
        removeRow(0);

    auto objectsItem = new GroupItem(TXT_LABEL_OBJECTS);
    appendRow(objectsItem);
    auto entitiesItem = new GroupItem(TXT_LABEL_ENTITIES);
    appendRow(entitiesItem);


    for (auto& ob : AppContext::get().loadedObjects()) {

        auto typeItem = new TypeItem(ob->get_type());

        // All objects are in this flat list, filter here

        if (ob->get_type().is_derived_from<nap::Entity>())
        {
            // Grab entities and stuff them in a group
            auto& e = dynamic_cast<nap::Entity&>(*ob);
            auto entityItem = new EntityItem(e);
            entitiesItem->appendRow({entityItem, typeItem});

            for (auto& comp : e.mComponents) {
                auto compItem = new ComponentItem(*comp);
                auto compTypeItem = new TypeItem(comp->get_type());
                entityItem->appendRow({compItem, compTypeItem});
            }

        }
        else if (!ob->get_type().is_derived_from<nap::Component>())
        {
            // If it's not a Component, stick under objects group
            objectsItem->appendRow({new ObjectItem(*ob), typeItem});
        }
    }

}

OutlinePanel::OutlinePanel() {
    setLayout(&mLayout);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
    mTreeView.tree().setColumnWidth(0, 300);
    mTreeView.tree().setSortingEnabled(true);
    connect(&AppContext::get(), &AppContext::fileOpened, this, &OutlinePanel::onFileOpened);
    connect(mTreeView.selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &OutlinePanel::onSelectionChanged);

}

void OutlinePanel::onFileOpened(const QString& filename) {
    mModel.refresh();
    mTreeView.tree().expandAll();
}

void OutlinePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
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
