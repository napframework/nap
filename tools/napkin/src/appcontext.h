/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once


#include "thememanager.h"
#include "document.h"
#include "resourcefactory.h"

#include <vector>
#include <QApplication>
#include <QObject>
#include <QProgressDialog>
#include <QUndoCommand>
#include <QMainWindow>

#include <nap/projectinfo.h>
#include <rtti/deserializeresult.h>
#include <rtti/rttiutilities.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <entity.h>
#include <QtCore/QSettings>

namespace napkin
{

	/**
	 * The AppContext (currently a singleton) holds the 'globally' kept application state. 
	 * All authored objects reside here.
	 * It has signals to notify the other application components of global state changes such as data file access and
	 * provides the client with convenience methods that may change the application state.
	 *
	 * This class currently acts much like a model in MVC: Operations on the data should happen through AppContext,
	 * such that the rest of the application can react and update accordingly.
	 *
	 * The app context manages nap::Core, it is therefore required that the entire context needs to be destroyed if core needs to be re-initialized.
	 * In other words: for every project that you want to load a new app context needs to be created and the old needs to be destroyed.
	 * Unfortunately this is necessary because core sources dynamic libraries that at this point in time can't be freed properly on all systems.
	 *
	 * The context points to a document that is manipulated by the editor. 
	 * If core fails to initialize, creation and manipulation of resources won't work. 
	 * Core is initialized when a project is loaded.
	 * 
	 * TODO: Data manipulation methods and signals should really live in their own class.
	 */
	class AppContext : public QObject
	{
		Q_OBJECT

	public:
		/**
		 * Singleton accessor
		 * @return The single instance of this class
		 */
		static AppContext& get();

		/**
		 * Construct the singleton
		 * In order to avoid order of destruction problems with ObjectPtrManager the app context has to be explicitly created and destructed.
		 */
		static AppContext& create();

		/**
		 * Destruct the singleton
		 * In order to avoid order of destruction problems with ObjectPtrManager the app context has to be explicitly created and destructed.
		 */
		static void destroy();

		/**
		 * @return if the app context is available
		 */ 
		static bool isAvailable();

		AppContext(); // Alas, this has to be public to be able to support the singleton unique_ptr construction

		AppContext(AppContext const&) = delete;

		void operator=(AppContext const&) = delete;

		~AppContext() override;

		/**
		 * Returns the instance of core managed by this context.
		 * @return The single nap::Core instance held by this AppContext
		 */
		nap::Core& getCore();

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// File operations
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/**
		 * Destroy existing data and reset the filename. newFileCreated will be when this happens.
		 */
		Document* newDocument();

		/**
		 * Load the specified file and replace the currently existing Objects.
		 * fileOpened will be called immediately when the loaded data is available.
		 * Any failures will be reported through nap::Logger
		 *
		 * @param filename The file to load, can be absolute or relative to the current working directory.
		 */
		Document* loadDocument(const QString& filename);

		/**
		 * Loads all service configurations.
		 */
		void loadServiceConfigs(const nap::ProjectInfo& projectInfo);

		/**
		 * Load the specified project into the application context
		 * @param projectFilename The json file that contains the project's definition/dependencies/etc
		 * @return A pointer to the loaded project info or nullptr when loading failed
		 */
		const nap::ProjectInfo* loadProject(const QString& projectFilename);

		/**
		 * @return The currently loaded project or a nullptr when no project is loaded
		 */
		const nap::ProjectInfo* getProjectInfo() const;

		/**
		 * Reload the current document from disk
		 */
		void reloadDocument();

		/**
		 * Load a json string as document
		 * @param data The json data to load.
		 * @return A Document instance if loading succeeded, nullptr otherwise
		 */
		Document* loadDocumentFromString(const std::string& data, const QString& filename = "");

		/**
		 * Save the current data to disk using the currently set filename.
		 * If no filename has been set, this method will do nothing.
		 * The filename can be set by invoking saveFileAs(const QString& filename) before calling this method.
		 * Any failures will be reported through nap::Logger, recovery should be handled prior to calling this method.
		 */
        bool saveDocument();

		/**
		 * Save the current data to disk.
		 * Any failures will be reported through nap::Logger
		 * @param filename The file to save the data to.
		 */
        bool saveDocumentAs(const QString& filename);

		/**
		 * Serialize the current document to a string
		 * @return The document, serialized to string
		 */
		std::string documentToString() const;

		/**
		 * (Re-)open the file that was opened last. Uses local user settings to persist the filename.
		 */
		void openRecentProject();

		/**
		 * @return The path of the file that was opened last.
		 */
		const QString getLastOpenedProjectFilename();

		/**
		 * Add a filename to the recently opened file list or bump an existing filename to the top
		 */
		void addRecentlyOpenedProject(const QString& filename);

		/**
		 * @return The list of recently opened project files
		 */
		QStringList getRecentlyOpenedProjects() const;

		/**
		 * Returns the currently loaded document, attempts to create it if it doesn't exist.
		 * Returns a nullptr if the document isn't available, this is the case when no project has been loaded.
		 * @return The current document, creates it if it doesn't exist.
		 */
		Document* getDocument();

		/**
		 * Returns the currently loaded document, attempts to create it if it doesn't exist.
		 * Returns a nullptr if the document isn't available, this is the case when no project has been loaded.
		 * @return The current document or nullptr if there is no document
		 */
		const Document* getDocument() const;

		/**
		 * @return if there is a document currently loaded
		 */
		bool hasDocument() const;

		/**
		 * If there's a file associated with the current service configuration
		 */
		bool hasServiceConfig() const;

		/**
		 * Returs file name associated with service configuration, can be null.
		 * @return service config file name, can be null
		 */
		const QString& getServiceConfigFilename() const;

		/**
		 * Updates service configurations
		 */
		void newServiceConfig();

		/**
		 * Saves service configuration to disk
		 */
		bool saveServiceConfig();

		/**
		 * Saves service configuration to disk using given filename
		 */
		bool saveServiceConfigAs(const QString& fileName);

		/**
		 * Convenience method to retrieve this QApplication's instance.
		 * @return The QApplication singleton.
		 */
		QApplication* getQApplication() const { return dynamic_cast<QApplication*>(qGuiApp); }

		/**
		 * @return The currently used undostack, @see QUndoStack
		 */
		QUndoStack& getUndoStack() { return getDocument()->getUndoStack(); }

		/**
		 * @return Access to the current application's ThemeManage
		 */
		ThemeManager& getThemeManager() { return mThemeManager; }

		/**
		 * @param command THe command to be executed on the current document
		 */
		void executeCommand(QUndoCommand* cmd) { getDocument()->executeCommand(cmd); }

		/**
		 * To be invoked after the application has shown
		 */
		void restoreUI();

		/**
		 * The resource factory can be used to grab icons per object type for example
		 */
		const ResourceFactory& getResourceFactory() const { return mResourceFactory; }

		/**
		 * Retrieve the application's main window
		 */
		QMainWindow* getMainWindow() const;

		/**
		 * Take an URI for file paths or object/property links and handle accordingly.
		 * @param uri The URI to handle, can be a string like 'file://something' for example
		 */
		void handleURI(const QString& uri);

		/**
		 * Disable opening of project from recently opened file list on startup
		 */
		void setOpenRecentProjectOnStartup(bool b);

		/**
		 * Set to exit upon failure loading any project
		 */
		void setExitOnLoadFailure(bool b);

		/**
		 * Set to exit upon success loading any project
		 */
		void setExitOnLoadSuccess(bool b);

	Q_SIGNALS:
		/**
		 * Qt Signal
		 * Fired when nap::Core has been initialized
		 */
		void coreInitialized();

		/**
		 * Qt Signal
		 * Fired when the global selection has changed.
		 * TODO: This will need to be changed into a multi-level/hierarchical selection context
		 */
		void selectionChanged(QList<nap::rtti::Object*> obj);


		/**
		 * Qt Signal
		 * Fired when another property must be selected
		 */
		void propertySelectionChanged(PropertyPath prop);

		/**
		 * Qt Signal
		 * Fired after a file has been opened and its objects made available.
		 * @param filename Name of the file that was opened
		 */
		void documentOpened(QString filename);

		/**
		* Qt Signal
		* Fired after a file has been closed and its objects are destructed
		* @param filename Name of the file that was opened
		*/
		void documentClosing(QString doc);

		/**
		 * Qt Signal
		 * Fires after a document has finished saving.
		 * @param filename The file the data was saved to.
		 */
		void documentSaved(QString filename);

		/**
		 * Qt Signal
		 * Fired after a new document has been created and the data is erased.
		 */
		void newDocumentCreated();

		/**
		 * Qt Signal
		 * Fired when something in the document has changed
		 */
		void documentChanged(Document* doc);

		/**
		 * Qt Signal
		 * Invoked when an Entity has been added to the system
		 * @param newEntity The newly added Entity
		 * @param parent The parent the new Entity was added to
		 */
		void entityAdded(nap::Entity* newEntity, nap::Entity* parent = nullptr);

		/**
		 * Qt Signal
		 * Invoked when a Component has been added to the system
		 * @param comp
		 * @param owner
		 */
		void componentAdded(nap::Component* comp, nap::Entity* owner);

		/**
		 * Qt Signal
		 * Invoked after any object has been added (this includes Entities)
		 * @param obj The newly added object
		 * TODO: Get rid of the following parameter, the client itself must decide how to react to this event.
		 * 		This is a notification, not a directive.
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 */
		void objectAdded(nap::rtti::Object* obj, bool selectNewObject);

		/**
		 * Qt Signal
		 * Invoked after an object has changed drastically
		 */
		void objectChanged(nap::rtti::Object* obj);

		/**
		 * Qt Signal
		 * Invoked just before an object is removed (including Entities)
		 * @param object The object about to be removed
		 */
		void objectRemoved(nap::rtti::Object* object);

		/**
		 * Qt Signal
		 * Invoked just after a property's value has changed
		 * @param object The object that has the changed property
		 * @param path The path to the property that has changed
		 */
		void propertyValueChanged(const PropertyPath path);

		/**
		 * Qt Signal
		 * Invoked just after a property child has been inserted
		 * @param path The path to the parent of the newly added child
		 * @param childIndex The index of the newly added child
		 */
		void propertyChildInserted(const PropertyPath& parentPath, size_t childIndex);

		/**
		 * Qt Signal
		 * Invoked just after a property child has been removed
		 * @param path The path to the parent of the newly added child
		 * @param childIndex The index of the child that was removed
		 */
		void propertyChildRemoved(const PropertyPath& parentPath, size_t childIndex);

		/**
		 * Will be used to relay thread-unsafe nap::Logger calls onto the Qt UI thread
		 * @param msg The log message being handled
		 */
		void logMessage(nap::LogMessage msg);

		/**
		 * Emits when an application-wide blocking operation started, progresses or finishes
		 * @param fraction How far we are along the process. A value of 0 is indeterminate, 1 means done and anything in-between means it's underway.
		 * @param message A short message describing what's happening.
		 */
		void blockingProgressChanged(float fraction, const QString& message = {});

	private:

		/**
		 * Whenever a new document is created/loaded, register its signals for listeners
		 */
		void connectDocumentSignals(bool connect = true);

		/**
		 * When a new document has been set
		 */
		void onUndoIndexChanged();

		/**
		 * Closes currently active document if there is one
		 */
		void closeDocument();

		// Slot to relay nap log messages into a Qt Signal (for thread safety)
		nap::Slot<nap::LogMessage> mLogHandler = { this, &AppContext::logMessage };

		nap::Core mCore;															// The nap::Core
		ThemeManager mThemeManager;			 										// The theme manager
		ResourceFactory mResourceFactory;											// Le resource factory
		bool mExitOnLoadFailure = false;											// Whether to exit on any project load failure
		bool mExitOnLoadSuccess = false;											// Whether to exit on any project load success
		bool mOpenRecentProjectAtStartup = true;									// Whether to load recent project at startup

		QString mCurrentFilename;													// The currently opened file
		std::unique_ptr<Document> mDocument = nullptr; 								// Keep objects here

		QString mCurrentConfigFilename;												// Current configuration filename
		std::vector<std::unique_ptr<nap::ServiceConfiguration>> mServiceConfigs;	// Current loaded service configuration
	};

};
