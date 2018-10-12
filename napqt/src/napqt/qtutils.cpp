#include "qtutils.h"

#include <QDir>
#include <QHBoxLayout>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QtGui>

QColor napkin::lerpCol(const QColor& a, const QColor& b, qreal p)
{
	QColor c;
	c.setRgbF(lerp(a.redF(), b.redF(), p),
              lerp(a.greenF(), b.greenF(), p),
              lerp(a.blueF(), b.blueF(), p));
	return c;
}

qreal napkin::roundToInterval(qreal v, qreal step)
{
	return qRound(v / step) * step;
}

qreal napkin::floorToInterval(qreal v, qreal step)
{
	return qFloor(v / step) * step;
}

qreal napkin::ceilToInterval(qreal v, qreal step)
{
	return qCeil(v / step) * step;
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

bool napkin::revealInFileBrowser(const QString& filename)
{
#ifdef _WIN32
	QStringList args;
	args << "/select," << QDir::toNativeSeparators(filename);
	QProcess::execute("explorer.exe", args);
#elif defined(__APPLE__)
	QStringList scriptArgs;
	scriptArgs << QLatin1String("-e")
			   << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(filename);
	QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
	scriptArgs.clear();
	scriptArgs << QLatin1String("-e") << QLatin1String("tell application \"Finder\" to activate");
	QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
	// Linux
	// We don't have a reliable way of selecting the file after revealing, just open the file browser
	QString dirname = QFileInfo(filename).dir().path();
	QProcess::startDetached("xdg-open " + dirname);
#endif
	return true;
}


bool napkin::openInExternalEditor(const QString& filename)
{
    return QDesktopServices::openUrl(QUrl::fromLocalFile(filename));
}


QString napkin::fileBrowserName()
{
#ifdef _WIN32
	return "Explorer";
#elif defined(__APPLE__)
	return "Finder";
#else
	return "file browser";
#endif
}

QPointF napkin::getTranslation(const QTransform& xf)
{
	return {xf.m31(), xf.m32()};
}

QSizeF napkin::getScale(const QTransform& xf)
{
	return {xf.m11(), xf.m22()};
}

void napkin::setTranslation(QTransform& xf, qreal x, qreal y)
{
	xf.setMatrix(xf.m11(), xf.m12(), xf.m13(), xf.m21(), xf.m22(), xf.m23(), x, y, xf.m33());
}

void napkin::setTranslation(QTransform& xf, const QPointF& p)
{
	setTranslation(xf, p.x(), p.y());
}

void napkin::setScale(QTransform& xf, qreal x, qreal y)
{
	xf.setMatrix(x, xf.m12(), xf.m13(), xf.m21(), y, xf.m23(), xf.m31(), xf.m32(), xf.m33());
}

void napkin::moveItemToFront(QGraphicsItem& item)
{
	qreal maxZ = 0;
	for (auto item : item.collidingItems(Qt::IntersectsItemBoundingRect))
		maxZ = qMax(maxZ, item->zValue());

	item.setZValue(maxZ + 0.001);
}

