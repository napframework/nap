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

		mWindTime += (deltaTime * mWindSpeed);

		// Set first texture (tileable)
		setSharedTexture(sm, nm, mTileableImageSelectComponent->getImage(), sColorTexOneName);

		// Set second texture (single)
		setSharedTexture(sm, nm, mSingleImageSelectComponent->getImage(), sColorTexTwoName);

		// Set texture scale one
		setSharedValue<UniformFloat, float>(sm, nm, sColorTexScaleOne, mColorTexScaleOne);

		// Set texture scale two
		setSharedValue<UniformFloat, float>(sm, nm, sColorTexScaleTwo, mColorTexScaleTwo);

		// Set video texture scale
		setSharedValue<UniformFloat, float>(sm, nm, sVideoTexScaleOne, mVideoTexScaleOne);

		// Set color mix
		setSharedValue<UniformFloat, float>(sm, nm, sColorTexMix, mColorTexMix);

		// Set diffuse color
		setSharedValue<UniformVec3, glm::vec3>(sm, nm, sDiffuseColor, mDiffuseColor.toVec3());

		// Set diffuse color mix
		setSharedValue<UniformFloat, float>(sm, nm, sDiffuseColorMix, mDiffuseColorMix);

		// Set video mix
		setSharedValue<UniformFloat, float>(sm, nm, sVideoTexMix, mVideoTexMix);

		// Set premultiply value
		setSharedValue<UniformFloat, float>(sm, nm, sPremultValue, mPremultValue);

		// Set light position
		setSharedValue<UniformVec3, glm::vec3>(sm, nm, sLightPos, mLightPos);

		// Set light intensity
		setSharedValue<UniformFloat, float>(sm, nm, sLightIntensity, mLightIntensity);

		// Set ambient intensity
		setSharedValue<UniformFloat, float>(sm, nm, sAmbientIntensity, mAmbientIntensity);

		// Set camera position
		setSharedValue<UniformVec3, glm::vec3>(sm, nm, sCameraPostion, math::extractPosition(mCameraTransform->getGlobalTransform()));

		// Diffuse intensity
		setSharedValue<UniformFloat, float>(sm, nm, sDiffuseIntensity, mDiffuseIntensity);

		// Set fog values
		setSharedValue<UniformFloat, float>(sm, nm, sFogMin, mFogMin);
		setSharedValue<UniformFloat, float>(sm, nm, sFogMax, mFogMax);
		setSharedValue<UniformFloat, float>(sm, nm, sFogInfluence, mFogInfluence);
		setSharedValue<UniformFloat, float>(sm, nm, sFogPower, mFogPower);
		setSharedValue<UniformVec3,  glm::vec3>(sm, nm, sFogColor, mFogColor.toVec3());

		// Set texture u and v time
		mTexTimeU += (deltaTime * mTextureSpeed.x);
		mTexTimeV += (deltaTime * mTextureSpeed.y);
		setSharedValue<UniformFloat, float>(sm, nm, sTexTimeU, mTexTimeU);
		setSharedValue<UniformFloat, float>(sm, nm, sTexTimeV, mTexTimeV);

		// Set video u and v time
		mVidTimeU += (deltaTime * mVideoTexureSpeed.x);
		mVidTimeV += (deltaTime * mVideoTexureSpeed.y);
		setSharedValue<UniformFloat, float>(sm, nm, sVidTimeU, mVidTimeU);
		setSharedValue<UniformFloat, float>(sm, nm, sVidTimeV, mVidTimeV);

		//////////////////////////////////////////////////////////////////////////
		// Not Shared Values
		//////////////////////////////////////////////////////////////////////////

		// Specular Intensity
		sm.getOrCreateUniform<UniformFloat>(sSpecularIntensity).setValue(mScanSpecIntens);
		nm.getOrCreateUniform<UniformFloat>(sSpecularIntensity).setValue(mNormalSpecIntens);

		// Specular Color
		sm.getOrCreateUniform<UniformVec3>(sSpecularColor).setValue(mScanSpecColor.toVec3());
		nm.getOrCreateUniform<UniformVec3>(sSpecularColor).setValue(mNormalSpecColor.toVec3());
			
		// Shininess Intensity
		sm.getOrCreateUniform<UniformFloat>(sShininess).setValue(mScanSpecShine);
		nm.getOrCreateUniform<UniformFloat>(sShininess).setValue(mNormalSpecShine);

		// Rotation value for normal
		sm.getOrCreateUniform<UniformFloat>(sRotValue).setValue(mScanRotValue);
		nm.getOrCreateUniform<UniformFloat>(sRotValue).setValue(mNormalRotValue);

		// Rotation axis for normal
		sm.getOrCreateUniform<UniformVec3>(sRotAngle).setValue(mScanRotAngle);
		nm.getOrCreateUniform<UniformVec3>(sRotAngle).setValue(mNormalRotAngle);

		//////////////////////////////////////////////////////////////////////////
		// Normal shader only values
		//////////////////////////////////////////////////////////////////////////

		nm.getOrCreateUniform<UniformFloat>(sWindScale).setValue(mWindScale);
		nm.getOrCreateUniform<UniformFloat>(sWindFreq).setValue(mWindFreq);
		nm.getOrCreateUniform<UniformFloat>(sWindRandom).setValue(mWindRandom);
		nm.getOrCreateUniform<UniformFloat>(sNormalRandom).setValue(mNormalRandom);
		nm.getOrCreateUniform<UniformFloat>(sNormalScale).setValue(mNormalScale);
		nm.getOrCreateUniform<UniformFloat>(sTime).setValue(mWindTime);
		nm.getOrCreateUniform<UniformFloat>(sDiffSpecInfluence).setValue(mDiffuseSpecInfl);
	}
}