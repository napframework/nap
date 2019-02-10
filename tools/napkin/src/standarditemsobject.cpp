#include "standarditemsobject.h"

#include "commands.h"
#include "sceneservice.h"
#include "naputils.h"

using namespace napkin;

GroupItem::GroupItem(const QString& name) : QStandardItem(name)
{
}

ObjectItem::ObjectItem(nap::rtti::Object* o, bool isPointer)
		: QObject(), mObject(o), mIsPointer(isPointer)
{
	refresh();
	setText(QString::fromStdString(o->mID));
    setIcon(AppContext::get().getResourceFactory().getIcon(*o));

    auto& ctx = AppContext::get();
    connect(&ctx, &AppContext::propertyValueChanged, [this](const PropertyPath& path) {
    	if (&path.getObject() == mObject)
    		refresh();
    });

	connect(&ctx, &AppContext::objectRemoved, this, &ObjectItem::onObjectRemoved);
}

bool ObjectItem::isPointer() const
{
	if (mIsPointer)
		return true;

	auto parentItem = dynamic_cast<ObjectItem*>(QStandardItem::parent());
	return parentItem && parentItem->isPointer();
}

void ObjectItem::refresh()
{
	QStandardItem::setText(getName());
}

const QString ObjectItem::getName() const
{
	return QString::fromStdString(mObject->mID);
}

nap::rtti::Object* ObjectItem::getObject() const
{
	return mObject;
}

void ObjectItem::setData(const QVariant& value, int role)
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

QVariant ObjectItem::data(int role) const
{
	if (role == Qt::ForegroundRole && isPointer())
	{
//		auto bgcol = QStandardItem::data(Qt::Window).value<QColor>();
		auto bgcol = Qt::white;
		auto fgcol = QStandardItem::data(role).value<QColor>();

		QColor color = nap::qt::lerpCol(bgcol, fgcol, 0.5);
		return QVariant::fromValue<QColor>(color);
	}
	return QStandardItem::data(role);
}

void ObjectItem::onObjectRemoved(nap::rtti::Object* o)
{
	for (int row=0, len=rowCount(); row < len; row++)
	{
		auto objectItem = dynamic_cast<ObjectItem*>(child(row));
		assert(objectItem);
		if (objectItem->getObject() == o)
		{
			removeRow(row);
			return;
		}
	}
}

EntityItem::EntityItem(nap::Entity& entity, bool isPointer) : ObjectItem(&entity, isPointer)
{

	for (auto& child : entity.mChildren)
		onEntityAdded(child.get(), &entity);

	for (auto& comp : entity.mComponents)
		onComponentAdded(comp.get(), &entity);

	auto ctx = &AppContext::get();
	connect(ctx, &AppContext::componentAdded, this, &EntityItem::onComponentAdded);
	connect(ctx, &AppContext::entityAdded, this, &EntityItem::onEntityAdded);
}

nap::Entity* EntityItem::getEntity()
{
	return rtti_cast<nap::Entity>(mObject);
}


void EntityItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	if (parent != mObject)
		return;

	appendRow({new EntityItem(*e, true), new RTTITypeItem(e->get_type())});
}

void EntityItem::onComponentAdded(nap::Component* comp, nap::Entity* owner)
{
	if (owner != mObject)
		return;

	auto compItem	 = new ComponentItem(*comp);
	auto compTypeItem = new RTTITypeItem(comp->get_type());
	appendRow({compItem, compTypeItem});
}

ComponentItem::ComponentItem(nap::Component& comp) : ObjectItem(&comp, false)
{
}

nap::Component& ComponentItem::getComponent()
{
	auto& o = *rtti_cast<nap::Component*>(mObject);
	return *o;
}

SceneItem::SceneItem(nap::Scene& scene) : ObjectItem(&scene, false)
{
	for (auto& entity : scene.getEntityResourcesRef())
		appendRow(new RootEntityItem(entity));
}

EntityInstanceItem::EntityInstanceItem(nap::Entity& e, RootEntityItem& rootEntityItem)
	: ObjectItem(&e, false), mRootEntityItem(rootEntityItem)
{
	for (auto comp : e.mComponents)
		onComponentAdded(comp.get(), &entity());
	for (auto childEntity : e.mChildren)
		onEntityAdded(childEntity.get(), &entity());

	auto ctx = &AppContext::get();
	connect(ctx, &AppContext::componentAdded, this, &EntityInstanceItem::onComponentAdded);
	connect(ctx, &AppContext::entityAdded, this, &EntityInstanceItem::onEntityAdded);
}

nap::RootEntity& EntityInstanceItem::rootEntity()
{
	return mRootEntityItem.rootEntity();
}

void EntityInstanceItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	if (parent != &entity())
		return;

	appendRow(new EntityInstanceItem(*e, mRootEntityItem));
}

void EntityInstanceItem::onComponentAdded(nap::Component* c, nap::Entity* owner)
{
	if (owner != &entity())
		return;

	appendRow(new ComponentInstanceItem(*c, mRootEntityItem));
}

RootEntityItem::RootEntityItem(nap::RootEntity& e)
	: ObjectItem(e.mEntity.get(), false), mRootEntity(&e)
{
	for (auto comp : e.mEntity->mComponents)
		onComponentAdded(comp.get(), e.mEntity.get());
	for (auto entity : e.mEntity->mChildren)
		onEntityAdded(entity.get(), e.mEntity.get());

	auto ctx = &AppContext::get();
	connect(ctx, &AppContext::componentAdded, this, &RootEntityItem::onComponentAdded);
	connect(ctx, &AppContext::entityAdded, this, &RootEntityItem::onEntityAdded);
}

nap::RootEntity& RootEntityItem::rootEntity()
{
	assert(mRootEntity);
	return *mRootEntity;
}

void RootEntityItem::onEntityAdded(nap::Entity* e, nap::Entity* parent)
{
	if (parent != mRootEntity->mEntity.get())
		return;

	appendRow(new EntityInstanceItem(*e, *this));
}

void RootEntityItem::onComponentAdded(nap::Component* c, nap::Entity* owner)
{
	if (owner != mRootEntity->mEntity.get())
		return;

	appendRow(new ComponentInstanceItem(*c, *this));
}

ComponentInstanceItem::ComponentInstanceItem(nap::Component& comp, RootEntityItem& entityItem)
	: ObjectItem(&comp, false), mEntityItem(entityItem)
{

}

nap::RootEntity& ComponentInstanceItem::rootEntity()
{
	return mEntityItem.rootEntity();
}

