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

/**
 * This item shows the name of an object's property
 */
class PropertyItem : public QStandardItem {
public:
    PropertyItem(const QString& name, nap::rtti::RTTIPath path) : QStandardItem(name), mPath(path) {
        setEditable(false);
        setForeground(softForeground());
    }

protected:
    nap::rtti::RTTIPath mPath;
};

/**
 * The property is has child properties
 */
class CompoundPropertyItem : public QStandardItem {
public:
    CompoundPropertyItem(const QString& name, nap::rtti::RTTIPath path, rttr::variant compound)
            : QStandardItem(name), mPath(path), mCompound(compound) {
        setForeground(softForeground());
        populateChildren();
    }

    void populateChildren();

private:
    nap::rtti::RTTIPath mPath;
    rttr::variant mCompound;

};

/**
 * The property is an editable list of child properties
 */
class ArrayPropertyItem : public QStandardItem {
public:
    ArrayPropertyItem(const QString& name, nap::rtti::RTTIPath& path, rttr::variant_array_view array)
            : QStandardItem(name), mArray(array), mPath(path) {
        populateChildren();
        setForeground(softForeground());
    }


private:
    void populateChildren();

    nap::rtti::RTTIPath mPath;
    rttr::variant_array_view mArray;

};


class PointerItem : public QStandardItem {
public:
    PointerItem(const QString& name) : QStandardItem(name) {}

private:
};

class PointerValueItem : public QStandardItem {
public:
    PointerValueItem(rttr::variant value);

//private:
//    nap::ObjectPtrBase mPointer;
};

/**
 * This item displays the value of an object property and allows the user to change it
 */
class PropertyValueItem : public QStandardItem {
public:
    PropertyValueItem(const QString& name, nap::rtti::RTTIPath path);

    QVariant data(int role) const override;

    void setData(const QVariant& value, int role) override;

private:
    nap::rtti::RTTIPath mRTTIPath;

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