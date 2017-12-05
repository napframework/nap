#include "historypanel.h"

napkin::HistoryPanel::HistoryPanel() : QWidget()
{
	mUndoView.setStack(&AppContext::get().getUndoStack());
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mUndoView);
}
