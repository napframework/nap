#pragma once

#include <nap/resourceptr.h>
#include <component.h>
#include <parameternumeric.h>

#include "sequencecontainer.h"

namespace nap
{
	namespace timeline
	{
		class SequencePlayerComponentInstance;

		/**
		*	SequencePlayerComponent
		*/
		class NAPAPI SequencePlayerComponent : public Component
		{
			RTTI_ENABLE(Component)
				DECLARE_COMPONENT(SequencePlayerComponent, SequencePlayerComponentInstance)
		public:

			/**
			* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
			* @param components the components this object depends on
			*/
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

			ResourcePtr<ParameterGroup>		mParameterGroup;
			ResourcePtr<SequenceContainer>	mSequenceContainer;
		};


		/**
		* SequencePlayerComponentInstance
		*/
		class NAPAPI SequencePlayerComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			SequencePlayerComponentInstance(EntityInstance& entity, Component& resource) :
				ComponentInstance(entity, resource) { }

			/**
			* Initialize SequencePlayerComponentInstance based on theSequencePlayerComponent resource
			* @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
			* @param errorState should hold the error message when initialization fails
			* @return if the SequencePlayerComponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			* update SequencePlayerComponentInstance. This is called by NAP core automatically
			* @param deltaTime time in between frames in seconds
			*/
			virtual void update(double deltaTime) override;

			/**
			* Play a sequence
			*/
			void play();

			/**
			* Stops playing, resets time
			*/
			void stop();

			/**
			* Pause the sequence
			*/
			void pause();

			/**
			* Plays sequence from beginning when finished
			* @param value, true or false
			*/
			void setIsLooping(bool isLooping) { mIsLooping = isLooping; }

			/**
			* Sets the current time of the sequence
			* @param time, time to set sequence to
			*/
			void setTime(const double time);

			/**
			* Sets the speed
			* @param time, time to set sequence to
			*/
			void setSpeed(const float speed) { mSpeed = speed; }

			/**
			* Skips to sequence in time
			* @param sequence, pointer to sequence
			*/
			void skipToSequence(const Sequence * sequence);

			/**
			* @return true if loaded sequence
			*/
			const bool getIsLoaded() const { return mSequenceContainer != nullptr; }

			/**
			* @return true if playing
			*/
			const bool getIsPlaying() const { return mIsPlaying; }

			/**
			* @return true if paused
			*/
			const bool getIsPaused() const { return mIsPaused; };

			/**
			* @return true if finished playing
			*/
			const bool getIsFinished() const { return mIsFinished; }

			/**
			* @return pointer to current sequence
			*/
			const Sequence* getCurrentSequence() const{ return mSequenceContainer->mSequences[mCurrentSequenceIndex];}

			/**
			* @return current time in sequence
			*/
			const double getCurrentTime() const { return mTime; }

			/**
			* @return duration of sequence
			*/
			const double getDuration() const { return mDuration; }

			/**
			* @return true if looping
			*/
			const bool getIsLooping() const { return mIsLooping; }

			/**
			* @return speed
			*/
			const float getSpeed() const { return mSpeed; }

			/**
			* @return vector of pointers to sequences
			*/
			const std::vector<Sequence*>& getSequences() { return mSequenceContainer->mSequences;}
		protected:
			//
			SequenceContainer* mSequenceContainer = nullptr;

			double mTime = 0.0;
			bool mIsPlaying = false;
			bool mIsPaused = false;
			bool mIsFinished = false;
			bool mIsLooping = false;
			int mCurrentSequenceIndex = 0;
			float mSpeed = 1.0f;

			double mDuration = 0.0;

			std::vector<Parameter*> mParameters = std::vector<Parameter*>();
		};
	}
}
