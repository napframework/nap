#pragma once

#include <rendercomponent.h>
#include <renderablemesh.h>
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <transformcomponent.h>
#include <color.h>
#include <spheremesh.h>
#include <cvclassifycomponent.h>
#include <renderablemeshcomponent.h>

namespace nap
{
	// Forward declare instance part
	class RenderableClassifyComponentInstance;

	/**
	 * RenderableClassifyComponent
	 */
	class NAPAPI RenderableClassifyComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderableClassifyComponent, RenderableClassifyComponentInstance)
	public:

		/**
		 * This component depends on a transform component
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Component Properties
		float mScale = 1.0f;										///< Property: 'Scale' scale of the copied meshes
		MaterialInstanceResource mMaterialInstanceResource;			///< Property: 'MaterialInstance' the material used to shade the text
		std::string mColorUniform = "color";						///< Property: 'ColorUniform' name of the color uniform binding (vec3) in the shader
		ResourcePtr<SphereMesh> mSphereMesh;						///< Property: 'Sphere' list of meshes to copy onto target
		ComponentPtr<CVClassifyComponent> mClassifyComponent;		///< Property: 'ClassifyComponent' components that contains detected objects
		ComponentPtr<RenderableMeshComponent> mPlaneComponent;		///< Property: 'PlaneComponent'component that renders the plane 
	};


	/**
	 * Custom Renderable Mesh Component
	 * This component renders a randomly selected mesh at the position of every vertex in the target mesh.
	 * Look at the onDraw() call to see how this component iterates over every point and renders a mesh at the vertex position.
	 */
	class NAPAPI RenderableClassifyComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderableClassifyComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableComponentInstance(entity, resource)									{ }

		/**
		 * Initialize RenderableCopyMeshComponentInstance based on the RenderableCopyMeshComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the renderablecopymeshcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @param deltaTime time in between calls in seconds	
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return material used for rendering the copied meshes	
		 */
		MaterialInstance& getMaterial();

		/**
		 * @return list of recent blob placements in world space
		 */
		const std::vector<glm::mat4>& getLocations() const		{ return mLocations; }

		/**
		 * @return radius of blob in world space
		 */
		const std::vector<float>& getSizes() const				{ return mSizes; }

		/**
		 * Link to the classification component, contains list of detected blobs
		 */
		ComponentInstancePtr<CVClassifyComponent> mClassifyComponent = { this, &RenderableClassifyComponent::mClassifyComponent };

		/**
		 * Link to the component that renders the plane
		 */
		ComponentInstancePtr<RenderableMeshComponent> mPlaneComponent = { this, &RenderableClassifyComponent::mPlaneComponent };

		bool	mOrient = true;										///< If copied meshes should be oriented towards the camera
		float	mScale = 1.0f;										///< Scale of the meshes that are copied
		int		mSeed = 0;											///< Random seed
		float	mRotationSpeed = 1.0f;								///< Influences rotation speed

	protected:
		/**
		* Draws a randomly selected mesh at the position of every vertex in the target mesh.
		* @param viewMatrix the camera world space location
		* @param projectionMatrix the camera projection matrix
		*/
		virtual void onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		/**
		 * Helper function that is used to extract a specific type of uniform out of a material.
		 * First it checks if the material actually has (supports) a uniform with the given name.
		 * If that is the case a new uniform with that name is created that can be used by this component only.
		 */
		template<typename T>
		T* extractUniform(const std::string& name, utility::ErrorState& error);

		TransformComponentInstance* mTransform = nullptr;				///< Transform used to position instanced meshes
		RenderableMesh mSphereMesh;										///< Valid mesh / material combinations
		MaterialInstance mMaterialInstance;								///< The MaterialInstance as created from the resource. 
		VertexAttribute<glm::vec3>* mPositionAttr = nullptr;			///< Handle to the vertices we want to stamp
		nap::UniformVec3* mColorUniform = nullptr;						///< Color uniform slot
		nap::UniformMat4* mProjectionUniform = nullptr;					///< Projection matrix uniform slot
		nap::UniformMat4* mViewUniform = nullptr;						///< View matrix uniform slot
		nap::UniformMat4* mModelUniform = nullptr;						///< Model matrix uniform slot
		std::vector<RGBColorFloat> mColors;								///< All selectable colors
		std::vector<glm::mat4> mLocations;								///< All world space blob locations
		std::vector<float> mSizes;										///< All world space blob dimensions
	};


	template<typename T>
	T* nap::RenderableClassifyComponentInstance::extractUniform(const std::string& name, utility::ErrorState& error)
	{
		// Find the uniform in the material first
		nap::Material* material = mMaterialInstance.getMaterial();
		T* found_uni = material->findUniform<T>(name);
		if (!error.check(found_uni != nullptr, "%s: unable to find uniform: %s", material->mID.c_str(), name.c_str()))
			return nullptr;

		// If found, create a uniform associated with the instance of that material
		return &(mMaterialInstance.getOrCreateUniform<T>(name));
	}

}
