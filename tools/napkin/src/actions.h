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

namespace napkin
{
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
	class OpenFileAction : public Action
	{

	public:
		OpenFileAction();

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
	 * Delete a set of objects
	 */
	class DeleteObjectAction : public Action
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

	private:
		nap::rtti::Object& mObject;
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