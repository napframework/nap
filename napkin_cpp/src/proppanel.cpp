#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include "proppanel.h"
#include "napgeneric.h"
#include "typeconversion.h"

using namespace nap;
using namespace nap::rtti;



QList<QStandardItem*> createItemRow(rttr::property prop, rttr::instance inst) {
    // Select item based on type
    auto value = prop.get_value(inst);
    auto value_type = value.get_type();
    auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
    bool is_wrapper = wrapped_type != value_type;

    QStandardItem* nameItem = nullptr;

    if (wrapped_type.is_array()) {
        nameItem = new ArrayPropertyItem(prop, inst);
    } else if (wrapped_type.is_associative_container()) {
        assert(false);
    } else if (wrapped_type.is_pointer()) {
        nameItem = new PropertyItem(prop, inst);
    } else if (rtti::isPrimitive(wrapped_type)) {
        nameItem = new PropertyItem(prop, inst);
    } else {
        nameItem = new CompoundPropertyItem(prop, inst);
    }

    auto valueItem = new PropertyValueItem(inst, prop);
    auto typeItem = new TypeItem(prop.get_type());
    return {nameItem, valueItem, typeItem};
}

PropertyItem::PropertyItem(rttr::property prop, rttr::instance inst) : QStandardItem(), mProperty(prop), mInstance(inst) {
    setText(prop.get_name().data());
    setEditable(false);
    setForeground(softForeground());
}

CompoundPropertyItem::CompoundPropertyItem(rttr::property prop, rttr::instance inst) : PropertyItem(prop, inst) {
    if (!TypeConverter::get(mProperty.get_type()))
        processChildren();
}

void CompoundPropertyItem::processChildren() {
    for (auto childprop : mProperty.get_type().get_properties()) {
        appendRow(createItemRow(childprop, mProperty));
    }
}

ArrayPropertyItem::ArrayPropertyItem(rttr::property prop, rttr::instance inst) : PropertyItem(prop, inst) {
    processChildren();
}

void ArrayPropertyItem::processChildren() {
    auto value = mProperty.get_value(mInstance);
    auto array = value.create_array_view();
    for (int i=0; i < array.get_size(); i++) {
        auto elementType = array.get_rank_type(i);
        nap::Logger::info(elementType.get_name().data());

        rttr::variant var = array.get_value(i);
        appendRow(createItemRow(mProperty, var));
    }
}


PropertyValueItem::PropertyValueItem(rttr::instance obj, rttr::property prop)
        : QStandardItem(), mInstance(obj), mProperty(prop) {
    mProperty.get_value(mInstance);

    if (!TypeConverter::get(mProperty.get_type())) {
        setEnabled(false);
        setForeground(Qt::red);
    }

}

QVariant PropertyValueItem::data(int role) const {
    auto type = mProperty.get_type();

    if (role == Qt::DisplayRole) {
        auto converter = TypeConverter::get(type);
        if (converter)
            return converter->toVariant(mProperty, mInstance);
        return QString("");
    }
    return QStandardItem::data(role);
}

void PropertyValueItem::setData(const QVariant& value, int role) {
    auto type = mProperty.get_type();

    if (role == Qt::EditRole) {
        auto converter = TypeConverter::get(type);

        if (converter)
            converter->fromVariant(mProperty, mInstance, value);
    }
    QStandardItem::setData(value, role);
}


void InspectorModel::setObjects(QList<RTTIObject*>& inst) {
    while (rowCount() > 0)
        removeRow(0);


    if (inst.empty())
        return;

    mInstance = inst.first();
    rttr::instance instance(mInstance);

    if (nullptr == mInstance)
        return;

    for (auto prop : mInstance->get_type().get_properties()) {
        if (!prop.is_valid()
            || prop.is_static()
            || prop.get_name() == "Components"
            || prop.get_name() == "Children")
            continue;

        appendRow(createItemRow(prop, instance));
    }
}

InspectorModel::InspectorModel() : QStandardItemModel() {
    setHorizontalHeaderLabels({"Name", "Value", "Type"});
}

InspectorPanel::InspectorPanel() {
    setLayout(&mLayout);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
    mTreeView.tree().setColumnWidth(0, 100);
    mTreeView.tree().setColumnWidth(1, 250);

}

void InspectorPanel::setObjects(QList<RTTIObject*>& objects) {
    mModel.setObjects(objects);
    mTreeView.tree().expandAll();
}

