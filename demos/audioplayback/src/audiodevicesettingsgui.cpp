#include "audiodevicesettingsgui.h"

#include <imgui/imgui.h>

namespace nap
{

    namespace audio
    {


        int getIndexOf(int value, std::vector<int>& list)
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
	                    if (device->maxInputChannels > 0)
	                    {
	                        if (deviceInfo.mIndex == mAudioService.getCurrentInputDeviceIndex())
	                            mInputDeviceSelection = driverInfo.mInputDevices.size() + 1;
	                        deviceInfo.mChannelCount = device->maxInputChannels;
	                        driverInfo.mInputDevices.push_back(deviceInfo);
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
            utility::ErrorState errorState;

            // Combo box with all available drivers

            bool driverChanged = ImGui::Combo("Driver", &mDriverSelection, [](void* data, int index, const char** out_text){
                std::vector<DriverInfo>* drivers = (std::vector<DriverInfo>*)(data);
                if (index == 0)
                    *out_text = "No Driver";
                else
                    *out_text = (*drivers)[index - 1].mName.c_str();
                return true;
            }, &mDrivers, mDrivers.size() + 1);
            if (driverChanged)
            {
                if (mAudioService.isOpened())
                {
                    if (mAudioService.isActive()) mAudioService.stop(errorState);
	                mAudioService.closeStream(errorState);
                }
                mInputDeviceSelection = 0;
                mOutputDeviceSelection = 0;
            }


            if (mDriverSelection > 0)
            {
                // Combo box with all input devices for the chosen driver

                bool inputDeviceChanged = false;
                if (mHasInputs)
                {
	                inputDeviceChanged = ImGui::Combo("Input Device", &mInputDeviceSelection, [](void* data, int index, const char** out_text){
		                std::vector<DeviceInfo>* devices = (std::vector<DeviceInfo>*)(data);
		                if (index == 0)
			                *out_text = "No Device";
		                else
			                *out_text = (*devices)[index - 1].mName.c_str();
		                return true;
	                }, &mDrivers[mDriverSelection - 1].mInputDevices, mDrivers[mDriverSelection - 1].mInputDevices.size() + 1);
                }

                // Combo box with all output devices for the chosen driver

                bool outputDeviceChanged = ImGui::Combo("Output Device", &mOutputDeviceSelection, [](void* data, int index, const char** out_text){
                    std::vector<DeviceInfo>* devices = (std::vector<DeviceInfo>*)(data);
                    if (index == 0)
                        *out_text = "No Device";
                    else
                        *out_text = (*devices)[index - 1].mName.c_str();
                    return true;
                }, &mDrivers[mDriverSelection - 1].mOutputDevices, mDrivers[mDriverSelection - 1].mOutputDevices.size() + 1);

                if (inputDeviceChanged || outputDeviceChanged)
                {
                    if (mAudioService.isOpened())
                    {
                        if (mAudioService.isActive()) mAudioService.stop(errorState);
	                    mAudioService.closeStream(errorState);
                    }
                    auto& driver = mDrivers[mDriverSelection - 1];
                    auto* inputDevice = (mHasInputs && mInputDeviceSelection > 0) ? &driver.mInputDevices[mInputDeviceSelection - 1] : nullptr;
                    auto* outputDevice = (mOutputDeviceSelection > 0) ? &driver.mOutputDevices[mOutputDeviceSelection - 1] : nullptr;
                    int inputChannelCount = inputDevice ? inputDevice->mChannelCount : 0;
                    int outputChannelCount = outputDevice ? outputDevice->mChannelCount : 0;

                    if (mAudioService.openStream(driver.mIndex, inputDevice ? inputDevice->mIndex : -1, outputDevice ? outputDevice->mIndex : -1, inputChannelCount, outputChannelCount, mSampleRates[mSampleRateIndex], mBufferSizes[mBufferSizeIndex], mBufferSizes[mBufferSizeIndex], errorState))
                    {
	                    mAudioService.start(errorState);
                    }
                }

                // Combo box for sample rate and buffersize

                bool sampleRateChanged = ImGui::Combo("Sample Rate", &mSampleRateIndex, mSampleRateNames, mSampleRates.size());
                bool bufferSizeChanged = ImGui::Combo("Buffer Size", &mBufferSizeIndex, mBufferSizeNames, mBufferSizes.size());

                // Sliders for channel counts

                bool inputChannelCountChanged = false;
                int inputChannelCount = mAudioService.getNodeManager().getInputChannelCount();
                if (mInputDeviceSelection > 0)
                {
                    int maxInputChannelCount = mDrivers[mDriverSelection - 1].mInputDevices[mInputDeviceSelection - 1].mChannelCount;
                    inputChannelCountChanged = ImGui::SliderInt("Input channels", &inputChannelCount, 1, maxInputChannelCount);
                }

                bool outputChannelCountChanged = false;
                int outputChannelCount = mAudioService.getNodeManager().getOutputChannelCount();
                if (mOutputDeviceSelection > 0)
                {
                    int maxOutputChannelCount = mDrivers[mDriverSelection - 1].mOutputDevices[mOutputDeviceSelection - 1].mChannelCount;
                    outputChannelCountChanged = ImGui::SliderInt("Output channels", &outputChannelCount, 1, maxOutputChannelCount);
                }

                if (sampleRateChanged || bufferSizeChanged || inputChannelCountChanged || outputChannelCountChanged)
                {
                    if (mAudioService.isOpened())
                    {
                        if (mAudioService.isActive()) mAudioService.stop(errorState);
	                    mAudioService.closeStream(errorState);
                    }
                    if (mAudioService.openStream(mAudioService.getCurrentHostApiIndex(),
												 mAudioService.getCurrentInputDeviceIndex(),
                                                 mAudioService.getCurrentOutputDeviceIndex(), inputChannelCount, outputChannelCount, mSampleRates[mSampleRateIndex], mBufferSizes[mBufferSizeIndex], mBufferSizes[mBufferSizeIndex], errorState))
                    {
	                    mAudioService.start(errorState);
                    }
                }

            }
        }




    }

}
