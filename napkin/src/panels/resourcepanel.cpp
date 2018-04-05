#include <commands.h>
#include "resourcepanel.h"

#include "generic/filterpopup.h"
#include "generic/qtutils.h"
#include "generic/naputils.h"
#include "standarditemsobject.h"


using namespace napkin;



napkin::ResourceModel::ResourceModel() : mObjectsItem(TXT_LABEL_OBJECTS), mEntitiesItem(TXT_LABEL_ENTITIES)
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


void napkin::ResourceModel::refresh()
{
	while (mEntitiesItem.rowCount() > 0)
		mEntitiesItem.removeRow(0);
	while (mObjectsItem.rowCount() > 0)
		mObjectsItem.removeRow(0);

	auto doc = AppContext::get().getDocument();

	if (doc == nullptr)
		return;

	for (nap::rtti::Object* ob : topLevelObjects(doc->getObjectPointers()))
		addObjectItem(*ob);
}

void ResourceModel::addObjectItem(nap::rtti::Object& ob)
{
	if (!shouldObjectBeVisible(ob))
		return;

	auto typeItem = new RTTITypeItem(ob.get_type());

	// Entity?
	if (ob.get_type().is_derived_from<nap::Entity>())
	{
		// Grab entities and stuff them in a group
		nap::Entity& e = *rtti_cast<nap::Entity>(&ob);

		auto entityItem = new EntityItem(e);
		mEntitiesItem.appendRow({entityItem, typeItem});
		return;
	}

	// ... now the rest in Objects...
	mObjectsItem.appendRow({new ObjectItem(&ob), typeItem});
}

void ResourceModel::removeObjectItem(const nap::rtti::Object& object)
{
	auto item = findItemInModel<napkin::ObjectItem>(*this, object);
	if (item == nullptr)
		return;

	removeRow(item->row(), item->parent()->index());
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
	mTreeView.getTreeView().setSortingEnabled(true);

	connect(&AppContext::get(), &AppContext::documentOpened, this, &ResourcePanel::onFileOpened);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &ResourcePanel::onNewFile);

	connect(mTreeView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this,
			&ResourcePanel::onSelectionChanged);

	mTreeView.setMenuHook(std::bind(&ResourcePanel::menuHook, this, std::placeholders::_1));
	//    connect(&AppContext::get(), &AppContext::dataChanged, this, &ResourcePanel::refresh);
	connect(&AppContext::get(), &AppContext::entityAdded, this, &ResourcePanel::onEntityAdded);
	connect(&AppContext::get(), &AppContext::componentAdded, this, &ResourcePanel::onComponentAdded);
	connect(&AppContext::get(), &AppContext::objectAdded, this, &ResourcePanel::onObjectAdded);
	connect(&AppContext::get(), &AppContext::objectRemoved, this, &ResourcePanel::onObjectRemoved);
	connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &ResourcePanel::onPropertyValueChanged);
}

void napkin::ResourcePanel::menuHook(QMenu& menu)
{
	auto item = mTreeView.getSelectedItem();
	if (item == nullptr)
		return;

	auto objItem = dynamic_cast<ObjectItem*>(item);
	if (objItem != nullptr)
	{
		auto entityItem = dynamic_cast<EntityItem*>(item);

		if (entityItem != nullptr)
		{
			// Selected item is an Entity
			auto parent = entityItem->getEntity();
			menu.addAction(new AddEntityAction(parent));

			// Components
			auto addComponentMenu = menu.addMenu("Add Component");
			for (const auto& type : getComponentTypes())
			{
				addComponentMenu->addAction(new AddComponentAction(*entityItem->getEntity(), type));
			}
		}

		menu.addAction(new DeleteObjectAction(*objItem->getObject()));
	}

	auto groupItem = dynamic_cast<GroupItem*>(item);
	if (groupItem != nullptr)
	{
		if (groupItem->text() == TXT_LABEL_ENTITIES)
		{
			menu.addAction(new AddEntityAction(nullptr));
		}
		else if (groupItem->text() == TXT_LABEL_OBJECTS)
		{
			// Resources
			menu.addAction("Add Object", [this]() {
				auto type = FilterPopup::getResourceType(this);
				if (type.is_valid())
					AppContext::get().executeCommand(new AddObjectCommand(type));
			});
		}
	}
}

void napkin::ResourcePanel::onNewFile()
{
	refresh();
}


void napkin::ResourcePanel::onFileOpened(const QString& filename)
{
	refresh();
}

void napkin::ResourcePanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	// Grab selected nap objects
	QList<nap::rtti::Object*> selectedObjects;
	for (auto m : mTreeView.getSelectedItems())
	{
		auto item = dynamic_cast<ObjectItem*>(m);
		if (!item)
			continue;
		selectedObjects << item->getObject();
	}

	selectionChanged(selectedObjects);
}

void napkin::ResourcePanel::refresh()
{
	mModel.refresh();
	mTreeView.getTreeView().expandAll();
}

void napkin::ResourcePanel::onEntityAdded(nap::Entity* entity, nap::Entity* parent)
{
	// TODO: Don't refresh the whole mModel
	mModel.refresh();
	mTreeView.getTreeView().expandAll();
	mTreeView.selectAndReveal(findItemInModel<napkin::ObjectItem>(mModel, *entity));
}

void napkin::ResourcePanel::onComponentAdded(nap::Component& comp, nap::Entity& owner)
{
	// TODO: Don't refresh the whole mModel
	mModel.refresh();
	mTreeView.getTreeView().expandAll();
	mTreeView.selectAndReveal(findItemInModel<ObjectItem>(mModel, comp));
}

void napkin::ResourcePanel::onObjectAdded(nap::rtti::Object& obj, bool selectNewObject)
{
	mModel.addObjectItem(obj);
	if (selectNewObject)
		mTreeView.selectAndReveal(findItemInModel<napkin::ObjectItem>(mModel, obj));
}

void ResourcePanel::selectObjects(const QList<nap::rtti::Object*>& obj)
{
	if (obj.size() > 0)
		mTreeView.selectAndReveal(findItemInModel<napkin::ObjectItem>(mModel, *obj[0]));
}


void napkin::ResourcePanel::onObjectRemoved(const nap::rtti::Object& object)
{
	mModel.removeObjectItem(object);
}

void napkin::ResourcePanel::onPropertyValueChanged(const PropertyPath& path)
{
	// Update object name?
	if (path.getProperty().get_name() == nap::rtti::sIDPropertyName)
	{
		auto objectItem = findItemInModel<napkin::ObjectItem>(mModel, path.getObject());
		if (objectItem != nullptr)
			objectItem->setText(QString::fromStdString(path.getObject().mID));
	}

	mModel.removeEmbeddedObjects();
}

