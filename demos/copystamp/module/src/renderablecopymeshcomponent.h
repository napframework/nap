#pragma once

#include <rendercomponent.h>
#include <renderablemesh.h>

namespace nap
{
	class RenderableCopyMeshComponentInstance;

	/**
	 *	renderablecopymeshcomponent
	 */
	class NAPAPI RenderableCopyMeshComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderableCopyMeshComponent, RenderableCopyMeshComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Properties
		MaterialInstanceResource mMaterialInstanceResource;		///< Property: 'MaterialInstance' the material used to shade the text
		std::string mColorUniform = "color";					///< Property: 'ColorUniform' name of the color uniform binding (vec3) in the shader
	};


	/**
	 * renderablecopymeshcomponentInstance	
	 */
	class NAPAPI RenderableCopyMeshComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RenderableCopyMeshComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableComponentInstance(entity, resource)									{ }

		/**
		 * Initialize renderablecopymeshcomponentInstance based on the renderablecopymeshcomponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the renderablecopymeshcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update renderablecopymeshcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	protected:
		/**
		* Draws the text into to active render target using the provided matrices.
		* Call this in derived classes based on extracted matrices.
		* @param viewMatrix the camera world space location
		* @param projectionMatrix the camera projection matrix
		* @param modelMatrix the location of the text in the world
		*/
		virtual void onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		TransformComponentInstance* mTransform = nullptr;				///< Transform used to position instanced meshes
		std::vector<RenderableMesh> mMeshes;							///< All the valid mesh / material combinations
		MaterialInstance mMaterialInstance;								///< The MaterialInstance as created from the resource. 
		VertexAttribute<glm::vec3>* mPositionAttr = nullptr;			///< Handle to the vertices we want to stamp
	};
}
