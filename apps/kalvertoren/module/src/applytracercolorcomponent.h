#pragma once
#include "applycolorcomponent.h"

#include <component.h>

namespace nap
{
	class ApplyTracerColorComponentInstance;

	/**
	 *	applytracercolorcomponent
	 */
	class NAPAPI ApplyTracerColorComponent : public ApplyColorComponent
	{
		RTTI_ENABLE(ApplyColorComponent)
		DECLARE_COMPONENT(ApplyTracerColorComponent, ApplyTracerColorComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * applytracercolorcomponentInstance	
	 */
	class NAPAPI ApplyTracerColorComponentInstance : public ApplyColorComponentInstance
	{
		RTTI_ENABLE(ApplyColorComponentInstance)
	public:
		ApplyTracerColorComponentInstance(EntityInstance& entity, Component& resource) :
			ApplyColorComponentInstance(entity, resource)									{ }

		/**
		 * Initialize applytracercolorcomponentInstance based on the applytracercolorcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the applytracercolorcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Applies tracer colors to the mesh
		 */
		virtual void applyColor(double deltaTime) override;

		/**
		 *	Resets the walker
		 */
		void reset()													{ mChannelTime = 0.0f; }

		/**
		 *	Sets the walker speed
		 */
		void setSpeed(float speed);

		/**
		 *	Selects a specific channel
		 * @param channel DMX channel to select
		 */
		void selectChannel(int channel);

	private:
		double					mChannelTime = 0.0f;
		float					mChannelSpeed = 1.0f;
		int						mSelectedChannel = 0;
		int						mCurrentChannel;
		int						mSelectChannel = 0;
		bool					mManualSelect = false;

		/**
		* Enable / disable manual channel selection
		* @param value if channel selection is manual
		*/
		void setManual(bool value)										{ mManualSelect = value; }

	};
}
