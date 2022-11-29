/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "../etherdreaminterface.h"

// External includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <j4cDAC.h>
#include <thread>

namespace nap
{
	NAPAPI const int16_t etherMinValue = std::numeric_limits<int16_t>::min();
	NAPAPI const int16_t etherMaxValue = std::numeric_limits<int16_t>::max();
}

/**
 *	The laser module is initialized the moment the dll is loaded (Windows only)
 */
bool nap::EtherDreamInterface::init()
{
	// The etherdream dac emits a signal once every second, make sure we wait long
	// enough to gather all available dacs (according to docs and verified)
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Now get all available dacs
	mAvailableDacs = EtherDreamGetCardNum();
	return true;
}


int nap::EtherDreamInterface::getCount() const
{
	return mAvailableDacs;
}


nap::EtherDreamInterface::EtherDreamInterface()
{}


nap::EtherDreamInterface::EStatus nap::EtherDreamInterface::getStatus(int number) const
{
	return EtherDreamGetStatus(&number) == GET_STATUS_READY ? EStatus::READY : EStatus::BUSY;
}


std::string nap::EtherDreamInterface::getName(int number) const
{
	char buffer[256];
	EtherDreamGetDeviceName(&number, buffer, 256);
	return std::string(buffer);
}


bool nap::EtherDreamInterface::connect(int number)
{
	return EtherDreamOpenDevice(&number);
}


bool nap::EtherDreamInterface::stop(int number)
{
	return EtherDreamStop(&number);
}


void nap::EtherDreamInterface::disconnect(int number)
{
	EtherDreamCloseDevice(&number);
}


void nap::EtherDreamInterface::close()
{
	EtherDreamClose();
}


bool nap::EtherDreamInterface::writeFrame(int number, const EtherDreamPoint* data, uint npoints, uint pps, uint repeatCount)
{
	// It's safe to convert the structure, same memory layout
	const EAD_Pnt_s* point_data = (const EAD_Pnt_s*)(data);
	
	// Write the frame
	int size = static_cast<int>(sizeof(EAD_Pnt_s) * npoints);
	return EtherDreamWriteFrame(&number, point_data, size, static_cast<uint16_t>(pps), static_cast<uint16_t>(repeatCount));
}