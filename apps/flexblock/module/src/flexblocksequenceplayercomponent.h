#pragma once

#include <component.h>
#include <nap/resourceptr.h>
#include <componentptr.h>

#include "flexblockcomponent.h"
#include "flexblocksequence.h"

namespace nap
{
	class FlexBlockSequencePlayerComponentInstance;

	/**
	 *	flexblocksequenceplayer
	 */
	class NAPAPI FlexBlockSequencePlayerComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockSequencePlayerComponent, FlexBlockSequencePlayerComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	
		ResourcePtr<FlexBlockSequence> mSequence;
		ComponentPtr<FlexBlockComponent> mFlexBlockComponent;
	};


	/**
	 * flexblocksequenceplayerInstance	
	 */
	class NAPAPI FlexBlockSequencePlayerComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FlexBlockSequencePlayerComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize flexblocksequenceplayerInstance based on the flexblocksequenceplayer resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the flexblocksequenceplayerInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update flexblocksequenceplayerInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		void play();
	protected:
		//
		ComponentInstancePtr<FlexBlockComponent> mFlexBlockComponentInstance
			= initComponentInstancePtr(this, &FlexBlockSequencePlayerComponent::mFlexBlockComponent);

		FlexBlockSequence* mSequence = nullptr;

		double mTime = 0.0;
		bool mIsPlaying = false;
		int mCurrentSequenceIndex = 0;

		std::vector<float> mInputs = std::vector<float>{
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f };
	};
}
