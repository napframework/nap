#include "delay.h"

#include <audio/utility/audiofunctions.h>

namespace nap
{
	
	namespace audio
	{
		
		Delay::Delay(unsigned int bufferSize)
		{
			mBuffer.resize(bufferSize, 0.0);
			mWriteIndex = 0;
		}
		
		
		void Delay::write(SampleValue sample)
		{
			mBuffer[mWriteIndex++] = sample;
			mWriteIndex = wrap(mWriteIndex, mBuffer.size());
		}
		
		
		SampleValue Delay::readInterpolating(float time)
		{
			// Update the read index
			SampleValue readIndex = mWriteIndex - time - 1;
			while (readIndex < 0) readIndex += mBuffer.size();
			
			unsigned int floorReadIndex = (unsigned int) readIndex;
			unsigned int integerIndex = wrap(floorReadIndex, mBuffer.size());
			unsigned int nextIntegerIndex = wrap(integerIndex + 1, mBuffer.size());
			
			SampleValue frac = readIndex - floorReadIndex;
			
			return lerp(mBuffer[integerIndex], mBuffer[nextIntegerIndex], frac);
		}
		
		
		SampleValue Delay::read(unsigned int time)
		{
			// Update the read index
			int readIndex = mWriteIndex - time - 1;
			while (readIndex < 0) readIndex += mBuffer.size();
			
			unsigned int integerIndex = wrap(readIndex, mBuffer.size());
			
			return mBuffer[integerIndex];
		}
		
		
		void Delay::clear()
		{
			for (unsigned int i = 0; i < mBuffer.size(); i++) {
				mBuffer[i] = 0.0;
			}
		}
		
		
	}
}
