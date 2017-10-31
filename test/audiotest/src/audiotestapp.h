#pragma once

// Local Includes
#include <app.h>


namespace nap
{
	class AudioTestApp : public BaseApp
	{
		RTTI_ENABLE(BaseApp)
	public:
		AudioTestApp(Core& core) : BaseApp(core)	{ }

		/**
		 *	Initialize our application specific resources
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Update app resources
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Shutdown app
		 */
		virtual void shutdown() override;
	}; 
}
