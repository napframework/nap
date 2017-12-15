#pragma once


#include <entity.h>
#include <nap/core.h>

#include "thememanager.h"
#include <QApplication>
#include <QObject>
#include <QUndoCommand>
#include <generic/resourcefactory.h>
#include <rtti/rttideserializeresult.h>
#include <rtti/rttiutilities.h>
#include <vector>

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
		nap::Core& getCore()
		{
			return mCore;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// File operations
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/**
		 * Destroy existing data and reset the filename. newFileCreated will be when this happens.
		 */
		void newFile();

		/**
		 * Load the specified file and replace the currently existing Objects.
		 * fileOpened will be called immediately when the loaded data is available.
		 * Any failures will be reported through nap::Logger
		 *
		 * @param filename The file to load, can be absolute or relative to the current working directory.
		 */
		void loadFile(const QString& filename);

		/**
		 * Save the current data to disk using the currently set filename.
		 * If no filename has been set, this method will do nothing.
		 * The filename can be set by invoking saveFileAs(const QString& filename) before calling this method.
		 * Any failures will be reported through nap::Logger, recovery should be handled prior to calling this method.
		 */
		void saveFile();

		/**
		 * Save the current data to disk.
		 * Any failures will be reported through nap::Logger
		 * @param filename The file to save the data to.
		 */
		void saveFileAs(const QString& filename);

		/**
		 * (Re-)open the file that was opened last. Uses local user settings to persist the filename.
		 */
		void openRecentFile();

		/**
		 * @return The path of the file that was opened last.
		 */
		const QString getLastOpenedFilename();

		/**
		 * TODO: This should be in a Document object
		 * @return The name of the currently opened file
		 * or an empty string if no file is open or the data hasn't been saved yet.
		 */
		const QString& getCurrentFilename()	{ return mCurrentFilename; }

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Data operations
		// TODO: Data structure operations should be accessed on the objects themselves
		// TODO: Properly name "objects", this is probably to broad of a term here
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		/**
		 * TODO: This should be in a Document object
		 * @return All the objects (resources?) that are currently loaded.
		 */
		nap::rtti::OwnedObjectList& getObjects() { return mObjects; }

		/**
		 * TODO: This should be in a Document object
		 * @return All the objects (resources?) that are currently loaded.
		 */
		nap::rtti::ObjectList getObjectPointers();

		/**
		 * TODO: This should be in a Document object
		 * Retrieve an (data) object by name/id
		 * @param name The name/id of the object to find
		 * @return The found object or nullptr if none was found
		 */
		nap::rtti::RTTIObject* getObject(const std::string& name);

		/**
		 * TODO: This should be in a Document object
		 * Get an object by name and type
		 */
		nap::rtti::RTTIObject* getObject(const std::string& name, const rttr::type& type);

		/**
		 * TODO: This should be in a Document object
		 * Get an object by name and type
		 */
		template<typename T>
		T* getObjectT(const std::string& name) { return rtti_cast<T>(getObject(name)); }

		/**
		 * TODO: This should be in a Document object
		 * Get all objects of the specified type
		 * @param type The type of objects to get
		 * @return All objects of the specified type, including derived types
		 */
		std::vector<nap::rtti::RTTIObject*> getObjectsOfType(const nap::rtti::TypeInfo& type) const;

		/**
		 * TODO: This should be in a Document object
		 * Retrieve the parent of the specified Entity
		 * @param entity The entity to find the parent from.
		 * @return The provided Entity's parent or nullptr if the Entity has no parent.
		 */
		nap::Entity* getParent(const nap::Entity& entity);

		/**
		 * TODO: This should be in a Document object
		 * Retrieve the Entity the provided Component belongs to.
		 * @param component The component of which to find the owner.
		 * @return The owner of the component
		 */
		nap::Entity* getOwner(const nap::Component& component);

		/**
		 * TODO: This should be in a Document object
		 * Create an entity. Its name/id will be automatically generated.
		 * @param parent The parent under which to create the Entity,
		 *      provide nullptr if the Entity should have no parent.
		 * @return The newly created entity.
		 */
		nap::Entity* createEntity(nap::Entity* parent = nullptr);

		/**
		 * TODO: This should be in a Document object
		 * Add a component of the specified type to an Entity.
		 * @param entity The entity to add the component to.
		 * @param type The type of the desired component.
		 * @return The newly created component.
		 */
		nap::Component* addComponent(nap::Entity& entity, rttr::type type);

		/**
		 * TODO: This should be in a Document object
		 * Add an object of the specified type.
		 * @param type The type of the desired object.
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 * @return The newly created object
		 */
		nap::rtti::RTTIObject* addObject(rttr::type type, bool selectNewObject);

		/**
		 * TODO: This should be in a Document object
		 * Obliterate the specified object
		 * @param object The object to be deleted.
		 */
		void deleteObject(nap::rtti::RTTIObject& object);

		/**
		 * TODO: This should be in a Document object
		 * Execute the specified command and push the provided command onto the undostack.
		 * @param cmd The command to be executed
		 */
		void executeCommand(QUndoCommand* cmd);

		/**
		 * Convenience method to retrieve this QApplication's instance.
		 * @return The QApplication singleton.
		 */
		QApplication* getQApplication()
		{
			return dynamic_cast<QApplication*>(qGuiApp);
		}

		/**
		 * @return The currently used undostack, @see QUndoStack
		 */
		QUndoStack& getUndoStack()
		{
			return mUndoStack;
		}

		/**
		 * @return Access to the current application's ThemeManage
		 */
		ThemeManager& getThemeManager()
		{
			return mThemeManager;
		}

		// To be invoked after the application has shown
		void restoreUI();

		/**
		 * The resource factory can be used to grab icons per object type for example
		 */
		const ResourceFactory& getResourceFactory() const { return mResourceFactory; }

	Q_SIGNALS:
		/**
		 * Qt Signal
		 * Fired when the global selection has changed.
		 * TODO: This will need to be changed into a multi-level/hierarchical selection context
		 */
		void selectionChanged(const std::vector<nap::rtti::RTTIObject*> obj);

		/**
		 * Qt Signal
		 * Fired after a file has been opened and its objects made available.
		 * @param filename Name of the file that was opened
		 */
		void fileOpened(const QString& filename);

		/**
		 * Qt Signal
		 * Fires after a file has finished saving.
		 * @param filename The file the data was saved to.
		 */
		void fileSaved(const QString& filename);

		/**
		 * Qt Signal
		 * Fired after a new file has been created and the data is erased.
		 */
		void newFileCreated();

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
		void propertyValueChanged(nap::rtti::RTTIObject& object, const nap::rtti::RTTIPath& path);

	private:
		AppContext();

		/**
		 * TODO: This should live in a Document class
		 * @param suggestedName
		 * @return
		 */
		std::string getUniqueName(const std::string& suggestedName);

		nap::rtti::OwnedObjectList mObjects; // All the objects
		QUndoStack mUndoStack;				 // The undostack of the application
		QString mCurrentFilename;			 // The current filename
		nap::Core mCore;					 // The nap::Core
		ThemeManager mThemeManager;			 // The theme manager
		ResourceFactory mResourceFactory;	// Le resource factory
	};
};