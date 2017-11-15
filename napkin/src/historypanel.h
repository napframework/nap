#pragma once

#include <QWidget>
#include <QUndoView>
#include <QtWidgets/QVBoxLayout>
#include "appcontext.h"

class HistoryPanel : public QWidget {
    Q_OBJECT
public:
    HistoryPanel() : QWidget() {
        mUndoView.setStack(&AppContext::get().undoStack());
        setLayout(&mLayout);
        mLayout.addWidget(&mUndoView);
    }

private:
    QVBoxLayout mLayout;
    QUndoView mUndoView;
};