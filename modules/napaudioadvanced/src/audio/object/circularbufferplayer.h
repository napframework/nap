#pragma once

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/circularbufferplayernode.h>

namespace nap
{

    namespace audio
    {

        class NAPAPI CircularBufferPlayer : public ParallelNodeObject<CircularBufferPlayerNode> {
            RTTI_ENABLE(ParallelNodeObjectBase)
            
        public:
            CircularBufferPlayer() = default;
        };

    }

}
