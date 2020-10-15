#include "controlnode.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ControlNode)
		RTTI_PROPERTY("output", &nap::audio::ControlNode::output, nap::rtti::EPropertyMetaData::Embedded)
		RTTI_FUNCTION("setValue", &nap::audio::ControlNode::setValue)
		RTTI_FUNCTION("getValue", &nap::audio::ControlNode::getValue)
		RTTI_FUNCTION("ramp", &nap::audio::ControlNode::ramp)
		RTTI_FUNCTION("stop", &nap::audio::ControlNode::stop)
		RTTI_FUNCTION("isRamping", &nap::audio::ControlNode::isRamping)
RTTI_END_CLASS

namespace nap
{
	
	namespace audio
	{
		
		ControlNode::ControlNode(NodeManager& manager) : Node(manager)
		{
			mValue.destinationReachedSignal.connect(mDestinationReachedSlot);
		}
		
		
		void ControlNode::setValue(ControllerValue value)
		{
			mNewDestination.store(value);
			mNewStepCount.store(0);
			mIsDirty.set();
		}
		
		
		ControllerValue ControlNode::getValue() const
		{
			
			if (mTranslator != nullptr)
				return mTranslator->translate(mCurrentValue.load());
			else
				return mCurrentValue.load();
		}
		
		
		void ControlNode::ramp(ControllerValue destination, TimeValue time, RampMode mode)
		{
			mNewDestination.store(destination);
			mNewStepCount.store(time * getNodeManager().getSamplesPerMillisecond());
			mNewMode.store(mode);
			mIsDirty.set();
		}
		
		
		void ControlNode::stop()
		{
			mNewDestination.store(mCurrentValue);
			mNewStepCount.store(0);
			mIsDirty.set();
		}
		
		
		void ControlNode::process()
		{
			update();
			auto& outputBuffer = getOutputBuffer(output);
			
			if (mTranslator != nullptr) {
				for (auto i = 0; i < outputBuffer.size(); ++i) {
					outputBuffer[i] = mTranslator->translate(mValue.getNextValue());
				}
			} else {
				for (auto i = 0; i < outputBuffer.size(); ++i) {
					outputBuffer[i] = mValue.getNextValue();
				}
			}
			mCurrentValue.store(mValue.getValue());
		}
		
		
		void ControlNode::update()
		{
			if (mIsDirty.check()) {
				auto stepCount = mNewStepCount.load();
				if (stepCount > 0)
					mValue.ramp(mNewDestination.load(), mNewStepCount.load(), mNewMode.load());
				else
					mValue.setValue(mNewDestination.load());
			}
		}
		
		
	}
	
}
