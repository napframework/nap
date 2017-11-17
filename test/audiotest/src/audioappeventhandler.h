#pragma once

#include "audiotestapp.h"
#include <appeventhandler.h>

namespace nap
{
	/**
	* Default app event handler
	* Forwards key presses and window events to the running application
	*/
	class AudioAppEventHandler : public BaseAppEventHandler
	{
		RTTI_ENABLE(BaseAppEventHandler)
	public:
		AudioAppEventHandler(AudioTestApp& app) : BaseAppEventHandler(app) { }

		/**
		* This call polls SDL for various messages, translates those messages
		* in nap events and forwards those to the default nap application
		*/
		virtual void process();
	};
}