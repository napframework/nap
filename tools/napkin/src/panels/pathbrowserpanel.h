#pragma once

#include <standarditemsobject.h>
#include <napqt/filtertreeview.h>
#include "actions.h"


namespace nap
{
	namespace rtti
	{
		class Object;
	}
}


namespace napkin
{

	class PathItem : public QStandardItem
	{
	public:
		PathItem(const PropertyPath& path);

		QVariant data(int role) const override;

		const PropertyPath& parentPath();

		const PropertyPath& path() const;
	private:
		PropertyPath mPath;
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
		QStandardItemModel mModel;     // Model
		nap::qt::FilterTreeView mTreeView; // Treeview
	};
}