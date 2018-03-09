// Local Includes
#include "../etherdreaminterface.h"

// External Includes
#include <stdio.h>
#include <etherdream.h>
#include <nap/logger.h>
#include <numeric>
#include <thread>

namespace nap
{

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

	// Gather all available dacs
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
	
	/**
	 * The *nix drivers for Etherdream don't provide the same getName method as in the Windows driver.
	 * getName on Windows just returns the last three parts of the MAC address as hex, whereas etherdream_get_id
	 * returns the same section of the MAC but bitshifted into a long.
	 *
	 * Here we do some convert from the bitshifted version of the MAC address to the hex format.
	 */
	
	// Grab the bitshifted id
	unsigned long bitwiseEtherdreamMac = etherdream_get_id(dac);
	
	// Decode first segment and re-shift for later calculations
	unsigned long firstSegment = bitwiseEtherdreamMac >> 16;
	unsigned long firstSegmentShifted = firstSegment << 16;
	
	// Decode second segment and re-shift for final calculation
	unsigned long secondSegment = (bitwiseEtherdreamMac - firstSegmentShifted) >> 8;
	unsigned long secondSegmentShifted = secondSegment << 8;
	
	// Subtract previous shifted values from original for third segment
	unsigned long thirdSegment = bitwiseEtherdreamMac - firstSegmentShifted - secondSegmentShifted;
	
	// Convert to hex
	std::stringstream stream;
	stream << std::hex << firstSegment << secondSegment << thirdSegment;
	return stream.str();
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
    return (etherdream_stop(dac) == 0);
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
    
    int eth_stat = etherdream_is_ready(dac);
    switch(eth_stat)
    {
        case 0:
            return EtherDreamInterface::EStatus::BUSY;
        case 1:
            return EtherDreamInterface::EStatus::READY;
        default:
            return EtherDreamInterface::EStatus::ERROR;
    }
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
    return (etherdream_write(dac, point_data, static_cast<int>(npoints), static_cast<int>(pps), static_cast<int>(repeatCount)) == 0);
}