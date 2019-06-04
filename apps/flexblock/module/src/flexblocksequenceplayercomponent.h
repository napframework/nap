#pragma once

#include <component.h>
#include <nap/resourceptr.h>
#include <parameter.h>
#include <parameternumeric.h>

#include "flexblockcomponent.h"
#include "flexblocksequence.h"

namespace nap
{
	class FlexBlockSequencePlayerComponentInstance;

	/**
	 *	FlexBlockSequencePlayerComponent
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

		ResourcePtr<ParameterGroup> mParameterGroup;
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
		
		void load(FlexBlockSequence* sequence);

		void play();
		
		void stop();

		void pause();

		void setIsLooping(bool value) { mIsLooping = value; }

		const bool getIsLoaded() { return mSequence != nullptr; }

		const bool getIsPlaying() { return mIsPlaying; }

		const bool getIsPaused() { return mIsPaused; };

		const bool getIsFinished() { return mIsFinished; }

		const FlexBlockSequence* getCurrentSequence() { return mSequence; };
	
		const FlexBlockSequenceElement* getCurrentElement() 
		{ 
			return mSequence->mElements[mCurrentSequenceIndex].get(); 
		};

		void setTime(double time);

		const double getCurrentTime() { return mTime; }

		const double getDuration() { return mDuration; }

		const bool getIsLooping() { return mIsLooping; }

		const std::vector<FlexBlockSequenceElement*> getElements();
	protected:
		//
		FlexBlockSequence* mSequence = nullptr;

		double mTime = 0.0;
		bool mIsPlaying = false;
		bool mIsPaused = false;
		bool mIsFinished = false;
		bool mIsLooping = false;
		int mCurrentSequenceIndex = 0;
		double mDuration = 0.0;

		std::vector<ParameterFloat*> mInputs = std::vector<ParameterFloat*>();
	};
}
