#pragma once

// internal includes
#include "framemesh.h"
#include "flexblockmesh.h"
#include "flexblockdata.h"
#include "flex.h"
#include "visualizenormalsmesh.h"
#include "flexblockserialcomponent.h"

// external includes
#include <component.h>
#include <boxmesh.h>
#include <renderablemeshcomponent.h>
#include <renderable2dtextcomponent.h>
#include <componentptr.h>
#include <perspcameracomponent.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	class FlexBlockComponentInstance;

	/**
	 * FlexBlockComponent controls a flexblock mesh, frame mesh and optionally writes
	 * data to serial motors. 
	 * FlexBlockComponent needs a flexblock shape definition. See FlexBlockShape and flexblockdata.h
	 */
	class NAPAPI FlexBlockComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockComponent, FlexBlockComponentInstance)
	public:
		
		/**
		 * Resource pointer to the mesh of the frame
		 */
		ResourcePtr<FrameMesh> mFrameMesh; ///< Property: 'FrameMesh' Reference to the frame mesh

		/**
		 * Resource pointer to the flexblock mesh 
		 */
		ResourcePtr<FlexBlockMesh> mFlexBlockMesh; ///< Property: 'FlexBlockMesh' Reference to the FlexBlockMesh 

		/**
		 * Resource pointer to the shape definition
		 */
		ResourcePtr<FlexBlockShape> mFlexBlockShape; ///< Property: 'FlexBlockShape' Reference to the shape definition of the block 
		
		/**
		 * Reference to the component of the FlexBlockSerialComponent
		 */
		ComponentPtr<FlexBlockSerialComponent> mFlexBlockSerialComponent; ///< Property: 'FlexBlockSerialComponent' Reference to the component of the flexblock serial component
	
		float mMillimeterToMotorsteps = 12.73239f; ///< Property: 'Millimeter to Motorsteps' value that we need to calculate how much steps we need the motor(s) to make

		int mMotorOffset = 7542; ///< Property: 'Motor Offset' value that we need to calculate the zero position of the motor

		float mSlackRange = 1.0f;

		float mSlackMinimum = -0.5f;

		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	* FlexBlockComponentInstance
	* Instantiated flexblock component
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
		void setMotorInput(int index, float value);

		/**
		 * Set slack parameter
		 * @param value slack value, typically between -0.5 and 0.5
		 */
		void setSlack(const float value);

		/**
		 * @return returns object points in local space
		 */
		const std::vector<glm::vec3>& getObjectPoints() const { return mObjectPoints; }

		/**
		 * @return returns frame points in local space
		 */
		const std::vector<glm::vec3>& getFramePoints() const { return mFramePoints; }
	protected:
		FrameMesh* mFrameMesh = nullptr;
		FlexBlockMesh* mFlexBlockMesh = nullptr;

		//
        ComponentInstancePtr<FlexBlockSerialComponent> mFlexBlockSerialComponentInstance = { this, &FlexBlockComponent::mFlexBlockSerialComponent  };
		// Initialize flexblock unique ptr to null
		std::unique_ptr<Flex> mFlexLogic = nullptr;

		//
		double mUpdateSerialTime = 0.0;
		
		std::vector<glm::vec3> mObjectPoints = std::vector<glm::vec3>(8);

		std::vector<glm::vec3> mFramePoints = std::vector<glm::vec3>(8);

		std::vector<float> mMotorInputs = std::vector<float>(8);

		float mMillimeterToMotorsteps;

		int mMotorOffset;

		float mSlackRange;

		float mSlackMinimum;
	};
}
