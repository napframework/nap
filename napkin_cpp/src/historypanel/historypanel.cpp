#include "historypanel.h"

#include <QItemSelectionModel>

#include "../appcontext.h"
#include "ui_historypanel.h"

HistoryPanel::HistoryPanel(QWidget* parent) : QDockWidget(parent), ui(new Ui::HistoryPanel)
{
	ui->setupUi(this);
    ui->undoView->setStack(&AppContext::get().undoStack());
//	ui->treeView->setModel(&mHistoryModel);
//
//	connect(&AppContext::get(), &AppContext::undoChanged, this, &HistoryPanel::onUndoChanged);
//
//	connect(ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this,
//			SLOT(onSelectionChanged(QModelIndex, QModelIndex)));
}

HistoryPanel::~HistoryPanel() { delete ui; }


void HistoryPanel::onUndoChanged(int index)
{
//	int idx = index;
//
//	int undoCount = AppContext::get().undoStackSize();
//	//	const QUndoStack* undoStack = AppContext::get().undoStack();
//
//	while (undoCount > mHistoryModel.rowCount()) {
//		QString text = AppContext::get().undoCommand(undoCount - 1)->text();
//		mHistoryModel.appendRow(new QStandardItem(text));
//	}
//
//	while (undoCount < mHistoryModel.rowCount()) {
//		mHistoryModel.removeRow(mHistoryModel.rowCount() - 1);
//	}
//
//	for (int i = 0, len = undoCount; i < len; i++) {
//		QStandardItem* historyItem = mHistoryModel.item(i);
//		if (i >= idx) {
//			historyItem->setForeground(palette().color(QPalette::Disabled, QPalette::Foreground));
//		} else {
//			historyItem->setForeground(ui->treeView->palette().foreground());
//		}
//	}
//
//	if (idx > 0) {
//		ui->treeView->selectionModel()->setCurrentIndex(mHistoryModel.item(idx - 1)->index(),
//														QItemSelectionModel::ClearAndSelect);
//	}
}

void HistoryPanel::onSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
//	int oldIdx = AppContext::get().undoIndex() - 1;
//	int newIdx = current.row();
//	if (oldIdx != newIdx) AppContext::get().setUndoIndex(newIdx + 1);
}
