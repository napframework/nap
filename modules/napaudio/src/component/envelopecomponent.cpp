#include "envelopecomponent.h"

// Nap includes
#include <nap/entity.h>

// Audio includes
#include <node/envelopenode.h>

// RTTI
RTTI_BEGIN_ENUM(nap::audio::ControlNode::RampMode)
    RTTI_ENUM_VALUE(nap::audio::ControlNode::RampMode::LINEAR, "Linear"),
    RTTI_ENUM_VALUE(nap::audio::ControlNode::RampMode::EXPONENTIAL, "Exponential")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::audio::EnvelopeGenerator::Segment)
    RTTI_PROPERTY("Duration", &nap::audio::EnvelopeGenerator::Segment::duration, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Destination", &nap::audio::EnvelopeGenerator::Segment::destination, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("DurationRelative", &nap::audio::EnvelopeGenerator::Segment::durationRelative, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("RampMode", &nap::audio::EnvelopeGenerator::Segment::mode, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::EnvelopeComponent)
    RTTI_PROPERTY("Segments", &nap::audio::EnvelopeComponent::mSegments, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::EnvelopeComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool EnvelopeComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            EnvelopeComponent* resource = rtti_cast<EnvelopeComponent>(getComponent());
            
            auto& nodeManager = getNodeManager();

            mEnvelopeGenerator = std::make_unique<EnvelopeGenerator>(nodeManager);
            mEnvelopeGenerator->trigger(resource->mSegments);
        }
        
    }
    
}
