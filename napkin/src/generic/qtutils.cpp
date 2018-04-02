#include "qtutils.h"

#include <QDir>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLabel>

#include <mathutils.h>
#include <standarditemsproperty.h>
#include <appcontext.h>
#include <nap/logger.h>

#include "panels/finderpanel.h"


QColor napkin::lerpCol(const QColor& a, const QColor& b, qreal p)
{
	QColor c;
	c.setRgbF(nap::math::lerp(a.redF(), b.redF(), p),
              nap::math::lerp(a.greenF(), b.greenF(), p),
              nap::math::lerp(a.blueF(), b.blueF(), p));
	return c;
}

const QColor& napkin::getSoftForeground()
{
	static QColor c = lerpCol(QApplication::palette().color(QPalette::Normal, QPalette::WindowText),
							  QApplication::palette().color(QPalette::Disabled, QPalette::WindowText), 0.5);
	return c;
}

const QColor& napkin::getSoftBackground()
{
	static QColor c = QApplication::palette().color(QPalette::Normal, QPalette::Window).darker(102);
	return c;
}

bool napkin::traverse(const QAbstractItemModel& model, ModelIndexFilter visitor, QModelIndex parent, int column)
{
	for (int r = 0; r < model.rowCount(parent); r++)
	{
		auto index = model.index(r, 0, parent);
		auto colindex = model.index(r, column, parent);
		if (!visitor(colindex))
			return false;
		if (model.hasChildren(index))
			if (!traverse(model, visitor, index))
				return false;
	}
	return true;
}

bool napkin::traverse(const QStandardItemModel& model, ModelItemFilter visitor, QModelIndex parent, int column)
{
	for (int r = 0; r < model.rowCount(parent); r++)
	{
		auto index = model.index(r, 0, parent);
		auto colindex = model.index(r, column, parent);
		if (!visitor(model.itemFromIndex(colindex)))
			return false;
		if (model.hasChildren(index))
			if (!traverse(model, visitor, index))
				return false;
	}
	return true;
}

QModelIndex napkin::findIndexInModel(const QAbstractItemModel& model, ModelIndexFilter condition, int column)
{
	QModelIndex foundIndex;

	traverse(model, [&foundIndex, condition](const QModelIndex& idx) -> bool {
		if (condition(idx))
		{
			foundIndex = idx;
			return false;
		}
		return true;
	}, QModelIndex(), column);

	return foundIndex;
}


QStandardItem* napkin::findItemInModel(const QStandardItemModel& model, ModelItemFilter condition, int column)
{
	QStandardItem* foundItem = nullptr;

	traverse(model, [&foundItem, condition](QStandardItem* item) -> bool {
		if (condition(item))
		{
			foundItem = item;
			return false;
		}
		return true;
	}, QModelIndex(), column);

	return foundItem;
}

void napkin::expandChildren(QTreeView* view, const QModelIndex& index, bool expanded)
{
	if (!index.isValid())
		return;

	if (expanded && !view->isExpanded(index))
		view->expand(index);
	else if (view->isExpanded(index))
		view->collapse(index);

	for (int i = 0, len = index.model()->rowCount(index); i < len; i++)
		expandChildren(view, index.child(i, 0), expanded);
}

bool napkin::directoryContains(const QString& dir, const QString& filename)
{
	auto absDir = QDir(dir).canonicalPath();
	auto absFile = QFileInfo(filename).canonicalFilePath();
	return absFile.startsWith(absDir);
}

void napkin::showPropertyListDialog(QWidget* parent, QList<PropertyPath> props, const QString& title, QString message)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(title);

	QVBoxLayout layout;
	dialog.setLayout(&layout);

	QLabel label;
	label.setText(message);
	layout.addWidget(&label);

	FinderPanel finder;
	FilterTreeView* tree = &finder.getTreeView();

	// On item double-click, close the dialog and reveal the property
	finder.connect(&tree->getTreeView(), &QTreeView::doubleClicked,
				   [tree, &dialog](const QModelIndex& idx)
	{
		const auto sourceIndex = tree->getFilterModel().mapToSource(idx);
		auto item = tree->getModel()->itemFromIndex(sourceIndex);
		auto propitem = dynamic_cast<PropertyDisplayItem*>(item);
		assert(propitem);
		AppContext::get().propertySelectionChanged(propitem->getPath());
		dialog.close();
	});

	finder.setPropertyList(props);
	layout.addWidget(&finder);

	QDialogButtonBox buttonBox(QDialogButtonBox::Close);
	layout.addWidget(&buttonBox);

	dialog.exec();
}

void napkin::revealInFileBrowser(const QString& filename)
{

	// TODO: Reveal files
	nap::Logger::error("Revealing files not supported yet: %s", filename.toStdString().c_str());
}

