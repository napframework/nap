#pragma once

#include <QWidget>
#include <QUndoView>
#include <QtWidgets/QVBoxLayout>
#include "appcontext.h"

class HistoryPanel : public QWidget {
Q_OBJECT
public:
    HistoryPanel() : QWidget()
    {
        mUndoView.setStack(&AppContext::get().undoStack());
        setLayout(&mLayout);
        layout()->setContentsMargins(0, 0, 0, 0);
        mLayout.addWidget(&mUndoView);
    }

private:
    QVBoxLayout mLayout;
    QUndoView mUndoView;
};