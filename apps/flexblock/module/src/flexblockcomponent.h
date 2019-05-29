#pragma once

#include "controlpointsmesh.h"
#include "framemesh.h"
#include "flexblockmesh.h"
#include "flexblockdata.h"
#include "flex.h"
#include "visualizenormalsmesh.h"
#include "flexblockserialcomponent.h"

#include <component.h>
#include <boxmesh.h>
#include <renderablemeshcomponent.h>
#include <componentptr.h>

namespace nap
{
	class FlexBlockComponentInstance;

	/**
	 *	FlexBlockComponent
	 */
	class NAPAPI FlexBlockComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockComponent, FlexBlockComponentInstance)
	public:
		//
		ResourcePtr<FrameMesh> mFrameMesh;
		ResourcePtr<FlexBlockMesh> mFlexBlockMesh;
		ResourcePtr<VisualizeNormalsMesh> mNormalsMesh;
		
		//
		ComponentPtr<FlexBlockSerialComponent> mFlexBlockSerialComponent;

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * FlexBlockComponentInstance	
	 */
	class NAPAPI FlexBlockComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FlexBlockComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
		
		virtual ~FlexBlockComponentInstance();

		/**
		 * Initialize FlexBlockComponentInstance based on the FlexBlockComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the FlexBlockComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update FlexBlockComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		* Update motor speed [0..8]
		* @param index index of motor [0..8]
		* @param value motor input 0-1
		*/
		void SetMotorInput(int index, float value);
	protected:
		FrameMesh* mFrameMesh = nullptr;
		FlexBlockMesh* mFlexBlockMesh = nullptr;
		VisualizeNormalsMesh* mNormalsMesh = nullptr;

		//
		ComponentInstancePtr<FlexBlockSerialComponent> mFlexBlockSerialComponentInstance
			= initComponentInstancePtr(this, &FlexBlockComponent::mFlexBlockSerialComponent);

		// Initialize flexblock unique ptr to null
		std::unique_ptr<Flex> mFlexLogic = nullptr;

		//
		double mUpdateSerialTime = 0.0;

		//
		std::vector<glm::vec3> toNapPoints(const std::vector<glm::vec3>& points);
	};
}
