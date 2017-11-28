#pragma once

#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QSet>
#include <QStandardItem>
#include <QString>
#include <QUndoCommand>
#include <entity.h>
#include <nap/logger.h>

#include "appcontext.h"
#include "napkinglobals.h"

/**
 * Base class for actions. Each subclass must implement the perform() method in which the actual work will be done.
 * In many cases perform() will create an instance of an appropriate command and execute it.
 */
class Action : public QAction
{
public:
	Action();

protected:
	virtual void perform() = 0;
};

/**
 * Create a new file.
 */
class NewFileAction : public Action
{
public:
	NewFileAction();

private:
	void perform() override;
};

/**
 * Display a file open dialog and open the file if confirmed.
 */
class OpenFileAction : public Action
{

public:
	OpenFileAction();

private:
	void perform() override;
};

/**
 * Save the currently open file, show a save file dialog if the file wasn't saved before.
 */
class SaveFileAction : public Action
{
public:
	SaveFileAction();

private:
	void perform() override;
};

/**
 * Present a save file dialog and store the file if confirmed.
 */
class SaveFileAsAction : public Action
{
public:
	SaveFileAsAction();

private:
	void perform() override;
};

/**
 * Add an entity to the provided parent.
 */
class AddEntityAction : public Action
{
public:
	AddEntityAction(nap::Entity* parent) : Action(), mParent(parent) { setText("Add Entity"); }

private:
	void perform() override { AppContext::get().createEntity(mParent); }

	nap::Entity* mParent;
};

/**
 * Add a component of the specified type to the provided Entity.
 */
class AddComponentAction : public Action
{
public:
	AddComponentAction(nap::Entity& entity, nap::rtti::TypeInfo type) : Action(), mEntity(entity), mComponentType(type)
	{
		setText(QString(type.get_name().data()));
	}

private:
	void perform() override { AppContext::get().addComponent(mEntity, mComponentType); }

private:
	nap::Entity& mEntity;
	nap::rtti::TypeInfo mComponentType;
};

/**
 * Add an object of the specified type to the ResourceManager.
 */
class AddObjectAction : public Action
{
public:
	AddObjectAction(rttr::type type) : Action(), mType(type) { setText(QString(type.get_name().data())); }

private:
	void perform() override { AppContext::get().addObject(mType); }

private:
	rttr::type mType;
};

/**
 * Delete a set of objects
 */
class DeleteObjectAction : public Action
{
public:
	DeleteObjectAction(nap::rtti::RTTIObject& object) : Action(), mObject(object) { setText("Delete"); }

private:
	void perform() override { AppContext::get().deleteObject(mObject); }

private:
	nap::rtti::RTTIObject& mObject;
};

/**
 * Change the current theme. The name must match a theme name defined in the ThemeManager
 */
class SetThemeAction : public Action
{
public:
	SetThemeAction(const QString& themeName) : Action(), mTheme(themeName)
	{
		setText(themeName.isEmpty() ? napkin::TXT_DEFAULT_THEME : themeName);
		setCheckable(true);
	}

private:
	void perform() override { AppContext::get().themeManager().setTheme(mTheme); }

	QString mTheme;
};