#pragma once

#include <QWidget>
#include <QList>
#include <QStandardItemModel>
#include <rtti/rttiobject.h>
#include <generic/filtertreeview.h>
#include <nap/logger.h>
#include <generic/customdelegate.h>
#include <nap/objectptr.h>
#include "generic/utility.h"


QList<QStandardItem*> createItemRow(const QString& name, rttr::property prop, rttr::instance inst);

class EmptyItem : public QStandardItem {
public:
    EmptyItem() : QStandardItem() {
        setEditable(false);
    }
};

/**
 * This item shows the name of an object's property
 */
class PropertyItem : public QStandardItem {
public:
    PropertyItem(rttr::property prop, rttr::instance inst) : QStandardItem(), mProperty(prop), mInstance(inst) {
        setText(prop.get_name().data());
        setEditable(false);
        setForeground(softForeground());
    }

protected:
    rttr::property mProperty;
    rttr::instance mInstance;
};

/**
 * The property is has child properties
 */
class CompoundPropertyItem : public QStandardItem {
public:
    CompoundPropertyItem(const QString& name, rttr::instance inst) : QStandardItem(name), mInstance(inst) {
        std::string nameStr(name.toStdString());
        processChildren();
    }

    void processChildren() {
        for (auto childprop : mInstance.get_type().get_properties()) {
            auto value = childprop.get_value(mInstance);
            std::string name = childprop.get_name().data();
            int valueInt = value.to_int();
            appendRow(createItemRow(name.c_str(), childprop, mInstance));
        }
    }

private:
    rttr::instance mInstance;
};

/**
 * The property is an editable list of child properties
 */
class ArrayPropertyItem : public QStandardItem {
public:
    ArrayPropertyItem(const QString& name, rttr::variant_array_view array) : QStandardItem(name), mArray(array) {
        processChildren();
    }

    void processChildren();

private:
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
    PropertyValueItem(rttr::property prop, nap::rtti::Instance inst);

    QVariant data(int role) const override;

    void setData(const QVariant& value, int role) override;

private:
    std::string mName;
    rttr::instance mInstance;
    const rttr::property mProperty;
};

/**
 * Data model backing the inspector panel tree view
 */
class InspectorModel : public QStandardItemModel {
public:
    InspectorModel();

    void setObjects(QList<nap::rtti::RTTIObject*>& inst);

private:
    nap::rtti::RTTIObject* mInstance = nullptr;
};

/**
 * The inspector panel allows for inspection and changing of object properties using a tree view
 */
class InspectorPanel : public QWidget {
Q_OBJECT
public:
    InspectorPanel();

    void setObjects(QList<nap::rtti::RTTIObject*>& objects);


private:
    InspectorModel mModel;
    FilterTreeView mTreeView;
    QVBoxLayout mLayout;
    CustomDelegate mCustomDelegate;
};