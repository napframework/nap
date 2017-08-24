
#include "attributemodel.h"
#include "attributeitem.h"
#include <nap/object.h>


void AttributeModel::setObject(nap::AttributeObject* object)
{

//	if (mObject) {
//		mObject->childAdded.disconnect(onAttributeAddedSlot);
//		mObject->childRemoved.disconnect(onAttributeRemovedSlot);
//	}


	if (object) {
		object->removed.connect([&](const nap::Object& obj) {
			setObject(nullptr);
		});
	}

	mObject = object;
	clear();
	if (!mObject) return;

	for (nap::AttributeBase* attrib : mObject->getAttributes())
		onAttributeAdded(*attrib);

	mObject->childAdded.connect(onAttributeAddedSlot);
	mObject->childRemoved.connect(onAttributeRemovedSlot);


    QStringList headers;
    headers << "Name";
    headers << "Value";
    headers << "Type";

    setHorizontalHeaderLabels(headers);
}


void AttributeModel::onAttributeRemoved(nap::Object& obj)
{
	if (!obj.getTypeInfo().isKindOf<nap::AttributeBase>()) return;

	nap::AttributeBase& attrib = static_cast<nap::AttributeBase&>(obj);
	int r = row(attrib);
	assert(r != -1);
	removeRow(r);
}


void AttributeModel::onAttributeAdded(nap::Object& obj)
{
	if (!obj.getTypeInfo().isKindOf<nap::AttributeBase>()) return;

	nap::AttributeBase& attrib = static_cast<nap::AttributeBase&>(obj);

	QList<QStandardItem*> items;
	items << new AttributeNameItem(attrib);
	items << new AttributeValueItem(attrib);
	items << new AttributeTypeItem(attrib);
	appendRow(items);
}


int AttributeModel::row(nap::Object& attr)
{
	for (int i = 0; i < rowCount(); i++) {
		AttributeItem* attribItem = (AttributeItem*)item(i);
		if (!attribItem) continue;
        nap::AttributeBase* a = (nap::AttributeBase *) &attr;
        nap::AttributeBase* b = &attribItem->attribute();
		if (a == b) return i;
	}
	return -1;
}
