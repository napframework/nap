#pragma once

#include <QDialog>

#include "filtertreeview.h"

namespace nap
{
	namespace qt
	{

		/**
		 * General purpose popup dialog showing a filterable tree.
		 */
		class FilterPopup : public QMenu
		{
		public:
			explicit FilterPopup(QWidget* parent, QStandardItemModel& model);

		public:


			/**
			 * Display a selection dialog with the selected strings. The user can filter the list and select an item.
			 * @param parent The parent widget to attach to
			 * @param strings The list of strings to choose from
			 * @return The selected string
			 */
			static const QString fromStringList(QWidget* parent, const QList<QString>& strings);

			/**
			 * Override to provide a reasonable size
			 */
			QSize sizeHint() const override;

			/**
			 * @return true if the user choice was confirmed, false if the dialog was dismissed
			 */
			bool wasAccepted() const { return mWasAccepted; }

			/**
			 * @return The item selected by the user
			 */
			QStandardItem* getSelectedItem() { return mTreeView.getSelectedItem(); }

		protected:
			/**
			 * Set focus etc
			 */
			void showEvent(QShowEvent* event) override;

			/**
			 * Capture keyboard for confirmation etc
			 */
			void keyPressEvent(QKeyEvent* event) override;

		private:
			void moveSelection(int dir);
			void confirm();

			bool mWasAccepted = false;
			FilterTreeView mTreeView;
			QVBoxLayout mLayout;
			QSize mSize = {400, 400};

		};

	} // namespace qt

} // namespace nap