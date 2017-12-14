#include <nap/logger.h>
#include <generic/utility.h>
#include "commands.h"
#include "appcontext.h"

using namespace napkin;

SetValueCommand::SetValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, QVariant newValue)
		: mObject(ptr), mPath(path), mNewValue(newValue)
{
	setText("Set value on: " + QString::fromStdString(path.toString()));
}

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

	AppContext::get().propertyValueChanged(*mObject, mPath);
}

void SetValueCommand::redo()
{
	// retrieve and store current value
	auto resolvedPath = resolve(*mObject, mPath);
	rttr::variant oldValueVariant = resolvedPath.getValue();
	assert(toQVariant(resolvedPath.getType(), oldValueVariant, mOldValue));

	// set new value
	bool ok;
	rttr::variant variant = fromQVariant(resolvedPath.getType(), mNewValue, &ok);
	assert(ok);
	resolvedPath.setValue(variant);

	AppContext::get().propertyValueChanged(*mObject, mPath);
}

SetPointerValueCommand::SetPointerValueCommand(nap::rtti::RTTIObject* ptr, nap::rtti::RTTIPath path, nap::rtti::RTTIObject* newValue) :
	mObject(ptr),
	mPath(path),
	mNewValue(newValue),
	mOldValue(nullptr)
{
	mOldValue = getPointee(*mObject, mPath);
}

void SetPointerValueCommand::undo()
{
	nap::rtti::ResolvedRTTIPath resolvedPath = resolve(*mObject, mPath);
	assert(resolvedPath.isValid());

	resolvedPath.setValue(mOldValue);
}

void SetPointerValueCommand::redo()
{
	nap::rtti::ResolvedRTTIPath resolvedPath = resolve(*mObject, mPath);
	assert(resolvedPath.isValid());

	bool value_set = resolvedPath.setValue(mNewValue);
	assert(value_set);
}


void AddObjectCommand::redo()
{

}
void AddObjectCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}


void DeleteObjectCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}

void DeleteObjectCommand::redo()
{
}


AddSceneCommand::AddSceneCommand()
{
}

void AddSceneCommand::undo()
{
	nap::Logger::fatal("Sorry, no undo for you");
}

void AddSceneCommand::redo()
{
	nap::Logger::warn("Adding scene to ResourceManager, but the objects are still held by AppContext");
	auto& ctx = AppContext::get();
	auto scene = ctx.getCore().getResourceManager()->getFactory().create(RTTI_OF(nap::Scene));
	scene->mID = "New Scene";
	ctx.objectAdded(*scene, true);
}

