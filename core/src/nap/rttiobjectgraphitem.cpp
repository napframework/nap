/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rttiobjectgraphitem.h"
#include "rtti/rttiutilities.h"
#include "rtti/object.h"
#include "utility/errorstate.h"

namespace nap
{
	/**
	 * Creates a graph item.
	 * @param object Object to wrap in the item that is created.
	 */
	const RTTIObjectGraphItem RTTIObjectGraphItem::create(rtti::Object* object)
	{
		RTTIObjectGraphItem item;
		item.mType = EType::Object;
		item.mObject = object;
			
		return item;
	}


	/**
	 * @return ID of the item. For objects, the ID is the object ID, for files, it is the filename.
	 */
	const std::string RTTIObjectGraphItem::getID() const
	{
		assert(mType == EType::File || mType == EType::Object);

		if (mType == EType::File)
			return mFilename;
		else 
			return mObject->mID;
	}


	/**
	 * Performs rtti traversal of pointers to both files and objects.
	 * @param pointees Output parameter, contains all objects and files this object points to.
	 * @param errorState If false is returned, contains information about the error.
	 * @return true is succeeded, false otherwise.
	 */
	bool RTTIObjectGraphItem::getPointees(std::vector<RTTIObjectGraphItem>& pointees, utility::ErrorState& errorState) const
	{
		std::vector<rtti::ObjectLink> object_links;
		rtti::findObjectLinks(*mObject, object_links);

		for (const rtti::ObjectLink& link : object_links)
		{
			if (link.mTarget == nullptr)
				continue;

			RTTIObjectGraphItem item;
			item.mType				= EType::Object;
			item.mObject			= link.mTarget;
			pointees.push_back(item);
		}

		std::vector<std::string> file_links;
		rtti::findFileLinks(*mObject, file_links);

		for (std::string& filename : file_links)
		{
			RTTIObjectGraphItem item;
			item.mType					= EType::File;
			item.mFilename				= filename;
			pointees.push_back(item);
		}

		return true;
	}
}