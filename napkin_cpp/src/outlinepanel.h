#pragma once

#include <QStandardItemModel>
#include <nap/resourcemanager.h>
#include "generic/filtertreeview.h"
#include "appcontext.h"

namespace nap { namespace rtti { class RTTIObject; }}

class ObjectItem : public QStandardItem {
public:
    ObjectItem(nap::rtti::RTTIObject& rttiObject);

    void refresh();

private:
    nap::rtti::RTTIObject& object;
};

class OutlineModel : public QStandardItemModel {
public:
    OutlineModel();


private:
    void onFileOpened(const QString& filename);

    void refresh();
};

class OutlinePanel : public QWidget {
public:
    OutlinePanel();

private:
    QVBoxLayout mLayout;
    OutlineModel mModel;
    FilterTreeView mTreeView;
};