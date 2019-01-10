#include "inspectorpanel.h"

#include <QApplication>
#include <QMimeData>

#include <utility/fileutils.h>
#include <napkinfiltertree.h>

#include "appcontext.h"
#include "commands.h"
#include "napkinglobals.h"
#include "standarditemsproperty.h"
#include <napqt/filterpopup.h>
#include "naputils.h"

using namespace nap::rtti;
using namespace napkin;

void InspectorModel::setPath(const PropertyPath& path)
{
	mPath = path;
	rebuild();
}

void InspectorModel::removeItems()
{
	while (rowCount() > 0)
		removeRow(0);
}

void InspectorModel::rebuild()
{
	removeItems();
	populateItems();
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


InspectorPanel::InspectorPanel() : mTreeView(new _FilterTreeView())
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

	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.getTreeView().setColumnWidth(0, 250);
	mTreeView.getTreeView().setColumnWidth(1, 250);
	mTreeView.getTreeView().setItemDelegateForColumn(1, &mWidgetDelegate);
	mTreeView.getTreeView().setDragEnabled(true);

	mTreeView.setMenuHook(std::bind(&InspectorPanel::onItemContextMenu, this, std::placeholders::_1));

	// TODO: Move this back to the model and let it update its state whenever properties change
	connect(&AppContext::get(), &AppContext::propertyValueChanged,
			this, &InspectorPanel::onPropertyValueChanged);

	connect(&AppContext::get(), &AppContext::propertySelectionChanged,
			this, &InspectorPanel::onPropertySelectionChanged);

}

void InspectorPanel::onItemContextMenu(QMenu& menu)
{
	QStandardItem* item = mTreeView.getSelectedItem();
	if (item == nullptr)
		return;

	auto parent_item = item->parent();
	if (parent_item != nullptr)
	{
		auto parent_array_item = dynamic_cast<ArrayPropertyItem*>(parent_item);
		if (parent_array_item != nullptr)
		{
			PropertyPath parent_property = parent_array_item->getPath();
			long element_index = item->row();
			menu.addAction("Remove Element", [parent_property, element_index]()
			{
				AppContext::get().executeCommand(new ArrayRemoveElementCommand(parent_property, element_index));
			});
		}
	}

	// File link?
	auto path_item = dynamic_cast<PropertyPathItem*>(item);
	if (path_item != nullptr)
	{
		const auto& type = path_item->getPath().getType();
		const auto& prop = path_item->getPath().getProperty();
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
                menu.addAction("Open in external editor" , [filename]()
                {
					nap::qt::openInExternalEditor(QString::fromStdString(filename));
                });
			}

		}
	}

	// Pointer?
	auto pointer_item = dynamic_cast<PointerItem*>(item);
	if (pointer_item != nullptr)
	{
		nap::rtti::Object* pointee = pointer_item->getPath().getPointee();
		QAction* action = menu.addAction("Select Resource", [pointer_item, pointee]
		{
			QList<nap::rtti::Object*> objects = {pointee};
			AppContext::get().selectionChanged(objects);
		});
		action->setEnabled(pointee != nullptr);
	}

	// Array item?
	auto* array_item = dynamic_cast<ArrayPropertyItem*>(item);
	if (array_item != nullptr)
	{
		PropertyPath array_path = array_item->getPath();

		if (array_path.isNonEmbeddedPointer())
		{
			// Build 'Add Existing' menu, populated with all existing objects matching the array type
			menu.addAction("Add...", [this, array_path]()
			{
				nap::rtti::Object* selected_object = showObjectSelector(this,
																					 array_path.getArrayElementType());
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

				if (!elementType.empty())
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
	rebuild();
}

void InspectorPanel::setPath(const PropertyPath& path)
{
	if (path.isValid())
	{
		mTitle.setText(QString::fromStdString(path.getName()));
		if (path.isInstance())
			mSubTitle.setText(QString::fromStdString(path.getType().get_name().data()) + " [INSTANCE]");
		else
			mSubTitle.setText(QString::fromStdString(path.getType().get_name().data()));
	}
	else
	{
		mTitle.setText("");
		mSubTitle.setText("");
	}

	mModel.setPath(path);
	mTreeView.getTreeView().expandAll();
}

void InspectorPanel::clear()
{
	mModel.removeItems();
}

void InspectorPanel::rebuild()
{
	mModel.rebuild();
	mTreeView.getTreeView().expandAll();
}

void InspectorPanel::onPropertySelectionChanged(const PropertyPath& prop)
{
	QList<nap::rtti::Object*> objects = {&prop.getObject()};
	AppContext::get().selectionChanged(objects);

	auto pathItem = nap::qt::findItemInModel(mModel, [prop](QStandardItem* item)
	{
		auto pitem = dynamic_cast<PropertyPathItem*>(item);
		if (pitem == nullptr)
			return false;
		return pitem->getPath() == prop;
	});

	mTreeView.selectAndReveal(pathItem);
}

bool InspectorModel::isPropertyIgnored(const PropertyPath& prop) const
{
	return prop.getName() == nap::rtti::sIDPropertyName;
}

void InspectorModel::populateItems()
{
	if (dynamic_cast<nap::Entity*>(&mPath.getObject()))
		return;

	for (auto propPath : mPath.getChildren())
	{
		if (!isPropertyIgnored(propPath))
			appendRow(createPropertyItemRow(propPath));
	}
}

QVariant InspectorModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::UserRole)
	{
		auto valueItem = dynamic_cast<PropertyValueItem*>(itemFromIndex(index));
		if (valueItem)
		{
			return QVariant::fromValue(valueItem->getPath());
		}
	}
	return QStandardItemModel::data(index, role);
}

bool InspectorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	return QStandardItemModel::setData(index, value, role);
}

nap::rtti::Object* InspectorModel::getObject()
{
	return &mPath.getObject();
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
	auto parent_item = item->parent();
	if ((parent_item != nullptr) && (dynamic_cast<ArrayPropertyItem*>(parent_item) != nullptr))
	{
		flags |= Qt::ItemIsDragEnabled;
	}

	// Is this item an array? Allow dropping
	if (dynamic_cast<ArrayPropertyItem*>(item) != nullptr)
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
		auto object_item = dynamic_cast<PropertyPathItem*>(itemFromIndex(index));
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

