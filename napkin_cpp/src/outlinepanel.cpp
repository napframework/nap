#include "outlinepanel.h"

ObjectItem::ObjectItem(nap::rtti::RTTIObject& rttiObject) : mObject(rttiObject) {
    refresh();
}

void ObjectItem::refresh() {
    setText(name());
}

const QString ObjectItem::name() const {
    return QString::fromStdString(mObject.mID);
}

ObjectTypeItem::ObjectTypeItem(nap::rtti::RTTIObject& o) : mObject(o) {
    setEditable(false);
    setText(name());
}

const QString ObjectTypeItem::name() const {
    return QString(mObject.get_type().get_name().data());
}


OutlineModel::OutlineModel() {
    setHorizontalHeaderLabels({"Name", "Type"});

}


void OutlineModel::refresh() {
    while (rowCount() > 0)
        removeRow(0);

    auto objectsItem = new GroupItem("Objects");
    appendRow(objectsItem);
    auto entitiesItem = new GroupItem("Entities");
    appendRow(entitiesItem);


    // The loaded list has no structure, build it here.
    for (auto& ob : AppContext::get().loadedObjects()) {

        auto typeItem = new ObjectTypeItem(*ob);
        if (ob.get()->get_type().is_derived_from<nap::Entity>()) {
            // Grab entities and stuff them in a group
            auto& e = static_cast<nap::Entity&>(*ob);
            auto entityItem = new EntityItem(e);
            entitiesItem->appendRow({entityItem, typeItem});

            for (auto& comp : e.mComponents) {
                auto compItem = new ComponentItem(*comp);
                auto compTypeItem = new ObjectTypeItem(*comp);
                entityItem->appendRow({compItem, compTypeItem});
            }

        } else if (!ob.get()->get_type().is_derived_from<nap::Component>()) {
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

}

void OutlinePanel::onFileOpened(const QString& filename) {
    mModel.refresh();
    mTreeView.tree().expandAll();
}