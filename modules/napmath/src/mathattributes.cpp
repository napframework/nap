// Local Includes
#include <rtti/rtti.h>
#include <rtti/rttipath.h>
#include "instanceproperty.h"

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

namespace nap
{
	using Vec2InstancePropertyValue = TypedInstancePropertyValue<glm::vec2>;
	using Vec3InstancePropertyValue = TypedInstancePropertyValue<glm::vec3>;
	using Vec4InstancePropertyValue = TypedInstancePropertyValue<glm::vec4>;
	using IVec2InstancePropertyValue = TypedInstancePropertyValue<glm::ivec2>;
	using IVec3InstancePropertyValue = TypedInstancePropertyValue<glm::ivec3>;
	using QuatInstancePropertyValue = TypedInstancePropertyValue<glm::quat>;
}

template<class PythonClassType>
static void sRegisterFloatVectorOperators(pybind11::module& module, PythonClassType& cls)
{
	cls.def(pybind11::self + pybind11::self);
	cls.def(pybind11::self += pybind11::self);
	cls.def(pybind11::self - pybind11::self);
	cls.def(pybind11::self -= pybind11::self);
	cls.def(pybind11::self * pybind11::self);
	cls.def(pybind11::self *= pybind11::self);
	cls.def(pybind11::self / pybind11::self);
	cls.def(pybind11::self /= pybind11::self);

	cls.def(pybind11::self *= float());
	cls.def(pybind11::self * float());

	cls.def(float() * pybind11::self);
}

template<class PythonClassType>
static void sRegisterIntVectorOperators(pybind11::module& module, PythonClassType& cls)
{
	cls.def(pybind11::self + pybind11::self);
	cls.def(pybind11::self += pybind11::self);
	cls.def(pybind11::self - pybind11::self);
	cls.def(pybind11::self -= pybind11::self);
	cls.def(pybind11::self * pybind11::self);
	cls.def(pybind11::self *= pybind11::self);
	cls.def(pybind11::self / pybind11::self);
	cls.def(pybind11::self /= pybind11::self);

	cls.def(pybind11::self *= int());
	cls.def(pybind11::self * int());

	cls.def(int() * pybind11::self);
}

template<class PythonClassType>
static void sRegisterQuatOperators(pybind11::module& module, PythonClassType& cls)
{
	cls.def(pybind11::self + pybind11::self);
	cls.def(pybind11::self += pybind11::self);
	cls.def(pybind11::self -= pybind11::self);
	cls.def(pybind11::self * pybind11::self);
	cls.def(pybind11::self *= pybind11::self);

	cls.def(pybind11::self *= float());
	cls.def(pybind11::self * float());

	cls.def(float() * pybind11::self);

	module.def("rotate", &glm::rotate<glm::quat::value_type, glm::highp>);
}

RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Vec2InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Vec3InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::Vec3InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::IVec2InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::IVec3InstancePropertyValue)
RTTI_DEFINE_INSTANCE_PROPERTY_VALUE(nap::QuatInstancePropertyValue)

RTTI_BEGIN_CLASS(glm::vec2)
	RTTI_CONSTRUCTOR(float, float)
	RTTI_PROPERTY("x", &glm::vec2::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec2::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterFloatVectorOperators)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::vec3)
	RTTI_CONSTRUCTOR(float, float, float)
	RTTI_PROPERTY("x", &glm::vec3::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec3::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec3::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterFloatVectorOperators)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::vec4)
	RTTI_CONSTRUCTOR(float, float, float, float)
	RTTI_PROPERTY("x", &glm::vec4::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::vec4::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::vec4::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::vec4::w, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterFloatVectorOperators)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::ivec2)
	RTTI_CONSTRUCTOR(int, int)
	RTTI_PROPERTY("x", &glm::ivec2::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec2::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterIntVectorOperators)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::ivec3)
	RTTI_CONSTRUCTOR(int, int, int)
	RTTI_PROPERTY("x", &glm::ivec3::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::ivec3::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::ivec3::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterIntVectorOperators)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(glm::quat)
	RTTI_PROPERTY("x", &glm::quat::x, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("y", &glm::quat::y, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("z", &glm::quat::z, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("w", &glm::quat::w, nap::rtti::EPropertyMetaData::Default)
	RTTI_CUSTOM_REGISTRATION_FUNCTION(sRegisterQuatOperators)
RTTI_END_CLASS