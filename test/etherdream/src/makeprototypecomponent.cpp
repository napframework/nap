#include "laseroutputcomponent.h"
#include "makeprototypecomponent.h"
#include "osclaserinputhandler.h"

// External Includes
#include <nap/entity.h>
#include <nap/core.h>
#include <nap/resourcemanager.h>

// nap::makeprototypecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::MakePrototypeComponent)
	RTTI_PROPERTY("SplineEntity", &nap::MakePrototypeComponent::mSplineEntity, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputEntity", &nap::MakePrototypeComponent::mLaserOutputEntity, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OSCAddresses", &nap::MakePrototypeComponent::mOSCAddresses, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::makeprototypecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MakePrototypeComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void MakePrototypeComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool MakePrototypeComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Get resource and manager
		MakePrototypeComponent* resource = getComponent<MakePrototypeComponent>();
		ResourceManagerService& resource_manager = *getEntityInstance()->getCore()->getService<nap::ResourceManagerService>();

		// Create the entities
		mSplineEntity = resource_manager.createEntity(*(resource->mSplineEntity), entityCreationParams, errorState).get();
		if (mSplineEntity == nullptr)
			return false;
		getEntityInstance()->addChild(*mSplineEntity);

		mLaserOutputEntity = resource_manager.createEntity(*(resource->mLaserOutputEntity), entityCreationParams, errorState).get();
		if (mLaserOutputEntity == nullptr)
			return false;
		getEntityInstance()->addChild(*mLaserOutputEntity);

		// Store some of the components we want to set up
		mLineBlender = mSplineEntity->findComponent<LineBlendComponentInstance>();
		if (!errorState.check(mLineBlender != nullptr, "unable to find line blend component"))
			return false;

		// Store components to render
		std::vector<nap::RenderableMeshComponentInstance*> render_components;
		mSplineEntity->getComponentsOfType<RenderableMeshComponentInstance>(render_components);
		if (!errorState.check(render_components.size() >= 2, "not enough renderable components"))
			return false;

		assert(utility::gStartsWith(render_components[0]->getComponent<RenderableMeshComponent>()->mID, "RenderLaserLineComponent"));
		assert(utility::gStartsWith(render_components[1]->getComponent<RenderableMeshComponent>()->mID, "RenderLaserNormalsComponent"));
		mLineRenderer = render_components[0];
		mNormalsRenderer = render_components[1];

		// Store component that updates normals
		mUpdateNormalsComponent = mSplineEntity->findComponent<UpdateNormalsComponentInstance>();
		if (!errorState.check(mUpdateNormalsComponent != nullptr, "unable to find update normals component"))
			return false;

		// Store component that traces the line
		mTraceComponent = mSplineEntity->findComponent<LineTraceComponentInstance>();
		if (!errorState.check(mTraceComponent != nullptr, "unable to find trace component"))
			return false;

		// Store component that outputs a line to the laser
		mOutputComponent = mLaserOutputEntity->findComponent<LaserOutputComponentInstance>();
		if (!errorState.check(mOutputComponent != nullptr, "unable to find trace component"))
			return false;

		// Store component that receives incoming osc events
		mOSCInputComponent = mSplineEntity->findComponent<OSCInputComponentInstance>();
		if (!errorState.check(mOSCInputComponent != nullptr, "unable to find osc input component"))
			return false;

		// Store osc address pattern
		mOSCAddressPattern = getComponent<MakePrototypeComponent>()->mOSCAddresses;

		// Set-up relationships

		// The laser output component needs to know where the line is relative to it's canvas, therefore we provide it with the line entity
		nap::LaserOutputComponentInstance* laser_output_comp = mLaserOutputEntity->findComponent<nap::LaserOutputComponentInstance>();
		assert(laser_output_comp != nullptr);
		laser_output_comp->setTransform(*mSplineEntity);

		// The OSC Input handler needs a reference to the laser output component entity to figure out where it can place the line (bounds)
		nap::OSCLaserInputHandlerInstance* osc_handler = mSplineEntity->findComponent<OSCLaserInputHandlerInstance>();
		osc_handler->setLaserOutput(*mLaserOutputEntity);

		return true;
	}


	bool MakePrototypeComponentInstance::setup(LaserCompound& settings, nap::utility::ErrorState& error)
	{
		// Set poly line the blender will blend to
		mLineBlender->setPolyLine(*(settings.mLineMesh));

		// Create and set renderable mesh for laser line renderable component
		nap::RenderableMesh mesh = mLineRenderer->createRenderableMesh(*(settings.mLineMesh), error);
		mLineRenderer->setMesh(mesh);

		// Create and set renderable mesh for normals renderable component
		nap::RenderableMesh normals_mesh = mNormalsRenderer->createRenderableMesh(*(settings.mNormalsMesh), error);
		mNormalsRenderer->setMesh(normals_mesh);

		// Set normals mesh to update
		mUpdateNormalsComponent->setMesh(*(settings.mNormalsMesh));

		// Set tracer mesh
		mTraceComponent->setPolyLine(*(settings.mTraceMesh));

		// Set laser output line
		mOutputComponent->setPolyLine(*(settings.mTraceMesh));

		// Set the output dac
		mOutputComponent->setDac(*(settings.mDac));

		// Modify the osc addresses and set to input component
		mOSCInputComponent->mAddressFilter.clear();
		
		for (const auto& address : mOSCAddressPattern)
		{
			std::string new_address = utility::stringFormat("/%d/%s", settings.mLaserID, address.c_str());
			mOSCInputComponent->mAddressFilter.emplace_back(new_address);
		}

		return true;
	}

}