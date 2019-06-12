#pragma once

// External Includes
#include <nap/resource.h>

#include "sequenceelement.h"

namespace nap
{
	namespace timeline
	{
		/**
		* Sequence
		*/
		class NAPAPI Sequence : public Resource
		{
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
			virtual bool process(double time, std::vector<Parameter*>& outParameters);

			void setStartTime(double startTime);

			const double getDuration() { return mDuration; }

			const double getStartTime() { return mStartTime; }

			void reset();

			const SequenceElement* getCurrentElement() const
			{ 
				return mElements[mCurrentElementIndex];
			}
		public:
			// properties
			std::vector<SequenceElement*> mElements;

			//
			std::vector<Parameter*> mStartParameters;
		protected:
			double mDuration = 0.0;
			double mStartTime = 0.0;
			int mCurrentElementIndex = 0;
		};
	}
}
