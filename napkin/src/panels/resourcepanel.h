#pragma once

#include <QStandardItemModel>
#include <nap/resourcemanager.h>
#include "generic/filtertreeview.h"
#include "appcontext.h"
#include "actions.h"
#include "generic/napgeneric.h"

namespace nap { namespace rtti { class RTTIObject; }}

/**
 * An empty item for grouping purposes.
 */
class GroupItem : public QStandardItem {
public:
    explicit GroupItem(const QString& name) : QStandardItem(name)
    {}

    int type() const override
    { return QStandardItem::UserType + 1; }
};

/**
 * An item representing a single nap::rtti::RTTIObject. The item will show the object's name.
 */
class ObjectItem : public QStandardItem {
public:
    explicit ObjectItem(nap::rtti::RTTIObject& o);

    int type() const override
    { return QStandardItem::UserType + 2; }

    void refresh();

    nap::rtti::RTTIObject& object() const
    { return mObject; }

    virtual const QString name() const;

protected:
    nap::rtti::RTTIObject& mObject;
};

/**
 * An item representing an Entity
 */
class EntityItem : public ObjectItem {
public:
    EntityItem(nap::Entity& entity);

    int type() const override
    { return QStandardItem::UserType + 4; }

    nap::Entity& entity()
    { return static_cast<nap::Entity&>(mObject); }
};

/**
 * And item representing a Component
 */
class ComponentItem : public ObjectItem {
public:
    ComponentItem(nap::Component& comp) : ObjectItem(comp)
    {}

    int type() const override
    { return QStandardItem::UserType + 5; }

    nap::Component& component()
    { return static_cast<nap::Component&>(mObject); }
};

/**
 * Model containing full list of resources in the system. Hierarchy is represented where possible.
 * The data is retrieved through AppContext
 */
class ResourceModel : public QStandardItemModel {
public:
    ResourceModel();

    /**
     * Clear all the items from the model and rebuild
     */
    void refresh();
};

/**
 *
 */
class ResourcePanel : public QWidget {
Q_OBJECT
public:
    ResourcePanel();

Q_SIGNALS:
    void selectionChanged(QList<nap::rtti::RTTIObject*>& obj);

private:
    void refresh();

    // Signal handlers
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
    ResourceModel mModel;
    FilterTreeView mTreeView;
};
