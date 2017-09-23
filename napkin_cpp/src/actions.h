#pragma once

#include <QAction>
#include <QKeySequence>
#include <QFileDialog>
#include <QString>
#include <QStandardItem>
#include <QSet>
#include <QUndoCommand>
#include <nap/entity.h>
#include <nap/logger.h>

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


class AddEntityAction : public QAction {
public:
    AddEntityAction(nap::Entity* parent) : QAction(), mParent(parent)
    {
        setText("Add Entity");
    }

private:
    void perform() {

//        mParent->mChildren.emplace_back(std::make_unique<nap::Entity>());
    }

    nap::Entity* mParent;
};

class AddComponentAction : public QAction {
public:
    AddComponentAction(nap::Entity& entity, nap::rtti::TypeInfo type) : QAction(), mEntity(entity), mComponentType(type) {}

private:
    void perform() {

    }

private:
    nap::Entity& mEntity;
    nap::rtti::TypeInfo mComponentType;
};


