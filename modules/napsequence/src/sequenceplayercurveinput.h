#pragma once

// local includes
#include "sequenceplayerinput.h"

// nap includes
#include <nap/resourceptr.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequencePlayerParameterSetterBase;
	class SequenceService;

	/**
	 * SequencePlayerCurveInput is used to link a parameter to a curve track
	 */
	class NAPAPI SequencePlayerCurveInput : public SequencePlayerInput
	{
		RTTI_ENABLE(SequencePlayerInput)
	public:
		SequencePlayerCurveInput(SequenceService& service);

		// properties
		ResourcePtr<Parameter>	mParameter; ///< Property: 'Parameter' parameter resource
		bool					mUseMainThread; ///< Property: 'Use Main Thread' update in main thread or player thread

		/**
		 * registers a parameter setter to the input. Parameter setters are called from main thread
		 * @param parameterSetter ptr to parameter setter
		 */
		void registerParameterSetter(SequencePlayerParameterSetterBase* parameterSetter);

		/**
		 * removes parameter setter
		 * @param parameterSetter ptr to parameter setter
		 */
		void removeParameterSetter(SequencePlayerParameterSetterBase* parameterSetter);
	protected:
		/**
		 * called from update loop sequence service main thread
		 * @param deltaTime time since last update
		 */
		virtual void update(double deltaTime) override ;

		// vector holding registered parameter setters
		std::vector<SequencePlayerParameterSetterBase*> mSetters;
	private:

	};

	using SequencePlayerCurveInputObjectCreator = rtti::ObjectCreator<SequencePlayerCurveInput, SequenceService>;
}
