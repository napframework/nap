#pragma once

#include <QStandardItem>
#include <rtti/rtti.h>
#include <rtti/rttideserializeresult.h>
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
	T* findItemInModel(const QStandardItemModel& model, const nap::rtti::RTTIObject& obj, int column = 0)
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
	nap::rtti::ResolvedRTTIPath resolve(const nap::rtti::RTTIObject& obj, nap::rtti::RTTIPath path);

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
	nap::rtti::RTTIObject* getPointee(const PropertyPath& path);

	/**
	 * Given a Pointer Property (i like this name), set its pointee using a string.
	 */
	bool setPointee(const nap::rtti::RTTIObject& obj, const nap::rtti::RTTIPath& path, const std::string& target);

}