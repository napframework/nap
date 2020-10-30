/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "service.h"

namespace nap
{
	/**
	* Service item that can be used in an ObjectGraph
	* This item wraps a service that points to another set of services
	*/
	class ServiceObjectGraphItem
	{
	public:
		using Type = Service*;

		// Default constructor
		ServiceObjectGraphItem() = default;

		// Default destructor
		~ServiceObjectGraphItem() = default;

		/**
		 * Creates a new item that is used to build the graph
		 * @param service the service to create the item for
		 * @param services all available services
		 */
		static const ServiceObjectGraphItem create(Service* service, std::vector<Service*>* services);

		/**
		 * @return the id associated with this graph item
		 */
		const std::string getID() const;

		/**
		 * Performs the traversal of services
		 * Every service can reference another service, based on those dependencies the graph is constructed
		 * @param pointees Output parameter, contains all objects and files this object points to.
		 * @param errorState If false is returned, contains information about the error.
		 * @return true is succeeded, false otherwise.
		 */
		bool getPointees(std::vector<ServiceObjectGraphItem>& pointees, utility::ErrorState& errorState);

		// The service this item references
		Service* mObject = nullptr;

		// Core is used to resolve all pointers
		std::vector<Service*>* mServices = nullptr;
	};
}