#include "historypanel.h"

napkin::HistoryPanel::HistoryPanel() : QWidget()
{
	connect(&AppContext::get(), &AppContext::documentOpened, this, &HistoryPanel::updateUndoStack);
	connect(&AppContext::get(), &AppContext::newDocumentCreated, this, &HistoryPanel::updateUndoStack);
}


void napkin::HistoryPanel::updateUndoStack()
{
	mLayout   = std::make_unique<QVBoxLayout>();
	mUndoView = std::make_unique<QUndoView>();

	setLayout(mLayout.get());
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout->addWidget(mUndoView.get());

	assert(AppContext::isAvailable());
	AppContext& context = AppContext::get();
	Document* doc = context.getDocument();
	QUndoStack& stack = doc->getUndoStack();
	mUndoView->setStack(&stack);
}
