#pragma once

// Nap includes
#include <nap/service.h>
#include <apiservice.h>
#include <websocketservice.h>

namespace nap 
{   
    /**
     * Main interface for processing api web socket events in NAP.
	 * The service depends on both the nap::APIService and nap::WebSocketService.
     */
    class NAPAPI APIWebSocketService : public nap::Service
    {
        RTTI_ENABLE(nap::Service)
    public:
		// Constructor
		APIWebSocketService(ServiceConfiguration* configuration);

        // Initialization
        virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * @return the api service
		 */
		APIService& getAPIService();

		/**
		 * @return const ref to the api service
		 */
		const APIService& getAPIService() const;

		/**
		 * @return the api service
		 */
		WebSocketService& getWebSocketService();

		/**
		 * @return const ref to the api service
		 */
		const WebSocketService& getWebSocketService() const;

	protected:
		/**
		 * Registers all objects that need a specific way of construction.
		 * @param factory the factory to register the object creators with.
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * This service depends on the api and web socket service
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		// Creation
		virtual void created() override;

	private:
		APIService* mAPIService = nullptr;
		WebSocketService* mWebSocketService = nullptr;
    };
}
