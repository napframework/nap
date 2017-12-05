#include "commands.h"

using namespace napkin;

void SetValueCommand::undo()
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

void SetValueCommand::redo()
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

SetValueCommand::SetValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, QVariant newValue)
        : mObject(ptr), mPath(path), mNewValue(newValue)
{
    setText("Set value on: " + QString::fromStdString(path.toString()));
}

void AddObjectCommand::undo()
{
}

void AddObjectCommand::redo()
{
}

void DeleteObjectCommand::undo()
{
}

void DeleteObjectCommand::redo()
{
}
