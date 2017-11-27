#pragma once


#include <rtti/rttipath.h>
#include <nap/objectptr.h>

#include <QtCore/QVariant>
#include <QUndoCommand>

#include "typeconversion.h"

class AddObjectCommand : public QUndoCommand {
public:
    void undo() override
    {

    }

    void redo() override
    {

    }
};

class DeleteObjectCommand : public QUndoCommand {
public:
    void undo() override
    {

    }

    void redo() override
    {

    }
};

class SetValueCommand : public QUndoCommand {
public:
    SetValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, QVariant newValue)
            : mObject(ptr), mPath(path), mNewValue(newValue)
    {
        setText("Set value on: " + QString::fromStdString(path.toString()));
    }

    void undo() override
    {
        // resolve path
        nap::rtti::ResolvedRTTIPath resolvedPath;
        mPath.resolve(mObject, resolvedPath);
        assert(resolvedPath.isValid());

        // set new value
        bool ok;
        rttr::variant variant = fromQVariant(resolvedPath.getType(), mOldValue, &ok);
        assert(ok);
        resolvedPath.setValue(variant);

    }

    void redo() override
    {
        // retrieve and store current value
        nap::rtti::ResolvedRTTIPath resolvedPath;
        mPath.resolve(mObject, resolvedPath);
        assert(resolvedPath.isValid());
        rttr::variant oldValueVariant = resolvedPath.getValue();
        assert(toQVariant(resolvedPath.getType(), oldValueVariant, mOldValue));

        // set new value
        bool ok;
        rttr::variant variant = fromQVariant(resolvedPath.getType(), mNewValue, &ok);
        assert(ok);
        resolvedPath.setValue(variant);
    }

private:
    nap::rtti::RTTIObject* mObject;
    nap::rtti::RTTIPath mPath;
    QVariant mNewValue;
    QVariant mOldValue;
};