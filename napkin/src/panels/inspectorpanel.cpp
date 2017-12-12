#include "inspectorpanel.h"
#include "commands.h"
#include "napkinglobals.h"
#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include <standarditemsproperty.h>

using namespace nap::rtti;


void napkin::InspectorModel::setObject(RTTIObject* object)
{
	mObject = object;

	rebuild();
}


void napkin::InspectorModel::rebuild()
{
	while (rowCount() > 0)
		removeRow(0);

	if (mObject != nullptr)
		populateItems();
}

napkin::InspectorModel::InspectorModel() : QStandardItemModel()
{
	setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_VALUE, TXT_LABEL_TYPE});
	connect(&AppContext::get(), &AppContext::propertyValueChanged,
			this, &InspectorModel::onPropertyValueChanged);
}


napkin::InspectorPanel::InspectorPanel()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.getTreeView().setColumnWidth(0, 250);
	mTreeView.getTreeView().setColumnWidth(1, 250);
	mTreeView.getTreeView().setItemDelegateForColumn(1, &mWidgetDelegate);

	mTreeView.setMenuHook(std::bind(&napkin::InspectorPanel::onItemContextMenu, this, std::placeholders::_1));
}

void napkin::InspectorPanel::onAddObjectArrayElement(ArrayPropertyItem* targetItem, nap::rtti::RTTIObject* object)
{
	nap::rtti::ResolvedRTTIPath resolved_path;
	targetItem->getPath().resolve(targetItem->getObject(), resolved_path);
	assert(resolved_path.isValid());

	nap::rtti::Variant array = resolved_path.getValue();
	nap::rtti::VariantArray array_view = array.create_array_view();

	const nap::rtti::TypeInfo array_type = array_view.get_rank_type(array_view.get_rank());
	const nap::rtti::TypeInfo wrapped_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;

	nap::rtti::Variant new_item = object;
	new_item.convert(wrapped_type);

	bool inserted = array_view.insert_value(array_view.get_size(), new_item);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	rebuild();
}

void napkin::InspectorPanel::onAddObjectArrayElement(ArrayPropertyItem* targetItem, const nap::rtti::TypeInfo& type)
{
	nap::rtti::ResolvedRTTIPath resolved_path;
	targetItem->getPath().resolve(targetItem->getObject(), resolved_path);
	assert(resolved_path.isValid());

	nap::rtti::Variant array = resolved_path.getValue();
	nap::rtti::VariantArray array_view = array.create_array_view();

	nap::rtti::RTTIObject* new_object = AppContext::get().addObject(type, false);
	bool inserted = array_view.insert_value(array_view.get_size(), new_object);
	assert(inserted);

	bool value_set = resolved_path.setValue(array);
	assert(value_set);

	rebuild();
}

void napkin::InspectorPanel::onItemContextMenu(QMenu& menu)
{
	QStandardItem* item = mTreeView.getSelectedItem();
	if (item == nullptr)
		return;

	ArrayPropertyItem* array_item = dynamic_cast<ArrayPropertyItem*>(item);
	if (array_item != nullptr)
	{
		// Determine the type of the array
		const nap::rtti::TypeInfo array_type = array_item->getArray().get_rank_type(array_item->getArray().get_rank());
		const nap::rtti::TypeInfo wrapped_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;

		if (wrapped_type.is_pointer())
		{
			// Build 'Add New' menu, populated with all types matching the array type
			std::vector<nap::rtti::TypeInfo> derived_types;
			getDerivedTypesRecursive(wrapped_type.get_raw_type(), derived_types);

			// Remove any types that are not actually createable (base classes and such)
			nap::rtti::Factory& factory = AppContext::get().getCore().getResourceManager()->getFactory();
			for (int index = derived_types.size() - 1; index >= 0; --index)
			{
				if (!factory.canCreate(derived_types[index]))
					derived_types.erase(derived_types.begin() + index);
			}
			
			// Sort on typename
			std::sort(derived_types.begin(), derived_types.end(), [](const nap::rtti::TypeInfo& typeA, const nap::rtti::TypeInfo& typeB) 
			{
				return typeA.get_name().compare(typeB.get_name()) < 0;
			});

			if (!derived_types.empty())
			{
				// If there's only a single type, make an item instead of submenu for ease of access
				if (derived_types.size() == 1)
				{
					const nap::rtti::TypeInfo& type = derived_types[0].get_raw_type();
					std::string title = nap::utility::stringFormat("Add New '%s'", type.get_name().data());
					menu.addAction(new FunctorAction(QString::fromStdString(title), [this, array_item, type]() { onAddObjectArrayElement(array_item, type); }));
				}
				else
				{
					QMenu* add_new_menu = menu.addMenu("Add New...");
					for (const nap::rtti::TypeInfo& type : derived_types)
						add_new_menu->addAction(new FunctorAction(QString::fromUtf8(type.get_name().data()), [this, array_item, type]() { onAddObjectArrayElement(array_item, type); }));
				}
			}

			// Build 'Add Existing' menu, populated with all existing objects matching the array type
 			QMenu* add_existing_menu = menu.addMenu("Add Existing...");
			std::vector<nap::rtti::RTTIObject*> objects = AppContext::get().getObjectsOfType(wrapped_type.get_raw_type());
			
			// Sort on object ID
			std::sort(objects.begin(), objects.end(), [](const nap::rtti::RTTIObject* objectA, const nap::rtti::RTTIObject* objectB)
			{
				return objectA->mID < objectB->mID;
			});
			
			// Add all objects to submenu
			for (nap::rtti::RTTIObject* object : objects)
				add_existing_menu->addAction(new FunctorAction(QString::fromStdString(object->mID), [this, array_item, object]() { onAddObjectArrayElement(array_item, object); }));
		}
		else
		{
// This doesn't currently work due to a bug in RTTR (unable to create primitive types through RTTR). Need to find a way to fix that first.
#if 0
			// For non-pointer arrays, we only allow adding new items
			menu.addAction(new FunctorAction("Add", [array_item]()
			{
				const nap::rtti::TypeInfo array_type = array_item->getArray().get_rank_type(array_item->getArray().get_rank());
				const nap::rtti::TypeInfo wrapped_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;
				
				nap::rtti::ResolvedRTTIPath resolved_path;
 				array_item->getPath().resolve(array_item->getObject(), resolved_path);
 				assert(resolved_path.isValid());

				nap::rtti::Variant array = resolved_path.getValue();
				nap::rtti::VariantArray array_view = array.create_array_view();
				
				rttr::variant new_value = wrapped_type.create();
				array_view.insert_value(array_view.get_size(), new_value);

				resolved_path.setValue(array);
			}));
#endif
		}

		menu.addSeparator();
	}
}

void napkin::InspectorPanel::setObject(RTTIObject* objects)
{
	mModel.setObject(objects);
	mTreeView.getTreeView().expandAll();
}


void napkin::InspectorPanel::rebuild()
{
	mModel.rebuild();
	mTreeView.getTreeView().expandAll();
}


void napkin::InspectorModel::populateItems()
{
	for (auto prop : mObject->get_type().get_properties())
	{
		if (!prop.is_valid() || prop.is_static() || prop.get_name() == PROP_CHILDREN ||
			prop.get_name() == PROP_COMPONENTS)
			continue;

		std::string name = prop.get_name().data();
		QString qName = QString::fromStdString(name);

		nap::rtti::RTTIPath path;
		path.pushAttribute(name);

		auto value = prop.get_value(mObject);
		auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

		appendRow(createPropertyItemRow(wrappedType, qName, mObject, path, prop, value));
	}
}

QVariant napkin::InspectorModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::UserRole)
	{
		auto valueItem = dynamic_cast<PropertyValueItem*>(itemFromIndex(index));
		if (valueItem)
		{
			return QVariant::fromValue(TypeWrapper(&valueItem->getValueType()));
		}
	}
	return QStandardItemModel::data(index, role);
}

bool napkin::InspectorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	return QStandardItemModel::setData(index, value, role);
//	if (role == Qt::EditRole)
//	{
//		auto valueItem = dynamic_cast<PropertyValueItem*>(itemFromIndex(index));
//		if (valueItem != nullptr)
//		{
//			valueItem->setData(value, Qt::EditRole);
//			AppContext::get().executeCommand(new SetValueCommand(value.))
//			return true;
//		}
//	}
}

nap::rtti::RTTIObject* napkin::InspectorModel::getObject()
{
	return mObject;
}


void napkin::InspectorModel::onPropertyValueChanged(RTTIObject& object, const RTTIPath& path)
{
	auto resolvedPath = resolve(object, path);

	ModelIndexFilter filter = [this, &object, &path](QModelIndex index) 
	{
		QStandardItem* item = itemFromIndex(index);
		if (item == nullptr)
			return false;

		auto objItem = dynamic_cast<PropertyValueItem*>(item);
		if (objItem == nullptr)
			return false;

		if (objItem->getObject() != &object)
			return false;

		return path == objItem->getPath();
	};

	QModelIndex foundIndex = findIndexInModel(*this, filter, 1);
	if (foundIndex.isValid())
		dataChanged(index(foundIndex.row(), 0), index(foundIndex.row(), columnCount()));
}
