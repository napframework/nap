#pragma once

#include <QStandardItemModel>
#include <nap/resourcemanager.h>
#include "generic/filtertreeview.h"
#include "appcontext.h"
#include "actions.h"
#include "napgeneric.h"

namespace nap { namespace rtti { class RTTIObject; }}


class GroupItem : public QStandardItem {
public:
    GroupItem(const QString& name) : QStandardItem(name) {}
    int type() const override { return QStandardItem::UserType + 1; }
};

class ObjectItem : public QStandardItem {
public:
    ObjectItem(nap::rtti::RTTIObject& o);
    int type() const override { return QStandardItem::UserType + 2; }

    void refresh();

    nap::rtti::RTTIObject& object() const
    { return mObject; }

    virtual const QString name() const;

protected:
    nap::rtti::RTTIObject& mObject;
};

class ObjectTypeItem : public QStandardItem {
public:
    ObjectTypeItem(nap::rtti::RTTIObject& o);
    int type() const override { return QStandardItem::UserType + 3; }

    const QString name() const;

protected:
    nap::rtti::RTTIObject& mObject;
};

class EntityItem : public ObjectItem {
public:
    EntityItem(nap::Entity& entity);
    int type() const override { return QStandardItem::UserType + 4; }

    nap::Entity& entity()
    { return static_cast<nap::Entity&>(mObject); }
};

class ComponentItem : public ObjectItem {
public:
    ComponentItem(nap::Component& comp) : ObjectItem(comp)  {}
    int type() const override { return QStandardItem::UserType + 5; }

    nap::Component& component()
    { return static_cast<nap::Component&>(mObject); }
};


class OutlineModel : public QStandardItemModel {
public:
    OutlineModel();
    void refresh();


};

class OutlinePanel : public QWidget {
Q_OBJECT
public:
    OutlinePanel();

signals:
    void selectionChanged(QList<nap::rtti::RTTIObject*>& obj);

private:
    void refresh();

    void onEntityAdded(nap::Entity* newEntity, nap::Entity* parent);
    void onComponentAdded(nap::Component& comp, nap::Entity& owner);
    void onObjectAdded(nap::rtti::RTTIObject& obj);
    void onObjectRemoved(nap::rtti::RTTIObject& obj);

    void onNewFile();
    void onFileOpened(const QString& filename);
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    ObjectItem* findItem(const nap::rtti::RTTIObject& obj);

    std::vector<rttr::instance> selectedInstances() const;

    void menuHook(QMenu& menu);
private:
    QVBoxLayout mLayout;
    OutlineModel mModel;
    FilterTreeView mTreeView;
};
