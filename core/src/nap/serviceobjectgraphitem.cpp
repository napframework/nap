#include "serviceobjectgraphitem.h"
#include "core.h"

namespace nap
{
	const nap::ServiceObjectGraphItem ServiceObjectGraphItem::create(Service* service, Core* core)
	{
		ServiceObjectGraphItem item;
		item.mObject = service;
		item.mCore = core;
		return item;
	}


	bool ServiceObjectGraphItem::getPointees(std::vector<ServiceObjectGraphItem>& pointees, utility::ErrorState& errorState)
	{
		std::vector<rtti::TypeInfo> service_links;
		mObject->getDependencies(service_links);

		for (const auto& link : service_links)
		{
			// Make sure the link is of a type service
			if (!errorState.check(link.get_raw_type().is_derived_from(RTTI_OF(nap::Service)), "type: %s is not a service", link.get_name().data()))
				return false;

			// Create item
			ServiceObjectGraphItem item;
			item.mObject = mCore->getOrCreateService(link);
			
			if (!errorState.check(item.mObject != nullptr, "unable to retrieve service: %s", link.get_name().data()))
				return false;
			
			// Copy over core
			item.mCore = mCore;
			pointees.push_back(item);
		}

		return true;
	}


	const std::string ServiceObjectGraphItem::getID() const
	{
		return mObject->getTypeName();
	}
}