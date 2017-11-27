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
#include "generic/napgeneric.h"
#include "widgetdelegate.h"


QList<QStandardItem*> createItemRow(const QString& name, rttr::property prop, rttr::instance inst);

class EmptyItem : public QStandardItem {
public:
    int type() const override
    { return QStandardItem::UserType + 10; }

    EmptyItem() : QStandardItem()
    {
        setEditable(false);
    }
};

class InvalidItem : public QStandardItem {
public:
    int type() const override
    { return QStandardItem::UserType + 11; }

    InvalidItem(const QString& name) : QStandardItem(name)
    {
        setForeground(Qt::red);
        setEditable(false);
    }
};


class BaseItem : public QStandardItem {
public:
    int type() const override
    { return QStandardItem::UserType + 12; }

    BaseItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path)
            : QStandardItem(name), mObject(object), mPath(path)
    {
        nap::rtti::ResolvedRTTIPath resolved;
        assert(path.resolve(object, resolved));
        assert(mObject);
    }

protected:
    nap::rtti::ResolvedRTTIPath resolvePath()
    {
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
    int type() const override
    { return QStandardItem::UserType + 13; }

    PropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path)
            : BaseItem(name, object, path)
    {
        setEditable(false);
        setForeground(softForeground());
    }

};

/**
 * The property is has child properties
 */
class CompoundPropertyItem : public BaseItem {
public:
    int type() const override
    { return QStandardItem::UserType + 14; }

    CompoundPropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path)
            : BaseItem(name, object, path)
    {
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
    int type() const override
    { return QStandardItem::UserType + 15; }

    ArrayPropertyItem(const QString& name, nap::rtti::RTTIObject* object,
                      const nap::rtti::RTTIPath& path, rttr::property prop, rttr::variant_array_view array)
            : BaseItem(name, object, path), mProperty(prop), mArray(array)
    {
        std::string pathStr = path.toString();
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
    int type() const override
    { return QStandardItem::UserType + 16; }

    PointerItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path)
            : BaseItem(name, object, path)
    {
        setForeground(softForeground());
    }

private:
};


class PointerValueItem : public QStandardItem {
public:
    int type() const override
    { return QStandardItem::UserType + 17; }

    PointerValueItem(nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path, rttr::type valueType)
            : QStandardItem(), mObject(object), mPath(path), mValueType(valueType)
    {
        setForeground(Qt::darkCyan);
        nap::rtti::ResolvedRTTIPath resolved;
        assert(path.resolve(object, resolved));

////    setIcon(QIcon(":/icons/link.svg"));
    }

    QVariant data(int role) const override;

    void setData(const QVariant& value, int role) override
    {
        QStandardItem::setData(value, role);
    }

    rttr::type valueType() { return mValueType; }

private:
    nap::rtti::RTTIObject* mObject;
    nap::rtti::RTTIPath mPath;
    rttr::type mValueType;
};


class EmbeddedPointerItem : public BaseItem {
public:
    int type() const override
    { return QStandardItem::UserType + 18; }

    EmbeddedPointerItem(const QString& name, nap::rtti::RTTIObject* object, nap::rtti::RTTIPath path)
            : BaseItem(name, object, path)
    {
        populateChildren();
    }

private:
    void populateChildren();

};

/**
 * This item displays the value of an object property and allows the user to change it
 */
class PropertyValueItem : public BaseItem {
public:
    int type() const override
    { return QStandardItem::UserType + 19; }

    PropertyValueItem(const QString& name, nap::rtti::RTTIObject* object, nap::rtti::RTTIPath path, rttr::type valueType)
            : BaseItem(name, object, path), mValueType(valueType)
    {
    }

    QVariant data(int role) const override;

    void setData(const QVariant& value, int role) override;

    rttr::type& valueType() { return mValueType; }
private:
    rttr::type mValueType;
};

/**
 * Data model backing the inspector panel tree view
 */
class InspectorModel : public QStandardItemModel {
public:
    InspectorModel();

    void setObject(nap::rtti::RTTIObject* object);

    nap::rtti::RTTIObject* object()
    { return mObject; }

    QVariant data(const QModelIndex& index, int role) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

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
//    CustomDelegate mCustomDelegate;
    PropertyValueItemDelegate mWidgetDelegate;
};