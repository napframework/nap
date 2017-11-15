#pragma once

#include "appcontext.h"
#include "napkin_utilities.h"
#include "patchpanel/patchscene.h"
#include <QList>
#include <nap/object.h>
#include <nap/objectpath.h>
#include <nap/serializer.h>
#include <regex>


class PasteCmd : public QUndoCommand
{
public:
	PasteCmd(const QString& text, nap::Object* parent = nullptr)
		: QUndoCommand(), mClipboardText(text)
	{
		if (parent)
			parentPath = nap::ObjectPath(*parent);
		setText(QString("Paste clipboard into '%1'")
					.arg(QString::fromStdString(parentPath.toString())));
	}
	virtual void undo() override;
	virtual void redo() override;


private:
	QString mClipboardText;
	nap::ObjectPath parentPath;
	nap::ObjectPath pastedPath;
};



class MoveOperatorsCmd : public QUndoCommand
{
public:
	MoveOperatorsCmd(QList<nap::Operator*> operators, const QPointF& delta) : mDelta(delta)
	{
		setText(QString("Moving %1 operators").arg(operators.size()));
		for (auto op : operators) {
			std::string path = nap::ObjectPath(op);
			mOperatorPaths << path;
			mOriginalPositions << getObjectEditorPosition(*op);
		}
	}
	void undo() override;
	void redo() override;


private:
	QList<nap::ObjectPath> mOperatorPaths;
	QList<QPointF> mOriginalPositions;
	QPointF mDelta;
};

class ConnectPlugsCmd : public QUndoCommand
{
public:
	ConnectPlugsCmd(nap::OutputPlugBase& srcPlug, nap::InputPlugBase& dstPlug)
		: QUndoCommand(), mSrcPlugPath(srcPlug), mDstPlugPath(dstPlug)
	{
		setText(
			QString("Connect plugs '%1' to '%2'")
				.arg(QString::fromStdString(mSrcPlugPath), QString::fromStdString(mDstPlugPath)));
	}

	void undo() override;
	void redo() override;

private:
	nap::ObjectPath mSrcPlugPath;
	nap::ObjectPath mDstPlugPath;
};


class RemoveObjectCmd : public QUndoCommand
{
public:
	//	RemoveObjectCmd(const nap::Object& object) : QUndoCommand() { mObjectPaths.append(object); }
	RemoveObjectCmd(const QList<nap::Object*>& objects) : QUndoCommand()
	{
		setText("Delete objects");
		auto rootObjects = keepRoots(objects);

		for (const nap::Object* ob : rootObjects)
			mObjectPaths << ob;
	}


	void redo() override;
	void undo() override;

private:
	QList<nap::ObjectPath> mObjectPaths;
	QList<nap::ObjectPath> mParentPaths;
	QList<QString> mSerializedObjects;
};

class CreateOperatorCmd : public QUndoCommand
{
public:
	CreateOperatorCmd(const nap::Patch& patch, RTTI::TypeInfo operatorType, const QPointF& pos)
		: QUndoCommand(), mPatchPath(patch), mOpTypeName(operatorType.getName()), mPos(pos)
	{
		setText(QString("Create operator of type '%1' in '%2")
					.arg(QString::fromStdString(mOpTypeName), QString::fromStdString(mPatchPath)));
	}

	void redo() override;
	void undo() override;

private:
	const nap::ObjectPath mPatchPath;
	nap::ObjectPath mOperatorPath;
	std::string mOpTypeName;
	QPointF mPos;
	//    const std::string mOperatorTypeName;
};


// Create a component based on an parent and the type of component
class CreateComponentCmd : public QUndoCommand
{
public:
	CreateComponentCmd(const nap::Entity& parent, RTTI::TypeInfo componentType)
		: QUndoCommand(), mParentPath(parent), mComponentType(componentType.getName())
	{
		setText(
			QString("Create component of type '%1' on '%2'")
				.arg(QString::fromStdString(mComponentType), QString::fromStdString(mParentPath)));
	}

	void redo() override;
	void undo() override;

private:
	const nap::ObjectPath mParentPath;
	nap::ObjectPath mComponentPath;
	const std::string mComponentType;
};

// Add a child entity to an entity.
class CreateEntityCmd : public QUndoCommand
{

public:
	CreateEntityCmd(nap::Entity& parent) : QUndoCommand(), mParentPath(parent)
	{
		setText(QString("Create new Entity under '%1'").arg(QString::fromStdString(mParentPath)));
	}

	void undo() override;
	void redo() override;

private:
	const nap::ObjectPath mParentPath;
	nap::ObjectPath mEntityPath;
};

// Set the name of an object
class SetNameCmd : public QUndoCommand
{
public:
	SetNameCmd(nap::Object& object, const QString& newName)
		: QUndoCommand(), mOldObjectPath(object), mNewName(newName.toStdString())
	{
		setText(QString("Set name of '%1' to '%2'")
					.arg(QString::fromStdString(mOldObjectPath), newName));
	}

	void undo() override;
	void redo() override;

private:
	nap::ObjectPath mNewObjectPath;
	nap::ObjectPath mOldObjectPath;
	std::string mOldName;
	std::string mNewName;
};


class AddAttributeCmd : public QUndoCommand
{
public:
	AddAttributeCmd(nap::AttributeObject& obj) : QUndoCommand(), mObjectPath(obj)
	{
		setText(QString("Add attribute to '%1'").arg(QString::fromStdString(nap::ObjectPath(obj))));
	}

	void undo() override;
	void redo() override;

private:
	nap::ObjectPath mObjectPath;
	nap::ObjectPath mAttributePath;
};

class SetAttributeValueCmd : public QUndoCommand
{
public:
	SetAttributeValueCmd(nap::AttributeBase& attrib, const QString& value)
		: QUndoCommand(), mAttrPath(attrib), mNewValue(value)
	{
		mOldValue = attributeToString(attrib);
		setText(QString("Set value of '%1' to '%2'")
					.arg(QString::fromStdString(mAttrPath.toString()), value));
	}

	void undo() override;
	void redo() override;


private:
	nap::ObjectPath mAttrPath;
	QString mNewValue;
	QString mOldValue;
};