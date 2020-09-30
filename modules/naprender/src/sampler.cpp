#include "uniform.h"
#include "imagefromfile.h"
#include "sampler.h"
#include "renderservice.h"

RTTI_BEGIN_ENUM(nap::EFilterMode)
	RTTI_ENUM_VALUE(nap::EFilterMode::Nearest, "Nearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::Linear, "Linear")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EAddressMode)
	RTTI_ENUM_VALUE(nap::EAddressMode::Repeat,			"Repeat"),
	RTTI_ENUM_VALUE(nap::EAddressMode::MirroredRepeat,	"MirroredRepeat"),
	RTTI_ENUM_VALUE(nap::EAddressMode::ClampToEdge,		"ClampToEdge"),
	RTTI_ENUM_VALUE(nap::EAddressMode::ClampToBorder,	"ClampToBorder")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EAnisotropicSamples)
	RTTI_ENUM_VALUE(nap::EAnisotropicSamples::Default,	"Default"),
	RTTI_ENUM_VALUE(nap::EAnisotropicSamples::Four,		"Four"),
	RTTI_ENUM_VALUE(nap::EAnisotropicSamples::Eight,	"Eight"),
	RTTI_ENUM_VALUE(nap::EAnisotropicSamples::Sixteen,	"Sixteen")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Sampler)
	RTTI_PROPERTY("Name",					&nap::Sampler::mName,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinFilter",				&nap::Sampler::mMinFilter,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxFilter",				&nap::Sampler::mMaxFilter,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MipMapMode",				&nap::Sampler::mMipMapMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AddressModeVertical",	&nap::Sampler::mAddressModeVertical,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AddressModeHorizontal",	&nap::Sampler::mAddressModeHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxLodLevel",			&nap::Sampler::mMaxLodLevel,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AnisotropicSamples",		&nap::Sampler::mMaxAnisotropy,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::SamplerArray)

RTTI_BEGIN_CLASS(nap::Sampler2D)
	RTTI_PROPERTY("Texture", &nap::Sampler2D::mTexture, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Sampler2DArray)
	RTTI_PROPERTY("Textures", &nap::Sampler2DArray::mTextures, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS
