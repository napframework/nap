#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <rtti/deserializeresult.h>

namespace nap
{
	namespace timeline
	{
		/**
		* SequenceElement
		*/
		class NAPAPI SequenceElement : public Resource
		{
			RTTI_ENABLE(Resource)
		public:
			virtual ~SequenceElement();

			/**
			* Initialize this object after de-serialization
			* @param errorState contains the error message when initialization fails
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			* Sets the parameter according to the values they are assigned to in this timeslot
			* @param time the elapsed time
			* @param endValues a reference to the parameters that need to be set
			* @return returns true if this element has to do something ( element falls in this timeframe )
			*/
			virtual bool process(double time, std::vector<Parameter*>& outParameters);

			/**
			* Set the start parameters of this time slot, this is set by the sequence and usually reference the parameters
			* of the sequence before this one
			* @param startParameters the start parameters
			*/
			virtual void setStartParameters(const std::vector<Parameter*>& startParameters);

			/**
			 * 
			 */
			void setPreviousElement(SequenceElement* element) { mPreviousElement = element; }

			/**
			* @return returns a reference to the end parameters of this sequence
			*/
			const std::vector<Parameter*>& getEndParameters() const { return mEndParameters; }

			/**
			* @return returns a reference to the start parameters of this sequence
			*/
			const std::vector<Parameter*>&  getStartParameters() const { return mStartParameters; }

			/**
			* This is called by the sequence to set the start time of this element
			*/
			void setStartTime(const double startTime) { mStartTime = startTime; }

			/**
			* @return returns start time of this element in sequence
			*/
			double getStartTime() const { return mStartTime; }

			/**
			* @return returns id of this sequence
			*/
			const std::string getID() const { return mID; }

			/**
			 * 
			 */
			SequenceElement* getPreviousElement() const { return mPreviousElement; }
		public:
			// properties
			float mDuration = 0.0f;
			bool mUsePreset	= false;
			std::string mPreset;
			std::vector<Parameter*> mEndParameters;
			std::string mName;
		protected:
			double mStartTime = 0.0;
			SequenceElement* mPreviousElement = nullptr;
			
			std::vector<Parameter*> mStartParameters;
			rtti::DeserializeResult mPresetReadResult;
		};
	}
}
