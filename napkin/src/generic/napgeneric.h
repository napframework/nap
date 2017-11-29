#pragma once

#include <QStandardItem>
#include <rtti/rtti.h>
#include <rtti/rttideserializeresult.h>
#include <rtti/rttiutilities.h>

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
	 * Given a complete list of objects, attempt to resolve the provided unresolved pointers
	 * @param objects The full list of objects to find the pointees in.
	 * @param unresolvedPointers The pointers to resolve
	 * @return True if the operation was successful, false otherwise.
	 */
	bool resolveLinks(const nap::rtti::OwnedObjectList& objects,
					  const nap::rtti::UnresolvedPointerList& unresolvedPointers);

	/**
	 * Filter the provided list of objects
	 * @param objects
	 * @param topLevelObjects
	 */
	nap::rtti::ObjectList topLevelObjects(const nap::rtti::ObjectList& objects);
}