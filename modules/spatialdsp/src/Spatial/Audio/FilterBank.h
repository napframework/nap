#pragma once

// Std includes
#include <atomic>

// Nap includes
#include <nap/resourceptr.h>
#include <utility/threading.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/dirtyflag.h>
#include <Spatial/Audio/MultiChannelWithInput.h>
#include <Spatial/Audio/biquad.h>

namespace nap
{
    
    namespace audio
    {
     
        /**
         * Processes a maximum of 8 parallel bandpass filters on the input signal using AVX2 optimization.
         */
        class NAPAPI FilterBankNode : public Node
        {
            RTTI_ENABLE(Node)
            
        public:
            FilterBankNode(NodeManager& manager) : Node(manager), mUpdateFunction(nullptr) { }
			~FilterBankNode();
            
            InputPin audioInput = { this }; /**< The audio input receiving the signal to be processed. */
            OutputPin output = { this }; /**< The audio output with the processed signal. */
            
            /**
             * Sets the number of filters being processed. The maximum is 8.
             */
            void setFilterCount(unsigned int count);
            
            /**
             * @return: The number of filters being processed
             */
            int getFilterCount() const { return mFilterCount.load(); }
            
            /**
             * Sets the parameters of all filters to the values within the vector arguments. If the sizes of the vectors are shorter than 8, the content will be repeated.
             */
            void setParameters(const std::vector<ControllerValue>& centerFrequency, const std::vector<ControllerValue>& bandWidth, const std::vector<ControllerValue>& gain);

        private:
            void process() override;
            
            std::atomic<int> mFilterCount = { 1 };
            BiquadFilter<float8> mFilter;
            
            using UpdateFunction = std::function<void()>;
            std::atomic<UpdateFunction*> mUpdateFunction;
            moodycamel::ConcurrentQueue<UpdateFunction*> mDeletionQueue;

        };
        
        
        /**
         * Object containing a @FilterBankNode on each channel.
         */
        using FilterBank = MultiChannelWithInput<FilterBankNode>;

    }
    
}



