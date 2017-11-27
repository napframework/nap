#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include <generic/comboboxdelegate.h>
#include "inspectorpanel.h"
#include "typeconversion.h"
#include "napkinglobals.h"
#include "commands.h"

using namespace nap::rtti;
using namespace napkin;


QList<QStandardItem*> createItemRow(rttr::type type, const QString& name,
                                    nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path,
                                    rttr::property prop, rttr::variant value)
{
    QList<QStandardItem*> items;
    if (type.is_array()) {
        items << new ArrayPropertyItem(name, object, path, prop, value.create_array_view());
        items << new EmptyItem();
        items << new RTTITypeItem(type);
    } else if (type.is_associative_container()) {
        assert(false);
    } else if (type.is_pointer()) {
        if (nap::rtti::hasFlag(prop, nap::rtti::EPropertyMetaData::Embedded)) {
            items << new EmbeddedPointerItem(name, object, path);
            items << new EmptyItem();
            items << new RTTITypeItem(type);
        } else {
            items << new PointerItem(name, object, path);
            items << new PointerValueItem(object, path, type);
            items << new RTTITypeItem(type);
        }
    } else if (nap::rtti::isPrimitive(type)) {
        items << new PropertyItem(name, object, path);
        items << new PropertyValueItem(name, object, path, type);
        items << new RTTITypeItem(type);
    } else {
        items << new CompoundPropertyItem(name, object, path);
        items << new EmptyItem();
        items << new RTTITypeItem(prop.get_type());
    }
    return items;
}

QVariant PropertyValueItem::data(int role) const
{

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        nap::rtti::ResolvedRTTIPath resolvedPath;
        assert(mPath.resolve(mObject, resolvedPath));
        QVariant variant;
        if (toQVariant(resolvedPath.getType(), resolvedPath.getValue(), variant)) {
            return variant;
        }

        return TXT_UNCONVERTIBLE_TYPE;
    }
    return QStandardItem::data(role);
}

void PropertyValueItem::setData(const QVariant& value, int role)
{
    if (role == Qt::EditRole) {
        auto undoCommand = new SetValueCommand(mObject, mPath, value);
        AppContext::get().undoStack().push(undoCommand);
    }

    if (role == Qt::DisplayRole) {
        nap::rtti::ResolvedRTTIPath resolvedPath;
        assert(mPath.resolve(mObject, resolvedPath));
        bool ok;
        auto resultValue = fromQVariant(resolvedPath.getType(), value, &ok);
        if (ok)
            resolvedPath.setValue(resultValue);
    }
    QStandardItem::setData(value, role);
}


void InspectorModel::setObject(RTTIObject* object)
{
    while (rowCount() > 0)
        removeRow(0);

    mObject = object;

    if (nullptr != mObject)
        populateItems();
}


InspectorModel::InspectorModel() : QStandardItemModel()
{
    setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_VALUE, TXT_LABEL_TYPE});


}


InspectorPanel::InspectorPanel()
{
    setLayout(&mLayout);
    layout()->setContentsMargins(0,0,0,0);
    mLayout.addWidget(&mTreeView);
    mTreeView.setModel(&mModel);
    mTreeView.tree().setColumnWidth(0, 250);
    mTreeView.tree().setColumnWidth(1, 250);
    mTreeView.tree().setItemDelegateForColumn(1, &mWidgetDelegate);
}

void InspectorPanel::setObject(RTTIObject* objects)
{
    mModel.setObject(objects);
    mTreeView.tree().expandAll();
}


void ArrayPropertyItem::populateChildren()
{
    auto array = mArray;

    for (int i = 0; i < array.get_size(); i++) {

        auto name = QString("%1").arg(i);

        nap::rtti::RTTIPath path = mPath;
        path.pushArrayElement(i);

        auto value = array.get_value(i);
        auto type = array.get_rank_type(array.get_rank());
        auto wrappedType = type.is_wrapper() ? type.get_wrapped_type() : value.get_type();

        appendRow(createItemRow(wrappedType, name, mObject, path, mProperty, value));
    }
}

void InspectorModel::populateItems()
{
    for (auto prop : mObject->get_type().get_properties()) {
        if (!prop.is_valid()
            || prop.is_static()
            || prop.get_name() == PROP_CHILDREN
            || prop.get_name() == PROP_COMPONENTS)
            continue;

        std::string name = prop.get_name().data();
        QString qName = QString::fromStdString(name);

        nap::rtti::RTTIPath path;
        path.pushAttribute(name);

        auto value = prop.get_value(mObject);
        auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

        appendRow(createItemRow(wrappedType, qName, mObject, path, prop, value));
    }
}

QVariant InspectorModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::UserRole) {
        auto valueItem = dynamic_cast<PropertyValueItem*>(itemFromIndex(index));
        if (valueItem) {
            return QVariant::fromValue(TypeWrapper(&valueItem->valueType()));
        }
    }
    return QStandardItemModel::data(index, role);
}

bool InspectorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole) {
        auto valueItem = dynamic_cast<PropertyValueItem*>(itemFromIndex(index));
        valueItem->setData(value, Qt::EditRole);
        return true;
    }
    return QStandardItemModel::setData(index, value, role);
}

void CompoundPropertyItem::populateChildren()
{
    auto resolved = resolvePath();
    auto compound = resolved.getValue();

    for (auto childprop : compound.get_type().get_properties()) {
        auto value = childprop.get_value(compound);
        std::string name = childprop.get_name().data();
        QString qName = QString::fromStdString(name);

        nap::rtti::RTTIPath path = mPath;
        path.pushAttribute(name);
        auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

        appendRow(createItemRow(wrappedType, qName, mObject, path, childprop, value));
    }
}

void EmbeddedPointerItem::populateChildren()
{
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


    auto object = pointee;

    for (auto childprop : object->get_type().get_properties()) {
        auto childValue = childprop.get_value(object);
        std::string name = childprop.get_name().data();
        QString qName = QString::fromStdString(name);

        nap::rtti::RTTIPath path;
        path.pushAttribute(name);

        auto wrappedType = childValue.get_type().is_wrapper() ?
                           childValue.get_type().get_wrapped_type() : childValue.get_type();


        appendRow(createItemRow(wrappedType, qName, object, path, childprop, childValue));
    }
}


QVariant PointerValueItem::data(int role) const
{
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

        if (nullptr != pointee)
            return QString::fromStdString(pointee->mID);
        else
            return "NULL";
    }
    return QStandardItem::data(role);
}
