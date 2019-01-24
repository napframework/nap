#include "standarditemsobject.h"

#include "commands.h"
#include "sceneservice.h"
#include "naputils.h"

napkin::GroupItem::GroupItem(const QString& name) : QStandardItem(name)
{
}

napkin::ObjectItem::ObjectItem(nap::rtti::Object* rttiObject) : mObject(rttiObject)
{
	refresh();
	setText(QString::fromStdString(rttiObject->mID));
    setIcon(AppContext::get().getResourceFactory().getIcon(*rttiObject));
}

void napkin::ObjectItem::refresh()
{
	QStandardItem::setText(getName());
}

const QString napkin::ObjectItem::getName() const
{
	return QString::fromStdString(mObject->mID);
}

nap::rtti::Object* napkin::ObjectItem::getObject() const
{
	return mObject;
}

void napkin::ObjectItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole)
	{
		PropertyPath prop_path(*mObject, nap::rtti::sIDPropertyName);

		// Ensure filename exists
		if (nap::rtti::hasFlag(prop_path.getProperty(), nap::rtti::EPropertyMetaData::FileLink))
		{
			auto filename = getAbsoluteResourcePath(value.toString());
			if (!QFileInfo::exists(filename))
			{
				nap::Logger::fatal("File not found: %s", filename.toStdString().c_str());
				return;
			}
		}

		AppContext::get().executeCommand(new SetValueCommand(prop_path, value.toString()));
		return;
	}
	QStandardItem::setData(value, role);
}

napkin::EntityItem::EntityItem(nap::Entity& entity) : ObjectItem(&entity)
{

	for (auto& child : entity.mChildren)
	{
		appendRow({new EntityItem(*child), new napkin::RTTITypeItem(child->get_type())});
	}

	for (auto& comp : entity.mComponents)
	{
		auto compItem	 = new ComponentItem(*comp);
		auto compTypeItem = new napkin::RTTITypeItem(comp->get_type());
		appendRow({compItem, compTypeItem});
	}
}

nap::Entity* napkin::EntityItem::getEntity()
{
	return rtti_cast<nap::Entity>(mObject);
}

napkin::ComponentItem::ComponentItem(nap::Component& comp) : ObjectItem(&comp)
{
}

nap::Component& napkin::ComponentItem::getComponent()
{
	auto& o = *rtti_cast<nap::Component*>(mObject);
	return *o;
}

napkin::SceneItem::SceneItem(nap::Scene& scene) : ObjectItem(&scene)
{
	for (auto& entity : scene.getEntityResourcesRef())
		appendRow(new RootEntityItem(entity));
}

napkin::EntityInstanceItem::EntityInstanceItem(nap::Entity& e, RootEntityItem& rootEntityItem)
	: ObjectItem(&e), mRootEntityItem(rootEntityItem)
{
	for (auto comp : e.mComponents)
		appendRow(new ComponentInstanceItem(*comp, rootEntityItem));
	for (auto entity : e.mChildren)
		appendRow(new EntityInstanceItem(*entity, rootEntityItem));
}

nap::RootEntity& napkin::EntityInstanceItem::rootEntity()
{
	return mRootEntityItem.rootEntity();
}

napkin::RootEntityItem::RootEntityItem(nap::RootEntity& e)
	: ObjectItem(e.mEntity.get()), mRootEntity(&e)
{
	for (auto comp : e.mEntity->mComponents)
		appendRow(new ComponentInstanceItem(*comp, *this));
	for (auto entity : e.mEntity->mChildren)
		appendRow(new EntityInstanceItem(*entity, *this));
}

nap::RootEntity& napkin::RootEntityItem::rootEntity()
{
	assert(mRootEntity);
	return *mRootEntity;
}

napkin::ComponentInstanceItem::ComponentInstanceItem(nap::Component& comp, RootEntityItem& entityItem)
	: ObjectItem(&comp), mEntityItem(entityItem)
{

}

nap::RootEntity& napkin::ComponentInstanceItem::rootEntity()
{
	return mEntityItem.rootEntity();
}

