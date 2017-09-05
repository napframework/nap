#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include <generic/comboboxdelegate.h>
#include "proppanel.h"
#include "napgeneric.h"
#include "typeconversion.h"
#include "globals.h"

using namespace nap::rtti;
using namespace napkin;


QVariant PropertyValueItem::data(int role) const {

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        auto rttiObject = dynamic_cast<InspectorModel*>(model())->object();
        nap::rtti::ResolvedRTTIPath resolvedPath;
        assert(mPath.resolve(rttiObject, resolvedPath));
        QVariant variant;
        if (toQVariant(resolvedPath.getType(), resolvedPath.getValue(), variant))
            return variant;

        return TXT_UNCONVERTIBLE_TYPE;
    }
    return QStandardItem::data(role);
}

void PropertyValueItem::setData(const QVariant& value, int role) {

    if (role == Qt::EditRole) {
        auto rttiObject = dynamic_cast<InspectorModel*>(model())->object();
        nap::rtti::ResolvedRTTIPath resolvedPath;
        assert(mPath.resolve(rttiObject, resolvedPath));
        bool ok;
        auto resultValue = fromQVariant(resolvedPath.getType(), value, &ok);
        if (ok)
            resolvedPath.setValue(resultValue);
    }
    QStandardItem::setData(value, role);
}


void InspectorModel::setObject(RTTIObject* object) {
    while (rowCount() > 0)
        removeRow(0);

    mObject = object;

    if (nullptr != mObject)
        populateItems();
}

void InspectorModel::populateItems() {
    for (auto prop : mObject->get_type().get_properties()) {
        if (!prop.is_valid()
            || prop.is_static()
            || prop.get_name() == PROP_CHILDLREN
            || prop.get_name() == PROP_COMPONENTS)
            continue;

        std::string name = prop.get_name().data();
        QString qName = QString::fromStdString(name);

        nap::rtti::RTTIPath path;
        path.pushAttribute(name);

        QList<QStandardItem*> items;

        auto value = prop.get_value(mObject);
        auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

        if (wrappedType.is_array()) {
            auto array = value.create_array_view();
            items << new ArrayPropertyItem(qName, mObject, path, prop, value.create_array_view());
            items << new EmptyItem();
            items << new TypeItem(value.get_type());
        } else if (wrappedType.is_associative_container()) {
            assert(false);
        } else if (wrappedType.is_pointer()) {
            if (nap::rtti::hasFlag(prop, nap::rtti::EPropertyMetaData::Embedded)) {
                items << new InvalidItem(qName + "[Embedded Pointer]");
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            } else {
                items << new InvalidItem(qName + "[Pointer]");
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            }
        } else if (nap::rtti::isPrimitive(wrappedType)) {
            items << new PropertyItem(qName, mObject, path);
            items << new PropertyValueItem(qName, mObject, path);
            items << new TypeItem(prop.get_type());
        } else {
            items << new CompoundPropertyItem(qName, mObject, path);
            items << new EmptyItem();
            items << new TypeItem(prop.get_type());
        }

        appendRow(items);
    }
}

InspectorModel::InspectorModel() : QStandardItemModel() {
    setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_VALUE, TXT_LABEL_TYPE});
}


InspectorPanel::InspectorPanel() {
    setLayout(&mLayout);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
    mTreeView.tree().setColumnWidth(0, 250);
    mTreeView.tree().setColumnWidth(1, 250);
    mTreeView.tree().setItemDelegateForColumn(1, &mCustomDelegate);
}

void InspectorPanel::setObject(RTTIObject* objects) {
    mModel.setObject(objects);
    mTreeView.tree().expandAll();
}


void ArrayPropertyItem::populateChildren() {
    auto array = mArray;
//    auto array = resolvePath().getValue().create_array_view();

    for (int i = 0; i < array.get_size(); i++) {
        QList<QStandardItem*> items;

        auto name = QString("[%1]").arg(i);

        nap::rtti::RTTIPath path = mPath;
        nap::rtti::ResolvedRTTIPath p;
        path.pushArrayElement(i);

        auto value = array.get_value(i);
        auto type = array.get_rank_type(array.get_rank());
        auto wrappedType = type.is_wrapper() ? type.get_wrapped_type() : value.get_type();

        if (wrappedType.is_array()) {
            items << new InvalidItem(name + "[Array]");
            items << new EmptyItem();
            items << new TypeItem(wrappedType);
        } else if (wrappedType.is_associative_container()) {
            assert(false);
        } else if (wrappedType.is_pointer()) {
            if (nap::rtti::hasFlag(mProperty, nap::rtti::EPropertyMetaData::Embedded)) {
                items << new InvalidItem(name + "[Embedded Pointer]");
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            } else {
                items << new InvalidItem(name + "[Pointer]");
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            }

            items << new InvalidItem(name + "[Pointer]");
            items << new EmptyItem();
            items << new TypeItem(wrappedType);
        } else if (nap::rtti::isPrimitive(wrappedType)) {
            items << new PropertyItem(name, mObject, path);
            items << new PropertyValueItem(name, mObject, path);
            items << new TypeItem(wrappedType);
        } else {
            items << new CompoundPropertyItem(name, mObject, path);
            items << new EmptyItem();
            items << new TypeItem(type);
        }


        appendRow(items);
    }
}


PointerValueItem::PointerValueItem(rttr::variant value) : QStandardItem() {

    auto value_type = value.get_type();
    auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
    bool is_wrapper = wrapped_type != value_type;
    RTTIObject* pointee = is_wrapper ? value.extract_wrapped_value().get_value<RTTIObject*>()
                                     : value.get_value<RTTIObject*>();


    if (nullptr == pointee) {
        setText("NULL");
    } else {
        setText(QString::fromStdString(pointee->mID));
    }

    setForeground(Qt::darkCyan);
//    setIcon(QIcon(":/icons/link.svg"));
    setEditable(false);
}

void CompoundPropertyItem::populateChildren() {
    auto resolved = resolvePath();
    auto compound = resolved.getValue();

    for (auto childprop : compound.get_type().get_properties()) {
        QList<QStandardItem*> items;


        auto value = childprop.get_value(compound);
        std::string name = childprop.get_name().data();
        QString qName = QString::fromStdString(name);

        nap::rtti::RTTIPath path = mPath;
        path.pushAttribute(name);

        auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

        if (wrappedType.is_array()) {
            items << new InvalidItem(qName + "[Array]");
            items << new EmptyItem();
            items << new TypeItem(wrappedType);
        } else if (wrappedType.is_associative_container()) { assert(false); }
        else if (wrappedType.is_pointer()) {
            items << new InvalidItem(qName + "[Pointer]");
            items << new EmptyItem();
            items << new TypeItem(wrappedType);
        } else if (nap::rtti::isPrimitive(wrappedType)) {
            items << new PropertyItem(qName, mObject, path);
            items << new PropertyValueItem(qName, mObject, path);
            items << new TypeItem(wrappedType);
        } else {
            items << new CompoundPropertyItem(qName, mObject, path);
            items << new EmptyItem();
            items << new TypeItem(childprop.get_type());
        }

        appendRow(items);
    }
}
