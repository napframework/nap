#include "autosettings.h"

#include <nap/logger.h>

#include <QMainWindow>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QTreeView>

using namespace napkin;

////////////////////////////////////////////////////////////////////////////////////////////////////
// QMainWindow Storer
////////////////////////////////////////////////////////////////////////////////////////////////////

template<>
void WidgetStorer<QMainWindow>::store(const QMainWindow& widget, const QString& key, QSettings& s) const
{
	s.setValue(key + "_GEO", widget.saveGeometry());
	s.setValue(key + "_STATE", widget.saveState());
}

template<>
void WidgetStorer<QMainWindow>::restore(QMainWindow& widget, const QString& key, const QSettings& s) const
{
	widget.restoreGeometry(s.value(key + "_GEO").toByteArray());
	widget.restoreState(s.value(key + "_STATE").toByteArray());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// QHeaderView Storer
////////////////////////////////////////////////////////////////////////////////////////////////////

template<>
void WidgetStorer<QHeaderView>::store(const QHeaderView& widget, const QString& key, QSettings& s) const
{
	s.setValue(key + "_GEO", widget.saveGeometry());
	s.setValue(key + "_STATE", widget.saveState());
}

template<>
void WidgetStorer<QHeaderView>::restore(QHeaderView& widget, const QString& key,
											  const QSettings& s) const
{
	widget.restoreGeometry(s.value(key + "_GEO").toByteArray());
	widget.restoreState(s.value(key + "_STATE").toByteArray());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void AutoSettings::registerStorer(WidgetStorerBase* s)
{
	mStorers << s;
}

void AutoSettings::store(QWidget& w) const
{
	QSettings s;
	storeRecursive(w, s);
}

void AutoSettings::restore(QWidget& w) const
{
	QSettings s;
	restoreRecursive(w, s);
}

void AutoSettings::storeRecursive(QWidget& w, QSettings& s) const
{
	auto storer = findStorer(w);
	if (storer)
	{
		auto key = uniqeObjectName(w);
//		nap::Logger::debug("Storing '%s'", key.toStdString().c_str());
		storer->storeWidget(w, key, s);
	}

	for (auto child : w.children())
	{
		auto widget = dynamic_cast<QWidget*>(child);
		if (widget != nullptr)
			storeRecursive(*widget, s);
	}
}

void AutoSettings::restoreRecursive(QWidget& w, const QSettings& s) const
{
	auto storer = findStorer(w);
	if (storer) {
		auto key = uniqeObjectName(w);
//		nap::Logger::debug("Restoring '%s'", key.toStdString().c_str());
		storer->restoreWidget(w, key, s);
	}


	for (const auto child : w.children())
	{
		auto widget = dynamic_cast<QWidget*>(child);
		if (widget != nullptr)
			restoreRecursive(*widget, s);
	}
}



AutoSettings::AutoSettings()
{
	registerStorer(new WidgetStorer<QMainWindow>());
	registerStorer(new WidgetStorer<QHeaderView>());
}

AutoSettings::~AutoSettings()
{
	for (auto storer : mStorers)
		delete storer;
	mStorers.clear();
}

WidgetStorerBase* AutoSettings::findStorer(const QWidget& w) const
{
	for (auto storer : mStorers)
		if (storer->canStore(w))
			return storer;
	return nullptr;
}

const QString AutoSettings::ensureHasName(QObject& w) const
{
	if (!w.objectName().isEmpty())
		return w.objectName();

	QString proposedName = w.metaObject()->className();
	QString name = proposedName;
	if (w.parent() != nullptr)
	{
		int i = 1;
		while (w.parent()->findChild<QObject*>(name, Qt::FindDirectChildrenOnly)) {
			name = QString("%1_%2").arg(proposedName, i);
			i += 1;
		}
	}
	w.setObjectName(name);

	return name;
}

QString AutoSettings::uniqeObjectName(QWidget& w) const
{
	QStringList names;
	QObject* current = &w;
	while (current != nullptr)
	{
		auto name = ensureHasName(*current);
		names.push_front(name);
		current = current->parent();
	}

	return names.join("/");
}

