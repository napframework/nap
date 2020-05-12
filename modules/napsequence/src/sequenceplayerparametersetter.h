#pragma once

// local includes
#include "sequenceplayercurveinput.h"

// external includes
#include <thread>
#include <mutex>
#include <nap/core.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of parameter setter
	 * Parameter setters are used to sync SequenceCurveTracks values send to parameters with the main thread
	 */
	class NAPAPI SequencePlayerParameterSetterBase
	{
	public:
		SequencePlayerParameterSetterBase(SequencePlayerCurveInput& input);

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerParameterSetterBase();

		/**
		 * setValue is called from the update function of the SequenceService
		 */
		virtual void setValue() = 0;
	protected:
		// mutex
		std::mutex				mMutex;

		//
		SequencePlayerCurveInput& mInput;
	};

	/**
	 * SequencePlayerParameterSetter
	 * Extended class, must have a parametertype ( ParameterFloat, ParameterLong, etc etc ) and its value type ( Float, long, etc etc )
	 */
	template<typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class NAPAPI SequencePlayerParameterSetter :
		public SequencePlayerParameterSetterBase
	{
	public:
		/**
		 * Constructor
		 * @param service, reference to service
		 * @param parameter, reference to parameter
		 */
		SequencePlayerParameterSetter(SequencePlayerCurveInput& input, PARAMETER_TYPE& parameter)
			: SequencePlayerParameterSetterBase(input),
			mParameter(parameter) {}

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerParameterSetter() {}

		/**
		 * Called from a SequencePlayerAdapter and thus the sequence player thread
		 * @value value that the parameter needs to have
		 */
		void storeValue(PARAMETER_VALUE_TYPE value)
		{
			std::lock_guard<std::mutex> l(mMutex);

			mValue = value;
		}

		/**
		 * Called from the SequenceService main thread update loop.
		 * This sets the value of the Parameter
		 */
		virtual void setValue() override
		{
			std::lock_guard<std::mutex> l(mMutex);

			mParameter.setValue(mValue);
		}
	private:
		// reference to parameter
		PARAMETER_TYPE&						mParameter;

		// the value
		PARAMETER_VALUE_TYPE				mValue;
	};
}
