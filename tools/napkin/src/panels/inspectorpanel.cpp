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

void napkin::InspectorModel::setObject(Object* object)
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
}

Qt::DropActions napkin::InspectorModel::supportedDragActions() const
{
	return Qt::MoveAction;
}

Qt::DropActions napkin::InspectorModel::supportedDropActions() const
{
	return Qt::MoveAction;
}


napkin::InspectorPanel::InspectorPanel() : mTreeView(new _FilterTreeView())
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

	mTreeView.setMenuHook(std::bind(&napkin::InspectorPanel::onItemContextMenu, this, std::placeholders::_1));

	// TODO: Move this back to the model and let it update its state whenever properties change
	connect(&AppContext::get(), &AppContext::propertyValueChanged,
			this, &InspectorPanel::onPropertyValueChanged);

	connect(&AppContext::get(), &AppContext::propertySelectionChanged,
			this, &InspectorPanel::onPropertySelectionChanged);

}

void napkin::InspectorPanel::onItemContextMenu(QMenu& menu)
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
				nap::rtti::Object* selected_object = napkin::showObjectSelector(this,
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

				rttr::type elementType = napkin::showTypeSelector(this, predicate);

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

void napkin::InspectorPanel::onPropertyValueChanged(const PropertyPath& path)
{
	rebuild();
}


void napkin::InspectorPanel::setObject(Object* object)
{
	if (object)
	{
		mTitle.setText(QString::fromStdString(object->mID));
		mSubTitle.setText(QString::fromStdString(object->get_type().get_name().data()));
	}
	else
	{
		mTitle.setText("");
		mSubTitle.setText("");
	}

	mModel.setObject(object);
	mTreeView.getTreeView().expandAll();
}


void napkin::InspectorPanel::rebuild()
{
	mModel.rebuild();
	mTreeView.getTreeView().expandAll();
}

void napkin::InspectorPanel::onPropertySelectionChanged(const PropertyPath& prop)
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

bool napkin::InspectorModel::isPropertyIgnored(const napkin::PropertyPath& prop) const
{
	if (prop.getName() == nap::rtti::sIDPropertyName)
		return true;
	return false;
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

		nap::rtti::Path path;
		path.pushAttribute(name);

		auto value = prop.get_value(mObject);
		auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

		PropertyPath propPath(*mObject, path);

		if (!isPropertyIgnored(propPath))
			appendRow(createPropertyItemRow(propPath));
	}
}

QVariant napkin::InspectorModel::data(const QModelIndex& index, int role) const
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

bool napkin::InspectorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	return QStandardItemModel::setData(index, value, role);
}

nap::rtti::Object* napkin::InspectorModel::getObject()
{
	return mObject;
}

Qt::ItemFlags napkin::InspectorModel::flags(const QModelIndex& index) const
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

QMimeData* napkin::InspectorModel::mimeData(const QModelIndexList& indexes) const
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

QStringList napkin::InspectorModel::mimeTypes() const
{
	QStringList types;
	types << sNapkinMimeData;
	return types;
}

