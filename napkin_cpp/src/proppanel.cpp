#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include <generic/comboboxdelegate.h>
#include "proppanel.h"
#include "napgeneric.h"
#include "typeconversion.h"
#include "globals.h"

using namespace nap::rtti;
using namespace napkin;


QList<QStandardItem*> createItemRow(rttr::type type, const QString& name,
                                    nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path,
                                    rttr::property prop, rttr::variant value) {
    QList<QStandardItem*> items;
    if (type.is_array()) {
        items << new ArrayPropertyItem(name, object, path, prop, value.create_array_view());
        items << new EmptyItem();
        items << new TypeItem(type);
    } else if (type.is_associative_container()) {
        assert(false);
    } else if (type.is_pointer()) {
        if (nap::rtti::hasFlag(prop, nap::rtti::EPropertyMetaData::Embedded)) {
            items << new EmbeddedPointerItem(name, object, path);
            items << new EmptyItem();
            items << new TypeItem(type);
        } else {
            items << new PointerItem(name, object, path);
            items << new PointerValueItem(object, path);
            items << new TypeItem(type);
        }
    } else if (nap::rtti::isPrimitive(type)) {
        items << new PropertyItem(name, object, path);
        items << new PropertyValueItem(name, object, path);
        items << new TypeItem(type);
    } else {
        items << new CompoundPropertyItem(name, object, path);
        items << new EmptyItem();
        items << new TypeItem(prop.get_type());
    }
    return items;
}

QVariant PropertyValueItem::data(int role) const {

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        nap::rtti::ResolvedRTTIPath resolvedPath;
        nap::Logger::info("Resolving: %s/%s", mObject->mID.c_str(), mPath.toString().c_str());
        assert(mPath.resolve(mObject, resolvedPath));
        QVariant variant;
        if (toQVariant(resolvedPath.getType(), resolvedPath.getValue(), variant))
            return variant;

        return TXT_UNCONVERTIBLE_TYPE;
    }
    return QStandardItem::data(role);
}

void PropertyValueItem::setData(const QVariant& value, int role) {

    if (role == Qt::EditRole) {
        nap::rtti::ResolvedRTTIPath resolvedPath;
        assert(mPath.resolve(mObject, resolvedPath));
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
                items << new EmbeddedPointerItem(qName, mObject, path);
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            } else {
                items << new PointerItem(qName, mObject, path);
                items << new PointerValueItem(mObject, path);
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

        auto name = QString("%1").arg(i);

        nap::rtti::RTTIPath path = mPath;
        path.pushArrayElement(i);

        auto value = array.get_value(i);
        auto type = array.get_rank_type(array.get_rank());
        auto wrappedType = type.is_wrapper() ? type.get_wrapped_type() : value.get_type();

        if (wrappedType.is_array()) {
            items << new ArrayPropertyItem(name, mObject, path, mProperty, value.create_array_view());
            items << new EmptyItem();
            items << new TypeItem(wrappedType);
        } else if (wrappedType.is_associative_container()) {
            assert(false);
        } else if (wrappedType.is_pointer()) {
            if (nap::rtti::hasFlag(mProperty, nap::rtti::EPropertyMetaData::Embedded)) {
                items << new EmbeddedPointerItem(name, mObject, path);
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            } else {
                items << new PointerItem(name, mObject, path);
                items << new PointerValueItem(mObject, path);
                items << new TypeItem(wrappedType);
            }

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
            items << new ArrayPropertyItem(qName, mObject, path, childprop, value.create_array_view());
            items << new EmptyItem();
            items << new TypeItem(wrappedType);
        } else if (wrappedType.is_associative_container()) { assert(false); }
        else if (wrappedType.is_pointer()) {

            if (nap::rtti::hasFlag(childprop, nap::rtti::EPropertyMetaData::Embedded)) {
                items << new EmbeddedPointerItem(qName, mObject, path);
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            } else {
                items << new PointerItem(qName, mObject, path);
                items << new PointerValueItem(mObject, path);
                items << new TypeItem(wrappedType);
            }
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

void EmbeddedPointerItem::populateChildren() {
    // First resolve the pointee, after that behave like compound
    nap::rtti::ResolvedRTTIPath resolvedPath;
    assert(mPath.resolve(mObject, resolvedPath));
    auto value = resolvedPath.getValue();

    auto value_type = value.get_type();
    auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
    bool is_wrapper = wrapped_type != value_type;
    nap::rtti::RTTIObject* pointee = is_wrapper
                                     ? value.extract_wrapped_value().get_value<nap::rtti::RTTIObject*>()
                                     : value.get_value<nap::rtti::RTTIObject*>();
    if (nullptr == pointee) {
        assert(false); // Embedded pointer always has a target?
        return;
    }

    // COMPOUND
    auto object = pointee;
    nap::Logger::info("Embedded pointer object: %s (%s)", object->mID.c_str(), object->get_type().get_name().data());

    for (auto childprop : object->get_type().get_properties()) {
        nap::Logger::info("\tprop: %s", childprop.get_name().data());
        auto childValue = childprop.get_value(object);
        std::string name = childprop.get_name().data();
        QString qName = QString::fromStdString(name);

        nap::rtti::RTTIPath path;
        path.pushAttribute(name);

        auto wrappedType = childValue.get_type().is_wrapper() ?
                           childValue.get_type().get_wrapped_type() : childValue.get_type();

        QList<QStandardItem*> items;
        if (wrappedType.is_array()) {
            items << new ArrayPropertyItem(qName, object, path, childprop, childValue.create_array_view());
            items << new EmptyItem();
            items << new TypeItem(wrappedType);
        } else if (wrappedType.is_associative_container()) {
            assert(false);
        } else if (wrappedType.is_pointer()) {

            if (nap::rtti::hasFlag(childprop, nap::rtti::EPropertyMetaData::Embedded)) {
                items << new EmbeddedPointerItem(qName, object, path);
                items << new EmptyItem();
                items << new TypeItem(wrappedType);
            } else {
                items << new PointerItem(qName, object, path);
                items << new PointerValueItem(object, path);
                items << new TypeItem(wrappedType);
            }
        } else if (nap::rtti::isPrimitive(wrappedType)) {
            items << new PropertyItem(qName, object, path);
            items << new PropertyValueItem(qName, object, path);
            items << new TypeItem(wrappedType);
        } else {
            items << new CompoundPropertyItem(qName, object, path);
            items << new EmptyItem();
            items << new TypeItem(childprop.get_type());
        }
        appendRow(items);
//        appendRow(createItemRow(wrappedType, qName, object, path, childprop, childValue));
    }
}


QVariant PointerValueItem::data(int role) const {
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        nap::rtti::ResolvedRTTIPath resolvedPath;
        assert(mPath.resolve(mObject, resolvedPath));
        auto value = resolvedPath.getValue();

        auto value_type = value.get_type();
        auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
        bool is_wrapper = wrapped_type != value_type;
        nap::rtti::RTTIObject* pointee = is_wrapper
                                         ? value.extract_wrapped_value().get_value<nap::rtti::RTTIObject*>()
                                         : value.get_value<nap::rtti::RTTIObject*>();

        if (nullptr != pointee) {
            return QString::fromStdString(pointee->mID);
        }
        return "NULL";
    }
    return QStandardItem::data(role);
}
