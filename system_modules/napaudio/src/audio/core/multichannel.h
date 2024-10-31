/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <rtti/objectptr.h>

// Audio includes
#include <audio/core/audiopin.h>

namespace nap
{
	namespace audio
	{
		/**
		 * Interface for any class that exposes multichannel audio output.
		 */
		class NAPAPI IMultiChannelOutput
		{
		public:
			/**
			 * Destructor
			 */
			virtual ~IMultiChannelOutput() = default;

			/**
			 * Override this method to specify the number of audio channels output by this object.
			 * @return the number of channels this object outputs.
			 */
			virtual int getChannelCount() const = 0;

			/**
			 * To be overridden by descendants.
			 * @param channel channel index to request the output for
			 * @return the output pin that outputs audio data for the specified channel.
			 */
			virtual OutputPin* getOutputForChannel(int channel) = 0;

			/**
			 * @return an output pin for a given channel, but returns nullptr if the channel is out of bounds.
			 */
			OutputPin* tryGetOutputForChannel(unsigned int);

		};


		/**
		 * Interface for any class that exposes multichannel audio input.
		 */
		class NAPAPI IMultiChannelInput
		{
		public:
			/**
			 * Destructor
			 */
			virtual ~IMultiChannelInput() = default;

			/**
			 * This method has to be overwritten to connect an output pin from another object to this object's input.
			 * @param channel index of the channel to connect to
			 * @param pin pin that will be connected to this object
			 */
			virtual void connect(unsigned int channel, OutputPin& pin) { }

			/**
			 * This method has to be overwritten by descendants.
			 * @return the number of input channels of audio this object receives.
			 */
			virtual int getInputChannelCount() const { return 0; }

			/**
			 * This method calls connect() but first checks wether the given channel is not out of bounds.
			 * @param channel channel index to connect to
			 * @param pin pin that will be connected to this object
			 */
			void tryConnect(unsigned int channel, OutputPin& pin);

			/**
			 * Convenience method that connects the outputs of inputObject to the inputs of this object.
			 * If this object has more input channels than inputObject has output channels they will be repeated.
			 * @param inputObject object to be connected to this object
			 */
			void connect(IMultiChannelOutput& inputObject);
		};
	}
}
