#pragma once

#include <vector>

#include <QApplication>
#include <QObject>
#include <QUndoCommand>
#include <QMainWindow>

#include <rtti/deserializeresult.h>
#include <rtti/rttiutilities.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <entity.h>

#include "thememanager.h"
#include "document.h"
#include "generic/resourcefactory.h"

namespace napkin
{

	/**
	 * The AppContext (currently a singleton) holds the 'globally' kept application state. All authored objects reside
	 * here.
	 * It has signals to notify the other application components of global state changes such as data file access and
	 * provides the client with convenience methods that may change the application state.
	 *
	 * This class currently acts much like a model in MVC:
	 * Operations on the data should happen through AppContext
	 * such that the rest of the application can react and update accordingly.
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
        static void create();
        
        /**
         * Destruct the singleton
         * In order to avoid order of destruction problems with ObjectPtrManager the app context has to be explicitly created and destructed.
         */
        static void destroy();
        
        AppContext(); // Alas, this has to be public to be able to support the singleton unique_ptr construction

		AppContext(AppContext const&) = delete;

		void operator=(AppContext const&) = delete;

		~AppContext() override;

		/**
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
		void saveDocument();

		/**
		 * Save the current data to disk.
		 * Any failures will be reported through nap::Logger
		 * @param filename The file to save the data to.
		 */
		void saveDocumentAs(const QString& filename);

		/**
		 * Serialize the current document to a string
		 * @return The document, serialized to string
		 */
		std::string documentToString() const;

		/**
		 * (Re-)open the file that was opened last. Uses local user settings to persist the filename.
		 */
		void openRecentDocument();

		/**
		 * @return The path of the file that was opened last.
		 */
		const QString getLastOpenedFilename();

		/**
		 * @return The current document
		 */
		Document* getDocument();

		/**
		 * @return The current document or nullptr if there is no document
		 */
		const Document* getDocument() const;

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

	Q_SIGNALS:
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
		 * Will be used to relay thread-unsafe nap::Logger calls onto the Qt UI thread
		 * @param msg The log message being handled
		 */
		void logMessage(nap::LogMessage msg);

	private:

		/**
		 * Whenever a new document is created/loaded, register its signals for listeners
		 */
		void connectDocumentSignals();

		/**
		 * When a new document has been set
		 */
		void onUndoIndexChanged();

		// Slot to relay nap log messages into a Qt Signal (for thread safety)
		nap::Slot<nap::LogMessage> mLogHandler = { this, &AppContext::logMessage };

		nap::Core mCore;						// The nap::Core
		bool mCoreInitialized = false;			// Keep track of core initialization state
		ThemeManager mThemeManager;			 	// The theme manager
		ResourceFactory mResourceFactory;		// Le resource factory
		std::unique_ptr<Document> mDocument = nullptr; 			// Keep objects here
	};
};
