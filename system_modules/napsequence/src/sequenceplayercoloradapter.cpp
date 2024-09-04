//
// Created by codeflow on 16-7-24.
//


// NAP includes
#include "mathutils.h"

// Local includes
#include "sequenceplayercoloradapter.h"
#include "sequencetrackcolor.h"
#include "sequencetrackcolor.h"
#include "sequencetracksegmentcolor.h"
#include "sequenceutils.h"


namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    SequencePlayerColorAdapter::SequencePlayerColorAdapter(const SequenceTrack& track, SequencePlayerColorOutput& output,
                                                           const SequencePlayer& player)
            : mPlayer(player), mTrack(track), mOutput(output)
    {
        if(mOutput.mUseMainThread)
        {
            mSetFunction = [this](const RGBAColorFloat& color) { mOutput.setColor(color); };
        } else
        {
            mSetFunction = [this](const RGBAColorFloat& color) { mOutput.storeColor(color); };
        }
    }

    void SequencePlayerColorAdapter::tick(double time)
    {
        // get track event
        const auto& color_track = static_cast<const SequenceTrackColor&>(mTrack);

        SequenceTrackSegment* prev_segment = nullptr;
        for(const auto& segment: color_track.mSegments)
        {
            double source_start_time = 0.0;
            RGBAColorFloat source_color = {0.0f, 0.0f, 0.0f, 0.0f};

            // check if we are in the segment
            if(time >= source_start_time && time < segment->mStartTime)
            {
                // first segment, so get previous color for blending
                if(prev_segment != nullptr)
                {
                    const auto &source = static_cast<const SequenceTrackSegmentColor&>(*prev_segment);
                    source_start_time = source.mStartTime;
                    source_color = source.mColor;
                }

                // get target
                const auto &target = static_cast<const SequenceTrackSegmentColor&>(*segment.get());

                // calculate the blend factor
                float blend = target.mCurve->evaluate((float)(time - source_start_time) / (float)(target.mStartTime - source_start_time));
                blend = math::clamp(blend, 0.0f, 1.0f);

                // obtain the color
                auto color = utility::colorspace::blendColors(source_color,target.mColor, blend, target.mBlendMethod);

                // call set or store on the output
                mSetFunction(color);

                break;
            }else
            {
                prev_segment = segment.get();
            }
        }
    }
}