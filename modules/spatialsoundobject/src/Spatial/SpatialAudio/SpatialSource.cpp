#include "SpatialSource.h"

// Spatial includes
#include <Spatial/SpatialAudio/MixdownComponent.h>
#include <Spatial/Core/RootProcess.h>
#include <Spatial/Core/SpatialService.h>

// Audio includes
#include <audio/service/audioservice.h>

// Nap includes
#include <mathutils.h>
#include <entity.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialSource)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialSourceInstance)
RTTI_END_CLASS


namespace nap
{

    namespace spatial
    {

        SpatialSourceInstance::SpatialSourceInstance(SpatialService& spatialService, int channelCount, ParameterComponentInstance& parameterComponent, const std::string& namePrefix) : mSpatialService(spatialService), mParticleCount(channelCount)
        {
            mParameterManager.init(parameterComponent, namePrefix, "");
        }


    }
}
