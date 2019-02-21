#include "standarditemsobject.h"

#include "commands.h"
#include "sceneservice.h"
#include "naputils.h"

using namespace napkin;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GroupItem::GroupItem(const QString& name) : QStandardItem(name)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ObjectItem::ObjectItem(nap::rtti::Object* o, bool isPointer)
		: QObject(), mObject(o), mIsPointer(isPointer)
{
	auto& ctx = AppContext::get();

	setText(QString::fromStdString(o->mID));
	setIcon(ctx.getResourceFactory().getIcon(*o));

	connect(&ctx, &AppContext::propertyValueChanged, this, &ObjectItem::onPropertyValueChanged);
	connect(&ctx, &AppContext::objectRemoved, this, &ObjectItem::onObjectRemoved);

	refresh();
}

const PropertyPath ObjectItem::propertyPath() const
{
	return PropertyPath(*mObject);
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

void ObjectItem::removeChildren()
{
	while (rowCount())
		removeRow(0);
}

void ObjectItem::onPropertyValueChanged(PropertyPath path)
{
	if (&path.getObject() == mObject)
		refresh();
}

void ObjectItem::onObjectRemoved(nap::rtti::Object* o)
{
	if (o == mObject)
	{
		auto parent = parentItem();
		if (parent)
			parent->removeRow(index().row());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EntityItem::EntityItem(nap::Entity& entity, bool isPointer) : ObjectItem(&entity, isPointer)
{

	for (auto& child : entity.mChildren)
		onEntityAdded(child.get(), &entity);

	for (auto& comp : entity.mComponents)
		onComponentAdded(comp.get(), &entity);

	auto& ctx = AppContext::get();
	connect(&ctx, &AppContext::componentAdded, this, &EntityItem::onComponentAdded);
	connect(&ctx, &AppContext::entityAdded, this, &EntityItem::onEntityAdded);
	connect(&ctx, &AppContext::propertyValueChanged, this, &EntityItem::onPropertyValueChanged);
}

nap::Entity* EntityItem::getEntity()
{
	return rtti_cast<nap::Entity>(mObject);
}

int EntityItem::childEntityIndex(EntityItem& childEntityItem)
{
	int i = 0;
	for (int row = 0; row < rowCount(); row++)
	{
		auto childItem = child(row, 0);
		if (childItem == &childEntityItem)
			return i;
		i++;
	}
	return -1;
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

	auto compItem = new ComponentItem(*comp);
	auto compTypeItem = new RTTITypeItem(comp->get_type());
	appendRow({compItem, compTypeItem});
}

void EntityItem::onPropertyValueChanged(const PropertyPath& path)
{
	PropertyPath childrenPath(*getEntity(), "Children");
	assert(childrenPath.isValid());
	if (path != childrenPath)
		return;

	removeChildren();
	for (auto child : getEntity()->mChildren)
		onEntityAdded(child.get(), getEntity());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentItem::ComponentItem(nap::Component& comp) : ObjectItem(&comp, false)
{
}

nap::Component& ComponentItem::getComponent()
{
	auto& o = *rtti_cast<nap::Component*>(mObject);
	return *o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SceneItem::SceneItem(nap::Scene& scene) : ObjectItem(&scene, false)
{
	for (auto& entity : scene.getEntityResourcesRef())
		appendRow(new RootEntityItem(entity));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

QString EntityInstanceItem::instanceName() const
{
	auto parent = parentItem();
	if (parent)
	{
		int i = 0;
		for (int row = 0, len = parent->rowCount(); row < len; row++)
		{
			auto sibling = dynamic_cast<EntityInstanceItem*>(parent->child(row));
			if (!sibling)
				continue;
			if (sibling == this)
				break;
			if (sibling->mObject->mID == entity().mID)
				i++;
		}
		return QString("%1:%2").arg(mObject->mID.c_str(), QString::number(i));
	}

	assert(false);
	return "FAULTY";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

nap::RootEntity& EntityInstanceItem::rootEntity() const
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

const PropertyPath EntityInstanceItem::propertyPath() const
{
	auto ownerItem = dynamic_cast<EntityInstanceItem*>(parentItem());
	if (ownerItem)
	{
		std::vector<std::string> namePath = {mObject->mID};

		auto parent = ownerItem;
		while (parent != nullptr && !dynamic_cast<RootEntityItem*>(parent))
		{
			namePath.insert(namePath.begin(), parent->instanceName().toStdString());
			parent = parent->parentEntityInstanceItem();
		}

		std::string path = "./" + nap::utility::joinString(namePath, "/");
		return PropertyPath(rootEntity(), *mObject, path);
	}

	auto sceneItem = dynamic_cast<SceneItem*>(parentItem());
	if (sceneItem) {
		std::string path = "./" + instanceName().toStdString();
		return { rootEntity(), *mObject, path };
	}
	return PropertyPath(mRootEntityItem.rootEntity(), entity());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RootEntityItem::RootEntityItem(nap::RootEntity& e)
		: EntityInstanceItem(*e.mEntity.get(), *this), mRootEntity(&e)
{
}

const PropertyPath RootEntityItem::propertyPath() const
{
	return EntityInstanceItem::propertyPath();
}

nap::RootEntity& RootEntityItem::rootEntity() const
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentInstanceItem::ComponentInstanceItem(nap::Component& comp, RootEntityItem& entityItem)
		: ObjectItem(&comp, false), mEntityItem(entityItem)
{

}

const PropertyPath ComponentInstanceItem::propertyPath() const
{
	return PropertyPath(rootEntity(), component(), componentPath());
}

nap::Component& ComponentInstanceItem::component() const
{
	return *dynamic_cast<nap::Component*>(mObject);
}

nap::RootEntity& ComponentInstanceItem::rootEntity() const
{
	return mEntityItem.rootEntity();
}

std::string ComponentInstanceItem::componentPath() const
{
	auto ownerItem = dynamic_cast<EntityInstanceItem*>(parentItem());
	if (ownerItem)
	{
		std::vector<std::string> namePath = {mObject->mID};

		auto parent = ownerItem;
		while (parent != nullptr && !dynamic_cast<RootEntityItem*>(parent))
		{
			namePath.insert(namePath.begin(), parent->instanceName().toStdString());
			parent = parent->parentEntityInstanceItem();
		}

		return "./" + nap::utility::joinString(namePath, "/");
	}
	assert(false);
	return {"INVALID"};
}

QVariant ComponentInstanceItem::data(int role) const
{
	if (role == Qt::BackgroundRole)
	{
		if (propertyPath().hasOverriddenChildren())
			return QVariant::fromValue<QColor>(QColor(Qt::yellow).lighter());
	}
	return ObjectItem::data(role);
}


