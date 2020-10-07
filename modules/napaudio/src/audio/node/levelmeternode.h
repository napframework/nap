#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/dirtyflag.h>
#include <audio/utility/safeptr.h>
#include <audio/core/process.h>

namespace nap
{
	namespace audio
	{
		
		// Forward declarations
		class AudioService;
		
		/**
		 * Node to measure the amplitude level of an audio signal.
		 * Can be used for VU meters or envelope followers for example.
		 * Can switch between measuring peaks of the signal or the root mean square.
		 */
		class NAPAPI LevelMeterNode : public Node
		{
		public:
			enum Type
			{
				PEAK, RMS
			};
			
			/**
			 * @param audioService: the NAP audio service.
			 * @param analysisWindowSize: the time window in milliseconds that will be used to generate one single output value. Also the period that corresponds to the analysis frequency.
			 * @param rootProcess: indicates that the node is registered as root process with the @AudioNodeManager and is processed automatically.
			 */
			LevelMeterNode(NodeManager& nodeManager, TimeValue analysisWindowSize = 10, bool rootProcess = true);
			
			virtual ~LevelMeterNode();
			
			InputPin input = {this}; /**< The input for the audio signal that will be analyzed. */
			
			/**
			 * @return: The current level of the analyzed signal.
			 */
			float getLevel();
			
			/**
			 * Set the Type of the analysis. PEAK means the highest absolute amplitude within the analyzed window will be output. RMS means the root mean square of all values within the analyzed window will be output.
			 */
			void setType(Type type) { mType = type; }
			
			void process() override;
		
		private:
			float calculateRms(); // Calculates output value out of one buffer of data using root mean square algorithm
			float
			calculatePeak(); // Calculates output value out of one buffer of data by determining the maximum amplitude of the buffer.
			
			SampleBuffer mBuffer; // Buffer being analyzed
			float mValue = 0; // Calculated output level value
			int mIndex = 0; // Current write index of the buffer being analyzed.
			Type mType = Type::RMS; // Algorithm currently being used to calculate the output level value from one buffer.
			DirtyFlag mDirty;
			
			bool mRootProcess = false;
		};
		
	}
}
