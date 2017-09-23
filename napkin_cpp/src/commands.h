#pragma once

#include <QUndoCommand>

class Command : public QUndoCommand {

};


class AddObjectCommand : Command {
public:
    void undo() override {

    }
    void redo() override {

    }
};

class DeleteObjectCommand : Command {
public:
    void undo() override {

    }
    void redo() override {

    }
};

class SetValueCommand : Command {
public:
    void undo() override {

    }
    void redo() override {

    }

};