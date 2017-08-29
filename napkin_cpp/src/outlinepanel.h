#pragma once

#include <QStandardItemModel>
#include <nap/resourcemanager.h>
#include "generic/filtertreeview.h"
#include "appcontext.h"

namespace nap { namespace rtti { class RTTIObject; }}


class GroupItem : public QStandardItem {
public:
    GroupItem(const QString& name) : QStandardItem(name) {}
};

class ObjectItem : public QStandardItem {
public:
    ObjectItem(nap::rtti::RTTIObject& o);

    void refresh();

    virtual const QString name() const;

protected:
    nap::rtti::RTTIObject& mObject;
};

class ObjectTypeItem : public QStandardItem {
public:
    ObjectTypeItem(nap::rtti::RTTIObject& o);

    const QString name() const;

protected:
    nap::rtti::RTTIObject& mObject;
};

class EntityItem : public ObjectItem {
public:
    EntityItem(nap::Entity& entity) : ObjectItem(entity) {}

    nap::Entity& entity() { return static_cast<nap::Entity&>(mObject); }
};

class ComponentItem : public ObjectItem {
public:
    ComponentItem(nap::Component& comp) : ObjectItem(comp) {}

    nap::Component& component() { return static_cast<nap::Component&>(mObject); }
};


class OutlineModel : public QStandardItemModel {
public:
    OutlineModel();


    void refresh();

};

class OutlinePanel : public QWidget {
public:
    OutlinePanel();

private:
    void onFileOpened(const QString& filename);

    QVBoxLayout mLayout;
    OutlineModel mModel;
    FilterTreeView mTreeView;
};