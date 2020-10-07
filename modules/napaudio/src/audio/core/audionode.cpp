#include "audionode.h"
#include "audionodemanager.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::Node)
	RTTI_FUNCTION("getBufferSize", &nap::audio::Node::getBufferSize)
	RTTI_FUNCTION("getSampleRate", &nap::audio::Node::getSampleRate)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		
		
		Node::Node(NodeManager& manager) : Process(manager)
		{
			manager.registerNode(*this);
		}
		
		
		Node::~Node()
		{
			if (mRegisteredWithNodeManager.load())
				getNodeManager().unregisterNode(*this);
		}
		
		
		SampleBuffer& Node::getOutputBuffer(OutputPin& output)
		{ return output.mBuffer; }
		
		
		void Node::setBufferSize(int bufferSize)
		{
			for (auto& output : mOutputs)
				output->setBufferSize(bufferSize);
			
			bufferSizeChanged(bufferSize);
		}
		
		
	}
	
}

