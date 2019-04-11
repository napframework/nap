#pragma once

#include <QDialog>
#include <QStringListModel>

#include "filtertreeview.h"

namespace nap
{
	namespace qt
	{

		/**
		 * A popup list of strings that can be filtered
		 */
		class FilterPopup : public QMenu
		{
		private:
			FilterPopup(QWidget* parent = nullptr);

		public:
			static QString show(QWidget* parent, const QStringList& items);
			static QString show(QWidget* parent, const QStringList& items, QPoint pos);

		protected:
			void keyPressEvent(QKeyEvent* event) override;
			void showEvent(QShowEvent* event) override;

		private:
			void setItems(const QStringList& items);
			void moveSelection(int d);
			void accept();
			void updateSize();

			int mBottomMargin = 10;
			int mMaxHeight = 500;
			QVBoxLayout mLayout;
			QStringListModel* mModel = nullptr;
			FilterTreeView mFilterTree;
			QString mChoice;
		};

	} // namespace qt

} // namespace nap