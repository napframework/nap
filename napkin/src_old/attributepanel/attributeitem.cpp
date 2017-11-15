#include "attributeitem.h"

#include "../appcontext.h"
#include "../commands.h"


AttributeNameItem::AttributeNameItem(nap::AttributeBase& attrib)
	: AttributeItem(attrib), onNameChangedSlot(this, &AttributeNameItem::onNameChanged)
{
	onNameChanged(attrib.getName());
    mAttribute.nameChanged.connect(onNameChangedSlot);
}


void AttributeNameItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole) {
		const QString newName = value.toString().trimmed();
		AppContext::get().execute(new SetNameCmd(mAttribute, newName));
	}
	QStandardItem::setData(value, role);
}


void AttributeNameItem::onNameChanged(const std::string& name) { setText(QString::fromStdString(name)); }


AttributeValueItem::AttributeValueItem(nap::AttributeBase& attrib)
	: AttributeItem(attrib), onValueChangedSlot(this, &AttributeValueItem::onValueChanged)
{
    mAttribute.valueChanged.connect(onValueChangedSlot);
    onValueChanged(mAttribute);
}
void AttributeValueItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole) {
        if (attributeToString(mAttribute) == value.toString())
            return;

        AppContext::get().execute(new SetAttributeValueCmd(mAttribute, value.toString()));
		return;
	}

	QStandardItem::setData(value, role);
}

void AttributeValueItem::onValueChanged(nap::AttributeBase& attrib)
{
    setText(attributeToString(mAttribute));
}
