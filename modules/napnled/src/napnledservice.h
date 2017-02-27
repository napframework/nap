#pragma once

#include <nap/service.h>
#include <nap/coreattributes.h>
#include <nap/configure.h>
#include <asio.hpp>

// Namespace
using asio::ip::tcp;

namespace nap
{
	/**
	 * @brief represents a struct that holds specific led information
	 */
	class NLedPanelInfo : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		NLedPanelInfo() = default;

		// Panel Attributes
		Attribute<int> id =			{ this, "id", -1 };
		Attribute<int> count =		{ this, "ledCount", 0 };
		Attribute<int> height =		{ this, "height", 0 };
		Attribute<int> width =		{ this, "width", 0 };
		Attribute<int> bufferSize = { this, "bufferSize", 0};
	};


	/**
	 * @brief client interface to nled server
	 */
	class NLedService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)
	public:
		/**
		 * Holds server status
		 */
		enum class Status : int8_t
		{
			Disconnected	= -1,
			Connected		= 0,
			ConnectionError	= 1,
			ClientError		= 2,
			ReadError		= 3,
		};

		/**
		 * Constructor
		 */
		NLedService() = default;

		/**
		 * Destructor
		 */
		virtual ~NLedService();

		/**
		 * Connect to server
		 */
		Status connect();

		/**
		 * Disconnect from server
		 */
		void disconnect();

		/**
		 * Name of the nled server
		 */
		Attribute<std::string> serverName	{ this, "serverName", "raspberrypi" };
		
		/**
		 * Port number of the nled server app
		 */
		Attribute<int> portNumber	{ this, "portNumber",  7845 };

	private:
		/**
		 * Reads led configuration and populates led panel information
		 */
		bool readLedConfig();

		/**
		 * Current server status
		 */
		Status mStatus = Status::Disconnected;

		//@name Network
		std::unique_ptr<tcp::socket>	mSocket = nullptr;
		asio::io_service				mNetworkService;
		asio::error_code				mError;

		//@name Panels

	};
}

RTTI_DECLARE(nap::NLedService)
RTTI_DECLARE(nap::NLedPanelInfo)