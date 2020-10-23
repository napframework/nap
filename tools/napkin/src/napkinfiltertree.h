/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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