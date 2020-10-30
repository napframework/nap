/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "serviceobjectgraphitem.h"
#include "core.h"

namespace nap
{
	const nap::ServiceObjectGraphItem ServiceObjectGraphItem::create(Service* service, std::vector<Service*>* services)
	{
		ServiceObjectGraphItem item;
		item.mObject = service;
		item.mServices = services;
		return item;
	}


	bool ServiceObjectGraphItem::getPointees(std::vector<ServiceObjectGraphItem>& pointees, utility::ErrorState& errorState)
	{
		std::vector<rtti::TypeInfo> service_links;
		mObject->getDependentServices(service_links);

		for (const auto& link : service_links)
		{
			// Make sure the link is of a type service
			if (!errorState.check(link.get_raw_type().is_derived_from(RTTI_OF(nap::Service)), "type: %s is not a service", link.get_name().data()))
				return false;

			// Create item
			ServiceObjectGraphItem item;

			// Get service it points to
			const auto& found_service = std::find_if((*mServices).begin(), (*mServices).end(), [&link](const auto& service)
			{
				return service->get_type() == link.get_raw_type();
			});

			// Make sure that service exists
			if (!errorState.check(found_service != (*mServices).end(), "unable to retrieve service: %s", link.get_name().data()))
				return false;

			// Copy data for pointee item
			item.mObject = *found_service;
			item.mServices = mServices;
			pointees.push_back(item);
		}

		return true;
	}


	const std::string ServiceObjectGraphItem::getID() const
	{
		return mObject->getTypeName();
	}
}