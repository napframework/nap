#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

#include "flexblockstance.h"

namespace nap
{
	/**
	* 
	*/
	enum class FlexBlockSequenceElementType 
	{
		Stance,
		Transition
	};

	/**
	 * FlexBlockSequenceElement
	 */
	class NAPAPI FlexBlockSequenceElement : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockSequenceElement();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		const FlexBlockSequenceElementType getSequenceType() { return mType; }
		const ResourcePtr<FlexBlockStance> getStance() { return mStance; }
		const ResourcePtr<FlexBlockStance> getNextStance() { return mNextStance; }
	public:
		// properties
		float mDuration = 0.0f;
		FlexBlockSequenceElementType mType = FlexBlockSequenceElementType::Stance;
		
		ResourcePtr<FlexBlockStance> mStance = nullptr;
		ResourcePtr<FlexBlockStance> mNextStance = nullptr;
	};
}
