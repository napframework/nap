#pragma once

#include <QWidget>
#include <QList>
#include <QStandardItemModel>
#include <rtti/rttiobject.h>
#include <generic/filtertreeview.h>
#include <nap/logger.h>
#include <generic/customdelegate.h>
#include <nap/objectptr.h>
#include <rtti/rttipath.h>
#include "generic/utility.h"
#include "napgeneric.h"


QList<QStandardItem*> createItemRow(const QString& name, rttr::property prop, rttr::instance inst);

class EmptyItem : public QStandardItem {
public:
    EmptyItem() : QStandardItem() {
        setEditable(false);
    }
};

class InvalidItem : public QStandardItem {
public:
    InvalidItem(const QString& name) : QStandardItem(name) {
        setForeground(Qt::red);
        setEditable(false);
    }
};


class BaseItem : public QStandardItem {
public:
    BaseItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path)
            : QStandardItem(name), mObject(object), mPath(path) {
        assert(mObject);
    }

protected:
    nap::rtti::ResolvedRTTIPath resolvePath() {
        nap::rtti::ResolvedRTTIPath resolvedPath;
        assert(mPath.resolve(mObject, resolvedPath));
        return resolvedPath;
    }

    nap::rtti::RTTIObject* mObject;
    const nap::rtti::RTTIPath mPath;

};

/**
 * This item shows the name of an object's property
 */
class PropertyItem : public BaseItem {
public:
    PropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path)
            : BaseItem(name, object, path) {
        setEditable(false);
        setForeground(softForeground());
    }

};

/**
 * The property is has child properties
 */
class CompoundPropertyItem : public BaseItem {
public:
    CompoundPropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path)
            : BaseItem(name, object, path) {
        setForeground(softForeground());
        populateChildren();
    }

private:
    void populateChildren();

};

/**
 * The property is an editable list of child properties
 */
class ArrayPropertyItem : public BaseItem {
public:
    ArrayPropertyItem(const QString& name, nap::rtti::RTTIObject* object,
                      const nap::rtti::RTTIPath& path, rttr::property prop, rttr::variant_array_view array)
            : BaseItem(name, object, path), mProperty(prop), mArray(array) {
        populateChildren();
        setForeground(softForeground());
    }

private:
    void populateChildren();

    rttr::property mProperty;
    rttr::variant_array_view mArray;
};

class PointerItem : public BaseItem {
public:
    PointerItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path)
            : BaseItem(name, object, path) {}

private:
};

class PointerValueItem : public QStandardItem {
public:
    PointerValueItem(rttr::variant value);

};

/**
 * This item displays the value of an object property and allows the user to change it
 */
class PropertyValueItem : public BaseItem {
public:
    PropertyValueItem(const QString& name, nap::rtti::RTTIObject* object, nap::rtti::RTTIPath path)
            : BaseItem(name, object, path) {
    }

    QVariant data(int role) const override;

    void setData(const QVariant& value, int role) override;

};

/**
 * Data model backing the inspector panel tree view
 */
class InspectorModel : public QStandardItemModel {
public:
    InspectorModel();

    void setObject(nap::rtti::RTTIObject* object);

    nap::rtti::RTTIObject* object() { return mObject; }

private:
    void populateItems();

    nap::rtti::RTTIObject* mObject = nullptr;
};

/**
 * The inspector panel allows for inspection and changing of object properties using a tree view
 */
class InspectorPanel : public QWidget {
Q_OBJECT
public:
    InspectorPanel();

    void setObject(nap::rtti::RTTIObject* object);


private:
    InspectorModel mModel;
    FilterTreeView mTreeView;
    QVBoxLayout mLayout;
    CustomDelegate mCustomDelegate;
};