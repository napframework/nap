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
	 * Filter the provided list of objects
	 * @param objects
	 * @param topLevelObjects
	 */
	nap::rtti::ObjectList topLevelObjects(const nap::rtti::ObjectList& objects);
}