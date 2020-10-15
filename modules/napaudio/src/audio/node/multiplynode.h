#pragma once

#include <audio/core/audionode.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Node that multiplies all the signals connected to its @inputs pin.
		 */
		class NAPAPI MultiplyNode : public Node
		{
		public:
			MultiplyNode(NodeManager& nodeManager) : Node(nodeManager) { }
			
			/**
			 * All signals connected to this pin will be multiplied.
			 */
			MultiInputPin inputs = {this};
			
			/**
			 * Outputs the signal containing a the multiplication result of all inputs.
			 */
			OutputPin audioOutput = {this};
		
		private:
			/**
			 * Calculate the output, perform the multiplication
			 */
			void process() override;
		};
		
	}
}
