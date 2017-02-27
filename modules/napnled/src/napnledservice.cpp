#include "napnledservice.h"
#include <nap/logger.h>
#include <nledservercommands.h>
#include <assert.h>

// using directives
using namespace asio::detail::socket_ops;

/**
@brief Reads a single int from the stream
**/
static INT32 readInt(tcp::socket& inSocket, asio::error_code& ioError)
{
	INT32 sample_data(-1);
	asio::read(inSocket, asio::buffer(&sample_data, sizeof(INT32)), ioError);
	return network_to_host_long(sample_data);
}

/**
@brief Writes a single int to the server stream
**/
static void sendInt(INT32 inInt, tcp::socket& inSocket, asio::error_code& ioError)
{
	INT32 formatted_int = host_to_network_long(inInt);
	asio::write(inSocket, asio::buffer(&formatted_int, sizeof(INT32)), ioError);
}

namespace nap
{
	NLedService::~NLedService()
	{
		disconnect();
	}

	/**
	 * Attemps to connect
	 */
	NLedService::Status NLedService::connect()
	{
		// Ensure the client is disconnected
		disconnect();

		// We need to turn the server name that was specified as a parameter to the application, into a TCP endpoint. To do this we use an ip::tcp::resolver object.
		tcp::resolver resolver(mNetworkService);

		// Create resolver and connect
		if (serverName.getValue() == "")
		{
			nap::Logger::warn("No led server name specified");
			mStatus = Status::ClientError;
			return mStatus;
		}
		nap::Logger::info("Connecting to: %s on Port: %d", serverName.getValue().c_str(), portNumber.getValue());

		// A resolver takes a query object and turns it into a list of endpoints. We construct a query using the name of the server, specified in argv[1], and the name of the service, in this case "daytime".
		tcp::resolver::query query(serverName.getValue(), std::to_string(portNumber.getValue()));

		// Connect to server
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, mError);
		if (mError)
		{
			nap::Logger::warn("Unable to resolve host: %s on port: %d", serverName.getValue().c_str(), portNumber.getValue());
			mStatus = Status::ConnectionError;
			return mStatus;
		}

		// Create a new one and connect
		mSocket = std::make_unique<tcp::socket>(mNetworkService);
		asio::connect(*mSocket, endpoint_iterator, mError);
		if (mError)
		{
			nap::Logger::warn("ERROR: Unable to connect to host: %s on port: %d", serverName.getValue().c_str(), portNumber.getValue());
			mStatus = Status::ConnectionError;
			return mStatus;
		}

		// Read led configuration
		if (!readLedConfig())
		{
			nap::Logger::warn("unable to receive led configuration from host: %s on port: %d", serverName.getValue().c_str(), portNumber.getValue());
			mStatus = Status::ReadError;
			return mStatus;
		}

		// Set state
		mStatus = Status::Connected;
		return mStatus;
	}


	/**
	 * Disconnect if previous connection was open
	 */
	void NLedService::disconnect()
	{
		if (mSocket)
		{
			if (mSocket->is_open())
			{
				mSocket->close();
			}
			mSocket.release();
		}

		// Reset socket
		mSocket = nullptr;
		mStatus = Status::Disconnected;

		// Clear all panel info
		this->clearChildren();
	}


	/**
	 * Read the led config and create led config objects
	 */
	bool NLedService::readLedConfig()
	{
		if (mSocket == nullptr || !mSocket->is_open())
		{
			nap::Logger::warn("unable to read led config, no network connection available!");
			return false;
		}

		// Make sure we have no children
		assert(this->getChildren().size() == 0);

		// Ask for led config
		asio::error_code error;
		sendInt(NLED_ID_GETCONFIG, *mSocket, error);
		if (error)
			return false;

		// Get number of panels
		int mPanelCount = readInt(*mSocket, error);
		if (error)
			return false;
		nap::Logger::info("Number of available Led Panels: %d", mPanelCount);

		// Read server panel info
		bool success(true);
		for (INT32 i = 0; i < mPanelCount; i++)
		{
			// Add panel
			nap::NLedPanelInfo& panel = this->addChild<nap::NLedPanelInfo>(nap::stringFormat("panel_%d", i));

			// Get panel id
			INT32 display_id = readInt(*mSocket, error);
			if (error)
			{
				//delete panel_info;
				success = false;
				break;
			}
			panel.id.setValue(display_id);

			// Get panel info
			INT32 info[4];
			for (int s = 0; s < 4; s++)
			{
				info[s] = readInt(*mSocket, error);
				if (error)
				{
					success = false;
					break;
				}
			}

			// Exit loop on error
			if (!success)
				break;

			// Set info
			panel.count.setValue(info[0]);
			panel.bufferSize.setValue(info[1]);
			panel.height.setValue(info[2]);
			panel.width.setValue(info[3]);
		}

		if (!success)
		{
			this->clearChildren();
			return false;
		}

		// Clear sampled panel information if an error occured
		return true;
	}

}

RTTI_DEFINE(nap::NLedService)