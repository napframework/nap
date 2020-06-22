#pragma once

// Local Includes
#include <sequencetracksegment.h>

// External Includes
#include <audio/resource/audiofileresource.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI SequenceTrackSegmentAudio : public SequenceTrackSegment
	{
		RTTI_ENABLE(SequenceTrackSegment)
	public:
		ResourcePtr<audio::AudioFileResource> mFile;
	private:
	};
}
