#pragma once

// Nap includes
#include <component.h>

// Audio includes
#include <audio/node/levelmeternode.h>
#include <audio/component/audiocomponentbase.h>
#include <audio/node/filternode.h>

namespace nap
{
    
    namespace audio
    {
        
        class LevelMeterComponentInstance;
        
        
        /**
         * Component to measure the amplitude level of the audio signal from an @AudioComponentBase.
         * A specific frequency band to be meusured can be specified.
         */
        class NAPAPI LevelMeterComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(LevelMeterComponent, LevelMeterComponentInstance)
            
        public:
            LevelMeterComponent() : Component() { }
            
            nap::ComponentPtr<AudioComponentBase> mInput; /**< The component whose audio output will be measured. */
            TimeValue mAnalysisWindowSize = 10; /**< Size of an analysis window in milliseconds */
            LevelMeterNode::Type mMeterType = LevelMeterNode::Type::RMS; /**< Type of analysis to be used: RMS for root mean square, PEAK for the peak of the analysis window */
            bool mFilterInput = false; /**< If set to true the input signal will be filtered before being measured. */
            ControllerValue mCenterFrequency = 10000.f; /**< Center frequency of the frequency band that will be analyzed. Only has effect when mFilterInput = true. */
            ControllerValue mBandWidth = 10000.f; /**< Width in Hz of the frequency band that will be analyzed. Only has effect when mFilterInput = true.*/
            ControllerValue mFilterGain = 1.0f; /**< Gain factor of the filtered input signal. Only has effect when mFilterInput = true. */
            
        private:
        };
        
        
        /**
         * Instance of component to measure the amplitude level of the audio signal from an @AudioComponentBase.
         * A specific frequency band to be meusured can be specified.
         */
        class NAPAPI LevelMeterComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(ComponentInstance)
        public:
            LevelMeterComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(utility::ErrorState& errorState) override;
            
            /**
             * Returns the current level for a certain channel
             */
            ControllerValue getLevel(int channel);
    
            /**
             * Sets the center frequency in Hz of the band that will be analyzed.
             * Only has effect when the property mFilterInput in @LevelMeterComponent is set to true.
             */
            void setCenterFrequency(ControllerValue centerFrequency);
            
            /**
             * Sets the bandwidth in Hz of the band that will be analyzed.
             * Only has effect when the property mFilterInput in @LevelMeterComponent is set to true.
             */
            void setBandWidth(ControllerValue bandWidth);
            
            /**
             * Sets the gain factor of the filtered signal.
             * Only has effect when the property mFilterInput in @LevelMeterComponent is set to true.
             */
            void setFilterGain(ControllerValue gain);
            
            /**
             * Returns the center frequency in Hz of the band that will be analyzed.
             * Always returns 0 when the property mFilterInput in @LevelMeterComponent is set to false.
             */
            ControllerValue getCenterFrequency() const;
            
            /**
             * Returns the bandwidth in Hz of the band that will be analyzed.
             * Always returns 0 when the property mFilterInput in @LevelMeterComponent is set to false.
             */
            ControllerValue getBandWidth() const;
            
            /**
             * Returns the gain factor of the filtered signal.
             * Always returns 0 when the property mFilterInput in @LevelMeterComponent is set to false.
             */
            ControllerValue getFilterGain() const;
            
        private:
            NodeManager& getNodeManager();
            
            nap::ComponentInstancePtr<AudioComponentBase> mInput = { this, &LevelMeterComponent::mInput };
            std::vector<std::unique_ptr<FilterNode>> mFilters;
            std::vector<std::unique_ptr<LevelMeterNode>> mMeters;
        };
        
    }
    
}
