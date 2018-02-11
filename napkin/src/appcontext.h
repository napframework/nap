#pragma once


#include <entity.h>
#include <nap/core.h>

#include "thememanager.h"
#include "document.h"
#include <QApplication>
#include <QObject>
#include <QUndoCommand>
#include <generic/resourcefactory.h>
#include <rtti/rttideserializeresult.h>
#include <rtti/rttiutilities.h>
#include <vector>
#include <QtWidgets/QMainWindow>

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

	Q_SIGNALS:
		/**
		 * Qt Signal
		 * Fired when the global selection has changed.
		 * TODO: This will need to be changed into a multi-level/hierarchical selection context
		 */
		void selectionChanged(const std::vector<nap::rtti::RTTIObject*> obj);


		/**
		 * Qt Signal
		 * Fired when another property must be selected
		 */
		void propertySelectionChanged(const PropertyPath& prop);

		/**
		 * Qt Signal
		 * Fired after a file has been opened and its objects made available.
		 * @param filename Name of the file that was opened
		 */
		void documentOpened(const QString& filename);

		/**
		 * Qt Signal
		 * Fires after a document has finished saving.
		 * @param filename The file the data was saved to.
		 */
		void documentSaved(const QString& filename);

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

	Q_SIGNALS:
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
		void componentAdded(nap::Component& comp, nap::Entity& owner);

		/**
		 * Qt Signal
		 * Invoked after any object has been added (this includes Entities)
		 * @param obj The newly added object
		 * TODO: Get rid of the following parameter, the client itself must decide how to react to this event.
		 * 		This is a notification, not a directive.
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 */
		void objectAdded(nap::rtti::RTTIObject& obj, bool selectNewObject);

		/**
		 * Qt Signal
		 * Invoked after an object has changed drastically
		 */
		void objectChanged(nap::rtti::RTTIObject& obj);

		/**
		 * Qt Signal
		 * Invoked just before an object is removed (including Entities)
		 * @param object The object about to be removed
		 */
		void objectRemoved(nap::rtti::RTTIObject& object);

		/**
		 * Qt Signal
		 * Invoked just after a property's value has changed
		 * @param object The object that has the changed property
		 * @param path The path to the property that has changed
		 */
		void propertyValueChanged(const PropertyPath& path);

	private:
		AppContext();

		/**
		 * Whenever a new document is created/loaded, register its signals for listeners
		 */
		void connectDocumentSignals();

		/**
		 * When a new document has been set
		 */
		void onUndoIndexChanged();


		nap::Core mCore;						// The nap::Core
		bool mCoreInitialized = false;			// Keep track of core initialization state
		ThemeManager mThemeManager;			 	// The theme manager
		ResourceFactory mResourceFactory;		// Le resource factory
		std::unique_ptr<Document> mDocument = nullptr; 			// Keep objects here
	};
};