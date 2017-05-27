#include "objectitem.h"
#include <nap/object.h>

ObjectItem::ObjectItem(nap::Object& object)
	: QStandardItem(), mObject(object), onNameChangedSlot(this, &ObjectItem::onNameChanged),
	  onChildNodeAddedSlot(this, &ObjectItem::onChildNodeAdded),
	  onChildNodeRemovedSlot(this, &ObjectItem::onChildNodeRemoved)
{
	setText(QString::fromStdString(object.getName()));
	const QIcon* icon = AppContext::get().iconStore().iconFor(object);
	setIcon(*icon);

	object.nameChanged.connect(onNameChangedSlot);
	object.childAdded.connect(onChildNodeAddedSlot);
	object.childRemoved.connect(onChildNodeRemovedSlot);

	for (const auto& child : object.getChildren())
		onChildNodeAdded(*child);
}


void ObjectItem::onNameChanged(const std::string& newName) { setText(QString::fromStdString(newName)); }


void ObjectItem::onChildNodeAdded(nap::Object& obj)
{
//	if (obj.getTypeInfo().isKindOf<nap::AttributeBase>()) return;
	appendRow(new ObjectItem(obj));
}


void ObjectItem::onChildNodeRemoved(nap::Object& obj)
{
	int r = row(obj);
	removeRow(r);
}


int ObjectItem::row(nap::Object& e)
{
	for (int row = 0; row < rowCount(); row++) {
		ObjectItem* item = (ObjectItem*)child(row);
		if (&item->mObject == &e) return row;
	}
	return -1;
}


void ObjectItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole) {
		QString newName = value.toString().trimmed();

		if (newName.toStdString() == object().getName()) return;
		AppContext::get().execute(new SetNameCmd(object(), newName));
		return;
	}
	QStandardItem::setData(value, role);
}