#include "sequenceplayeraudioadapter.h"
#include "sequenceplayer.h"
#include "sequencetrackaudio.h"
#include "sequencetracksegmentaudio.h"

#include <nap/logger.h>

namespace nap
{
	static bool sRegisteredInFactory = SequencePlayerAdapter::registerFactory(RTTI_OF(SequenceTrackEvent), [](SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)->std::unique_ptr<SequencePlayerAdapter>
	{
		assert(output.get_type() == RTTI_OF(SequencePlayerAudioOutput)); // type mismatch

		auto& audio_output = static_cast<SequencePlayerAudioOutput&>(output);

		auto adapter = std::make_unique<SequencePlayerAudioAdapter>(track, audio_output, player);
		return std::move(adapter);
	});


	SequencePlayerAudioAdapter::SequencePlayerAudioAdapter(SequenceTrack& track, SequencePlayerAudioOutput& output, const SequencePlayer& player)
		: mTrack(track), mOutput(output)
	{
		SequenceTrackAudio& audio_track = static_cast<SequenceTrackAudio&>(mTrack);

		for (auto& segment : audio_track.mSegments)
		{
			SequenceTrackSegmentAudio& segment_audio = static_cast<SequenceTrackSegmentAudio&>(*segment.get());
			mOutput.addBufferPlayer(segment_audio.mFile.get());
		}
	}


	SequencePlayerAudioAdapter::~SequencePlayerAudioAdapter()
	{
		SequenceTrackAudio& audio_track = static_cast<SequenceTrackAudio&>(mTrack);

		for (auto& segment : audio_track.mSegments)
		{
			SequenceTrackSegmentAudio& segment_audio = static_cast<SequenceTrackSegmentAudio&>(*segment.get());
			mOutput.removeBufferPlayer(segment_audio.mFile.get());
		}
	}


	void SequencePlayerAudioAdapter::tick(double time)
	{
	}
}
