#pragma once

#include <QStandardItemModel>
#include <nap/attributeobject.h>

class AttributeModel : public QStandardItemModel
{
	Q_OBJECT
public:
	AttributeModel() : QStandardItemModel() {}

	void setObject(nap::AttributeObject* object);

	nap::AttributeObject* object() const { return mObject; }

private:
    int row(nap::Object& attr);

    void onAttributeAdded(nap::Object& obj);
    nap::Slot<nap::Object&> onAttributeAddedSlot = { this, &AttributeModel::onAttributeAdded };

    void onAttributeRemoved(nap::Object& obj);
    nap::Slot<nap::Object&> onAttributeRemovedSlot = { this, &AttributeModel::onAttributeRemoved };

	nap::AttributeObject* mObject = nullptr;
};
