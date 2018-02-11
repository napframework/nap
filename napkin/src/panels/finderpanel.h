#pragma once


#include <QWidget>
#include <QStandardItemModel>
#include "generic/filtertreeview.h"
#include "generic/propertypath.h"

namespace napkin
{


	class FinderPanel : public QWidget
	{
	public:
		FinderPanel();

		void setPropertyList(const QList<PropertyPath>& properties);

	private:
		QVBoxLayout mLayout;
		QStandardItemModel mModel;
		FilterTreeView mTree;
	};
}