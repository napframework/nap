#include "historypanel.h"

HistoryPanel::HistoryPanel() : QWidget()
{
	mUndoView.setStack(&AppContext::get().undoStack());
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mUndoView);
}
