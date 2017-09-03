#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include <generic/comboboxdelegate.h>
#include "proppanel.h"
#include "napgeneric.h"
#include "typeconversion.h"
#include "globals.h"

using namespace nap;
using namespace nap::rtti;
using namespace napkin;

QString toString(rttr::property prop, rttr::instance inst) {
    return TypeConverter::toQVariant(prop, inst).toString();
}


QList<QStandardItem*> createItemRow(const QString& name, rttr::property prop, rttr::instance inst) {
    // Select item based on type
    auto value = prop.get_value(inst);
    auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

    if (wrappedType.is_array())
        return {new ArrayPropertyItem(prop.get_name().data(), value.create_array_view()),
                new EmptyItem(),
                new TypeItem(value.get_type())};

    if (wrappedType.is_associative_container())
        assert(false);

    if (wrappedType.is_pointer()) {
        return {new PointerItem(prop.get_name().data()),
                new PointerValueItem(prop.get_value(inst)),
                new TypeItem(prop.get_type())};
    }

    if (rtti::isPrimitive(wrappedType))
        return {new PropertyItem(prop, inst),
                new PropertyValueItem(prop, inst),
                new TypeItem(prop.get_type())};

    return {new CompoundPropertyItem(prop.get_name().data(), value),
            new EmptyItem(),
            new TypeItem(prop.get_type())};

}

QList<QStandardItem*> createArrayElementRow(rttr::variant_array_view mArray, int i) {
    auto name = QString("[%1]").arg(i);

    auto value = mArray.get_value(i);
    auto type = mArray.get_rank_type(1);
    auto wrappedType = type.is_wrapper() ? type.get_wrapped_type() : value.get_type();


    if (wrappedType.is_array())
        return {new ArrayPropertyItem(name, value.create_array_view()),
                new EmptyItem(),
                new TypeItem(type)};

    if (wrappedType.is_associative_container())
        assert(false);

    if (wrappedType.is_pointer())
        return {
                new PointerItem(name),
                new PointerValueItem(value),
                new TypeItem(type)};

    if (nap::rtti::isPrimitive(wrappedType))
        return {
                new QStandardItem(name),
                new EmptyItem(),
                new TypeItem(type)};

    return {new CompoundPropertyItem(name, value),
            new EmptyItem(),
            new TypeItem(type)};

}

//
PropertyValueItem::PropertyValueItem(rttr::property prop, nap::rtti::Instance inst)
        : QStandardItem(), mInstance(inst), mProperty(prop) {

    nap::Logger::info("%s: %s <%s>",
                      mProperty.get_name().data(),
                      TypeConverter::toQVariant(mProperty, mInstance).toString().toStdString().c_str(),
                      mProperty.get_type().get_name().data());
    /* Result:
        LOG[info] mMaxLodLevel: 20 <int>
        LOG[info] mInternalFormat: 6402 <int>
        LOG[info] mWidth: 640 <int>
        LOG[info] mHeight: 480 <int>
        LOG[info] mFormat: 6402 <unsigned int>
    */
    if (!TypeConverter::get(mProperty.get_type())) {
        setEnabled(false);
        setForeground(Qt::red);
        setText(TXT_UNCONVERTIBLE_TYPE);
    }
    mName = mProperty.get_name().data();
}

// Qt Retrieves data for display in treeview cell
QVariant PropertyValueItem::data(int role) const {

    nap::Logger::info("%s: %s <%s>",
                      mProperty.get_name().data(),
                      TypeConverter::toQVariant(mProperty, mInstance).toString().toStdString().c_str(),
                      mProperty.get_type().get_name().data());

    /* Result:
        LOG[info] mMaxLodLevel: 24 <int>
        LOG[info] mInternalFormat: 17814496 <int>
        LOG[info] mWidth: 0 <int>
        LOG[info] mHeight: 1249506750 <int>
        LOG[info] mFormat: 6402 <unsigned int>
        LOG[info] mType: 17725216 <unsigned int>
    */

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        QVariant variant = TypeConverter::toQVariant(mProperty, mInstance);
        std::string variantValue = variant.toString().toStdString();
        return variant;
    }
    return QStandardItem::data(role);
}

void PropertyValueItem::setData(const QVariant& value, int role) {
    auto type = mProperty.get_type();
    QStandardItem::setData(value, role);

    if (role == Qt::EditRole) {
        auto converter = TypeConverter::get(type);

        if (converter)
            converter->fromVariant(mProperty, mInstance, value);
    }
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
            || prop.get_name() == PROP_CHILDLREN
            || prop.get_name() == PROP_COMPONENTS)
            continue;

        appendRow(createItemRow(prop.get_name().data(), prop, instance));
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

void InspectorPanel::setObjects(QList<RTTIObject*>& objects) {
    mModel.setObjects(objects);
    mTreeView.tree().expandAll();
}

void ArrayPropertyItem::processChildren() {
    for (int i = 0; i < mArray.get_size(); i++)
        appendRow(createArrayElementRow(mArray, i));
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
