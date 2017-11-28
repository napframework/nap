#pragma once


#include <nap/objectptr.h>
#include <rtti/rttipath.h>

#include <QUndoCommand>
#include <QtCore/QVariant>

#include "typeconversion.h"

class AddObjectCommand : public QUndoCommand
{
public:
	void undo() override {}

	void redo() override {}
};

class DeleteObjectCommand : public QUndoCommand
{
public:
	void undo() override {}

	void redo() override {}
};

class SetValueCommand : public QUndoCommand
{
public:
	SetValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, QVariant newValue)
		: mObject(ptr), mPath(path), mNewValue(newValue)
	{
		setText("Set value on: " + QString::fromStdString(path.toString()));
	}

	void undo() override;

	void redo() override;

private:
	nap::rtti::RTTIObject* mObject;
	nap::rtti::RTTIPath mPath;
	QVariant mNewValue;
	QVariant mOldValue;
};