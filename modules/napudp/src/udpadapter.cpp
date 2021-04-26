#include "udpadapter.h"
#include "udpthread.h"

namespace nap
{
	bool UDPAdapter::init(utility::ErrorState& errorState)
	{
		if(!errorState.check(mThread !=nullptr, "Threadpool has to be assigned"))
			return false;

		mThread->registerAdapter(this);

		return true;
	}


	void UDPAdapter::onDestroy()
	{
		mThread->removeAdapter(this);
	}
}