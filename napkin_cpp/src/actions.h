#pragma once

#include <QAction>
#include <QKeySequence>
#include <QFileDialog>
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

//class DeleteAction : public QAction {
//public:
//    DeleteAction(QObject)
//};