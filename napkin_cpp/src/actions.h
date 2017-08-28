#pragma once

#include <QAction>
#include <QKeySequence>
#include <QFileDialog>
#include "appcontext.h"

class OpenFileAction : public QAction {

public:
    OpenFileAction(QObject* parent);

private:
    void perform();

};


class SaveFileAction : public QAction {
public:
    SaveFileAction(QObject* parent);

private:
    void perform();

};

class SaveFileAsAction : public QAction {
public:
    SaveFileAsAction(QObject* parent);

private:
    void perform();

};
