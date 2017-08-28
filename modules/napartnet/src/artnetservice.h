#pragma once

// External Includes
#include <nap/service.h>
#include <nap/entity.h>

namespace nap
{
	using ArtnetNode = void*;

	/**
	 * Main interface for rendering to various Etherdream Dacs
	 * The service is responsible for opening / closing the general Etherdream library
	 * and allows for rendering data to the available dacs
	 */
	class NAPAPI ArtnetService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default Constructor
		ArtnetService() = default;

		// Default Destructor
		virtual ~ArtnetService();

		// Initialization
		bool init(nap::utility::ErrorState& errorState);

		/**
		 * @return the artnet node instance associated with this application
		 */
		ArtnetNode getNode() const										{ return mNode; }

	private:
		// The artnet node associated with this application instance
		// The node is something we probably want as a resource, although I am not sure if we want
		// Multiple nodes per running instance.
		ArtnetNode mNode = nullptr;
	};
}