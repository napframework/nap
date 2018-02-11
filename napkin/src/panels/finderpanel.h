#pragma once


#include <QWidget>
#include <QStandardItemModel>
#include "generic/filtertreeview.h"
#include "generic/propertypath.h"

namespace napkin
{
	class PropertyDisplayItem : public QStandardItem
	{
	public:
		explicit PropertyDisplayItem(const PropertyPath& prop);

		const PropertyPath& getPath() const { return mProp; }

	private:
		const PropertyPath mProp;
	};


	class FinderPanel : public QWidget
	{
	public:
		FinderPanel();

		void setPropertyList(const QList<PropertyPath>& properties);

		FilterTreeView& getTreeView();

	private:
		QVBoxLayout mLayout;
		QStandardItemModel mModel;
		FilterTreeView mTree;
	};
}