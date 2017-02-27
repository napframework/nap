#pragma once

#include <ofThread.h>
#include <asio.hpp>
#include <ofPixels.h>
#include <ofImage.h>

// Namespace
using asio::ip::tcp;

// Forward declarations
class nofPanelInfo;
class nofNLedClient;

// Typedefs
using PanelInfoMap = std::map<int, std::unique_ptr<nofPanelInfo>>;

/**
@brief LedInfo

Holds led specific data fetched from the server.
This objects is thread safe.
**/
class nofPanelInfo
{
	friend class nofNLedClient;

public:
	// Image buffer copy mode
	enum class EPixelCopyMode : int
	{
		Fast,				//< Uses nearest neighbour sampling
		Slow				//< Used bilinear sampling through ofImage copy
	};

	nofPanelInfo() : mID(-1), 
		mLedCount(-1), mBufferSize(-1), mLedHeight(-1),
		mLedWidth(-1), mCopyMode(EPixelCopyMode::Fast) { }

	///@name Getters
	int				GetLedID() const				{ return mID; }
	int				GetLedCount() const				{ return mLedCount; }
	int				GetBufferSize() const			{ return mLedCount; }
	int				GetWidth() const				{ return mLedWidth; }
	int				GetHeight() const				{ return mLedHeight; }

	///@name Not thread safe
	ofPixels&		GetPixels()						{ return mBufferPixels; }

	///@name Setters (thread safe)
	// void			SetSourcePixels(ofPixels& inSourcePixels);

	///@name Copy mode to use
	EPixelCopyMode	mCopyMode = EPixelCopyMode::Fast;

private:
	int			mID;				//< Unique panel id
	int			mLedCount;			//< Amount of individual leds
	int			mBufferSize;		//< Buffer size
	int			mLedHeight;			//< Amount of leds in height
	int			mLedWidth;			//< Amount of leds in width
	ofPixels	mBufferPixels;		//< Pixels to send over
	ofMutex		mMutex;				//< Mutex lock
};


/**
 * nled client that runs on it's own thread
 * This object slices up the texture and sends it to 
 * all available led devices
 */
class nofNLedClient : public ofThread
{
public:
	/**
	* Holds client status
	*/
	enum class Status : int8_t
	{
		Disconnected = -1,
		Connected = 0,
		ConnectionError = 1,
		ClientError = 2,
		ReadError = 3,
	};

	/**
	 * Default constructor
	 */
	nofNLedClient() = default;

	/**
	 * Destructor
	 */
	virtual ~nofNLedClient();

	/**
	 * Called as long as the thread is running
	 */
	void threadedFunction() override;

	/**
	 * Start running
	 */
	void start();

	/**
	 * Stop running
	 */
	void stop();

	/**
	 * @return current status
	 */
	Status getStatus() const						{ return mStatus; }

	/**
	 * Set server name
	 */
	void setServerName(const std::string& name)		{ mServerName = name; }

	/**
	 * Set port
	 */
	void setPortNumber(int number)					{ mPortNumber = number; }

private:
	// Current connection status
	Status mStatus = Status::Disconnected;

	// Server name
	std::string mServerName = "raspberrypi";

	// Port number of server app
	int mPortNumber = 7845;

	/**
	* Reads led configuration and populates led panel information
	*/
	bool readLedConfig();

	//@name Network
	std::unique_ptr<tcp::socket>	mSocket = nullptr;
	asio::io_service				mNetworkService;
	asio::error_code				mError;

	// Holds all led panels
	PanelInfoMap					mPanels;

	/**
	* Connects the client to server
	*/
	Status connect();

	/**
	* Disconnect from client
	*/
	void disconnect();
};