#pragma once

#include <QAction>
#include <QKeySequence>
#include <QFileDialog>
#include <QString>
#include <QStandardItem>
#include <QtCore/QSet>
#include "appcontext.h"

class OpenFileAction : public QAction {

public:
    OpenFileAction();

private:
    void perform();

};


class SaveFileAction : public QAction {
public:
    SaveFileAction();

private:
    void perform();

};

class SaveFileAsAction : public QAction {
public:
    SaveFileAsAction();

private:
    void perform();

};


class AddRTTIObject : public QAction {
public:
    AddRTTIObject(rttr::type type) : QAction()
    {
        setText(QString("Add %1").arg(type.get_name().data()));
    }

private:
    void perform();
};

class ActionFactory {
public:
    QList<QAction*> actionsFor(QList<QStandardItem*> items) {
        // TODO
    }

    QList<QAction*> actionsFor(QStandardItem* item) {
        // TODO
    }
};

//class DeleteAction : public QAction {
//public:
//    DeleteAction(QObject)
//};