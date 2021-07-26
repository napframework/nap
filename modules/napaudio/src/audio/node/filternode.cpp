/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "filternode.h"

#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::audio::FilterNode::EMode)
	RTTI_ENUM_VALUE(nap::audio::FilterNode::EMode::LowPass, "LowPass"),
	RTTI_ENUM_VALUE(nap::audio::FilterNode::EMode::HighPass, "HighPass"),
	RTTI_ENUM_VALUE(nap::audio::FilterNode::EMode::BandPass, "BandPass"),
	RTTI_ENUM_VALUE(nap::audio::FilterNode::EMode::LowRes, "LowRes"),
	RTTI_ENUM_VALUE(nap::audio::FilterNode::EMode::HighRes, "HighRes")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::FilterNode)
	RTTI_PROPERTY("audioInput", &nap::audio::FilterNode::audioInput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("audioOutput", &nap::audio::FilterNode::audioOutput, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_FUNCTION("setMode", &nap::audio::FilterNode::setMode)
	RTTI_FUNCTION("setFrequency", &nap::audio::FilterNode::setFrequency)
	RTTI_FUNCTION("setResonance", &nap::audio::FilterNode::setResonance)
	RTTI_FUNCTION("setBand", &nap::audio::FilterNode::setBand)
	RTTI_FUNCTION("setGain", &nap::audio::FilterNode::setGain)
	RTTI_FUNCTION("prepare", &nap::audio::FilterNode::prepare)
	RTTI_FUNCTION("getMode", &nap::audio::FilterNode::getMode)
	RTTI_FUNCTION("getFrequency", &nap::audio::FilterNode::getFrequency)
	RTTI_FUNCTION("getResonance", &nap::audio::FilterNode::getResonance)
	RTTI_FUNCTION("getBand", &nap::audio::FilterNode::getBand)
	RTTI_FUNCTION("getGain", &nap::audio::FilterNode::getGain)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		
		
		void FilterNode::process()
		{
			if (mIsDirty.check())
				update();

			auto inputBuffer = audioInput.pull();
			auto& outputBuffer = getOutputBuffer(audioOutput);
			
			if (inputBuffer) {
				for (auto i = 0; i < outputBuffer.size(); ++i) {
					mInput.write((*inputBuffer)[i]);
					auto temp = a0.getNextValue() * mInput.read(0) + a1.getNextValue() * mInput.read(1) + a2.getNextValue() * mInput.read(2) - b1.getNextValue() * mOutput.read(0) -
					            b2.getNextValue() * mOutput.read(1);
					
					mOutput.write(temp);
					outputBuffer[i] = temp;
				}
			} else {
				// process with 0 input
				for (auto i = 0; i < outputBuffer.size(); ++i) {
					mInput.write(0);
					auto temp = a0.getNextValue() * mInput.read(0) + a1.getNextValue() * mInput.read(1) + a2.getNextValue() * mInput.read(2) - b1.getNextValue() * mOutput.read(0) -
					            b2.getNextValue() * mOutput.read(1);
					
					mOutput.write(temp);
					outputBuffer[i] = temp;
				}
			}
		}


		void FilterNode::prepare(ControllerValue frequency, ControllerValue resonanceBand, ControllerValue gain)
		{
			mFrequency = frequency;
			mResonance = pow(10., -(resonanceBand * 0.1));
			mBand = resonanceBand;
			mGain = gain;
			mOutput.clear();
			mInput.clear();
			calcCoeffs();
			a0.reset(a0Dest);
			a1.reset(a1Dest);
			b1.reset(b1Dest);
			b2.reset(b2Dest);

		}

		
		
		void FilterNode::setMode(EMode mode)
		{
			mMode = mode;
			calcCoeffs();
			mIsDirty.set();
		}
		
		
		void FilterNode::setFrequency(ControllerValue frequency)
		{
			mFrequency = math::max<float>(frequency, 1.f);
			calcCoeffs();
			mIsDirty.set();
		}
		
		
		void FilterNode::setResonance(ControllerValue resonance)
		{
			mResonance = pow(10., -(resonance * 0.1));
			calcCoeffs();
			mIsDirty.set();
		}
		
		
		void FilterNode::setBand(ControllerValue band)
		{
			mBand = band;
			if (mBand <= 0)
				mBand = 1;
			calcCoeffs();
			mIsDirty.set();
		}
		
		
		void FilterNode::setGain(ControllerValue gain)
		{
			mGain = gain;
			calcCoeffs();
			mIsDirty.set();
		}
		
		
		void FilterNode::update()
		{
			a0.setValue(a0Dest);
			a1.setValue(a1Dest);
			b1.setValue(b1Dest);
			b2.setValue(b2Dest);
		}
		
		
		void FilterNode::calcCoeffs()
		{
			switch (mMode) {
				case EMode::LowPass:
				{
					ControllerValue c, d, cSqr, q;
					c	 = 1 / tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					a0Dest = mGain / (1 + M_SQRT2 * c + cSqr);
					a1Dest = 2 * a0Dest;
					a2Dest = a0Dest;
					b1Dest = 2 * a0Dest * (1 - cSqr);
					b2Dest =a0Dest * (1 - M_SQRT2 * c + cSqr);
					break;
				}
				case EMode::HighPass:
				{
					ControllerValue c, d, cSqr, q;
					c = tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					a0Dest = mGain / (1 + M_SQRT2 * c + cSqr);
					a1Dest = -2 * a0Dest;
					a2Dest = a0Dest;
					b1Dest = 2 * a0Dest * (cSqr - 1);
					b2Dest = a0Dest * (1 - M_SQRT2 * c + cSqr);
					break;
				}
				case EMode::BandPass:
				{
					ControllerValue c, d, cSqr, q;
					c = 1 / tan(M_PI * mBand / getSampleRate());
					d = 2 * cos(2 * M_PI * mFrequency / getSampleRate());
					a0Dest = mGain / (1 + c);
					a1Dest = 0;
					a2Dest = -a0Dest;
					b1Dest = a2Dest * c * d;
					b2Dest = a0Dest * (c - 1);
					break;
				}
				case EMode::LowRes:
				{
					ControllerValue c, d, cSqr, q;
					c = 1 / tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					q = M_SQRT2 * mResonance;
					a0Dest = mGain / (1 + q * c + cSqr);
					a1Dest = 2 * a0Dest;
					a2Dest = a0Dest;
					b1Dest = 2 * a0Dest * (1 - cSqr);
					b2Dest = a0Dest * (1 - q * c + cSqr);
					break;
				}
				case EMode::HighRes:
				{
					ControllerValue c, d, cSqr, q;
					c = tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					q = M_SQRT2 * mResonance;
					a0Dest = mGain / (1 + q * c + cSqr);
					a1Dest = -2 * a0Dest;
					a2Dest = a0Dest;
					b1Dest = 2 * a0Dest * (cSqr - 1);
					b2Dest = a0Dest * (1 - q * c + cSqr);
					break;
				}
			}
		}
		
		
	}
}
