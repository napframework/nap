#pragma once

#include <QtGui/QDragEnterEvent>
#include <QtWidgets/QTreeView>

namespace napkin {
	/**
	 * Specialize dragging behavior for napkin
	 */
	class _FilterTreeView : public QTreeView
	{
	public:
		_FilterTreeView();

	protected:
		/**
		 * Override from QTreeView
		 */
		void dragEnterEvent(QDragEnterEvent* event) override;

		/**
		 * Override from QTreeView
		 */
		void dragMoveEvent(QDragMoveEvent* event) override;

		/**
		 * Override from QTreeView
		 */
		void dropEvent(QDropEvent* event) override;

	};
}