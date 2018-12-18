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

		QVBoxLayout mLayout; // The main layout
		QUndoView mUndoView; // The actual undo list
	};
};