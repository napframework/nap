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
		* TimelineSequenceElement
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
			* Initialize this object after de-serialization
			* @param time the elapsed time
			* @param endValues a reference to the parameters that need to be set
			* @return returns true if this element has to do something
			*/
			virtual bool process(double time, std::vector<ResourcePtr<Parameter>>& endParameters);

			virtual void setStartParameters(const std::vector<ResourcePtr<Parameter>>& startParameters);

			const std::vector<ResourcePtr<Parameter>>& getEndParameters() { return mEndParameters; }

			const std::vector<ResourcePtr<Parameter>>& getStartParameters() { return mStartParameters; }

			/**
			* This is called by the sequence to set the start time of this element
			*/
			void setStartTime(double startTime) { mStartTime = startTime; }

			/**
			* @return returns start time of this element in sequence
			*/
			const double getStartTime() { return mStartTime; }
		public:
			// properties
			float mDuration = 0.0f;
			std::string mPreset;
			bool mUsePreset = false;
			std::vector<ResourcePtr<Parameter>> mEndParameters;
		protected:
			double mStartTime = 0.0;

			std::vector<ResourcePtr<Parameter>> mStartParameters;
			rtti::DeserializeResult mPresetReadResult;
		};
	}
}
