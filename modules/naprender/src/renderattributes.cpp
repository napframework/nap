// Local Includes
#include <rtti/rtti.h>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

// TODO: move glm RTTI definitions to some central place

RTTI_BEGIN_CLASS(glm::vec2)
	RTTI_PROPERTY("x", &glm::vec2::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec2::y, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::vec3)
	RTTI_PROPERTY("x", &glm::vec3::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec3::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec3::z, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::vec4)
	RTTI_PROPERTY("x", &glm::vec4::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec4::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec4::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::vec4::w, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::ivec2)
	RTTI_PROPERTY("x", &glm::ivec2::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec2::y, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::ivec3)
	RTTI_PROPERTY("x", &glm::ivec3::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec3::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::ivec3::z, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::quat)
	RTTI_PROPERTY("x", &glm::quat::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::quat::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::quat::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::quat::w, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS