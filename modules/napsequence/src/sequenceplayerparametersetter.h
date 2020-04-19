#pragma once

#include <thread>
#include <mutex>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class SequenceService;
	class SequenceEventReceiver;
	class SequenceEvent;

	/**
	 * SequencePlayerParameterSetter
	 * Base class of parameter setter
	 * Parameter setters are used to sync SequenceCurveTracks values send to parameters with the main thread
	 */
	class SequencePlayerParameterSetterBase
	{
	public:
		/**
		 * Constructor
		 * @param service is the SequenceService
		 */
		SequencePlayerParameterSetterBase(SequenceService& service);

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerParameterSetterBase();

		/**
		 * setValue
		 * setValue is called from the update function of the SequenceService
		 */
		virtual void setValue() = 0;
	protected:
		SequenceService&		mService;
		std::mutex				mMutex;
	};

	/**
	 * SequencePlayerParameterSetter
	 * Extended class, must have a parametertype ( ParameterFloat, ParameterLong, etc etc ) and its value type ( Float, long, etc etc )
	 */
	template<typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerParameterSetter :
		public SequencePlayerParameterSetterBase
	{
	public:
		/**
		 * Constructor
		 * @param service, reference to service
		 * @param parameter, reference to parameter
		 */
		SequencePlayerParameterSetter(SequenceService& service, PARAMETER_TYPE& parameter)
			: SequencePlayerParameterSetterBase(service),
			mParameter(parameter) {}

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerParameterSetter() {}

		/**
		 * storeValue
		 * Called from a SequencePlayerAdapter and thus the sequence player thread
		 * @value value that the parameter needs to have
		 */
		void storeValue(PARAMETER_VALUE_TYPE value)
		{
			std::lock_guard<std::mutex> l(mMutex);

			mValue = value;
		}

		/**
		 * setValue
		 * Called from the SequenceService main thread update loop.
		 * This sets the value of the Parameter
		 */
		virtual void setValue() override
		{
			std::lock_guard<std::mutex> l(mMutex);

			mParameter.setValue(mValue);
		}
	private:
		PARAMETER_TYPE&						mParameter;
		PARAMETER_VALUE_TYPE				mValue;
	};
}
