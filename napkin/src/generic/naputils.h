#pragma once

#include <QStandardItem>
#include <QFileInfo>

#include <rtti/rtti.h>
#include <rtti/deserializeresult.h>
#include <rtti/rttiutilities.h>

#include "propertypath.h"
#include "qtutils.h"

namespace napkin
{

	/**
	 * An item displaying an RTTI Type
	 */
	class RTTITypeItem : public QStandardItem
	{

	public:
		RTTITypeItem(const nap::rtti::TypeInfo& type);

	private:
		void refresh();

	private:
		const nap::rtti::TypeInfo& type;
	};


	/**
	 * Filter the provided list of objects
	 * @param objects
	 * @param topLevelObjects
	 */
	nap::rtti::ObjectList topLevelObjects(const nap::rtti::ObjectList& objects);


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

		findIndexInModel(model, [&model, &foundItem, &obj](const QModelIndex& idx) -> bool {
			QStandardItem* item = model.itemFromIndex(idx);
			if (item == nullptr)
				return false;

			auto objItem = dynamic_cast<T*>(item);
			if (objItem == nullptr)
				return false;

			if (objItem->getObject() == &obj)
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
	std::vector<rttr::type> getResourceTypes();

	/**
	 * Given a Pointer Property (or how do you call them), find the object it's pointing to
	 */
	nap::rtti::Object* getPointee(const PropertyPath& path);

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

}