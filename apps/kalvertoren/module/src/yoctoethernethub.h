#pragma once

// Local Includes
#include "yoctoluxsensor.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Manages a yocotopuce ethernet hub and connection
	 */
	class YoctoEthernetHub : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~YoctoEthernetHub();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::string		mAddress;								///< Property:'Address' hub ip address or host name
		std::string		mUsername;								///< Property:'Username' ethernet user name
		std::string		mPass;									///< Property:'Pass' ethernet user password
		int				mPort = 80;								///< Property:'Port' ethernet port
		bool			mConnect = true;						///< Property:'Connect' if on initialization this object should connect to the ethernet hub
		std::vector<ResourcePtr<YoctoLuxSensor>> mSensors;		///< Property:'Sensors' list of connected lux sensors;
	};
}
