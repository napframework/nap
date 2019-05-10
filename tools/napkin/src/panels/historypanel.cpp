#include "historypanel.h"

napkin::HistoryPanel::HistoryPanel() : QWidget()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mUndoView);

	connect(&AppContext::get(), &AppContext::documentOpened, this, &HistoryPanel::updateUndoStack);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &HistoryPanel::updateUndoStack);
}

void napkin::HistoryPanel::updateUndoStack()
{
//	auto doc = AppContext::get().getDocument();
//	auto& undostack = doc->getUndoStack();
//	mUndoView.setStack(&undostack);
}
