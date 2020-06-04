#include <Spatial/Core/SpatialTypes.h>

// Audio includes
#include <audio/core/audiopin.h>

RTTI_BEGIN_CLASS(glm::vec3)
    RTTI_CONSTRUCTOR(float, float, float)
    RTTI_PROPERTY("x", &glm::vec3::x, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("y", &glm::vec3::y, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("z", &glm::vec3::z, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::vec4)
    RTTI_CONSTRUCTOR(float, float, float, float)
    RTTI_PROPERTY("x", &glm::vec4::x, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("y", &glm::vec4::y, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("z", &glm::vec4::z, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("a", &glm::vec4::a, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

using SpatialOutputSignal = nap::Signal<const nap::spatial::SpatialOutput&>;
RTTI_BEGIN_CLASS(SpatialOutputSignal)
    RTTI_FUNCTION("connect", (void(SpatialOutputSignal::*)(const pybind11::function))&SpatialOutputSignal::connect)
RTTI_END_CLASS

using ParticleSignal = nap::Signal<const nap::spatial::Particle&>;
RTTI_BEGIN_CLASS(ParticleSignal)
    RTTI_FUNCTION("connect", (void(ParticleSignal::*)(const pybind11::function))&ParticleSignal::connect)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::SpatialTransform)
    RTTI_CONSTRUCTOR(glm::vec3, glm::vec3, glm::vec4)
    RTTI_PROPERTY("Position", &nap::spatial::SpatialTransform::mPosition, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Rotation", &nap::spatial::SpatialTransform::mRotation, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Scale", &nap::spatial::SpatialTransform::mScale, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialOutput)
    RTTI_FUNCTION("setTransform", &nap::spatial::SpatialOutput::setTransform)
    RTTI_FUNCTION("getPosition", &nap::spatial::SpatialOutput::getPosition)
    RTTI_FUNCTION("getRotation", &nap::spatial::SpatialOutput::getRotation)
    RTTI_FUNCTION("getScale", &nap::spatial::SpatialOutput::getScale)
    RTTI_FUNCTION("setOutputPin", &nap::spatial::SpatialOutput::setOutputPin)
    RTTI_FUNCTION("getOutputPin", &nap::spatial::SpatialOutput::getOutputPin)
    RTTI_FUNCTION("getTransformChangedSignal", &nap::spatial::SpatialOutput::getTransformChangedSignal)
    RTTI_FUNCTION("getOutputPinChangedSignal", &nap::spatial::SpatialOutput::getOutputPinChangedSignal)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::Particle)
    RTTI_FUNCTION("setTransform", &nap::spatial::Particle::setTransform)
    RTTI_FUNCTION("getPosition", &nap::spatial::Particle::getPosition)
    RTTI_FUNCTION("getRotation", &nap::spatial::Particle::getRotation)
    RTTI_FUNCTION("getScale", &nap::spatial::Particle::getScale)
    RTTI_FUNCTION("setOutputPin", &nap::spatial::Particle::setOutputPin)
    RTTI_FUNCTION("getOutputPin", &nap::spatial::Particle::getOutputPin)
    RTTI_FUNCTION("setActive", &nap::spatial::Particle::setActive)
    RTTI_FUNCTION("isActive", &nap::spatial::Particle::isActive)
    RTTI_FUNCTION("getTransformChangedSignal", &nap::spatial::Particle::getTransformChangedSignal)
    RTTI_FUNCTION("getOutputPinChangedSignal", &nap::spatial::Particle::getOutputPinChangedSignal)
    RTTI_FUNCTION("getActiveChangedSignal", &nap::spatial::Particle::getActiveChangedSignal)
RTTI_END_CLASS


namespace nap
{

    namespace spatial
    {

        SpatialOutput::SpatialOutput(audio::OutputPin* pin, const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale)
        {
            mTransform.mPosition = position;
            mTransform.mRotation = rotation;
            mTransform.mScale = scale;
            mOutputPin = pin;
        }


        void SpatialOutput::setTransform(const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale)
        {
            if (position == mTransform.mPosition && scale == mTransform.mScale && rotation == mTransform.mRotation)
                return;
           
            mTransform.mPosition = position;
            mTransform.mRotation = rotation;
            mTransform.mScale = scale;
            transformChangedSignal(*this);
        }


        void SpatialOutput::setOutputPin(audio::OutputPin& pin)
        {
            mOutputPin = &pin;
            outputPinChangedSignal(*this);
        }


        Particle::Particle(audio::OutputPin* pin, const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale, bool active)
        : mOutput(pin, position, rotation, scale)
        {
            mIsActive = active;

        }


        void Particle::setActive(bool active)
        {
        	if (active != mIsActive)
        	{
				mIsActive = active;
				activeChangedSignal(*this);
			}
        }


        void Particle::setTransform(const glm::vec3& position, const glm::vec4& rotation, const glm::vec3& scale)
        {
            mOutput.setTransform(position, rotation, scale);
        }

    }

}
