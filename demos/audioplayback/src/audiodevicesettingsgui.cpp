/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiodevicesettingsgui.h"
#include "nap/logger.h"

#include <imgui/imgui.h>

namespace nap
{
    namespace audio
    {
        static int getIndexOf(int value, std::vector<int>& list)
        {
            for (auto i = 0; i < list.size(); ++i)
                if (list[i] == value)
                    return i;

            return -1;
        }


        AudioDeviceSettingsGui::AudioDeviceSettingsGui(AudioService& audioService, bool hasInputs) : mAudioService(audioService), mHasInputs(hasInputs)
        {
            mDriverSelection = mAudioService.getCurrentHostApiIndex() + 1;
            mInputDeviceSelection = 0;
            mOutputDeviceSelection = 0;

            for (auto hostApiIndex = 0; hostApiIndex < mAudioService.getHostApiCount(); ++hostApiIndex)
            {
                DriverInfo driverInfo;
                driverInfo.mIndex = hostApiIndex;
                driverInfo.mName = mAudioService.getHostApiName(hostApiIndex);
                auto devices = mAudioService.getDevices(hostApiIndex);
                for (auto deviceIndex = 0; deviceIndex < devices.size(); deviceIndex++)
                {
                    auto device = devices[deviceIndex];
                    DeviceInfo deviceInfo;
                    deviceInfo.mName = device->name;
                    deviceInfo.mIndex = mAudioService.getDeviceIndex(hostApiIndex, deviceIndex);
                    if (mHasInputs)
                    {
                        if (device->maxInputChannels > 0)
                        {
                            if (deviceInfo.mIndex == mAudioService.getCurrentInputDeviceIndex())
                                mInputDeviceSelection = driverInfo.mInputDevices.size() + 1;

                            deviceInfo.mChannelCount = device->maxInputChannels;
                            driverInfo.mInputDevices.push_back(deviceInfo);
                        }
                    }

                    if (device->maxOutputChannels > 0)
                    {
                        if (deviceInfo.mIndex == mAudioService.getCurrentOutputDeviceIndex())
                            mOutputDeviceSelection = driverInfo.mOutputDevices.size() + 1;

                        deviceInfo.mChannelCount = device->maxOutputChannels;
                        driverInfo.mOutputDevices.push_back(deviceInfo);
                    }
                }
                mDrivers.push_back(driverInfo);
            }

            mBufferSizeIndex = getIndexOf(mAudioService.getCurrentBufferSize(), mBufferSizes);
            mSampleRateIndex = getIndexOf(int(mAudioService.getNodeManager().getSampleRate()), mSampleRates);
        }


        void AudioDeviceSettingsGui::drawGui()
        {
            utility::ErrorState error_state;
            bool change = false;
            auto settings = mAudioService.getDeviceSettings();

            // Combo box with all available drivers
            change = ImGui::Combo("Driver", &mDriverSelection, [](void* data, int index, const char** out_text)
            {
                auto* drivers = static_cast<std::vector<DriverInfo>*>(data);
                if (index == 0)
                    *out_text = "No Driver";
                else
                    *out_text = drivers->at(index - 1).mName.c_str();
                return true;
            }, &mDrivers, mDrivers.size() + 1);

            if (mDriverSelection > 0)
            {
                // driver has changed
                if (change)
                {
                    settings.mHostApi = mDrivers[mDriverSelection-1].mName;

                    mInputDeviceSelection = 0;
                    mOutputDeviceSelection = 0;

                    mSampleRateIndex = getIndexOf(int(mAudioService.getNodeManager().getSampleRate()), mSampleRates);
                    if (mSampleRateIndex < 0)
                        mSampleRateIndex = 0;

                    mBufferSizeIndex = getIndexOf(mAudioService.getCurrentBufferSize(), mBufferSizes);
                    if (mBufferSizeIndex < 0)
                        mBufferSizeIndex = 0;
                }

                assert(mDriverSelection-1 < mDrivers.size());

                // update inputs
                mHasInputs = !mDrivers[mDriverSelection-1].mInputDevices.empty();

                // input and output device info pointers
                DeviceInfo* input_device = nullptr;
                DeviceInfo* output_device = nullptr;

                // Combo box with all input devices for the chosen driver
                if (mHasInputs)
                {
	                change = ImGui::Combo("Input Device", &mInputDeviceSelection, [](void* data, int index, const char** out_text)
                    {
		                auto* devices = static_cast<std::vector<DeviceInfo>*>(data);
		                if(index == 0)
			                *out_text = "No Device";
		                else
			                *out_text = devices->at(index - 1).mName.c_str();
		                return true;
	                }, &mDrivers[mDriverSelection - 1].mInputDevices, mDrivers[mDriverSelection - 1].mInputDevices.size() + 1) || change;

                    if (mInputDeviceSelection > 0)
                    {
                        input_device = &mDrivers[mDriverSelection-1].mInputDevices[mInputDeviceSelection-1];
                    }
                }

                // Combo box with all output devices for the chosen driver
                change = ImGui::Combo("Output Device", &mOutputDeviceSelection, [](void* data, int index, const char** out_text)
                {
                    auto* devices = static_cast<std::vector<DeviceInfo>*>(data);
                    if(index == 0)
                        *out_text = "No Device";
                    else
                        *out_text = devices->at(index - 1).mName.c_str();
                    return true;
                }, &mDrivers[mDriverSelection - 1].mOutputDevices, mDrivers[mDriverSelection - 1].mOutputDevices.size() + 1) || change;

                if (mOutputDeviceSelection > 0)
                {
                    output_device = &mDrivers[mDriverSelection-1].mOutputDevices[mOutputDeviceSelection-1];
                }

                // Combo box for sample rate and buffersize
                change = ImGui::Combo("Sample Rate", &mSampleRateIndex, mSampleRateNames, mSampleRates.size()) || change;
                change = ImGui::Combo("Buffer Size", &mBufferSizeIndex, mBufferSizeNames, mBufferSizes.size()) || change;

                // Sliders for channels
                int input_channels = mAudioService.getNodeManager().getInputChannelCount();
                if (input_device!= nullptr)
                {
                    int max_input_channels = input_device->mChannelCount;
                    change = ImGui::SliderInt("Input channels", &input_channels, 1, max_input_channels) || change;
                }
                int output_channels = mAudioService.getNodeManager().getOutputChannelCount();
                if (output_device!= nullptr)
                {
                    int max_output_channels = output_device->mChannelCount;
                    change = ImGui::SliderInt("Output channels", &output_channels, 1, max_output_channels) || change;
                }

                // a change in settings has occurred, close & open audio stream
                if (change)
                {
                    // apply settings
                    settings.mOutputDevice = output_device != nullptr ? output_device->mName : "";
                    settings.mInputDevice = input_device != nullptr ? input_device->mName : "";
                    settings.mBufferSize = mBufferSizes[mBufferSizeIndex];
                    settings.mInternalBufferSize = mBufferSizes[mBufferSizeIndex];
                    settings.mSampleRate = mSampleRates[mSampleRateIndex];
                    settings.mHostApi = mDriverSelection - 1 >= 0 ? mDrivers[mDriverSelection-1].mName : "";
                    settings.mOutputChannelCount = output_channels;
                    settings.mInputChannelCount = input_channels;

                    if (mAudioService.isOpened())
                    {
                        if (mAudioService.isActive())
                            mAudioService.stop(error_state);

	                    mAudioService.closeStream(error_state);
                    }
                    if (mAudioService.openStream(settings, error_state))
                    {
	                    mAudioService.start(error_state);
                    }
                }
            }
            else
			{
                if (change)
                {
                    if (mAudioService.isOpened())
                    {
                        if (mAudioService.isActive())
                            mAudioService.stop(error_state);
                    }
                }
            }
        }
    }
}
