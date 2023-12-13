/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "propertypath.h"
#include "rttiitem.h"

#include <QStandardItem>
#include <QFileInfo>

#include <rtti/rtti.h>
#include <rtti/deserializeresult.h>
#include <rtti/rttiutilities.h>
#include <napqt/qtutils.h>
#include <shader.h>

namespace napkin
{
	/**
	* An item that displays an RTTI Type
	*/
	class RTTITypeItem : public RTTIItem
	{
		Q_OBJECT
	public:
		RTTITypeItem(const nap::rtti::TypeInfo& type);
		QVariant data(int role) const override;
	private:
		const nap::rtti::TypeInfo& mType;
	};


	/**
	* Flat list of objects
	*/
	class FlatObjectModel : public QStandardItemModel
	{
	public:
		FlatObjectModel(const std::vector<nap::rtti::Object*> objects);
	};

	using TypePredicate = std::function<bool(const rttr::type& type)>;

	/**
	 * @param type the class we want to find immediate derived types for
	 * @return all immediate derived types of the given class
	 */
	std::vector<rttr::type> getImmediateDerivedTypes(const rttr::type& type);

	/**
	 * Recurse all loaded subclasses of the specified type and write to stdout.
	 * @param type type to recurse and print
	 * @param indent additional information to print
	 */
	void dumpTypes(rttr::type type, const std::string& indent = "");

	/**
	 * Returns all top level, non-embedded, objects.
	 * @return all top level, non embedded, objects
	 */
	nap::rtti::ObjectSet topLevelObjects();

	/**
	 * Display a selection dialog with all available types, filtered by an optional predicate
	 * @param parent The widget to attach to
	 * @return The resulting selected type.
	 */
	nap::rtti::TypeInfo showTypeSelector(QWidget* parent, const TypePredicate& predicate);

	/**
	 * Display a selection dialog with all available objects, filtered by type T
	 * @param parent The parent widget to attach to.
	 * @param objects The objects to select from
	 * @return The selected object or nullptr if no object was selected
	 */
	nap::rtti::Object* showObjectSelector(QWidget* parent, const std::vector<nap::rtti::Object*>& objects);

	/**
	 * Display a selection dialog with all available objects, filtered by type T
	 * @param parent The parent widget to attach to.
	 * @param objects The objects to select from
	 * @return The selected object or nullptr if no object was selected
	 */
	nap::rtti::TypeInfo showMaterialSelector(QWidget* parent, const PropertyPath& prop, std::string& outName);

	/**
	 * Show a dialog box containing the given properties and a custom message.
	 * @param parent The parent widget to attach the dialog to
	 * @param props The properties to display in the dialog
	 * @param message The custom to display alongside the list of properties
	 */
	bool showPropertyListConfirmDialog(QWidget* parent, QList<PropertyPath> props, const QString& title,
									   QString message);

	/**
	 * Attempts to compile & load a shader
	 * @param shader the shader to load (compile)
	 * @param core core environment
	 * @param error contains the error
	 */
	bool loadShader(nap::BaseShader& shader, nap::Core& core, nap::utility::ErrorState& error);

	/**
	 * Traverse a model and find the QStandardItem subclass representing the specified object
	 * @param model The model to search
	 * @param condition The filter function that when it returns true, the traversal will stop and return the current
	 * index.
	 * @return The model index representing the item to be found.
	 */
	template<typename T>
	T* findItemInModel(const QStandardItemModel& model, const nap::rtti::Object& obj, int column = 0)
	{
		T* foundItem = nullptr;
		nap::qt::findIndexInModel(model, [&model, &foundItem, &obj](const QModelIndex& idx) -> bool {
			auto* item = qitem_cast<RTTIItem*>(model.itemFromIndex(idx));
			if (item == nullptr)
				return false;

			auto objItem = qobject_cast<T*>(item);
			if (objItem == nullptr)
				return false;

			if (&objItem->getObject() == &obj)
			{
				foundItem = objItem;
				return true;
			}
			return false;
		}, column);
		return foundItem;
	}

	/**
	 * Resolve a property path
	 */
	nap::rtti::ResolvedPath resolve(const nap::rtti::Object& obj, nap::rtti::Path path);

	/**
	 * @return All nap component types in the rtti system
	 */
	std::vector<rttr::type> getComponentTypes();

	/**
	 * @return All nap resource types in the rtti system
	 */
	std::vector<rttr::type> getTypes(TypePredicate predicate);

	/**
	 * Get the reference directory for resources.
	 * @param reference The path to the reference directory or file.
	 * 	If nothing is provided, this will fall back to the currently opened document
	 * @return An absolute path to the reference directory.
	 */
	QString getResourceReferencePath(const QString& reference = QString());

	/**
	 * Resolve a resource path to an absolute path
	 * @param relPath The file path relative to the JSON Document
	 * @param anchod The file the given relative path is relative to.
	 * 	If not provided, assume the anchor is the currently opened document.
	 * @return An absolute path to the provided filename
	 */
	QString getAbsoluteResourcePath(const QString& relPath, const QString& reference = QString());

	/**
	 * Make a file path relative to the given anchor path.
	 * @param absPath The absolute path to make relative
	 * @param reference The file path to make the given path relative to.
	 * 	If not provided, assume the anchor is the currently opened document.
	 * @return The file path relative to the given anchor.
	 */
	QString getRelativeResourcePath(const QString& absPath, const QString& reference = QString());


	/**
	 * Convert a filename to a file URI.
	 * @param filename The filename to convert
	 * @return A valid URI
	 */
	std::string toLocalURI(const std::string& filename);

	/**
	 * Convert a local file URI into an absolute path
	 *
	 */
	std::string fromLocalURI(const std::string& fileuri);

	/**
	 * Create an URI to an object
	 * @param object The object the URI should point to
	 * @return An URI
	 */
	std::string toURI(const nap::rtti::Object& object);

	/**
	 * Create an URI to a property
	 * @param path The property the URI should point to
	 * @return An URI
	 */
	std::string toURI(const PropertyPath& path);

	/**
	 * Make a type name more readable, for use in napkin when creating objects
	 * @param type
	 * @return
	 */
	std::string friendlyTypeName(rttr::type type);

	/**
	 * Compare two ComponentInstance paths and see if they represent the same component instance
	 * WARNING: This only works with absolute paths, paths that start from the root entity
	 *
	 * @param rootEntity The root entity to start from
	 * @param comp The component to compare
	 * @param a First path to compare
	 * @param b Second path to compare
	 * @return true if the paths represent the same instance, false otherwise
	 */
	bool isComponentInstancePathEqual(const nap::RootEntity& rootEntity, const nap::Component& comp,
									  const std::string& a, const std::string& b);


	/**
	 * Check if every character in this string is a number
	 * @return true if every character in this string is a number, false otherwise
	 */
	bool isNumber(const std::string& s);


	/**
	 * Split a "string:234" into its name "string" and index 234
	 * @return true if both are found, false if there's only a string part
	 */
	bool nameAndIndex(const std::string& nameIndex, std::string& name, int& index);

	/**
	 * Find a child of an entity using its name and disambiguating index
	 * @return The entity if found
	 */
	nap::Entity* findChild(nap::Entity& parent, const std::string& name, int index=-1);

}
