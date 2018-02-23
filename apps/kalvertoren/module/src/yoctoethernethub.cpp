#include "yoctoethernethub.h"

#include <utility/stringutils.h>
#include <yocto_api.h>
#include <nap/logger.h>

// nap::yoctoethernethub run time class definition 
RTTI_BEGIN_CLASS(nap::YoctoEthernetHub)
	RTTI_PROPERTY("Address",	&nap::YoctoEthernetHub::mAddress,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Username",	&nap::YoctoEthernetHub::mUsername,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Pass",		&nap::YoctoEthernetHub::mPass,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Port",		&nap::YoctoEthernetHub::mPort,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Connect",	&nap::YoctoEthernetHub::mConnect,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Sensors",	&nap::YoctoEthernetHub::mSensors,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	YoctoEthernetHub::~YoctoEthernetHub()
	{
	}


	bool YoctoEthernetHub::init(utility::ErrorState& errorState)
	{
		// Don't connect when registration is set to false
		if (!mConnect)
			return true;

		// Register / connect to the ethernet hub
		std::string error;
		std::string hub_address = utility::stringFormat("http://%s:%s@%s:%d", mUsername.c_str(), mPass.c_str(), mAddress.c_str(), mPort);
		YRETCODE ret_code = yRegisterHub(hub_address, error);

		// Make sure connection could be established
		if (!errorState.check(ret_code == YAPI_SUCCESS, "%s: unable to establish connection to hub: %s:%d, error: %s", mID.c_str(), mAddress.c_str(), mPort, error.c_str()))
			return false;

		// Some debug display info
		nap::Logger::info("successfully connected to yocto ethernet hub: %s:%d", mAddress.c_str(), mPort);

		// Now start reading from all the connected light sensors
		for (auto& sensor : mSensors)
			sensor->start();

		return true;
	}
}