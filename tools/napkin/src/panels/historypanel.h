/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "appcontext.h"

#include <QUndoView>
#include <QWidget>
#include <QtWidgets/QVBoxLayout>

namespace napkin
{
	/**
	 * Panel showing the undostack and allow the user to move up and down in it.
	 */
	class HistoryPanel : public QWidget
	{
		Q_OBJECT
	public:
		HistoryPanel();



	private:
		/**
		 * Ensure we have the correct undo stack
		 */
		void updateUndoStack();

		std::unique_ptr<QVBoxLayout> mLayout = nullptr; // The main layout
		std::unique_ptr<QUndoView> mUndoView = nullptr; // The actual undo list
	};
};