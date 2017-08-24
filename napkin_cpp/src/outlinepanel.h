#pragma once

#include "generic/filtertreeview.h"
#include <QWidget>

namespace nap { namespace rtti { class RTTIObject; }}

class ObjectItem : public QStandardItem {
public:
    ObjectItem(nap::rtti::RTTIObject& rttiObject) : QStandardItem(), object(rttiObject) {
//        refresh();
    }

//    void refresh() {
//        setText(QString::fromStdString(object.mID));
//    }

private:
    nap::rtti::RTTIObject& object;
};

class OutlinePanel : public QWidget {
public:
    OutlinePanel() {
        setLayout(&layout);
        layout.addWidget(&treeView);
    }

private:
    QVBoxLayout layout;
    FilterTreeView treeView;
};