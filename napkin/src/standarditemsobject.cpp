#include "standarditemsobject.h"

#include "commands.h"
#include "sceneservice.h"
#include "generic/qtutils.h"
#include "generic/naputils.h"

napkin::GroupItem::GroupItem(const QString& name) : QStandardItem(name)
{
}

napkin::ObjectItem::ObjectItem(nap::rtti::Object* rttiObject) : mObject(rttiObject)
{
	refresh();

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
	setText(QString::fromStdString(scene.mID));
    for (auto entity : scene.getEntityResources())
    {
		appendRow(new EntityInstanceItem(*entity.mEntity));
    }
}

napkin::EntityInstanceItem::EntityInstanceItem(nap::Entity& e) : ObjectItem(&e)
{
	auto name = QString::fromStdString(e.mID);
	if (name.isEmpty())
		name = "Unnamed";

	// TODO: This crashes
	//    auto entityName = QString::fromStdString(e.getEntity()->mID);

	//	name = QString("%1 (%2)").arg(name, entityName);
	setText(name);

//	for (auto e : mEntityInstance.getChildren())
//	{
//		appendRow(new EntityInstanceItem(*e));
//	}
}

