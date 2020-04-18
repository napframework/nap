#pragma once

#include <thread>
#include <mutex>

namespace nap
{
	class SequenceService;
	class SequenceEventReceiver;
	class SequenceEvent;

	class SequencePlayerParameterSetterBase
	{
	public:
		SequencePlayerParameterSetterBase(SequenceService& service);

		virtual ~SequencePlayerParameterSetterBase();

		virtual void setValue() = 0;
	protected:
		SequenceService&		mService;
		std::mutex				mMutex;
	};

	template<typename PARAMETER_TYPE, typename PARAMETER_VALUE_TYPE>
	class SequencePlayerParameterSetter :
		public SequencePlayerParameterSetterBase
	{
	public:
		SequencePlayerParameterSetter(SequenceService& service, PARAMETER_TYPE& parameter)
			: SequencePlayerParameterSetterBase(service),
			mParameter(parameter) {}

		virtual ~SequencePlayerParameterSetter() {}

		/**
		* Called from player thread
		*/
		void storeValue(PARAMETER_VALUE_TYPE value)
		{
			std::lock_guard<std::mutex> l(mMutex);

			mValue = value;
		}

		/**
		* Called from service main thread
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
