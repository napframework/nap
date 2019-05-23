#pragma once

#include "controlpointsmesh.h"
#include "framemesh.h"

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
		ComponentPtr<RenderableMeshComponent> mBoxRenderer;

		//
		ResourcePtr<ControlPointsMesh> mControlPointsMesh;
		ResourcePtr<FrameMesh> mFrameMesh;

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
		FlexBlockComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

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
		
		void SetControlPoint(int index, glm::vec3 position);
		glm::vec3 GetControlPoint(int index);
	protected:
		void updateBox();

		ComponentInstancePtr<RenderableMeshComponent> mBoxRendererInstance 
			= initComponentInstancePtr(this, &FlexBlockComponent::mBoxRenderer);

		// Position Attribute data
		VertexAttribute<glm::vec3>* mVertexAttribute = nullptr;

		// Normal Attribute data
		VertexAttribute<glm::vec3>* mNormalAttribute = nullptr;

		//
		std::vector<glm::vec3> mControlPoints;

		ResourcePtr<ControlPointsMesh> mControlPointsMesh;
		ResourcePtr<FrameMesh> mFrameMesh;
	};
}
