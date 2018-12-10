#pragma once

#include <rtti/rtti.h>
#include <rtti/object.h>

#include <napqt/filtertreeview.h>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>
namespace napkin
{

	/**
	 * Model holding type meta data for all types in the system
	 * @deprecated Not really useful for the end-user
	 */
	class TypeModel : public QStandardItemModel
	{
	public:
		TypeModel();

	private:
        /**
         * Refresh
         */
		void refresh();
	};


	/**
	 * Show all types loaded in the system
	 * @deprecated Not really useful for the end-user
	 */
	class HierarchyPanel : public QWidget
	{
	public:
		HierarchyPanel();

	private:
		QVBoxLayout mLayout;	  // Layout
		napqt::FilterTreeView mTreeView; // TreeView
		TypeModel mModel;		  // The model
	};
};