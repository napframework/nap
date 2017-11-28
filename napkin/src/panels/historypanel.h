#pragma once

#include "appcontext.h"
#include <QUndoView>
#include <QWidget>
#include <QtWidgets/QVBoxLayout>

class HistoryPanel : public QWidget
{
	Q_OBJECT
public:
	HistoryPanel();

private:
	QVBoxLayout mLayout;
	QUndoView mUndoView;
};