#include "filterpopup.h"

#include <QKeyEvent>
#include <mathutils.h>

#include "appcontext.h"
#include "standarditemsobject.h"
#include "qtutils.h"

napkin::FlatObjectModel::FlatObjectModel(const rttr::type& baseType) : mBaseType(baseType)
{
	for (auto& object : AppContext::get().getDocument()->getObjects())
	{
		if (!object.get()->get_type().is_derived_from(baseType))
			continue;

		auto item = new ObjectItem(object.get());
		item->setEditable(false);
		appendRow(item);
	}
}

napkin::FlatTypeModel::FlatTypeModel(const rttr::type& basetype) : mBaseType(basetype)
{
	std::vector<nap::rtti::TypeInfo> derived_types;
	nap::rtti::getDerivedTypesRecursive(basetype.get_raw_type(), derived_types);

	// Remove any types that are not actually createable (base classes and such)
	nap::rtti::Factory& factory = AppContext::get().getCore().getResourceManager()->getFactory();
	for (long index = derived_types.size() - 1; index >= 0; --index)
	{
		if (!factory.canCreate(derived_types[index]))
			derived_types.erase(derived_types.begin() + index);
	}

	for (const rttr::type t : derived_types) {
		auto name = QString::fromUtf8(t.get_name().data());
		auto item = new QStandardItem(name);
		appendRow(item);
	}

}



napkin::FilterPopup::FilterPopup(QWidget* parent) : QMenu(parent)
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.getTreeView().setHeaderHidden(true);
	mTreeView.getLineEdit().setFocusPolicy(Qt::StrongFocus);
	mTreeView.setIsItemSelector(true);
	connect(&mTreeView.getTreeView(), &QTreeView::doubleClicked, this, &FilterPopup::confirm);
	setMinimumSize(400, 400);
}

void napkin::FilterPopup::showEvent(QShowEvent* event)
{
	QMenu::showEvent(event);
	mTreeView.getLineEdit().setFocus();
}

nap::rtti::RTTIObject* napkin::FilterPopup::getObject(QWidget* parent, const rttr::type& typeConstraint)
{
	auto dialog = new FilterPopup(parent);
	dialog->mTreeView.setModel(new FlatObjectModel(typeConstraint));

	dialog->exec(QCursor::pos());

	auto item = dynamic_cast<ObjectItem*>(dialog->mTreeView.getSelectedItem());
	if (item != nullptr)
		return item->getObject();

	return nullptr;
}

nap::rtti::TypeInfo napkin::FilterPopup::getDerivedType(QWidget* parent, const rttr::type& baseType)
{
	auto dialog = new FilterPopup(parent);
	dialog->mTreeView.setModel(new FlatTypeModel(baseType));
	dialog->exec(QCursor::pos());

	auto selected_item = dialog->mTreeView.getSelectedItem();
	if (selected_item == nullptr)
		return rttr::type::empty();

	return nap::rtti::TypeInfo::get_by_name(selected_item->text().toStdString().c_str());
}



void napkin::FilterPopup::keyPressEvent(QKeyEvent* event)
{
	QMenu::keyPressEvent(event);

	if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
		confirm();
	else if (event->key() == Qt::Key_Up)
		moveSelection(-1);
	else if (event->key() == Qt::Key_Down)
		moveSelection(1);
}

void napkin::FilterPopup::moveSelection(int dir)
{
	auto selectedIndexes = mTreeView.getSelectionModel()->selection().indexes();
	if (selectedIndexes.size() == 0)
		return;

	int row = selectedIndexes.at(0).row();
	int newRow = nap::math::clamp(row + dir, 0, mTreeView.getModel()->rowCount() - 1);

	if (row == newRow)
		return;

	auto leftIndex = mTreeView.getModel()->index(newRow, 0);
	auto rightIndex = mTreeView.getModel()->index(newRow, mTreeView.getModel()->columnCount() - 1);
	QItemSelection selection(leftIndex, rightIndex);

	mTreeView.getSelectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}


void napkin::FilterPopup::confirm()
{
	close();
}

