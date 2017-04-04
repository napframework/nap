#include "ofnledclient.h"
#include <nap/logger.h>
#include <assert.h>
#include <nledservercommands.h>

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


nofNLedClient::~nofNLedClient()
{
	stop();
}


void nofNLedClient::threadedFunction()
{
	while (isThreadRunning())
	{
		//nap::Logger::info("should send data to panels");
	}
}


nofNLedClient::Status nofNLedClient::connect()
{
	// Ensure the client is disconnected
	disconnect();

	// We need to turn the server name that was specified as a parameter to the application, into a TCP endpoint. To do this we use an ip::tcp::resolver object.
	tcp::resolver resolver(mNetworkService);

	// Create resolver and connect
	if (mServerName == "")
	{
		nap::Logger::warn("No led server name specified");
		mStatus = Status::ClientError;
		return mStatus;
	}
	nap::Logger::info("Connecting to: %s on Port: %d", mServerName.c_str(), mPortNumber);

	// A resolver takes a query object and turns it into a list of endpoints. We construct a query using the name of the server, specified in argv[1], and the name of the service, in this case "daytime".
	tcp::resolver::query query(mServerName, std::to_string(mPortNumber));

	// Connect to server
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, mError);
	if (mError)
	{
		nap::Logger::warn("Unable to resolve host: %s on port: %d", mServerName.c_str(), mPortNumber);
		mStatus = Status::ConnectionError;
		return mStatus;
	}

	// Create a new one and connect
	mSocket = std::make_unique<tcp::socket>(mNetworkService);
	asio::connect(*mSocket, endpoint_iterator, mError);
	if (mError)
	{
		nap::Logger::warn("ERROR: Unable to connect to host: %s on port: %d", mServerName.c_str(), mPortNumber);
		mStatus = Status::ConnectionError;
		return mStatus;
	}

	// Set state
	mStatus = Status::Connected;
	return mStatus;
}


// Disconnect client from server
void nofNLedClient::disconnect()
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
}


// Connect and start running
void nofNLedClient::start()
{
	// Try to connect
	if (connect() != Status::Connected)
	{
		nap::Logger::warn("unable to start nled client, errors occurred");
		return;
	}

	// Read led configuration
	if (!readLedConfig())
	{
		nap::Logger::warn("unable to receive led configuration from host: %s on port: %d", mServerName, mPortNumber);
		mStatus = Status::ReadError;
		return;
	}

	// Start running thread
	startThread();
}


// Stop the thread from running
void nofNLedClient::stop()
{
	// Stop sending data
	if (isThreadRunning())
	{
		stopThread();
	}

	// Disconnect
	disconnect();
}


// Read led configuration
bool nofNLedClient::readLedConfig()
{
	if (mSocket == nullptr || !mSocket->is_open())
	{
		nap::Logger::warn("unable to read led config, no network connection available!");
		return false;
	}

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
		// Get panel id
		INT32 display_id = readInt(*mSocket, error);
		if (error)
		{
			//delete panel_info;
			success = false;
			break;
		}

		if (mPanels.find(display_id) != mPanels.end())
		{
			nap::Logger::warn("Panel with unique id: %d already specified, skipping...", display_id);
			continue;
		}

		std::unique_ptr<nofPanelInfo> new_panel = std::make_unique<nofPanelInfo>();
		new_panel->mID = display_id;

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
		new_panel->mLedCount = info[0];
		new_panel->mBufferSize = info[1];
		new_panel->mLedHeight = info[2];
		new_panel->mLedWidth = info[3];

		// Allocate the pixels
		new_panel->mBufferPixels.allocate(new_panel->mLedWidth, new_panel->mLedHeight, OF_PIXELS_RGB);

		// Add
		mPanels.emplace(display_id, std::move(new_panel));
	}

	if (!success)
	{
		mPanels.clear();
		return false;
	}

	// Clear sampled panel information if an error occured
	return true;
}
