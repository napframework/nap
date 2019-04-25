#pragma once

// Local Includes
#include "yoctorangefinder.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Manages a yocotopuce ethernet hub and connection
	 */
	class YoctoEthernetHub : public Device
	{
		RTTI_ENABLE(Device)
	public:
		virtual ~YoctoEthernetHub();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Start the device. Will be called after init()
		 *
		 * @param errorState The error state
		 * @return: true on success
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stop the device. Will be called before the object is reloaded
		 */
		virtual void stop() override;

		std::string		mAddress;								///< Property:'Address' hub ip address or host name
		std::string		mUsername;								///< Property:'Username' ethernet user name
		std::string		mPass;									///< Property:'Pass' ethernet user password
		int				mPort = 80;								///< Property:'Port' ethernet port
		bool			mEnabled = true;						///< Property:'Connect' if on initialization this object should connect to the ethernet hub
		std::vector<ResourcePtr<YoctoRangeFinder>> mSensors;	///< Property:'Sensors' list of connected lux sensors;
		bool			mAllowFailure = true;					///< Property:'Connection failure won't influence initialization'

	private:
		/**
		 *	@return a string that represents the url to the yocto hub
		 */
		std::string getUrl() const;
	};
}
