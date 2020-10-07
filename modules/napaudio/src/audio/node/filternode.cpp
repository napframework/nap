#include "filternode.h"

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
					auto temp = a0 * mInput.read(0) + a1 * mInput.read(1) + a2 * mInput.read(2) - b1 * mOutput.read(0) -
					            b2 * mOutput.read(1);
					
					mOutput.write(temp);
					outputBuffer[i] = temp;
				}
			} else {
				// process with 0 input
				for (auto i = 0; i < outputBuffer.size(); ++i) {
					mInput.write(0);
					auto temp = a0 * mInput.read(0) + a1 * mInput.read(1) + a2 * mInput.read(2) - b1 * mOutput.read(0) -
					            b2 * mOutput.read(1);
					
					mOutput.write(temp);
					outputBuffer[i] = temp;
				}
			}
		}
		
		
		void FilterNode::setMode(EMode mode)
		{
			mMode = mode;
			mIsDirty.set();
		}
		
		
		void FilterNode::setFrequency(ControllerValue frequency)
		{
			mFrequency = frequency;
			if (mFrequency <= 0)
				mFrequency = 1;
			mIsDirty.set();
		}
		
		
		void FilterNode::setResonance(ControllerValue resonance)
		{
			mResonance = pow(10., -(resonance * 0.1));
			mIsDirty.set();
		}
		
		
		void FilterNode::setBand(ControllerValue band)
		{
			mBand = band;
			if (mBand <= 0)
				mBand = 1;
			mIsDirty.set();
		}
		
		
		void FilterNode::setGain(ControllerValue gain)
		{
			mGain = gain;
			mIsDirty.set();
		}
		
		
		void FilterNode::update()
		{
			ControllerValue c, d, cSqr, q;
			switch (mMode) {
				case EMode::LowPass:
					c = 1 / tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					a0 = (1 / (1 + M_SQRT2 * c + cSqr));
					a1 = 2 * a0;
					a2 = a0;
					b1 = 2 * a0 * (1 - cSqr);
					b2 = a0 * (1 - M_SQRT2 * c + cSqr);
					break;
				case EMode::HighPass:
					c = tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					a0 = 1 / (1 + M_SQRT2 * c + cSqr);
					a1 = -2 * a0;
					a2 = a0;
					b1 = 2 * a0 * (cSqr - 1);
					b2 = a0 * (1 - M_SQRT2 * c + cSqr);
					break;
				case EMode::BandPass:
					c = 1 / tan(M_PI * mBand / getSampleRate());
					d = 2 * cos(2 * M_PI * mFrequency / getSampleRate());
					a0 = 1 / (1 + c);
					a1 = 0;
					a2 = -a0;
					b1 = a2 * c * d;
					b2 = a0 * (c - 1);
					break;
				case EMode::LowRes:
					c = 1 / tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					q = M_SQRT2 * mResonance;
					a0 = (1 / (1 + q * c + cSqr));
					a1 = 2 * a0;
					a2 = a0;
					b1 = 2 * a0 * (1 - cSqr);
					b2 = a0 * (1 - q * c + cSqr);
					break;
				case EMode::HighRes:
					c = tan(M_PI * mFrequency / getSampleRate());
					cSqr = c * c;
					q = M_SQRT2 * mResonance;
					a0 = 1 / (1 + q * c + cSqr);
					a1 = -2 * a0;
					a2 = a0;
					b1 = 2 * a0 * (cSqr - 1);
					b2 = a0 * (1 - q * c + cSqr);
					break;
			}
			a0 *= mGain;
			a1 *= mGain;
			a2 *= mGain;
		}
		
		
	}
}
