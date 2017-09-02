#pragma once

#include <QWidget>
#include <QList>
#include <QStandardItemModel>
#include <rtti/rttiobject.h>
#include <generic/filtertreeview.h>
#include <nap/logger.h>

/**
 * This item shows the name of an object's property
 */
class PropertyItem : public QStandardItem {
public:
    PropertyItem(rttr::property prop, rttr::instance inst);

protected:
    rttr::property mProperty;
    rttr::instance mInstance;
};

/**
 * The property is has child properties
 */
class CompoundPropertyItem : public PropertyItem {
public:
    CompoundPropertyItem(rttr::property prop, rttr::instance inst);
    void processChildren();

};

/**
 * The property is an editable list of child properties
 */
class ArrayPropertyItem : public PropertyItem {
public:
    ArrayPropertyItem(rttr::property, rttr::instance inst);
    void processChildren();
};



/**
 * This item displays the value of an object property and allows the user to change it
 */
class PropertyValueItem : public QStandardItem {
public:
    PropertyValueItem(nap::rtti::Instance obj, rttr::property prop);

    QVariant data(int role) const override;

    void setData(const QVariant& value, int role) override;

private:
    nap::rtti::Instance mInstance;
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
};