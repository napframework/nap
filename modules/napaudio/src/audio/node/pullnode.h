#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/safeptr.h>
#include <audio/core/process.h>

namespace nap
{
	namespace audio
	{
		
		// Forward declarations
		class AudioService;
		
		/**
		 * Node to pull its input without doing anything with it, just to make sure it's processed.
		 * This is handy in order to simulate multispeaker applications processing load properly without needing the actual outputs.
		 * The PullNode is a root node that will be directly processed by the node manager.
		 */
		class NAPAPI PullNode final : public Node
		{
			RTTI_ENABLE(Node)
		public:
			/**
			 * @param nodeManager: The node manager this node runs on
			 * @param rootProcess: true if the node registered as root process and processed from creation.
			 */
			PullNode(NodeManager& nodeManager, bool rootProcess = true);
			
			~PullNode() override final;
			
			/**
			 * The processing chain connected to this input will be processed even when not being used for anything.
			 */
			InputPin audioInput = {this};
		
		private:
			void process() override;
			
			bool mRootProcess = false;
		};
		
		
	}
}





