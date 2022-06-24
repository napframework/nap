/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "autosettings.h"
#include "gridview.h"

#include <QMainWindow>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QTreeView>
#include <QComboBox>

using namespace nap::qt;

void AutoSettings::registerStorer(std::unique_ptr<WidgetStorerBase> s)
{
	mStorers.emplace_back(std::move(s));
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
		auto widget = qobject_cast<QWidget*>(child);
		if (widget != nullptr)
			storeRecursive(*widget, s);
	}
}

void AutoSettings::restoreRecursive(QWidget& w, const QSettings& s) const
{
	auto storer = findStorer(w);
	if (storer) {
		auto key = uniqeObjectName(w);
		if (!mExclusions.contains(&w))
			storer->restoreWidget(w, key, s);
	}


	for (const auto child : w.children())
	{
		auto widget = qobject_cast<QWidget*>(child);
		if (widget != nullptr && !mExclusions.contains(widget))
			restoreRecursive(*widget, s);
	}
}



AutoSettings::AutoSettings()
{
	registerDefaults();
}

WidgetStorerBase* AutoSettings::findStorer(const QWidget& w) const
{
	for (const auto& storer : mStorers)
		if (storer->canStore(w))
			return storer.get();
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

AutoSettings::AutoSettings(AutoSettings const&)
{
	registerDefaults();
}

void AutoSettings::registerDefaults()
{
	registerStorer<MainWindowWidgetStorer>();
	registerStorer<HeaderViewWidgetStorer>();
	registerStorer<SplitterStorer>();
	registerStorer<GridViewStorer>();
}

AutoSettings& AutoSettings::get()
{
	static AutoSettings instance;
	return instance;
}


