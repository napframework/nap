/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "appcontext.h"
#include "napkinglobals.h"

#include <QAction>
#include <QFileDialog>
#include <QKeySequence>
#include <QSet>
#include <QStandardItem>
#include <QString>
#include <QUndoCommand>
#include <entity.h>
#include <nap/logger.h>

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
		Action();

	protected:
		/**
		 * This method will be called if the action is triggered
		 */
		virtual void perform() = 0;
	};

	/**
	 * Base class for action that operates specifically on a QStandardItem
	 */
	class StandardItemAction : public Action
	{
	public:
		virtual bool isValidFor(const QStandardItem& item) { return true; }
	};

	/**
	 * Create a new file.
	 */
	class NewFileAction : public Action
	{
	public:
		NewFileAction();

	private:
		/**
		 * Implemented from Action
		 */
		void perform() override;
	};

	/**
	 * Display a file open dialog and open the file if confirmed.
	 */
	class OpenProjectAction : public Action
	{

	public:
		OpenProjectAction();

	private:
		/**
		 * Implemented from Action
		 */
		void perform() override;
	};

	class ReloadFileAction : public Action
	{
	public:
		ReloadFileAction();

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
		/**
		 * Implemented from Action
		 */
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
		/**
		 * Implemented from Action
		 */
		void perform() override;
	};

	/**
	 * Create a Resource
	 */
	class CreateResourceAction : public StandardItemAction
	{
	public:
		explicit CreateResourceAction();

	private:
		void perform() override;
	};

	/**
	 * Create an Entity
	 */
	class CreateEntityAction : public StandardItemAction
	{
	public:
		explicit CreateEntityAction();

	private:
		void perform() override;
	};

	/**
	 * Add an Entity as child of another Entity
	 */
	class AddChildEntityAction : public StandardItemAction
	{
	public:
		explicit AddChildEntityAction(nap::Entity& entity);

	private:
		void perform() override;

		nap::Entity* entity;
	};

	/**
	 * Add a Component to an Entity
	 */
	class AddComponentAction : public StandardItemAction
	{
	public:
		explicit AddComponentAction(nap::Entity& entity);

	private:
		void perform() override;

		nap::Entity* entity;
	};

	/**
	 * Delete a set of objects
	 */
	class DeleteObjectAction : public StandardItemAction
	{
	public:
        /**
         * @param object The object to delete
         */
		explicit DeleteObjectAction(nap::rtti::Object& object);

	private:
		/**
		 * Implemented from Action
		 */
		void perform() override;

		nap::rtti::Object& mObject;
	};

	/**
	 * Remove a child Entity from its parent
	 */
	class RemoveChildEntityAction : public StandardItemAction
	{
	public:
		explicit RemoveChildEntityAction(EntityItem& entityItem);

	private:
		void perform() override;

		EntityItem* entityItem;
	};

	/**
	 * Remove something defined by the propertypath
	 */
	class RemovePathAction : public Action
	{
	public:
		explicit RemovePathAction(const PropertyPath& path);

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
        /**
         * @param themeName The theme to set
         */
        explicit SetThemeAction(const QString& themeName);

	private:
		/**
		 * Implemented from Action
		 */
		void perform() override;

		QString mTheme; // The theme to set
	};

}