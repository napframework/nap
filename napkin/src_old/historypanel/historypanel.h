#pragma once

#include <QDockWidget>
#include <QStandardItemModel>
#include <QUndoStack>

namespace Ui
{
	class HistoryPanel;
}

class HistoryPanel : public QDockWidget
{
	Q_OBJECT
public:
	explicit HistoryPanel(QWidget* parent = 0);
	~HistoryPanel();

private slots:
	void onUndoChanged(int idx);
    void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

private:
	Ui::HistoryPanel*  ui;
	QStandardItemModel mHistoryModel;
    QStandardItem* mOriginalItem = nullptr;
};

