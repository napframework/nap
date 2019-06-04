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
#include <renderable2dtextcomponent.h>
#include <componentptr.h>
#include <perspcameracomponent.h>

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
		ResourcePtr<FlexBlockShape> mFlexBlockShape;
		
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

		/*
		*/
		const std::vector<glm::vec3>& getObjectPoints() { return mObjectPoints; }
		const std::vector<glm::vec3>& getFramePoints() { return mFramePoints; }

		void toNapPoints(const std::vector<glm::vec3>& points, std::vector<glm::vec3>& outPoints);
		const int remapMotorInput(const int index);
	protected:
		FrameMesh* mFrameMesh = nullptr;
		FlexBlockMesh* mFlexBlockMesh = nullptr;

		//
		ComponentInstancePtr<FlexBlockSerialComponent> mFlexBlockSerialComponentInstance
			= initComponentInstancePtr(this, &FlexBlockComponent::mFlexBlockSerialComponent);

		// Initialize flexblock unique ptr to null
		std::unique_ptr<Flex> mFlexLogic = nullptr;

		//
		double mUpdateSerialTime = 0.0;
		
		std::vector<glm::vec3> mObjectPoints = std::vector<glm::vec3>(8);

		std::vector<glm::vec3> mFramePoints = std::vector<glm::vec3>(8);
	};
}
