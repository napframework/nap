/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "inspectorpanel.h"
#include "appcontext.h"
#include "commands.h"
#include "napkinglobals.h"
#include "standarditemsproperty.h"
#include "naputils.h"
#include "napkinutils.h"
#include "napkin-resources.h"
#include "propertymapper.h"

#include <QApplication>
#include <QMimeData>
#include <QScrollBar>
#include <QMessageBox>

#include <utility/fileutils.h>
#include <napqt/filterpopup.h>
#include <nap/group.h>
#include <fcurve.h>
#include <material.h>
#include <renderservice.h>

using namespace nap::rtti;
using namespace napkin;

/**
 * Shows a dialog that asks the user if the required property should be removed 
 * @return if the required property should be removed
 */
static bool getRemoveRequiredProperty()
{
	QMessageBox msg(AppContext::get().getMainWindow());
	msg.setWindowTitle("Required property");
	msg.setText("This is a required property, are you sure you want to remove it?");
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msg.setDefaultButton(QMessageBox::Cancel);
	msg.setIconPixmap(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_QUESTION).pixmap(32, 32));
	return msg.exec() == QMessageBox::Yes;
}


void InspectorModel::setPath(const PropertyPath& path)
{
	mPath = path;
	clearItems();
	populateItems();
}


const PropertyPath& InspectorModel::path() const
{
	return mPath;
}

void InspectorModel::clearItems()
{
	removeRows(0, rowCount());
}


InspectorModel::InspectorModel() : QStandardItemModel()
{
	setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_VALUE, TXT_LABEL_TYPE});
}


Qt::DropActions InspectorModel::supportedDragActions() const
{
	return Qt::MoveAction;
}


Qt::DropActions InspectorModel::supportedDropActions() const
{
	return Qt::MoveAction;
}


InspectorPanel::InspectorPanel() : mTreeView(new QTreeView())
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);

	auto font = mTitle.font();
	font.setPointSize(14);
	mTitle.setFont(font);
	mSubTitle.setAlignment(Qt::AlignRight);

	mHeaderLayout.addWidget(&mTitle);
	mHeaderLayout.addWidget(&mSubTitle);
	mLayout.addLayout(&mHeaderLayout);
	mHeaderLayout.setContentsMargins(0, 6, 0, 0);

	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.getTreeView().setColumnWidth(0, 250);
	mTreeView.getTreeView().setColumnWidth(1, 250);
	mTreeView.getTreeView().setItemDelegateForColumn(1, &mWidgetDelegate);
	mTreeView.getTreeView().setDragEnabled(true);

	createMenuCallbacks();
	mTreeView.setMenuHook(std::bind(&InspectorPanel::onItemContextMenu, this, std::placeholders::_1));

    connect(mTreeView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, &InspectorPanel::onSelectionChanged);
	connect(&AppContext::get(), &AppContext::propertySelectionChanged, this, &InspectorPanel::onPropertySelectionChanged);
	connect(&AppContext::get(), &AppContext::documentClosing, this, &InspectorPanel::onFileClosing);
	connect(&AppContext::get(), &AppContext::objectRenamed, this, &InspectorPanel::onObjectRenamed);
	connect(&AppContext::get(), &AppContext::serviceConfigurationClosing, this, &InspectorPanel::onFileClosing);
	connect(&mModel, &InspectorModel::childAdded, this, &InspectorPanel::onChildAdded);
	connect(&AppContext::get(), &AppContext::arrayIndexSwapped, this, &InspectorPanel::onIndexSwapped);

	mPathLabel.setText("Path:");
	mSubHeaderLayout.addWidget(&mPathLabel);
	mPathField.setReadOnly(true);
	mSubHeaderLayout.addWidget(&mPathField);

	mLayout.addLayout(&mSubHeaderLayout);

}

void InspectorPanel::onItemContextMenu(QMenu& menu)
{
	// Get property path item
	auto path_item = qitem_cast<PropertyPathItem*>(mTreeView.getSelectedItem());
	if (path_item != nullptr)
	{
		mMenuController.populate(*path_item, menu);
	}
}


void InspectorPanel::setPath(const PropertyPath& path)
{
	// Clear everything
	clear();

	// Bail if path isn't valid
	mPath = path;
	if (!path.isValid())
		return;

	// Update title and subtitle
	mPathField.setText(QString::fromStdString(path.toString()));
	mTitle.setText(path.getName().c_str());
	mSubTitle.setText(path.getType().get_name().data());

	// These types are excluded from property editing
	static const std::vector<nap::rtti::TypeInfo> typeExceptions
	{
		RTTI_OF(nap::math::FloatFCurve)
	};

	// Check if path is an exception
	auto path_obj = path.getObject();
	assert(path_obj != nullptr); auto obj_type = path_obj->get_type();
	auto it = std::find_if(typeExceptions.begin(), typeExceptions.end(), [&obj_type](const nap::rtti::TypeInfo& typeException)
		{
			return obj_type.is_derived_from(typeException);
		});

	// Add if not an exception
	if (it == typeExceptions.end())
	{
		mModel.setPath(path);
		expandTree(QModelIndex());
	}
}


void InspectorPanel::clear()
{
	mModel.clearItems();
	mPathField.setText("");
	mTitle.setText("");
	mSubTitle.setText("");
}


void napkin::InspectorPanel::expandTree(const QModelIndex& parent)
{
	// Get parent item
	QStandardItem* parent_item = parent.isValid() ? mModel.itemFromIndex(parent) : nullptr;
	for (int r = 0; r < mModel.rowCount(parent); r++)
	{
		// Get child item
		QModelIndex child_index = mModel.index(r, 0, parent);
		QStandardItem* child_item =  mModel.itemFromIndex(child_index);
		assert(child_item != nullptr);

		// Don't expand when item in array
		if(qitem_cast<ArrayPropertyItem*>(parent_item) != nullptr)
			continue;

		// Don't expand embedded resources
		if(qitem_cast<EmbeddedPointerItem*>(child_item) != nullptr)
			continue;

		// Don't expand color
		CompoundPropertyItem* compound = qitem_cast<CompoundPropertyItem*>(child_item);
		if (compound != nullptr && compound->getPath().isColor())
			continue;

		// Do expand the rest
		auto mapped_idx = mTreeView.getProxyModel().mapFromSource(child_index);
		mTreeView.getTreeView().expand(mapped_idx);

		// Repeat
		expandTree(child_index);
	}
}


void napkin::InspectorPanel::onIndexSwapped(const PropertyPath& prop, size_t fromIndex, size_t toIndex)
{
	// Check property
	if (prop.getObject() != mPath.getObject())
		return;

	// Get item for property
	auto path_item = nap::qt::findItemInModel(mModel, [&prop](QStandardItem* item)
		{
			auto pitem = qitem_cast<PropertyPathItem*>(item);
			return pitem != nullptr ? pitem->getPath() == prop : false;
		});

	// Select new index 
	if (path_item != nullptr)
	{
		assert(toIndex < path_item->rowCount());
		mTreeView.select(path_item->child(toIndex), false);
	}
}


void napkin::InspectorPanel::onChildAdded(QList<QStandardItem*> items)
{
	assert(items.size() > 0);
	auto parent = items.first()->parent();
	if (qitem_cast<const ArrayPropertyItem*>(parent) != nullptr)
	{
		mTreeView.select(items[0], false);
	}
}


void napkin::InspectorPanel::onFileClosing(const QString& filename)
{
	mModel.clearPath();
	clear();
}


void napkin::InspectorPanel::onObjectRenamed(nap::rtti::Object& object, const std::string& oldName, const std::string& newName)
{
	// Update path if object that was renamed is currently referenced
	if (mPath.referencesObject(oldName))
	{
		mPath.updateObjectName(oldName, newName);
		mPathField.setText(QString::fromStdString(mPath.toString()));
		mTitle.setText(mPath.getName().c_str());
	}
}


void napkin::InspectorPanel::createMenuCallbacks()
{
	// In array -> add option to move up
	mMenuController.addOption([](auto& item, auto& menu)
	{
		// Check if parent is an array property
		auto parent_array = qobject_cast<ArrayPropertyItem*>(item.parentItem());
		if (parent_array == nullptr || item.getPath().isInstanceProperty())
			return;

		// Ensure item can be moved up
		auto idx = static_cast<size_t>(item.row());
		if (idx == 0)
			return;

		// Create label based on type
		QString label = QString("Move '%1' up").arg(item.getPath().getPointee() != nullptr ?
			item.getPath().getPointee()->mID.c_str() :
			parent_array->getPath().getArrayElementType().get_name().to_string().c_str()
		);

		// Add action
		const auto& parent_property = parent_array->getPath();
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_MOVE_UP), label, [parent_property, idx]()
			{
				AppContext::get().executeCommand(new ArraySwapElement(parent_property, idx, idx - 1));
			});
	});

	// In array -> add option to move down
	mMenuController.addOption([](auto& item, auto& menu)
	{
		// Check if parent is an array property
		auto parent_array = qobject_cast<ArrayPropertyItem*>(item.parentItem());
		if (parent_array == nullptr || item.getPath().isInstanceProperty())
			return;

		// Ensure item can be moved down
		auto idx = static_cast<size_t>(item.row());
		if (idx == parent_array->getPath().getArrayLength() - 1)
			return;

		// Create label based on type
		QString label = QString("Move '%1' down").arg(item.getPath().getPointee() != nullptr ?
			item.getPath().getPointee()->mID.c_str() :
			parent_array->getPath().getArrayElementType().get_name().to_string().c_str()
		);

		// Add action
		const auto& parent_property = parent_array->getPath();
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_MOVE_DOWN), label, [parent_property, idx]()
			{
				AppContext::get().executeCommand(new ArraySwapElement(parent_property, idx, idx + 1));
			});
	});

	// In array -> add option to remove
	mMenuController.addOption([](auto& item, auto& menu)
	{
		// Check if parent is an array property
		auto parent_array = qobject_cast<ArrayPropertyItem*>(item.parentItem());
		if (parent_array == nullptr || item.getPath().isInstanceProperty())
			return;

		// Create label based on type
		QString label = QString("Remove '%1'").arg(item.getPath().getPointee() != nullptr ?
			item.getPath().getPointee()->mID.c_str() :
			parent_array->getPath().getArrayElementType().get_name().to_string().c_str()
		);

		// Add action
		int element_index = item.row();
		const auto& parent_property = parent_array->getPath();
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_REMOVE), label, [parent_property, element_index]()
			{
				AppContext::get().executeCommand(new ArrayRemoveElementCommand(parent_property, element_index));
			});
	});

	// String & file link -> show file options
	mMenuController.addOption([](auto& item, auto& menu)
	{
		const auto& path = item.getPath();
		const auto& type = path.getType();
		if (!type.is_derived_from(RTTI_OF(std::string)))
			return;

		const auto& prop = path.getProperty();
		if (!nap::rtti::hasFlag(prop, nap::rtti::EPropertyMetaData::FileLink))
			return;

		// TODO: This always fails -> path is relative & needs to be resolved
		std::string filename = path.getValue().to_string();
		if (!nap::utility::fileExists(filename))
			return;

		menu.addAction("Show file in " + nap::qt::fileBrowserName(), [filename](){
				nap::qt::revealInFileBrowser(QString::fromStdString(filename));
			});

		menu.addAction("Open in external editor", [filename]() {
				nap::qt::openInExternalEditor(QString::fromStdString(filename));
			});
	});

	// Instance property -> remove override
	mMenuController.addOption([](auto& item, auto& menu)
	{
		const auto& path = item.getPath();
		if (!path.isInstanceProperty() || !path.isOverridden())
			return;

		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_REMOVE),
			"Remove Override", [path]()
			{
				PropertyPath p = path;
				p.removeOverride();
			});
	});

	// Pointer -> select resource
	mMenuController.addOption<PointerItem>([](auto& item, auto& menu)
	{
		auto pointer_item = static_cast<PointerItem*>(&item);
		nap::rtti::Object* pointee = pointer_item->getPath().getPointee();
		if (pointee != nullptr)
		{
			menu.addAction(AppContext::get().getResourceFactory().getIcon(*pointee),
				"Select Resource", [pointee]
				{
					QList<nap::rtti::Object*> objects = { pointee };
					AppContext::get().selectionChanged(objects);
				});
		}
	});

	// Pointer -> clear link
	mMenuController.addOption<PointerItem>([](auto& item, auto& menu)
	{
		auto* pointer_item = static_cast<PointerItem*>(&item);
		const auto& path = pointer_item->getPath();
		auto* pointee = path.getPointee();
		if (pointee != nullptr)
		{
			QString label = QString("Clear '%1'").arg(pointee->mID.c_str());
			menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_CLEAR), label, [path]()
				{
					// Bail if the user doesn't want to clear the required property
					if (nap::rtti::hasFlag(path.getProperty(), EPropertyMetaData::Required))
					{
						if (!getRemoveRequiredProperty())
							return;
					}
					AppContext::get().executeCommand(new SetPointerValueCommand(path, nullptr));
				});
		}
	});

	// Embedded pointer -> create
	mMenuController.addOption<EmbeddedPointerItem>([this](auto& item, auto& menu)
	{
		auto pointer_item = static_cast<EmbeddedPointerItem*>(&item);
		if( pointer_item->getPath().getPointee() != nullptr ||
			pointer_item->getPath().isInstanceProperty())
			return;

		const auto& path = pointer_item->getPath();
		auto type = path.getWrappedType();
		QString label = QString("Create %1...").arg(type.get_raw_type().get_name().data());
		menu.addAction(AppContext::get().getResourceFactory().getIcon(type), label, [this, path, type]
			{
				TypePredicate predicate = [&type](auto t) { return t.is_derived_from(type); };
				rttr::type chosenType = showTypeSelector(this, predicate);
				if (chosenType.is_valid())
				{
					path.getDocument()->executeCommand(new ReplaceEmbeddedPointerCommand(path, chosenType));
				}
			});
	});

	// Embedded pointer -> replace
	mMenuController.addOption<EmbeddedPointerItem>([this](auto& item, auto& menu)
	{
		auto pointer_item = static_cast<EmbeddedPointerItem*>(&item);
		if (pointer_item->getPath().getPointee() == nullptr ||
			pointer_item->getPath().isInstanceProperty())
			return;

		auto pointee = pointer_item->getPath().getPointee();
		const auto& path = pointer_item->getPath();
		auto type = path.getWrappedType();

		QString label = QString("Replace '%1'").arg(pointee->mID.c_str());
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_CHANGE), label, [this, path, type]
			{
				TypePredicate predicate = [&type](auto t) { return t.is_derived_from(type); };
				rttr::type chosenType = showTypeSelector(this, predicate);
				if (!chosenType.is_valid())
					return;

				path.getDocument()->executeCommand(new ReplaceEmbeddedPointerCommand(path, chosenType));
			});
	});

	// Embedded pointer -> delete
	mMenuController.addOption<EmbeddedPointerItem>([this](auto& item, auto& menu)
	{
		auto pointer_item = static_cast<EmbeddedPointerItem*>(&item);
		if (pointer_item->getPath().getPointee() == nullptr ||
			pointer_item->getPath().isInstanceProperty())
			return;

		// Only add option to delete if not in array.
		auto parent_array_item = qobject_cast<ArrayPropertyItem*>(pointer_item->parentItem());
		if (parent_array_item != nullptr)
			return;

		auto pointee = pointer_item->getPath().getPointee();
		QString label = QString("Delete '%1'").arg(pointee->mID.c_str());
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_DELETE), label, [pointer_item, pointee]
			{
				// Bail if the user doesn't want to delete the required property
				if (nap::rtti::hasFlag(pointer_item->getPath().getProperty(), EPropertyMetaData::Required))
				{
					if (!getRemoveRequiredProperty())
						return;
				}

				// TODO: Make this a command
				auto doc = pointer_item->getPath().getDocument();
				auto pointeePath = PropertyPath(*pointee, *doc);
				auto ownerPath = doc->getEmbeddedObjectOwnerPath(*pointeePath.getObject());
				doc->removeObject(*pointeePath.getObject());
				if (ownerPath.isValid())
				{
					doc->propertyValueChanged(ownerPath);
				}
			});
	});

	// Array -> add material binding
	mMenuController.addOption<ArrayPropertyItem>([this](auto& item, auto& menu)
	{
		auto* array_item = static_cast<ArrayPropertyItem*>(&item);
		if (!array_item->getPath().getArrayEditable() || array_item->getPath().isInstanceProperty())
			return;

		// Check if we can map it to a material
		PropertyPath array_path = array_item->getPath();
		auto array_type = array_path.getArrayElementType();

		// Add material mapping action
		auto material_mapper = MaterialPropertyMapper::mappable(array_path);
		if (material_mapper != nullptr)
		{
			QString label = QString("Add %1...").arg(QString::fromUtf8(array_type.get_raw_type().get_name().data()));
			menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_ADD), label, [this, mapper = std::move(material_mapper)]()
			{
				mapper->map(this);
			});
		}
	});

	// Array -> add resource pointer
	mMenuController.addOption<ArrayPropertyItem>([this](auto& item, auto& menu)
	{
		auto* array_item = static_cast<ArrayPropertyItem*>(&item);
		if (!array_item->getPath().getArrayEditable() || array_item->getPath().isInstanceProperty())
			return;

		PropertyPath array_path = array_item->getPath();
		auto array_type = array_path.getArrayElementType();

		// Check if it's a regular resource pointer
		if (!array_path.isNonEmbeddedPointer() ||
			MaterialPropertyMapper::mappable(array_path))
			return;

		// Build 'Add Existing' menu, populated with all existing objects matching the array type
		QString label = QString("Add %1...").arg(QString::fromUtf8(array_type.get_raw_type().get_name().data()));
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_ADD), label, [this, array_path, array_type]()
			{
				auto objects = AppContext::get().getDocument()->getObjects(array_type);
				nap::rtti::Object* selected_object = showObjectSelector(this, objects);
				if (selected_object != nullptr)
					AppContext::get().executeCommand(new ArrayAddExistingObjectCommand(array_path, *selected_object));
			});
	});

	// Array -> add embedded pointer
	mMenuController.addOption<ArrayPropertyItem>([this](auto& item, auto& menu)
	{
		auto* array_item = static_cast<ArrayPropertyItem*>(&item);
		if (!array_item->getPath().getArrayEditable() || array_item->getPath().isInstanceProperty())
			return;

		PropertyPath array_path = array_item->getPath();
		auto array_type = array_path.getArrayElementType();

		// Check if it's a regular resource pointer
		auto material_mapper = MaterialPropertyMapper::mappable(array_path);
		if (!array_path.isEmbeddedPointer() ||
			MaterialPropertyMapper::mappable(array_path))
			return;

		// Regular array entry handling
		QString label = QString("Add %1...").arg(QString::fromUtf8(array_type.get_raw_type().get_name().data()));
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_ADD), label, [this, array_path, array_type]()
			{
				TypePredicate predicate = [array_type](auto t) { return t.is_derived_from(array_type); };
				rttr::type elementType = showTypeSelector(this, predicate);
				if (elementType.is_valid())
					AppContext::get().executeCommand(new ArrayAddNewObjectCommand(array_path, elementType));
			});
	});

	// Array -> add regular value
	mMenuController.addOption<ArrayPropertyItem>([this](auto& item, auto& menu)
	{
		auto* array_item = static_cast<ArrayPropertyItem*>(&item);
		if (!array_item->getPath().getArrayEditable() || array_item->getPath().isInstanceProperty())
			return;

		// Bail if it's a pointer or the property is mappable
		PropertyPath array_path = array_item->getPath();
		if (array_path.isPointer() || MaterialPropertyMapper::mappable(array_path))
			return;

		// Regular array entry handling
		auto array_type = array_path.getArrayElementType();
		QString label = QString("Add %1").arg(QString::fromUtf8(array_type.get_raw_type().get_name().data()));
		menu.addAction(AppContext::get().getResourceFactory().getIcon(QRC_ICONS_ADD), label, [array_path]()
			{
				AppContext::get().executeCommand(new ArrayAddValueCommand(array_path));
			});
	});
}


void InspectorPanel::onPropertySelectionChanged(const PropertyPath& prop)
{
	QList<nap::rtti::Object*> objects = {prop.getObject()};
	AppContext::get().selectionChanged(objects);
	auto pathItem = nap::qt::findItemInModel(mModel, [&prop](QStandardItem* item)
	{
		auto pitem = qitem_cast<PropertyPathItem*>(item);
		return pitem != nullptr ? pitem->getPath() == prop : false;
	});
	mTreeView.select(pathItem, true);
}


void InspectorPanel::onObjectRemoved(Object* obj)
{
	// If the currently edited object is being removed, clear the view
	if (obj == mModel.path().getObject())
		setPath({});
}


void napkin::InspectorModel::clearPath()
{
	mPath = PropertyPath();
	clearItems();
}


bool InspectorModel::isPropertyIgnored(const PropertyPath& prop) const
{
	return prop.getName() == nap::rtti::sIDPropertyName;
}


void napkin::InspectorModel::onChildAdded(const QList<QStandardItem*> items)
{
	childAdded(items);
}


void InspectorModel::populateItems()
{
	// Skip entities
	auto rtti_object = mPath.getObject();
	if (rtti_cast<nap::Entity>(rtti_object) != nullptr)
		return;

	// Create items (and child items) for every property
	for (const auto& prop_path : mPath.getChildren())
	{
		// Skip ID
		if (prop_path.getName() == nap::rtti::sIDPropertyName)
			continue;

		// Skip member and child property of groups
		nap::IGroup* group_obj = rtti_cast<nap::IGroup>(rtti_object);
		if (group_obj != nullptr)
		{
			if (group_obj->getMembersProperty()  == prop_path.getProperty() ||
				group_obj->getChildrenProperty() == prop_path.getProperty())
			{
				continue;
			}
		}

		// Create item
		auto row = createPropertyItemRow(prop_path);
		for (const auto& item : row)
		{
			auto path_item = qitem_cast<const PropertyPathItem*>(item);
			if (path_item != nullptr)
			{
				connect(path_item, &PropertyPathItem::childAdded, this, &napkin::InspectorModel::onChildAdded);
			}
		}
		appendRow(row);
	}
}


void InspectorPanel::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
}


QVariant InspectorModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
		case Qt::UserRole:
		{
			auto value_item = qitem_cast<PropertyPathItem*>(itemFromIndex(index));
			if (value_item != nullptr)
			{
				return QVariant::fromValue(value_item->getPath());
			}
		}
		default:
		{
			return QStandardItemModel::data(index, role);
		}
	}
}


bool InspectorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	return QStandardItemModel::setData(index, value, role);
}


nap::rtti::Object* InspectorModel::getObject()
{
	return mPath.getObject();
}


Qt::ItemFlags InspectorModel::flags(const QModelIndex& index) const
{
	auto flags = QStandardItemModel::flags(index);

	// First always disable dragging & dropping
	flags &= ~Qt::ItemIsDragEnabled;
	flags &= ~Qt::ItemIsDropEnabled;

	// Not an item? Early out
	auto item = itemFromIndex(index);
	if (item == nullptr)
		return flags;

	// Is this item an array element? Enable dragging
	if (qitem_cast<ArrayPropertyItem*>(item->parent()) != nullptr)
	{
		flags |= Qt::ItemIsDragEnabled;
	}

	// Is this item an array? Allow dropping
	if (qitem_cast<ArrayPropertyItem*>(item) != nullptr)
	{
		flags |= Qt::ItemIsDropEnabled;
	}
	return flags;
}


QMimeData* InspectorModel::mimeData(const QModelIndexList& indices) const
{
	if (indices.empty())
		return nullptr;

	auto mime_data = new QMimeData();

	QString mime_text;

	// As soon as the first valid item is found, use that, ignore subsequent items
	// TODO: Handle dragging multiple items
	for (auto index : indices)
	{
		auto object_item = qitem_cast<PropertyPathItem*>(itemFromIndex(index));
		if (object_item == nullptr)
			continue;

		mime_text = QString::fromStdString(object_item->getPath().toString());
		break;
	}
	mime_data->setData(sNapkinMimeData, mime_text.toLocal8Bit());

	return mime_data;
}


QStringList InspectorModel::mimeTypes() const
{
	QStringList types;
	types << sNapkinMimeData;
	return types;
}

