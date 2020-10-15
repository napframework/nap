/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <component.h>
#include <audio/utility/safeptr.h>

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
		 * A specific frequency band to be measured can be specified.
		 */
		class NAPAPI LevelMeterComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(LevelMeterComponent, LevelMeterComponentInstance)
		
		public:
			LevelMeterComponent() : Component()
			{}
			
			nap::ComponentPtr<AudioComponentBase> mInput; ///< property: 'Input' The component whose audio output will be measured.
			TimeValue mAnalysisWindowSize = 10; ///< property: 'AnalysisWindowSize' Size of an analysis window in milliseconds.
			LevelMeterNode::Type mMeterType = LevelMeterNode::Type::RMS; ///< property: 'MeterType' Type of analysis to be used: RMS for root mean square, PEAK for the peak of the analysis window.
			bool mFilterInput = false; ///< If set to true the input signal will be filtered before being measured.
			ControllerValue mCenterFrequency = 10000.f; ///< property: 'CenterFrequency' Center frequency of the frequency band that will be analyzed. Only has effect when mFilterInput = true.
			ControllerValue mBandWidth = 10000.f; ///< property: 'BandWidth' Width in Hz of the frequency band that will be analyzed. Only has effect when mFilterInput = true.
			ControllerValue mFilterGain = 1.0f; ///< property: 'FilterGain' Gain factor of the filtered input signal. Only has effect when mFilterInput = true.
			int mChannel = 0; ///< property: 'Channel' Channel of the input that will be analyzed.
		
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
			LevelMeterComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity,
			                                                                                             resource)
			{}
			
			// Initialize the component
			bool init(utility::ErrorState& errorState) override;
			
			/**
			 * @return the current level for a certain channel
			 */
			ControllerValue getLevel();
			
			/**
			 * Sets the center frequency in Hz of the band that will be analyzed.
			 * Only has effect when the property mFilterInput in LevelMeterComponent is set to true.
			 */
			void setCenterFrequency(ControllerValue centerFrequency);
			
			/**
			 * Sets the bandwidth in Hz of the band that will be analyzed.
			 * Only has effect when the property mFilterInput in LevelMeterComponent is set to true.
			 */
			void setBandWidth(ControllerValue bandWidth);
			
			/**
			 * Sets the gain factor of the filtered signal.
			 * Only has effect when the property mFilterInput in LevelMeterComponent is set to true.
			 */
			void setFilterGain(ControllerValue gain);
			
			/**
			 * @return the center frequency in Hz of the band that will be analyzed.
			 * Always returns 0 when the property mFilterInput in LevelMeterComponent is set to false.
			 */
			ControllerValue getCenterFrequency() const;
			
			/**
			 * @return the bandwidth in Hz of the band that will be analyzed.
			 * Always returns 0 when the property mFilterInput in LevelMeterComponent is set to false.
			 */
			ControllerValue getBandWidth() const;
			
			/**
			 * @return the gain factor of the filtered signal.
			 * Always returns 0 when the property mFilterInput in @LevelMeterComponent is set to false.
			 */
			ControllerValue getFilterGain() const;
			
			/**
			 * Connects a different audio component as input to be analyzed.
			 */
			void setInput(AudioComponentBaseInstance& input);
		
		private:
			ComponentInstancePtr<AudioComponentBase> mInput = {this,
			                                                   &LevelMeterComponent::mInput}; // Pointer to component that outputs this components audio input
			SafeOwner<LevelMeterNode> mMeter = nullptr; // Node doing the actual analysis
			SafeOwner<FilterNode> mFilter = nullptr; // Filter filtering the audio signal for each channel before analysis
			
			LevelMeterComponent* mResource = nullptr;
			AudioService* mAudioService = nullptr;
			
			ControllerValue mCenterFrequency = 10000.f; // Center frequency of the frequency band that will be analyzed. Only has effect when mFilterInput = true.
			ControllerValue mBandWidth = 10000.f; // 'BandWidth' Width in Hz of the frequency band that will be analyzed. Only has effect when mFilterInput = true.
			ControllerValue mFilterGain = 1.0f; // 'FilterGain' Gain factor of the filtered input signal. Only has effect when mFilterInput = true.
		};
		
	}
	
}
