#pragma once

#include <QStandardItem>
#include <nap/attribute.h>
#include <nap/typeconverter.h>

class AttributeItem : public QStandardItem
{
public:
	AttributeItem(nap::AttributeBase& attrib) : QStandardItem(), mAttribute(attrib) {}

	nap::AttributeBase& attribute() const { return mAttribute; }
protected:
	nap::AttributeBase& mAttribute;

private:
};




class AttributeNameItem : public AttributeItem
{
public:
         AttributeNameItem(nap::AttributeBase& attrib);
    void setData(const QVariant& value, int role) override;

private:
	void onNameChanged(const std::string& name);
	nap::Slot<const std::string&> onNameChangedSlot;
};



class AttributeValueItem : public AttributeItem
{
public:
	AttributeValueItem(nap::AttributeBase& attrib);
	~AttributeValueItem() {}
	void setData(const QVariant& value, int role) override;

private:
	void onValueChanged(nap::AttributeBase& attrib);
	nap::Slot<nap::AttributeBase&> onValueChangedSlot;

};



class AttributeTypeItem : public AttributeItem
{
public:
	AttributeTypeItem(nap::AttributeBase& attrib) : AttributeItem(attrib)
	{
		setText(QString::fromStdString(attrib.getValueType().getName()));
		setEditable(false);
	}
};