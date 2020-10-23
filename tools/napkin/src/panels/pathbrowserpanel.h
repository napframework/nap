/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "actions.h"
#include <standarditemsobject.h>
#include <napqt/filtertreeview.h>

namespace nap
{
	namespace rtti
	{
		class Object;
	}
}


namespace napkin
{
	class PathBrowserModel : public QAbstractItemModel
	{
	public:
		QModelIndex index(int row, int column, const QModelIndex& parent) const override;
		QModelIndex parent(const QModelIndex& child) const override;
		int rowCount(const QModelIndex& parent) const override;
		int columnCount(const QModelIndex& parent) const override;
		QVariant data(const QModelIndex& index, int role) const override;

		void clear();
		void addPath(const PropertyPath& path);
	private:
		QList<std::shared_ptr<PropertyPath>> mRootPaths;
		mutable QMap<PropertyPath*, QList<std::shared_ptr<PropertyPath>>> mCache;
	};


	class PathBrowserPanel : public QWidget
	{
	Q_OBJECT
	public:
		PathBrowserPanel();

		/**
		 * @return The tree view held by this panel
		 */
		nap::qt::FilterTreeView& treeView();

	Q_SIGNALS:
		void selectionChanged(QList<PropertyPath> obj);

	private:
		QVBoxLayout mLayout;      // Layout
		PathBrowserModel mModel;     // Model
		nap::qt::FilterTreeView mTreeView; // Treeview
	};
}