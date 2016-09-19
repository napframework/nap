#include "commands.h"
#include <nap/coremodule.h>

void PasteCmd::undo()
{
	nap::Object* pasted = pastedPath.resolve(AppContext::get().core().getRoot());
	assert(pasted);
	mClipboardText = AppContext::get().serialize(pasted);
	pasted->getParentObject()->removeChild(*pasted);
}

void PasteCmd::redo()
{
	nap::Object* parent = parentPath.resolve(AppContext::get().core().getRoot());
	assert(parent);
	AppContext::get().deserialize(mClipboardText, parent);
}

void MoveOperatorsCmd::undo()
{
	nap::Object& root = AppContext::get().core().getRoot();
	for (int i = 0; i < mOperatorPaths.size(); i++) {
		nap::Operator* op = mOperatorPaths[i].resolve<nap::Operator>(root);
		setObjectEditorPosition(*op, mOriginalPositions[i]);
	}
}

void MoveOperatorsCmd::redo()
{
	nap::Object& root = AppContext::get().core().getRoot();
	for (int i = 0; i < mOperatorPaths.size(); i++) {
		nap::Operator* op = mOperatorPaths[i].resolve<nap::Operator>(root);
		assert(op);
		setObjectEditorPosition(*op, mOriginalPositions[i] + mDelta);
	}
}

void ConnectPlugsCmd::undo()
{
	nap::Entity& root = AppContext::get().core().getRoot();
	nap::OutputPlugBase* srcPlug = mSrcPlugPath.resolve<nap::OutputPlugBase>(root);
	nap::InputPlugBase* dstPlug = mDstPlugPath.resolve<nap::InputPlugBase>(root);
	assert(srcPlug);
	assert(dstPlug);

	dstPlug->disconnect(*srcPlug);
}

void ConnectPlugsCmd::redo()
{
	nap::Entity& root = AppContext::get().core().getRoot();
	nap::OutputPlugBase* srcPlug = mSrcPlugPath.resolve<nap::OutputPlugBase>(root);
	nap::InputPlugBase* dstPlug = mDstPlugPath.resolve<nap::InputPlugBase>(root);
	assert(srcPlug);
	assert(dstPlug);

	dstPlug->connect(*srcPlug);
}

void RemoveObjectCmd::redo()
{
	mSerializedObjects.clear();
	mParentPaths.clear();
	for (auto path : mObjectPaths) {
		nap::Object* object = path.resolve(AppContext::get().core().getRoot());
		assert(object);
		nap::ObjectPath parentPath = object->getParentObject();
		mParentPaths << parentPath;
		mSerializedObjects.append(AppContext::get().serialize(object));
		object->getParentObject()->removeChild(*object);
	}
}

void RemoveObjectCmd::undo()
{

	for (int i = 0; i < mParentPaths.size(); i++) {
		nap::Object* parent = mParentPaths[i].resolve(AppContext::get().core().getRoot());
		assert(parent);
		AppContext::get().deserialize(mSerializedObjects[i], parent);
	}
}

void CreateOperatorCmd::redo()
{
	auto patch = (nap::Patch*)mPatchPath.resolve(AppContext::get().core().getRoot());
	assert(patch);

	RTTI::TypeInfo type = RTTI::TypeInfo::getByName(mOpTypeName);
	if (!type.isValid()) {
		nap::Logger::fatal("Operator type not found: '%s'", mOpTypeName.c_str());
		return;
	}

	nap::Operator* op = (nap::Operator*)type.createInstance();
	setObjectEditorPosition(*op, mPos);

	assert(op);
	op->setName(mOpTypeName);

	op = &patch->addOperator(std::move(std::unique_ptr<nap::Operator>(op)));
	mOperatorPath = op;
}

void CreateOperatorCmd::undo()
{
	auto op = (nap::Operator*)mOperatorPath.resolve(AppContext::get().core().getRoot());
	assert(op);

	auto patch = (nap::Patch*)mPatchPath.resolve(AppContext::get().core().getRoot());
	assert(patch);

	patch->removeOperator(*op);
}

void CreateComponentCmd::redo()
{
	nap::Entity* parent = (nap::Entity*)mParentPath.resolve(AppContext::get().core().getRoot());

	RTTI::TypeInfo componentType = RTTI::TypeInfo::getByName(mComponentType);
	if (!componentType.isValid()) {
		nap::Logger::fatal("Component type not found: '%s'", mComponentType.c_str());
		return;
	}


	if (!componentType.canCreateInstance()) {
		nap::Logger::fatal("Cannot create instance of type: '%s'",componentType.getName().c_str());
		return;
	}
	nap::Component* comp = static_cast<nap::Component*>(componentType.createInstance());
	assert(comp);

	comp->setName(stripNamespace(mComponentType));
	mComponentPath = parent->addComponent(std::unique_ptr<nap::Component>(comp));
}

void CreateComponentCmd::undo()
{
	nap::Object& refObject = AppContext::get().core().getRoot();

	nap::Entity* parent = (nap::Entity*)mParentPath.resolve(refObject);
	assert(parent);

	nap::Component* comp = (nap::Component*)mComponentPath.resolve(refObject);
	assert(comp);

	parent->removeComponent(*comp);
}

void CreateEntityCmd::undo()
{
	nap::Object& refObject = AppContext::get().core().getRoot();

	nap::Entity* parent = (nap::Entity*)mParentPath.resolve(refObject);
	assert(parent);

	nap::Entity* e = (nap::Entity*)mEntityPath.resolve(refObject);
	assert(e);

	parent->removeEntity(*e);
}

void CreateEntityCmd::redo()
{
	nap::Entity* parent = (nap::Entity*)mParentPath.resolve(AppContext::get().core().getRoot());
	assert(parent);
	nap::ObjectPath p(parent->addEntity("New Entity"));
	mEntityPath = p;
}

void SetNameCmd::undo()
{
	nap::Object* object = mNewObjectPath.resolve(AppContext::get().core().getRoot());
	assert(object);
	object->setName(mOldName);
}

void SetNameCmd::redo()
{
	nap::Object* object = mOldObjectPath.resolve(AppContext::get().core().getRoot());
	mOldName = object->getName();
	object->setName(mNewName);
	mNewObjectPath = object;
}

void AddAttributeCmd::undo()
{
	nap::AttributeBase* attrib =
		(nap::AttributeBase*)mAttributePath.resolve(AppContext::get().core().getRoot());
	assert(attrib);
	attrib->getParent()->removeChild(*attrib);
}

void AddAttributeCmd::redo()
{
	nap::Entity& root = AppContext::get().core().getRoot();
	nap::AttributeObject* object = mObjectPath.resolve<nap::AttributeObject>(root);
	assert(object);
	mAttributePath = object->addAttribute("NewAttribute", RTTI_OF(nap::Attribute<float>));
}

void SetAttributeValueCmd::undo()
{
	nap::AttributeBase* attrib =
		(nap::AttributeBase*)mAttrPath.resolve(AppContext::get().core().getRoot());
	assert(attrib);
	attributeFromString(*attrib, mOldValue);
}

void SetAttributeValueCmd::redo()
{
	nap::AttributeBase* attrib =
		(nap::AttributeBase*)mAttrPath.resolve(AppContext::get().core().getRoot());
	assert(attrib);
	attributeFromString(*attrib, mNewValue);
}