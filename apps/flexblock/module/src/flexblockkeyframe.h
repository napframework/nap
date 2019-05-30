#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
	/**
	 * FlexBlockKeyFrame
	 */
	class NAPAPI FlexBlockKeyFrame : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockKeyFrame();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<float> mInputs = std::vector<float>{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	};
}
