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

#include <QApplication>
#include <QMimeData>
#include <QScrollBar>

#include <utility/fileutils.h>
#include <napqt/filterpopup.h>
#include <nap/group.h>

using namespace nap::rtti;
using namespace napkin;

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

	mTreeView.setMenuHook(std::bind(&InspectorPanel::onItemContextMenu, this, std::placeholders::_1));

	// TODO: Move this back to the model and let it update its state whenever properties change
	connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &InspectorPanel::onPropertyValueChanged);
	connect(&AppContext::get(), &AppContext::propertySelectionChanged, this, &InspectorPanel::onPropertySelectionChanged);
	connect(&AppContext::get(), &AppContext::documentClosing, this, &InspectorPanel::onFileClosing);
	connect(&AppContext::get(), &AppContext::serviceConfigurationClosing, this, &InspectorPanel::onFileClosing);

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
	if (path_item == nullptr)
		return;
		
	// In Array?
	auto parent_item = path_item->parentItem();
	auto parent_array_item = qobject_cast<ArrayPropertyItem*>(parent_item);
	if (parent_array_item != nullptr)
	{
		auto parent_array_item = static_cast<ArrayPropertyItem*>(parent_item);
		PropertyPath parent_property = parent_array_item->getPath();
		long element_index = path_item->row();
		menu.addAction("Remove Element", [parent_property, element_index]()
			{
				AppContext::get().executeCommand(new ArrayRemoveElementCommand(parent_property, element_index));
			});
	}

	// File link?
	auto& path = path_item->getPath();
	const auto& type = path.getType();
	const auto& prop = path.getProperty();
	if (type.is_derived_from<std::string>() && nap::rtti::hasFlag(prop, nap::rtti::EPropertyMetaData::FileLink))
	{
		bool ok;
		std::string filename = path_item->getPath().getValue().to_string(&ok);
		if (nap::utility::fileExists(filename))
		{
			menu.addAction("Show file in " + nap::qt::fileBrowserName(), [filename]()
			{
				nap::qt::revealInFileBrowser(QString::fromStdString(filename));
			});
			menu.addAction("Open in external editor", [filename]()
			{
				nap::qt::openInExternalEditor(QString::fromStdString(filename));
			});
		}

	}

	if (path.isInstanceProperty() && path.isOverridden())
	{
		menu.addAction("Remove override", [path]() {
			PropertyPath p = path;
			p.removeOverride();
		});
	}

	// Pointer?
	if (qobject_cast<PointerItem*>(path_item) != nullptr)
	{
		nap::rtti::Object* pointee = path_item->getPath().getPointee();
		QAction* action = menu.addAction("Select Resource", [pointee]
		{
			QList<nap::rtti::Object*> objects = {pointee};
			AppContext::get().selectionChanged(objects);
		});
		action->setEnabled(pointee != nullptr);
	}

	// Embedded pointer?
	if (qobject_cast<EmbeddedPointerItem*>(path_item) != nullptr)
	{
		nap::rtti::Object* pointee = path_item->getPath().getPointee();
		QString label = QString(pointee ? "Replace" : "Create") + " Instance";
		menu.addAction(label, [this, path_item]
		{
			auto path = path_item->getPath();
			auto type = path.getWrappedType();

			TypePredicate predicate = [type](auto t) { return t.is_derived_from(type); };
			rttr::type chosenType = showTypeSelector(this, predicate);
			if (!chosenType.is_valid())
				return;

			path.getDocument()->executeCommand(new ReplaceEmbeddedPointerCommand(path, chosenType));
		});

		if (pointee)
		{
			menu.addAction("Remove Instance", [path_item, pointee]
			{
				// TODO: Make this a command
				auto doc = path_item->getPath().getDocument();
				auto pointeePath = PropertyPath(*pointee, *doc);
				auto ownerPath = doc->getEmbeddedObjectOwnerPath(*pointeePath.getObject());
				doc->removeObject(*pointeePath.getObject());
				if (ownerPath.isValid())
				{
					doc->propertyValueChanged(ownerPath);
				}
			});
		}
	}

	// Array item?
	if (qobject_cast<ArrayPropertyItem*>(path_item))
	{
		PropertyPath array_path = path_item->getPath();
		if (array_path.isNonEmbeddedPointer())
		{
			// Build 'Add Existing' menu, populated with all existing objects matching the array type
			menu.addAction("Add...", [this, array_path]()
			{
				auto objects = AppContext::get().getDocument()->getObjects(array_path.getArrayElementType());
				nap::rtti::Object* selected_object = showObjectSelector(this, objects);
				if (selected_object != nullptr)
					AppContext::get().executeCommand(new ArrayAddExistingObjectCommand(array_path, *selected_object));
			});

		}
		else if (array_path.isEmbeddedPointer())
		{
			menu.addAction("Create...", [this, array_path]()
			{
				auto type = array_path.getArrayElementType();
				TypePredicate predicate = [type](auto t) { return t.is_derived_from(type); };
				rttr::type elementType = showTypeSelector(this, predicate);
				if (elementType.is_valid())
					AppContext::get().executeCommand(new ArrayAddNewObjectCommand(array_path, elementType));
			});
		}
		else
		{
			auto element_type = array_path.getArrayElementType();
			menu.addAction(QString("Add %1").arg(QString::fromUtf8(element_type.get_name().data())), [array_path]()
			{
				AppContext::get().executeCommand(new ArrayAddValueCommand(array_path));
			});

		}
		menu.addSeparator();
	}
}

void InspectorPanel::onPropertyValueChanged(const PropertyPath& path)
{
	// Skip groups, they're not visible in the inspector, only their children
	if (path.hasProperty() && path.getObject()->get_type().is_derived_from(RTTI_OF(nap::IGroup)))
	{
		setPath({});
		return;
	}

	// Get vertical scroll pos so we can restore it later (HACK)
	int verticalScrollPos = mTreeView.getTreeView().verticalScrollBar()->value();

	// Regular property changed, not the ID. Rebuild the model and apply selection
	if (path.getName() != sIDPropertyName)
	{
		rebuild(path);
	}
	// If the object name changed, the property path in the model is now invalid because it's string-based.
	else
	{
		// Get parent of property and parent as potential object
		auto parent = path.getParent();
		auto object = parent.getObject();

		assert(object != nullptr);
		auto doc = path.getDocument();

		// If the object is embedded and the owner is not a group, refresh, but make sure to
		// only show the root object. Walk up embedded owners until the root object or a group is found.
		auto embedded_owner = doc->getEmbeddedObjectOwner(*object);
		if (embedded_owner && !embedded_owner->get_type().is_derived_from(RTTI_OF(nap::IGroup)))
		{
			while (true)
			{
				auto embedded_owner_parent = doc->getEmbeddedObjectOwner(*embedded_owner);
				if (!embedded_owner_parent || embedded_owner_parent->get_type().is_derived_from(RTTI_OF(nap::IGroup)))
					break;
				embedded_owner = embedded_owner_parent;
			}
			setPath(PropertyPath(*embedded_owner, *doc));
		}
		else
		{
			setPath(parent);
		}
	}

	// Set scroll pos
	mTreeView.getTreeView().verticalScrollBar()->setValue(verticalScrollPos);
}

void InspectorPanel::setPath(const PropertyPath& path)
{
	auto doc = mModel.path().getDocument();

	if (doc != nullptr)
		disconnect(doc, &Document::removingObject, this, &InspectorPanel::onObjectRemoved);

	if (path.isValid())
	{
		mTitle.setText(QString::fromStdString(path.getName()));
		mSubTitle.setText(QString::fromStdString(path.getType().get_name().data()));
	}
	else
	{
		mTitle.setText("");
		mSubTitle.setText("");
	}
	mPathField.setText(QString::fromStdString(path.toString()));

	mModel.setPath(path);
	doc = path.getDocument();
	if (doc != nullptr)
		connect(doc, &Document::removingObject, this, &InspectorPanel::onObjectRemoved);

	mTreeView.getTreeView().expandAll();
}

void InspectorPanel::clear()
{
	mModel.clearItems();
	mPathField.setText("");
	mTitle.setText("");
	mSubTitle.setText("");
}

void napkin::InspectorPanel::rebuild(const PropertyPath& selection)
{
	// Rebuild model
	mModel.clearItems();
	mModel.populateItems();
	mTreeView.getTreeView().expandAll();

	// Find item based on path name
	auto pathItem = nap::qt::findItemInModel(mModel, [selection](QStandardItem* item)
	{
		auto pitem = qitem_cast<PropertyPathItem*>(item);
		return pitem != nullptr ? pitem->getPath().toString() == selection.toString() :
			false;
	});

	if (pathItem != nullptr)
		mTreeView.selectAndReveal(pathItem);
}


void napkin::InspectorPanel::onFileClosing(const QString& filename)
{
	mModel.clearPath();
	clear();
}


void InspectorPanel::onPropertySelectionChanged(const PropertyPath& prop)
{
	QList<nap::rtti::Object*> objects = {prop.getObject()};
	AppContext::get().selectionChanged(objects);

	auto pathItem = nap::qt::findItemInModel(mModel, [prop](QStandardItem* item)
	{
		auto pitem = qitem_cast<PropertyPathItem*>(item);
		return pitem != nullptr ? pitem->getPath() == prop : false;
	});

	mTreeView.selectAndReveal(pathItem);
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

void InspectorModel::populateItems()
{
	if (rtti_cast<nap::Entity>(mPath.getObject()))
		return;

	for (const auto& propPath : mPath.getChildren())
	{
		if (!isPropertyIgnored(propPath))
			appendRow(createPropertyItemRow(propPath));
	}
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
		break;
	}
	case Qt::TextColorRole:
	{
		auto value_item = qitem_cast<PropertyPathItem*>(itemFromIndex(index));
		if (value_item != nullptr)
		{
			bool correct_item = qobject_cast<PointerValueItem*>(value_item) != nullptr ||
				qobject_cast<PropertyValueItem*>(value_item) != nullptr;

			if (value_item->getPath().isInstanceProperty() && correct_item)
			{
				auto& themeManager = AppContext::get().getThemeManager();
				if (value_item->getPath().isOverridden())
				{
					return QVariant::fromValue<QColor>(themeManager.getColor(theme::color::instancePropertyOverride));
				}
			}
		}
		break;
	}
	default:
		break;
	}
	return QStandardItemModel::data(index, role);
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

QMimeData* InspectorModel::mimeData(const QModelIndexList& indexes) const
{
	if (indexes.empty())
		return nullptr;

	auto mime_data = new QMimeData();

	QString mime_text;

	// As soon as the first valid item is found, use that, ignore subsequent items
	// TODO: Handle dragging multiple items
	for (auto index : indexes)
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

