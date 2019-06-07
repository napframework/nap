#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>
#include <rtti/deserializeresult.h>

namespace nap
{
	class FlexBlockSequenceElementValueContainer;

	/**
	 * FlexBlockSequenceElement
	 */
	class NAPAPI TimelineSequenceElement : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~TimelineSequenceElement();

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
		virtual bool process(double time, std::vector<Parameter*>& endParameters);

		virtual void setStartParameters(const std::vector<Parameter*>& startParameters);

		const std::vector<Parameter*>& getParameters() { return mParameters; }

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
		std::vector<Parameter*> mParameters;
	protected:
		double mStartTime = 0.0;

		std::vector<Parameter*> mStartParameters;
		rtti::DeserializeResult mReadPresetResult;
	};
}
