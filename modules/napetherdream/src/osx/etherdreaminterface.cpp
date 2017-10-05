// Local Includes
#include "../etherdreaminterface.h"

// External Includes
#include <stdio.h>
#include <etherdream.h>
#include <nap/logger.h>
#include <thread>

namespace nap
{
	NAPAPI const int16_t etherMinValue = std::numeric_limits<int16_t>::min();
	NAPAPI const int16_t etherMaxValue = std::numeric_limits<int16_t>::max();
}

// Helper method used for retrieving dac
static etherdream* getEtherDreamDac(int number)
{
    return etherdream_get(static_cast<unsigned long>(number));
}


nap::EtherDreamInterface::EtherDreamInterface()     { }


bool nap::EtherDreamInterface::init()
{
	mAvailableDacs = 0;
	if (etherdream_lib_start() != 0)
		return false;

	// The etherdream dac emits a signal once every second, make sure we wait long
	// enough to gather all available dacs (according to docs and verified)
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	mAvailableDacs = etherdream_dac_count();
	return true;
}


int nap::EtherDreamInterface::getCount() const
{
    return mAvailableDacs;
}


std::string	nap::EtherDreamInterface::getName(int number) const
{
    etherdream* dac = getEtherDreamDac(number);
    if(dac == nullptr)
    {
        return "";
    }
    return std::to_string(etherdream_get_id(dac));
}


bool nap::EtherDreamInterface::connect(int number)
{
    etherdream* dac = getEtherDreamDac(number);
    if(dac == nullptr)
    {
        nap::Logger::warn("can't connect to etherdream dac: %d, invalid dac number", number);
        return false;
    }
    return (etherdream_connect(dac) == 0);
}


bool nap::EtherDreamInterface::stop(int number)
{
    etherdream* dac = getEtherDreamDac(number);
    if(dac == nullptr)
    {
        nap::Logger::warn("can't stop etherdream dac: %d, invalid dac number", number);
        return false;
    }
    return etherdream_stop(dac);
}


void nap::EtherDreamInterface::disconnect(int number)
{
    etherdream* dac = getEtherDreamDac(number);
    if(dac == nullptr)
    {
        nap::Logger::warn("can't disconnect etherdream dac: %d, invalid dac number", number);
        return;
    }
    etherdream_disconnect(dac);
}


void nap::EtherDreamInterface::close()
{
    // OSX / Linux driver doesn't support general close
    return;
}


nap::EtherDreamInterface::EStatus nap::EtherDreamInterface::getStatus(int number) const
{
    etherdream* dac = getEtherDreamDac(number);
    if(dac == nullptr)
    {
        nap::Logger::warn("can't query etherdream status for dac: %d, invalid dac number", number);
        return EtherDreamInterface::EStatus::ERROR;
    }
    return static_cast<EtherDreamInterface::EStatus>(etherdream_is_ready(dac));
}


bool nap::EtherDreamInterface::writeFrame(int number, const EtherDreamPoint* data, uint npoints, uint pps, uint repeatCount)
{
    etherdream* dac = getEtherDreamDac(number);
    if(dac == nullptr)
    {
        nap::Logger::warn("can't write frame to etherdream dac: %d, invalid dac number", number);
        return false;
    }
    
    // perform a c-style cast to etherdream point struct
    // the memory layout is the same
    etherdream_point* point_data = (etherdream_point*)(data);
    
    // Write frame
    return etherdream_write(dac, point_data, static_cast<int>(npoints), static_cast<int>(pps), static_cast<int>(repeatCount));
}

