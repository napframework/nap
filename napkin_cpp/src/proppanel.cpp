#include "proppanel.h"
#include "napgeneric.h"

using namespace nap;
using namespace nap::rtti;


PropertyNameItem::PropertyNameItem(rttr::property prop) : QStandardItem(), mProperty(prop) {
    setText(prop.get_name().data());
}

PropertyValueItem::PropertyValueItem(RTTIObject& obj, rttr::property prop)
        : QStandardItem(), mInstance(obj), mProperty(prop) {

}

QVariant PropertyValueItem::data(int role) const {
    auto type = mProperty.get_type();

    if (role == Qt::DisplayRole) {
        auto value = mProperty.get_value(mInstance);

        if (type.is_arithmetic()) {
            if (type == TypeInfo::get<bool>())
                return value.to_bool();

            if (type == TypeInfo::get<int>())
                return value.to_int();

            if (type == TypeInfo::get<float>())
                return value.to_float();

        } else if (type == TypeInfo::get<std::string>()) {
            return QString::fromStdString(value.to_string());
        }

        return QString("<<UNKNOWN>>");
    }
    return QStandardItem::data(role);
}

void PropertyValueItem::setData(const QVariant& value, int role) {
    auto type = mProperty.get_type();
    bool success = false;
    if (role == Qt::EditRole) {

        if (type.is_arithmetic()) {
            if (type == TypeInfo::get<bool>()) {
                mProperty.set_value(mInstance, value.toBool());

            } else if (type == TypeInfo::get<int>()) {
                int v = value.toInt(&success);
                if (success)
                    mProperty.set_value(mInstance, v);

            } else if (type == TypeInfo::get<float>()) {
                float v = value.toFloat(&success);
                if (success)
                    mProperty.set_value(mInstance, v);
            }
        } else if (type == TypeInfo::get<std::string>()) {
            mProperty.set_value(mInstance, value.toString().toStdString());
        }

    }
    QStandardItem::setData(value, role);
}


void InspectorModel::setObjects(QList<RTTIObject*>& inst) {
    while (rowCount() > 0)
        removeRow(0);


    if (inst.empty())
        return;

    mInstance = inst.first();

    if (nullptr == mInstance)
        return;

    for (auto prop : mInstance->get_type().get_properties()) {
        if (!prop.is_valid()
            || prop.is_static()
            || prop.get_name() == "Components"
            || prop.get_name() == "Children")
            continue;

        auto nameItem = new PropertyNameItem(prop);
        auto valueItem = new PropertyValueItem(*mInstance, prop);
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
