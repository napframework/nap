#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <entity.h>
#include <mathutils.h>

// nap::updatematerialcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
	RTTI_PROPERTY("ScanMeshComponent",				&nap::UpdateMaterialComponent::mScanMeshComponent,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalMeshComponent",			&nap::UpdateMaterialComponent::mNormalMeshComponent,			nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("TileableImageSelectComponent",	&nap::UpdateMaterialComponent::mTileableImageSelectComponent,	nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("SingleImageSelectComponent",		&nap::UpdateMaterialComponent::mSingleImageSelectComponent,		nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("CameraTransformComponent",		&nap::UpdateMaterialComponent::mCameraTransformComponent,		nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("PremultValue",					&nap::UpdateMaterialComponent::mPremultValue,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("ColorTexMix",					&nap::UpdateMaterialComponent::mColorTexMix,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("DiffuseColorMix",				&nap::UpdateMaterialComponent::mDiffuseColorMix,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("VideoMaskValue",					&nap::UpdateMaterialComponent::mVideoMaskValue,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("VideoContrastValue",				&nap::UpdateMaterialComponent::mVideoContrastValue,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("VideoTexMix",					&nap::UpdateMaterialComponent::mVideoTexMix,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("DiffuseColor",					&nap::UpdateMaterialComponent::mDiffuseColor,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("MaskColor",						&nap::UpdateMaterialComponent::mMaskColor,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("ColorTexScaleOne",				&nap::UpdateMaterialComponent::mColorTexScaleOne,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("ColorTexScaleTwo",				&nap::UpdateMaterialComponent::mColorTexScaleTwo,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("TextureSpeed",					&nap::UpdateMaterialComponent::mTextureSpeed,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("VideoTexScaleOne",				&nap::UpdateMaterialComponent::mVideoTexScaleOne,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("VideoTextureSpeed",				&nap::UpdateMaterialComponent::mVideoTextureSpeed,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("LightPos",						&nap::UpdateMaterialComponent::mLightPos,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("LightIntensity",					&nap::UpdateMaterialComponent::mLightIntensity,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("AmbientIntensity",				&nap::UpdateMaterialComponent::mAmbientIntensity,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("DiffuseIntensity",				&nap::UpdateMaterialComponent::mDiffuseIntensity,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalSpecColor",				&nap::UpdateMaterialComponent::mNormalSpecColor,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalSpecColorBlend",			&nap::UpdateMaterialComponent::mNormalSpecColorBlend,			nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalSpecIntens",				&nap::UpdateMaterialComponent::mNormalSpecIntens,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalSpecShine",				&nap::UpdateMaterialComponent::mNormalSpecShine,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalScale",					&nap::UpdateMaterialComponent::mNormalScale,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalRandom",					&nap::UpdateMaterialComponent::mNormalRandom,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("DiffuseSpecInfl",				&nap::UpdateMaterialComponent::mDiffuseSpecInfl,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("NormalRotValue",					&nap::UpdateMaterialComponent::mNormalRotValue,					nap::rtti::EPropertyMetaData::Required);	
	RTTI_PROPERTY("ScanSpecColor",					&nap::UpdateMaterialComponent::mScanSpecColor,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("ScanSpecColorBlend",				&nap::UpdateMaterialComponent::mScanSpecColorBlend,				nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("ScanSpecIntens",					&nap::UpdateMaterialComponent::mScanSpecIntens,					nap::rtti::EPropertyMetaData::Required);	
	RTTI_PROPERTY("ScanSpecShine",					&nap::UpdateMaterialComponent::mScanSpecShine,					nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("ScanRotValue",					&nap::UpdateMaterialComponent::mScanRotValue,					nap::rtti::EPropertyMetaData::Required);	
	RTTI_PROPERTY("WindSpeed",						&nap::UpdateMaterialComponent::mWindSpeed,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("WindScale",						&nap::UpdateMaterialComponent::mWindScale,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("WindFreq",						&nap::UpdateMaterialComponent::mWindFreq,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("WindRandom",						&nap::UpdateMaterialComponent::mWindRandom,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("FogMin",							&nap::UpdateMaterialComponent::mFogMin,							nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("FogMax",							&nap::UpdateMaterialComponent::mFogMax,							nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("FogPower",						&nap::UpdateMaterialComponent::mFogPower,						nap::rtti::EPropertyMetaData::Required);
	RTTI_PROPERTY("FogInfluence",					&nap::UpdateMaterialComponent::mFogInfluence,					nap::rtti::EPropertyMetaData::Required);	
RTTI_END_CLASS

// nap::updatematerialcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateMaterialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
static const std::string sColorTexOneName	= "colorTextureOne";
static const std::string sColorTexTwoName	= "colorTextureTwo";
static const std::string sColorTexScaleOne	= "colorTexScaleOne";
static const std::string sColorTexScaleTwo	= "colorTexScaleTwo";
static const std::string sColorTexMix		= "colorTexMix";
static const std::string sDiffuseColor		= "diffuseColor";
static const std::string sDiffuseColorMix	= "diffuseColorMix";
static const std::string sLightPos			= "lightPos";
static const std::string sLightIntensity	= "lightIntensity";
static const std::string sAmbientIntensity	= "ambientIntensity";
static const std::string sSpecularIntensity = "specularIntensity";
static const std::string sSpecularColor		= "specularColor";
static const std::string sSpecularColorBlend = "specularColorBlend";
static const std::string sShininess			= "shininess";
static const std::string sTime				= "time";
static const std::string sWindSpeed			= "noiseSpeed";
static const std::string sWindScale			= "noiseScale";
static const std::string sWindFreq			= "noiseFreq";
static const std::string sWindRandom		= "noiseRandom";
static const std::string sNormalRandom		= "randomLength";
static const std::string sNormalScale		= "normalScale";
static const std::string sCameraPostion		= "cameraPosition";
static const std::string sPremultValue		= "preMultiplyTexValue";
static const std::string sDiffSpecInfluence = "diffuseSpecularInfluence";
static const std::string sTexTimeU			= "textureTimeU";
static const std::string sTexTimeV			= "textureTimeV";
static const std::string sDiffuseIntensity	= "diffuseIntensity";
static const std::string sRotValue			= "rotationValue";
static const std::string sRotAngle			= "rotationAxis";
static const std::string sFogPower			= "fogPower";
static const std::string sFogMin			= "fogMin";
static const std::string sFogMax			= "fogMax";
static const std::string sFogColor			= "fogColor";
static const std::string sFogInfluence		= "fogInfluence";
static const std::string sVideoTexMix		= "videoColorMix";
static const std::string sVideoTexScaleOne	= "videoTexScaleOne";
static const std::string sVidTimeU			= "videoTimeU";
static const std::string sVidTimeV			= "videoTimeV";
static const std::string svideoContrast		= "videoContrast";
static const std::string svideoMaskValue	= "videoMaskValue";
static const std::string smaskColor			= "maskColor";

namespace nap
{
	template<typename Uniform, typename Type>
	static void setSharedValue(MaterialInstance& materialOne, MaterialInstance& materialTwo, const std::string& name, Type value)
	{
		// Set texture scale one
		Uniform& uni_one = materialOne.getOrCreateUniform<Uniform>(name);
		uni_one.setValue(value);

		Uniform& uni_two = materialTwo.getOrCreateUniform<Uniform>(name);
		uni_two.setValue(value);
	}


	static void setSharedTexture(MaterialInstance& materialOne, MaterialInstance& materialTwo, Texture2D& texture, const std::string& name)
	{
		// Set texture scale one
		UniformTexture2D& uni_one = materialOne.getOrCreateUniform<UniformTexture2D>(name);
		uni_one.setTexture(texture);

		// Set texture scale one
		UniformTexture2D& uni_two = materialTwo.getOrCreateUniform<UniformTexture2D>(name);
		uni_two.setTexture(texture);
	}


	void UpdateMaterialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool UpdateMaterialComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}

	void UpdateMaterialComponentInstance::update(double deltaTime)
	{
		MaterialInstance& sm = mScanMeshComponent->getMaterialInstance();
		MaterialInstance& nm = mNormalsMeshComponent->getMaterialInstance();

		//////////////////////////////////////////////////////////////////////////
		// Shared Value
		//////////////////////////////////////////////////////////////////////////

		mWindTime += (deltaTime * mUpdateMaterialResource->mWindSpeed->mValue);

		// Set first texture (tileable)
		setSharedTexture(sm, nm, mTileableImageSelectComponent->getImage(), sColorTexOneName);

		// Set second texture (single)
		setSharedTexture(sm, nm, mSingleImageSelectComponent->getImage(), sColorTexTwoName);

		// Set texture scale one
		setSharedValue<UniformFloat, float>(sm, nm, sColorTexScaleOne, mUpdateMaterialResource->mColorTexScaleOne->mValue);

		// Set texture scale two
		setSharedValue<UniformFloat, float>(sm, nm, sColorTexScaleTwo, mUpdateMaterialResource->mColorTexScaleTwo->mValue);

		// Set video texture scale
		setSharedValue<UniformFloat, float>(sm, nm, sVideoTexScaleOne, mUpdateMaterialResource->mVideoTexScaleOne->mValue);

		// Set color mix
		setSharedValue<UniformFloat, float>(sm, nm, sColorTexMix, mUpdateMaterialResource->mColorTexMix->mValue);

		// Set diffuse color
		setSharedValue<UniformVec3, glm::vec3>(sm, nm, sDiffuseColor, mUpdateMaterialResource->mDiffuseColor->mValue.toVec3());

		// Set diffuse color mix
		setSharedValue<UniformFloat, float>(sm, nm, sDiffuseColorMix, mUpdateMaterialResource->mDiffuseColorMix->mValue);

		// Set video mix
		setSharedValue<UniformFloat, float>(sm, nm, sVideoTexMix, mUpdateMaterialResource->mVideoTexMix->mValue);

		// Set video mask value
		setSharedValue<UniformFloat, float>(sm, nm, svideoMaskValue, mUpdateMaterialResource->mVideoMaskValue->mValue);

		// Set video contrast value
		setSharedValue<UniformFloat, float>(sm, nm, svideoContrast, mUpdateMaterialResource->mVideoContrastValue->mValue);

		// Set premultiply value
		setSharedValue<UniformFloat, float>(sm, nm, sPremultValue, mUpdateMaterialResource->mPremultValue->mValue);

		// Set light position
		setSharedValue<UniformVec3, glm::vec3>(sm, nm, sLightPos, mUpdateMaterialResource->mLightPos->mValue);

		// Set light intensity
		setSharedValue<UniformFloat, float>(sm, nm, sLightIntensity, mUpdateMaterialResource->mLightIntensity->mValue);

		// Set ambient intensity
		setSharedValue<UniformFloat, float>(sm, nm, sAmbientIntensity, mUpdateMaterialResource->mAmbientIntensity->mValue);

		// Set camera position
		setSharedValue<UniformVec3, glm::vec3>(sm, nm, sCameraPostion, math::extractPosition(mCameraTransform->getGlobalTransform()));

		// Diffuse intensity
		setSharedValue<UniformFloat, float>(sm, nm, sDiffuseIntensity, mUpdateMaterialResource->mDiffuseIntensity->mValue);

		// Mask color
		setSharedValue<UniformVec3, glm::vec3>(sm, nm, smaskColor, mUpdateMaterialResource->mMaskColor->mValue.toVec3());

		// Set fog values
		setSharedValue<UniformFloat, float>(sm, nm, sFogMin, mUpdateMaterialResource->mFogMin->mValue);
		setSharedValue<UniformFloat, float>(sm, nm, sFogMax, mUpdateMaterialResource->mFogMax->mValue);
		setSharedValue<UniformFloat, float>(sm, nm, sFogInfluence, mUpdateMaterialResource->mFogInfluence->mValue);
		setSharedValue<UniformFloat, float>(sm, nm, sFogPower, mUpdateMaterialResource->mFogPower->mValue);
		setSharedValue<UniformVec3,  glm::vec3>(sm, nm, sFogColor, mFogColor.toVec3());

		// Set texture u and v time
		mTexTimeU += (deltaTime * mUpdateMaterialResource->mTextureSpeed->mValue.x);
		mTexTimeV += (deltaTime * mUpdateMaterialResource->mTextureSpeed->mValue.y);
		setSharedValue<UniformFloat, float>(sm, nm, sTexTimeU, mTexTimeU);
		setSharedValue<UniformFloat, float>(sm, nm, sTexTimeV, mTexTimeV);

		// Set video u and v time
		mVidTimeU += (deltaTime * mUpdateMaterialResource->mVideoTextureSpeed->mValue.x);
		mVidTimeV += (deltaTime * mUpdateMaterialResource->mVideoTextureSpeed->mValue.y);
		setSharedValue<UniformFloat, float>(sm, nm, sVidTimeU, mVidTimeU);
		setSharedValue<UniformFloat, float>(sm, nm, sVidTimeV, mVidTimeV);

		//////////////////////////////////////////////////////////////////////////
		// Not Shared Values
		//////////////////////////////////////////////////////////////////////////

		// Specular Intensity
		sm.getOrCreateUniform<UniformFloat>(sSpecularIntensity).setValue(mUpdateMaterialResource->mScanSpecIntens->mValue);
		nm.getOrCreateUniform<UniformFloat>(sSpecularIntensity).setValue(mUpdateMaterialResource->mNormalSpecIntens->mValue);

		// Specular Color
		sm.getOrCreateUniform<UniformVec3>(sSpecularColor).setValue(mUpdateMaterialResource->mScanSpecColor->mValue.toVec3());
		nm.getOrCreateUniform<UniformVec3>(sSpecularColor).setValue(mUpdateMaterialResource->mNormalSpecColor->mValue.toVec3());
		
		// Specular Color Blend (Hair)
		nm.getOrCreateUniform<UniformFloat>(sSpecularColorBlend).setValue(mUpdateMaterialResource->mNormalSpecColorBlend->mValue);
		sm.getOrCreateUniform<UniformFloat>(sSpecularColorBlend).setValue(mUpdateMaterialResource->mScanSpecColorBlend->mValue);

		// Shininess Intensity
		sm.getOrCreateUniform<UniformFloat>(sShininess).setValue(mUpdateMaterialResource->mScanSpecShine->mValue);
		nm.getOrCreateUniform<UniformFloat>(sShininess).setValue(mUpdateMaterialResource->mNormalSpecShine->mValue);

		// Rotation value for normal
		sm.getOrCreateUniform<UniformFloat>(sRotValue).setValue(mUpdateMaterialResource->mScanRotValue->mValue);
		nm.getOrCreateUniform<UniformFloat>(sRotValue).setValue(mUpdateMaterialResource->mNormalRotValue->mValue);

		// Rotation axis for normal
		sm.getOrCreateUniform<UniformVec3>(sRotAngle).setValue(mScanRotAngle);
		nm.getOrCreateUniform<UniformVec3>(sRotAngle).setValue(mNormalRotAngle);

		//////////////////////////////////////////////////////////////////////////
		// Normal shader only values
		//////////////////////////////////////////////////////////////////////////

		nm.getOrCreateUniform<UniformFloat>(sWindScale).setValue(mUpdateMaterialResource->mWindScale->mValue);
		nm.getOrCreateUniform<UniformFloat>(sWindFreq).setValue(mUpdateMaterialResource->mWindFreq->mValue);
		nm.getOrCreateUniform<UniformFloat>(sWindRandom).setValue(mUpdateMaterialResource->mWindRandom->mValue);
		nm.getOrCreateUniform<UniformFloat>(sNormalRandom).setValue(mUpdateMaterialResource->mNormalRandom->mValue);
		nm.getOrCreateUniform<UniformFloat>(sNormalScale).setValue(mUpdateMaterialResource->mNormalScale->mValue);
		nm.getOrCreateUniform<UniformFloat>(sTime).setValue(mWindTime);
		nm.getOrCreateUniform<UniformFloat>(sDiffSpecInfluence).setValue(mUpdateMaterialResource->mDiffuseSpecInfl->mValue);
	}
}