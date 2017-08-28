#include "outlinepanel.h"

ObjectItem::ObjectItem(nap::rtti::RTTIObject& rttiObject) : object(rttiObject) {
    refresh();
}

void ObjectItem::refresh() {
    setText(QString::fromStdString(object.mID));
}

OutlineModel::OutlineModel() {
    connect(&AppContext::instance(), &AppContext::fileOpened, this, &OutlineModel::onFileOpened);
}

void OutlineModel::onFileOpened(const QString& filename) {
    refresh();
}

void OutlineModel::refresh() {
    while (rowCount() > 0)
        removeRow(0);


    auto resman = AppContext::instance().core().getOrCreateService<nap::ResourceManagerService>();
    for (auto& ob : resman->getObjects()) {
        appendRow(new ObjectItem(*ob));
    }
}

OutlinePanel::OutlinePanel() {
    setLayout(&mLayout);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
}
