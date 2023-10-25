/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "appcontext.h"
#include "napkinglobals.h"
#include "napkin-resources.h"

#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QSet>
#include <QStandardItem>
#include <QString>
#include <QUrl>
#include <QUndoCommand>
#include <entity.h>
#include <nap/logger.h>
#include <nap/group.h>
#include <shader.h>

namespace napkin
{
	class EntityItem;

	/**
	 * Base class for actions. Each subclass must implement the perform() method in which the actual work will be done.
	 * In many cases perform() will create an instance of an appropriate command and execute it.
	 */
	class Action : public QAction
	{
	public:
		/**
		 * @param parent parent of action, nullptr if there is no parent
		 * @param text action description
		 * @param iconName name of icon to load, nullptr if there is no icon
		 */
		Action(QObject* parent, const char* text, const char* iconName);
	protected:
		/**
		 * This method will be called if the action is triggered
		 */
		virtual void perform() = 0;

	private:
		void loadIcon();
		void onThemeChanged(const Theme* theme)		{ loadIcon(); }
		QString mIconName;
	};


	/**
	 * Create a new file.
	 */
	class NewFileAction : public Action
	{
	public:
		NewFileAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Display a file open dialog and open the file if confirmed.
	 */
	class OpenProjectAction : public Action
	{
	public:
		OpenProjectAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Reload current data file
	 */
	class ReloadFileAction : public Action
	{
	public:
		ReloadFileAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Save the currently open file, show a save file dialog if the file wasn't saved before.
	 */
	class SaveFileAction : public Action
	{
	public:
		SaveFileAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Present a save file dialog and store the file if confirmed.
	 */
	class SaveFileAsAction : public Action
	{
	public:
		SaveFileAsAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Presents a load file dialog, to load a different data file
	 */
	class OpenFileAction : public Action
	{
	public:
		OpenFileAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Updates project data path to point to current loaded document
	 */
	class UpdateDefaultFileAction : public Action
	{
	public:
		UpdateDefaultFileAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Creates a service configuration
	 */
	class NewServiceConfigAction : public Action
	{
	public:
		NewServiceConfigAction(QObject* parent);
	private:
		void perform();
	};


	/**
	 * Saves a service configuration to disk
	 */
	class SaveServiceConfigAction : public Action
	{
	public: 
		SaveServiceConfigAction(QObject* parent);
	private:
		void perform();
	};


	/**
	 * Saves a service configuration to disk
	 */
	class SaveServiceConfigurationAs : public Action
	{
	public:
		SaveServiceConfigurationAs(QObject* parent);
	private:
		void perform();
	};


	/**
	 * Open a service configuration from disk
	 */
	class OpenServiceConfigAction : public Action
	{
	public:
		OpenServiceConfigAction(QObject* parent);
	private:
		void perform();
	};


	/**
	 * Sets current service configuration to be project default
	 */
	class SetAsDefaultServiceConfigAction : public Action
	{
	public:
		SetAsDefaultServiceConfigAction(QObject* parent);
	private:
		void perform();
	};


	/**
	 * Create a Resource
	 */
	class CreateResourceAction : public Action
	{
	public:
		CreateResourceAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Create a Resource Group 
	 */
	class CreateGroupAction : public Action
	{
	public:
		CreateGroupAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Add a new resource to a group
	 */
	class AddNewResourceToGroupAction : public Action
	{
	public:
		AddNewResourceToGroupAction(QObject* parent, nap::IGroup& group);
	private:
		void perform() override;
		nap::IGroup* mGroup = nullptr;
	};


	/**
	 * Add a new group to a group
	 */
	class AddChildGroupAction : public Action
	{
	public:
		AddChildGroupAction(QObject* parent, nap::IGroup& group);
	private:
		void perform() override;
		nap::IGroup* mGroup = nullptr;
	};


	/**
	 * Parents a resource under a to be selected group
	 * @param resource resource to parent under a group
	 * @param currentGroup current parent group, nullptr if there is no parent
	 */
	class MoveResourceToGroupAction : public Action
	{
	public:
		explicit MoveResourceToGroupAction(QObject* parent, nap::rtti::Object& resource, nap::IGroup* currentGroup);
	private:
		void perform() override;
		nap::IGroup* mCurrentGroup = nullptr;
		nap::rtti::Object* mObject = nullptr;
	};


	/**
	 * Parents a group under a different group
	 * @param group group to move
	 * @param parentGroup current parent group, nullptr if there is no parent
	 */
	class MoveGroupAction : public Action
	{
	public:
		explicit MoveGroupAction(QObject* parent, nap::IGroup& group, nap::IGroup* parentGroup);
	private:
		void perform() override;
		nap::IGroup* mGroup = nullptr;
		nap::IGroup* mParentGroup = nullptr;
	};


	/**
	 * Add an existing resource to a group.
	 * If the resource is not specified, a dialog to select a resource is presented.
	 * If the group is not specified, a dialog to select the group is presented
	 */
	class AddExistingResourceToGroupAction : public Action
	{
	public:
		explicit AddExistingResourceToGroupAction(QObject* parent, nap::IGroup& group);
	private:
		void perform() override;
		nap::IGroup* mGroup = nullptr;
	};


	/**
	 * Removes a resource from a group, moving it to the root of the document
	 */
	class RemoveResourceFromGroupAction : public Action
	{
	public:
		explicit RemoveResourceFromGroupAction(QObject* parent, nap::IGroup& group, nap::rtti::Object& resource);
		void perform() override;
	private:
		nap::IGroup* mGroup = nullptr;
		nap::rtti::Object* mObject = nullptr;
	};


	/**
	 * Removes a resource from a group, moving it to the root of the document
	 */
	class RemoveGroupFromGroupAction : public Action
	{
	public:
		explicit RemoveGroupFromGroupAction(QObject* parent, nap::IGroup& group, nap::rtti::Object& resource);
		void perform() override;
	private:
		nap::IGroup* mGroup = nullptr;
		nap::rtti::Object* mObject = nullptr;
	};


	/**
	 * Create an Entity
	 */
	class CreateEntityAction : public Action
	{
	public:
		explicit CreateEntityAction(QObject* parent);
	private:
		void perform() override;
	};


	/**
	 * Add an Entity as child of another Entity
	 */
	class AddChildEntityAction : public Action
	{
	public:
		explicit AddChildEntityAction(QObject* parent, nap::Entity& entity);
	private:
		void perform() override;
		nap::Entity* mEntity;
	};


	/**
	 * Add a Component to an Entity
	 */
	class AddComponentAction : public Action
	{
	public:
		explicit AddComponentAction(QObject* parent, nap::Entity& entity);
	private:
		void perform() override;
		nap::Entity* mEntity;
	};

	/**
	 * Delete a single object.
	 * If another object points to the object to delete, ask the user for confirmation.
	 */
	class DeleteObjectAction : public Action
	{
	public:
		explicit DeleteObjectAction(QObject* parent, nap::rtti::Object& object);
	private:
		void perform() override;
		nap::rtti::Object& mObject;
	};


	/**
	 * Compile a shader.
	 */
	class LoadShaderAction : public Action
	{
	public:
		explicit LoadShaderAction(QObject* parent, nap::BaseShader& object);
	private:
		void perform() override;
		nap::BaseShader& mShader;
	};


	/**
	 * Delete a group, including all children in the group.
	 * If another object points to any of the children in the group, ask the user for confirmation.
	 */
	class DeleteGroupAction : public Action
	{
	public:
		explicit DeleteGroupAction(QObject* parent, nap::IGroup& group);
	private:
		void perform() override;
		nap::IGroup& mGroup;
	};


	/**
	 * Remove a child Entity from its parent
	 */
	class RemoveChildEntityAction : public Action
	{
	public:
		explicit RemoveChildEntityAction(QObject* parent, EntityItem& entityItem);
	private:
		void perform() override;
		EntityItem* mEntityItem;
	};


	/**
	 * Remove something defined by the propertypath
	 */
	class RemovePathAction : public Action
	{
	public:
		explicit RemovePathAction(QObject* parent, const PropertyPath& path);
	private:
		void perform() override;
		PropertyPath mPath;
	};


	/**
	 * Change the current theme. The name must match a theme name defined in the ThemeManager
	 */
	class SetThemeAction : public Action
	{
	public:
        explicit SetThemeAction(QObject* parent, const QString& themeName);
	private:
		void perform() override;
		QString mTheme;	// The theme to set
	};


	/**
	 * Open URL in browser
	 */
	class OpenURLAction : public Action
	{
	public:
        explicit OpenURLAction(QObject* parent, const char* text, const QUrl& address);
	private:
		void perform() override;
		QUrl mAddress;
	};
}
