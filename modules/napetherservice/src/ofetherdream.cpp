// Include ether dream
#include <ofetherdream.h>

// Windows includes
#include <assert.h>

// OF Includes
#include <ofLog.h>

// Time
#include <ofUtils.h>

/**
@brief Setup

Call this in the setup function for the ether dream you want to control (starting at 0)
**/
void nofNEtherDream::Init(int inEtherDream, int inPPS)
{
	// Set the dac number
	mEtherDacNumber = inEtherDream;

	// Set point draw rate
	mPPS = inPPS;

	// Make sure the lib is loaded
	if(!netherdream::IsLoaded() && !netherdream::LoadEtherDreamLib())
	{
		ofLogError("EtherDreamLib") << "Unable to load library!";
		mState = State::EtherDreamLibFailure;
		return;
	}

	// Initialize
	if(!Connect()) ofLogWarning("EtherDream") << "Unable to initialize ether dream dac!";
	else ofLogNotice("EtherDream") << "Successfully initialized ether dream dac!";
}


/**
@brief Init

Initializes the dac on the network
**/
bool nofNEtherDream::Connect()
{
	assert(mEtherDacNumber >= 0);
	ofLogNotice("EtherDreamLib") << "Initializing ether dream dac: " << mEtherDacNumber;

	// Get device
	int device_count = netherdream::GetNumberOfDevices();
	if(!device_count || mEtherDacNumber >= device_count)
	{
		ofLogError("EtherDreamLib") << "Dac with number: " << mEtherDacNumber << "not found";
		mState = State::EtherDreamNotFound;
		return false;
	}

	// Open Device
	if(!netherdream::OpenDevice(mEtherDacNumber))
	{
		ofLogError("EtherDreamLib") << "Unable to open ether dream: " << mEtherDacNumber;
		mState = State::EtherDreamDacFailure;
		return false;
	}

	// Get name
	mName = netherdream::GetDeviceName(mEtherDacNumber);
	ofLogNotice("EtherDreamLib") << "Found ether dream dac with name: " << mName;

	// Set state
	mState = State::EtherDreamFound;
	return true;
}



/**
@brief Send

Sends the stored information to the dac
**/
void nofNEtherDream::SendData(const std::vector<EAD_Pnt_s>& inPoints)
{
	// Don't attempt to write when we're not initialized correctly
	if(!IsReady())
	{
		ofLogWarning("EtherDream") << "not initialized correctly!";
		return;
	}

	// Don't write zero length data buffers
	if(inPoints.empty())
		return;
	
	// Make sure we can write, this is a blocking call -> remove
	float max_time = ofGetElapsedTimef() + mTimeOut;
	while (netherdream::GetStatus(mEtherDacNumber) == netherdream::Status::Busy)
	{
		if (ofGetElapsedTimef() > max_time)
		{
			ofLogWarning("EtherdreamLib") << "laser device still busy, skipping...";
			return;
		}
	}

	// Write
	int size = inPoints.size() * sizeof(EAD_Pnt_s);
	netherdream::WriteFrame(mEtherDacNumber, inPoints.data(), size, mPPS, 1);
}



/**
@brief Kill

Stops the thread and kills the ether dream communication
**/
void nofNEtherDream::Kill()
{
	// If the dac wasn't ready (ie, errors occured), return
	if(!IsReady())
		return;

	// Stop running and close device
	netherdream::Stop(mEtherDacNumber);
	netherdream::CloseDevice(mEtherDacNumber);
}