#pragma once

#include <nap/resourceptr.h>
#include <component.h>
#include <parameternumeric.h>

#include "sequencecontainer.h"

namespace nap
{
	namespace timeline
	{
		//////////////////////////////////////////////////////////////////////////

		class SequencePlayerComponentInstance;

		/**
		 *	SequencePlayerComponent
		 *	SequencePlayerComponent loads a default show from the show directory.
		 *  A show is a list of sequences. Each sequence holds sequence elements. Elements can contain pauses or transitions. 
		 *  The sequence player component is responsible for playing a set of given sequences. Also it contains functions and methods 
		 *  to edit the current show.
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

			std::string						mDefaultShow;
			ResourcePtr<ParameterGroup>		mParameterGroup;
		};

		//////////////////////////////////////////////////////////////////////////
		/**
		 * SequencePlayerComponentInstance
		 * The instance of the sequence player component
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
			 * Reconstructs sequence container and recalculates duration,
			 * this call is necessary when changing, adding or removing sequences/elements
			 */
			void reconstruct();

			/**
			 * Removes sequence from sequence container, removes owned sequences as well;
			 * param sequence, sequence that needs to be removed
			 */
			void removeSequence(const Sequence* sequence);

			/**
			 * Removes sequence element from given sequence
			 * @param sequence, sequence containing the element
			 * @param element, element that needs to be removed
			 */
			void removeSequenceElement(const Sequence* sequence, const SequenceElement* element);

			/**
			 * Serializes and saves the current sequence container as a json file, fileName must include filename extension.
			 * returns true if succeeded
			 * @param showName, name of the file, must include filename extension ( .json )
			 * @param errorState, reference to errorState
			 * @return true if successful
			 */
			bool save(std::string fileName, utility::ErrorState& errorState);

			/**
			 * Loads given file name, returns true if successful
			 * @param fileName, name of the file, must include filename extension ( .json )
			 * @param errorState, reference to errorState
			 * @return true if successful
			 */
			bool load(std::string fileName, utility::ErrorState& errorState);

			/**
			 * Inserts sequence into sequence container, player holds unique pointer to sequence inserted
			 * @param sequence unique_ptr to sequence, will be moved
			 */
			void insertSequence(std::unique_ptr<Sequence> sequence);

			/**
			 * @return true if loaded sequence
			 */
			const bool getIsLoaded() const { return mSequenceContainer != nullptr; }

			/**
			 * @return true if playing
			 */
			const bool getIsPlaying() const { return mIsPlaying; }

			/**
			 * Evaluate acts the same as process but doesn't advance the
			 * current element index or finishes the player
			 */
			const void evaluate(double time, std::vector<Parameter*> &output, const int offset = 0) const;

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
			Sequence* getCurrentSequence() const { return mSequenceContainer->getSequences()[mCurrentSequenceIndex]; }

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
			 * Returns pointer to sequence at given time, can be nullptr
			 */
			Sequence * getSequenceAtTime(double time) const;

			/**
			 * @return reference to vector of pointers to sequences
			 */
			const std::vector<Sequence*>& getSequences() const { return mSequenceContainer->getSequences();}

			/**
			 * @return returns current show name
			 */
			const std::string getShowName() const { return mShowName; }
		protected:
			double mTime				= 0.0;
			bool mIsPlaying				= false;
			bool mIsPaused				= false;
			bool mIsFinished			= false;
			bool mIsLooping				= false;
			size_t mCurrentSequenceIndex	= 0;
			float mSpeed				= 1.0f;
			double mDuration			= 0.0;

			std::string mShowName;
			std::vector<Parameter*> mParameters = std::vector<Parameter*>();
			rtti::OwnedObjectList mOwnedObjects;
			std::vector<std::unique_ptr<Sequence>> mOwnedSequences;

			std::unique_ptr<timeline::SequenceContainer> mSequenceContainer = nullptr;
		};
	}
}
