#include <commands.h>
#include "resourcepanel.h"

#include <napqt/filterpopup.h>
#include <napqt/qtutils.h>
#include "naputils.h"


using namespace napkin;


napkin::ResourceModel::ResourceModel()
		: mObjectsItem(TXT_LABEL_RESOURCES, GroupItem::GroupType::Resources),
		  mEntitiesItem(TXT_LABEL_ENTITIES, GroupItem::GroupType::Entities)
{
	setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_TYPE});
	appendRow(&mObjectsItem);
	appendRow(&mEntitiesItem);
}

// TODO: rename
bool shouldObjectBeVisible(const nap::rtti::Object& obj)
{
	auto doc = AppContext::get().getDocument();

	// Exclude components
	if (obj.get_type().is_derived_from<nap::Component>())
		return false;

	// Exclude embeded objects
	if (doc->isPointedToByEmbeddedPointer(obj))
		return false;

	// Non-root entities are culled
	if (obj.get_type().is_derived_from<nap::Entity>())
		if (doc->getParent(*rtti_cast<const nap::Entity>(&obj)))
			return false;

	return true;
}


void napkin::ResourceModel::populate()
{
	auto doc = AppContext::get().getDocument();
	assert(doc != nullptr);
	for (nap::rtti::Object* ob : topLevelObjects(doc->getObjectPointers()))
		addObjectItem(*ob);
}


void napkin::ResourceModel::clear()
{
	mEntitiesItem.removeRows(0, mEntitiesItem.rowCount());
	mObjectsItem.removeRows(0, mObjectsItem.rowCount());
}


ObjectItem* ResourceModel::addObjectItem(nap::rtti::Object& ob)
{
	auto typeItem = new RTTITypeItem(ob.get_type());

	if (ob.get_type().is_derived_from<nap::Entity>())
	{
		auto entityItem = new EntityItem(*rtti_cast<nap::Entity>(&ob));
		mEntitiesItem.appendRow({entityItem, typeItem});

		return entityItem;
	}

	if (!shouldObjectBeVisible(ob))
		return nullptr;

	// ... now the rest in Objects...
	auto item = new ObjectItem(&ob, false);
	mObjectsItem.appendRow({item, typeItem});
	return item;
}

void ResourceModel::removeObjectItem(const nap::rtti::Object& object)
{
	auto item = findItemInModel<napkin::ObjectItem>(*this, object);
	if (item == nullptr)
		return;

	removeRow(item->row(), static_cast<QStandardItem*>(item)->parent()->index());
}

void ResourceModel::removeEmbeddedObjects()
{
	auto doc = AppContext::get().getDocument();

	// First, gather the objects that are pointed to by embedded pointers,
	// we are going to change the model layout
	QList<const nap::rtti::Object*> removeObjects;
	for (int row=0; row < mObjectsItem.rowCount(); row++)
	{
		auto item = dynamic_cast<ObjectItem*>(mObjectsItem.child(row, 0));
		assert(item != nullptr);
		auto obj = item->getObject();
		if (doc->isPointedToByEmbeddedPointer(*obj))
			removeObjects << obj;

	}

	// Now remove
	for (const auto obj : removeObjects)
		removeObjectItem(*obj);
}


napkin::ResourcePanel::ResourcePanel()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.getTreeView().setColumnWidth(0, 300);
	mTreeView.getTreeView().setSortingEnabled(false);

	connect(&AppContext::get(), &AppContext::documentOpened, this, &ResourcePanel::onFileOpened);
	connect(&AppContext::get(), &AppContext::documentClosing, this, &ResourcePanel::onFileClosing);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &ResourcePanel::onNewFile);

	connect(mTreeView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this,
			&ResourcePanel::onSelectionChanged);

	mTreeView.setMenuHook(std::bind(&ResourcePanel::menuHook, this, std::placeholders::_1));
	// connect(&AppContext::get(), &AppContext::dataChanged, this, &ResourcePanel::refresh);
	connect(&AppContext::get(), &AppContext::entityAdded, this, &ResourcePanel::onEntityAdded);
	connect(&AppContext::get(), &AppContext::componentAdded, this, &ResourcePanel::onComponentAdded);
	connect(&AppContext::get(), &AppContext::objectAdded, this, &ResourcePanel::onObjectAdded);
	connect(&AppContext::get(), &AppContext::objectRemoved, this, &ResourcePanel::onObjectRemoved);
	connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &ResourcePanel::onPropertyValueChanged);
}



void napkin::ResourcePanel::menuHook(QMenu& menu)
{
	auto selectedItem = mTreeView.getSelectedItem();
	if (selectedItem == nullptr)
		return;

	auto objItem = dynamic_cast<ObjectItem*>(selectedItem);
	auto entityItem = dynamic_cast<EntityItem*>(selectedItem);

	if (entityItem)
	{
		menu.addAction(new AddChildEntityAction(*entityItem->getEntity()));
		menu.addAction(new AddComponentAction(*entityItem->getEntity()));

		if (entityItem->isPointer())
		{
			auto parentItem = dynamic_cast<EntityItem*>(entityItem->parentItem());
			if (parentItem)
				menu.addAction(new RemovePathAction(entityItem->propertyPath()));

		}

		menu.addAction(new DeleteObjectAction(*objItem->getObject()));
	}
	else if (objItem)
	{
		menu.addAction(new DeleteObjectAction(*objItem->getObject()));
	}

	auto groupItem = dynamic_cast<GroupItem*>(selectedItem);
	if (groupItem)
	{
		if (groupItem->groupType() == GroupItem::GroupType::Entities)
		{
			menu.addAction(new CreateEntityAction());
		}
		else if (groupItem->groupType() == GroupItem::GroupType::Resources)
		{
			// Resources
			menu.addAction(new CreateResourceAction());
		}
	}
}


void napkin::ResourcePanel::onNewFile()
{
	populate();
}


void napkin::ResourcePanel::onFileOpened(const QString& filename)
{
	populate();
}


void napkin::ResourcePanel::onFileClosing(const QString& filename)
{
	clear();
}


void napkin::ResourcePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	emitSelectionChanged();
}


void napkin::ResourcePanel::refresh()
{
	clear();
	populate();
}


void napkin::ResourcePanel::clear()
{
	mTreeView.getTreeView().selectionModel()->clear();
	mModel.clear();
}


void napkin::ResourcePanel::populate()
{
	mModel.populate();
	mTreeView.getTreeView().expandAll();
	emitSelectionChanged();
}


void napkin::ResourcePanel::onEntityAdded(nap::Entity* entity, nap::Entity* parent)
{
	// TODO: Don't refresh the whole mModel
	refresh();
	mTreeView.selectAndReveal(findItemInModel<napkin::ObjectItem>(mModel, *entity));
}

void napkin::ResourcePanel::onComponentAdded(nap::Component* comp, nap::Entity* owner)
{
	// TODO: Don't refresh the whole mModel
	refresh();
	mTreeView.selectAndReveal(findItemInModel<ObjectItem>(mModel, *comp));
}

void napkin::ResourcePanel::onObjectAdded(nap::rtti::Object* obj, bool selectNewObject)
{
	auto item = mModel.addObjectItem(*obj);
	if (selectNewObject)
		mTreeView.selectAndReveal(item);
}

void ResourcePanel::selectObjects(const QList<nap::rtti::Object*>& obj)
{
	if (obj.size() > 0)
		mTreeView.selectAndReveal(findItemInModel<napkin::ObjectItem>(mModel, *obj[0]));
}

void napkin::ResourcePanel::onObjectRemoved(const nap::rtti::Object* object)
{
	mModel.removeObjectItem(*object);
}

void napkin::ResourcePanel::onPropertyValueChanged(const PropertyPath& path)
{
	// Update object name?
	if (path.getProperty().get_name() == nap::rtti::sIDPropertyName)
	{
		auto obj = path.getObject();
		auto objectItem = findItemInModel<napkin::ObjectItem>(mModel, *obj);
		if (objectItem != nullptr)
			objectItem->setText(QString::fromStdString(obj->mID));
	}

	mModel.removeEmbeddedObjects();
}

void ResourcePanel::emitSelectionChanged()
{
	// Grab selected nap objects
	QList<PropertyPath> selectedPaths;
	for (auto m : mTreeView.getSelectedItems())
	{
		auto item = dynamic_cast<ObjectItem*>(m);
		if (!item)
			continue;
		selectedPaths << item->propertyPath();
	}

	selectionChanged(selectedPaths);
}

