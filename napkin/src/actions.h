#pragma once

#include "actionstore.h"
#include <QKeySequence>
#include <QString>
#include <nap/attributeobject.h>
#include <nap/object.h>



class DuplicateAction : public Action
{
public:
	DuplicateAction() : Action("Duplicate") { setCategory("Edit"); }
	virtual void perform() const override {}
};



class NewAction : public Action
{
public:
	NewAction() : Action("&New")
	{
		setShortcut(QKeySequence::New);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("File");
	}
	QString iconName() override { return "page_white"; }
	virtual void perform() const;
};



class OpenAction : public Action
{
public:
	OpenAction() : Action("Open...")
	{
		//		setShortcut(QKeySequence::Open);
		setShortcut(QKeySequence::Open);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("File");
	}
	virtual QString iconName() override { return "folder_page"; }
	virtual void perform() const;
};



class SaveAsAction : public Action
{
public:
	SaveAsAction() : Action("Save as...")
	{
		setShortcut(QKeySequence::SaveAs);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("File");
	}
	virtual QString iconName() override { return "disk"; }
	virtual void perform() const;
};



class SaveAction : public Action
{
public:
	SaveAction() : Action("Save")
	{
		setShortcut(QKeySequence::Save);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("File");
		//    setIcon(ICON(disk));
	}
	virtual QString iconName() override { return "disk"; }
	virtual void perform() const;
};


class CutAction : public Action
{
public:
	CutAction() : Action("Cut")
	{
		setShortcut(QKeySequence::Cut);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("Edit");
	}
	bool canActOn(const nap::Object& obj) const override { return true; }
	bool isAvailable();
	virtual QString iconName() override { return "cut_red"; }
	virtual void perform() const;
};


class CopyAction : public Action
{
public:
	CopyAction() : Action("Copy")
	{
		setShortcut(QKeySequence::Copy);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("Edit");
	}
	bool canActOn(const nap::Object& obj) const override { return true; }
	bool isAvailable();
	virtual QString iconName() override { return "page_copy"; }
	virtual void perform() const;
};

class PasteAction : public Action
{
public:
	PasteAction() : Action("Paste")
	{
		setShortcut(QKeySequence::Paste);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("Edit");
	}
	virtual QString iconName() override { return "page_paste"; }

	bool canActOn(const nap::Object& obj) const override { return true; }
	virtual void perform() const;
};

class DeleteAction : public Action
{
public:
	DeleteAction() : Action("Delete")
	{
		setShortcut(QKeySequence::Delete);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("Edit");
	}
	virtual QString iconName() override { return "delete"; }
	virtual bool canActOn(const nap::Object& obj) const override { return obj.getParentObject() != nullptr; }

	bool isAvailable();
	void perform() const;
};


class QuitAction : public Action
{
public:
	QuitAction() : Action("Quit")
	{
		setShortcut(QKeySequence::Quit);
		setShortcutContext(Qt::ApplicationShortcut);
		setCategory("File");
	}
	virtual void perform() const;
};

class UndoAction : public Action
{
public:
	UndoAction() : Action("Undo")
	{
		setShortcut(QKeySequence::Undo);
		setCategory("Edit");
	}
	bool isAvailable();
	virtual QString iconName() override { return "arrow_undo"; }
	virtual void perform() const;
};

class RedoAction : public Action
{
public:
	RedoAction() : Action("Redo")
	{
		setShortcut(QKeySequence::Redo);
		setCategory("Edit");
	}
	bool isAvailable();
	virtual QString iconName() override { return "arrow_redo"; }
	virtual void perform() const;
};


class AddAttributeAction : public Action
{
public:
	AddAttributeAction() : Action("Add Attribute") { setCategory("Edit"); }
	bool isAvailable();
	virtual bool canActOn(const nap::Object& obj) const override
	{
		return obj.getTypeInfo().isKindOf<nap::AttributeObject>();
	}
	virtual void perform() const;
};

class CreateEntityAction : public Action
{
public:
	CreateEntityAction() : Action("Create Entity") { setCategory("Edit"); }
	bool isAvailable();
	bool canActOn(const nap::Object& obj) const override { return obj.getTypeInfo().isKindOf<nap::Entity>(); }
	virtual void perform() const;
};

class CreateOperatorAction : public Action
{
public:
	CreateOperatorAction() : Action("Create Operator...") { setCategory("Edit"); }
	virtual bool canActOn(const nap::Object& obj) const override
	{
		return obj.getTypeInfo().isKindOf<nap::Patch>();
	}
	virtual void perform() const;
};

class CreateComponentAction : public Action
{
public:
	CreateComponentAction() : Action("Create Component...") { setCategory("Edit"); }
	bool isAvailable();
	bool canActOn(const nap::Object& obj) const override { return obj.getTypeInfo().isKindOf<nap::Entity>(); }
	void perform() const;
};
