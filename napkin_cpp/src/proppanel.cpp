#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include "proppanel.h"
#include "napgeneric.h"
#include "typeconversion.h"

using namespace nap;
using namespace nap::rtti;


PropertyNameItem::PropertyNameItem(rttr::property prop) : QStandardItem(), mProperty(prop) {
    setText(prop.get_name().data());
    setEditable(false);
    setForeground(softForeground());
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
        return QString("<<UNKNOWN>>");
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


        auto nameItem = new PropertyNameItem(prop);
        auto valueItem = new PropertyValueItem(instance, prop);
        auto typeItem = new TypeItem(prop.get_type());
        appendRow({nameItem, valueItem, typeItem});
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
}
