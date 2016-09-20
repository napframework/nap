#pragma once

/**
@brief nofNetherDream

Represents a OpenFrameworks compatible io interface to the Ether-Dream dac controller
The Ether-Dream can be used to control an ILDA compatible laser.

Note that this interface can run threaded. First call setup, after that start.
Make sure to call Kill on application exit.

Based on the interface provided here: https://github.com/memo/ofxEtherdream
**/

// Of includes
#include <ofThread.h>

// Etherdream includes
#include <netherdream.h>

// Std Includes
#include <vector>

using LaserPoints = std::unique_ptr<std::vector<EAD_Pnt_s>>;

class nofNEtherDream : public ofThread
{
public:

	// Ether dream state
	enum class State : int
	{
		EtherDreamUninitialized		= 0,
		EtherDreamNotFound			= 1,
		EtherDreamLibFailure		= 2,
		EtherDreamDacFailure		= 3,
		EtherDreamFound				= 4
	};

	///@name Construction / Destruction
	nofNEtherDream() : mState(State::EtherDreamUninitialized),
		mPPS(30000), mWaitBeforesend(true), mEtherDacNumber(-1)		{ }
	~nofNEtherDream()												{ Kill(); }

	///@name State
	State			GetState() const								{ return mState; }
	bool			IsReady() const									{ return mState == State::EtherDreamFound; }
	
	///@name Interface
	void			Kill();
	void			Init(int inEtherDream = 0,	int inPPS = 30000);	//< Call this on setup!
	void			Start();
	void			SendData(LaserPoints inPoints);					//< Send information to the dac

protected:
	void			threadedFunction() override;

private:

	bool			Connect();										//< Initializes the system by querying the dac

	State			mState;											//< Current ethernet dream state
	int				mPPS;											//< Points per second
	bool			mWaitBeforesend;								//< If frames are only send after completion
	bool			mAutoConnect;									//< If we try to auto connect
	int				mEtherDacNumber;								//< Associated dac number
	std::string		mName;											//< Associated dac name
	float			mTimeOut = 2.0f;

	// Laserpoints
	LaserPoints		mLaserPoints = nullptr;							//< Protected laser points
};

