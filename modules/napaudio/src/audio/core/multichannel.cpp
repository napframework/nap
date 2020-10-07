#include "multichannel.h"

namespace nap
{
	
	namespace audio
	{
		
		OutputPin* IMultiChannelOutput::tryGetOutputForChannel(unsigned int channel)
		{
			if (channel < getChannelCount())
				return getOutputForChannel(channel);
			else
				return nullptr;
		}
		
		
		void IMultiChannelInput::tryConnect(unsigned int channel, OutputPin& pin)
		{
			if (channel < getInputChannelCount())
				connect(channel, pin);
		}
		
		
		void IMultiChannelInput::connect(IMultiChannelOutput& inputObject)
		{
			for (auto channel = 0; channel < getInputChannelCount(); ++channel)
				connect(channel, *inputObject.getOutputForChannel(channel % inputObject.getChannelCount()));
		}
		
	}
	
}
