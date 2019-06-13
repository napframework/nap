#pragma once

// External Includes
#include <nap/resource.h>

#include "sequenceelement.h"

namespace nap
{
	namespace timeline
	{
		class SequenceContainer;

		/**
		* Sequence
		*/
		class NAPAPI Sequence : public Resource
		{
			friend class SequenceContainer;

			RTTI_ENABLE(Resource)
		public:
			virtual ~Sequence();

			/**
			* Initialize this object after de-serialization
			* @param errorState contains the error message when initialization fails
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			* Sets the parameter according to the values they are assigned to in this sequence time slot
			* @param time the elapsed time
			* @param endValues a reference to the parameters that need to be set
			* @return returns true if this element has to do something ( sequence falls in this time slot )
			*/
			virtual int process(double time, std::vector<Parameter*>& outParameters);

			/**
			* Resets current element index
			*/
			void reset();

			/**
			* @return returns the total duration of this sequence
			*/
			const double getDuration() const { return mDuration; }

			/**
			* @return returns the start time of this sequence
			*/
			const double getStartTime() const { return mStartTime; } 

			/**
			* @return returns the id of this resource
			*/
			const std::string getID() const { return mID; }

			/**
			* @return returns pointer to current element pointed to by current element index
			*/
			const SequenceElement* getCurrentElement() const{ return mElements[mCurrentElementIndex]; }
		public:
			// properties
			std::vector<SequenceElement*> mElements;

			//
			std::vector<Parameter*> mStartParameters;
		protected:
			double mDuration = 0.0;
			double mStartTime = 0.0;
			int mCurrentElementIndex = 0;

			/**
			* Sets the start time of this sequence, can only be called by friend class SequenceContainer
			* @param startTime, the new start time
			*/
			void setStartTime(double startTime);
		};
	}
}
