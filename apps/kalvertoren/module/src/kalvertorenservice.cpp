// Local Includes
#include "kalvertorenservice.h"

// External includes
#include <yocto_api.h>

RTTI_DEFINE_CLASS(nap::KalvertorenService)

namespace nap
{
	KalvertorenService::~KalvertorenService()
	{
		yFreeAPI();
	}


	bool KalvertorenService::init(utility::ErrorState& error)
	{
		return true;
	}


	void KalvertorenService::shutdown()
	{
	}
}