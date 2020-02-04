#include "selectpathcomponent.h"

// External Includes
#include <entity.h>

// nap::selectpathcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::SelectPathComponent)
	RTTI_PROPERTY("Paths",				&nap::SelectPathComponent::mPaths,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",				&nap::SelectPathComponent::mIndex,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Renderer",			&nap::SelectPathComponent::mPathRenderer,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Controller",			&nap::SelectPathComponent::mPathController,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathPosition",		&nap::SelectPathComponent::mPathPosition,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathAxis",			&nap::SelectPathComponent::mPathAxis,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathAngle",			&nap::SelectPathComponent::mPathAngle,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathScale",			&nap::SelectPathComponent::mPathScale,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PathUniformScale",	&nap::SelectPathComponent::mPathUniformScale,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::selectpathcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SelectPathComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool SelectPathComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over meshes
		nap::SelectPathComponent* resource = getComponent<SelectPathComponent>();

		// Create a link between the material and path for every path you can select from.
		// This RenderableMesh objects bind a mesh to material and allow it to be rendered
		// Using the render component
		for (auto& path : resource->mPaths)
		{
			RenderableMesh render_path = mRenderer->createRenderableMesh(*path, errorState);
			if (!render_path.isValid())
			{
				errorState.fail("%s: unable to create render-able mesh");
				return false;
			}
			
			mRenderPaths.emplace_back(render_path);
			mPaths.emplace_back(path.get());
		}

		// Make sure we have some videos
		if (!errorState.check(mRenderPaths.size() > 0, "%s: no paths to select", resource->mID.c_str()))
			return false;

		// Copy current transform values for path
		mPathTransForm = getEntityInstance()->findComponent<nap::TransformComponentInstance>();
		if (!errorState.check(mPathTransForm != nullptr, "%s: path has no transform", resource->mID.c_str()))
			return false;

		// Update path when index changes
		resource->mIndex->valueChanged.connect(mPathIndexChangedSlot);
		resource->mPathAxis->valueChanged.connect(mPathAxisChangedSlot);
		resource->mPathAngle->valueChanged.connect(mPathAngleChangedSlot);
		resource->mPathPosition->valueChanged.connect(mPathPositionChangedSlot);
		resource->mPathScale->valueChanged.connect(mPathScaleChangedSlot);
		resource->mPathUniformScale->valueChanged.connect(mPathUniformScaleChangedSlot);

		// Store angle related values because they are both required to calculate quaternion
		mPathAxisParam  = resource->mPathAxis.get();
		mPathAngleParam = resource->mPathAngle.get();

		// Select path based on current index
		selectPath(resource->mIndex->mValue);

		// Update transforms based on new values
		updatePathAxis(resource->mPathAxis->mValue);
		updatePosition(resource->mPathPosition->mValue);
		updateScale(resource->mPathScale->mValue);
		updateUniformScale(resource->mPathUniformScale->mValue);

		return true;
	}


	void SelectPathComponentInstance::update(double deltaTime)
	{

	}


	void SelectPathComponentInstance::selectPath(int index)
	{
		mCurrentPath = &mRenderPaths[index];
		mRenderer->setMesh(*mCurrentPath);
		mController->setPath(*(mPaths[index]));
	}


	void SelectPathComponentInstance::updatePathAxis(glm::vec3 eulerAngles)
	{
		mPathTransForm->setRotate(glm::angleAxis(math::radians(mPathAngleParam->mValue), glm::normalize(eulerAngles)));
	}


	void SelectPathComponentInstance::updatePathAngle(float angle)
	{
		mPathTransForm->setRotate(glm::angleAxis(math::radians(angle), glm::normalize(mPathAxisParam->mValue)));
	}


	void SelectPathComponentInstance::updatePosition(glm::vec3 position)
	{
		mPathTransForm->setTranslate(position);
	}


	void SelectPathComponentInstance::updateScale(glm::vec3 scale)
	{
		mPathTransForm->setScale(scale);
	}


	void SelectPathComponentInstance::updateUniformScale(float value)
	{
		mPathTransForm->setUniformScale(value);
	}


	bool SelectPathComponent::init(utility::ErrorState& error)
	{
		mIndex->setRange(0, mPaths.size() - 1);
		return true;
	}
}