#include "../etherdreaminterface.h"

// External includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <j4cDAC.h>


/**
 *	The laser module is initialized the moment the dll is loaded (Windows only)
 */
bool nap::EtherDreamInterface::init()
{
	return true;
}


int nap::EtherDreamInterface::getCount() const
{
	return EtherDreamGetCardNum();
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
