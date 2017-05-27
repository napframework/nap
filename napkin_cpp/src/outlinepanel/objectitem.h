#pragma once

#include <QStandardItem>
#include <nap/component.h>
#include <nap/entity.h>
#include <src/appcontext.h>
#include <src/commands.h>

class ObjectItem : public QStandardItem
{
public:
	ObjectItem(nap::Object& object);
	int row(nap::Object& e);
	nap::Object& object() { return mObject; }

    void setData(const QVariant& value, int role) override;

private:
	void onNameChanged(const std::string& newName);
	nap::Slot<const std::string&> onNameChangedSlot;

	void onChildNodeAdded(nap::Object& obj);
	nap::Slot<nap::Object&> onChildNodeAddedSlot;

	void onChildNodeRemoved(nap::Object& obj);
	nap::Slot<nap::Object&> onChildNodeRemovedSlot;

	nap::Object& mObject;
};
