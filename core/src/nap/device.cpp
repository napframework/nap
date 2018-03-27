#include "device.h"

RTTI_DEFINE_BASE(nap::Device)

namespace nap
{
	Device::~Device()
	{
		stop();
	}
}
