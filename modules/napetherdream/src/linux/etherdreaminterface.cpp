// Local Includes
#include "../etherdreaminterface.h"

// External Includes
#include <stdio.h>
#include <etherdream.h>
#include <nap/logger.h>

// Helper method used for retrieving dac
static etherdream* getEtherDreamDac(int number)
{
    return etherdream_get(static_cast<unsigned long>(number));
}


nap::EtherDreamInterface::EtherDreamInterface()     { }



bool nap::EtherDreamInterface::init()
{
    return etherdream_lib_start() == 0 ? true : false;
}


int nap::EtherDreamInterface::getCount() const
{
    return etherdream_dac_count();
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
    return etherdream_connect(dac);
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
    return etherdream_write(dac, point_data, static_cast<int>(npoints), static_cast<int>(pps), static_cast<int>(repeatCount));
}

