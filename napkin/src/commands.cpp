#include "commands.h"

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
