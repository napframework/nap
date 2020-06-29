#pragma once

#include <rendercomponent.h>
#include <renderablemesh.h>
#include <rtti/objectptr.h>
#include <componentptr.h>
#include <transformcomponent.h>
#include <color.h>
#include <uniforminstance.h>
#include <materialinstance.h>
#include <renderservice.h>

namespace nap
{
	// Forward declare instance part
	class RenderableCopyMeshComponentInstance;

	/**
	 * RenderableCopyMeshComponent
	 * The instance created from this resource draws a randomly selected mesh at every vertex of the target mesh.
	 * Note that isn't necessarily the fastest way to do this, just a minimal example. Optimize it as you wish. 
	 */
	class NAPAPI RenderableCopyMeshComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderableCopyMeshComponent, RenderableCopyMeshComponentInstance)
	public:

		/**
		 * This component depends on a transform component
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Component Properties
		bool mOrient = true;										///< Property: 'Orientation' if the models should be rotated towards the normal
		float mScale = 1.0f;										///< Property: 'Scale' scale of the copied meshes
		float mRotationSpeed = 1.0f;								///< Property: 'Speed of rotation'
		float mRandomScale = 0.0f;									///< Property: 'RandomScale' amount of random scale to apply (0-1)
		float mRandomRotation = 0.0f;								///< Property: 'RandomRotation' amount of random rotation to apply (0-1)
		MaterialInstanceResource mMaterialInstanceResource;			///< Property: 'MaterialInstance' the material used to shade the text
		std::string mColorUniform = "color";						///< Property: 'ColorUniform' name of the color uniform binding (vec3) in the shader
		std::vector<rtti::ObjectPtr<IMesh>> mCopyMeshes;			///< Property: 'CopyMeshes' list of meshes to copy onto target
		rtti::ObjectPtr<IMesh> mTargetMesh;							///< Property: 'Target' mesh to copy meshes onto
		ComponentPtr<TransformComponent> mCamera;					///< Property: 'Camera' link to camera, used for orientation
	};


	/**
	 * Custom Renderable Mesh Component
	 * This component renders a randomly selected mesh at the position of every vertex in the target mesh.
	 * Look at the onDraw() call to see how this component iterates over every point and renders a mesh at the vertex position.
	 */
	class NAPAPI RenderableCopyMeshComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderableCopyMeshComponentInstance(EntityInstance& entity, Component& resource);

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
		 * Link to camera, can be used to make the copied meshes look at the camera
		 */
		ComponentInstancePtr<TransformComponent> mCamera = { this, &RenderableCopyMeshComponent::mCamera };

		bool	mOrient = true;										///< If copied meshes should be oriented towards the camera
		float	mScale = 1.0f;										///< Scale of the meshes that are copied
		float	mRandomScale = 0.0f;								///< Amount of random scale to apply	
		int		mSeed = 0;											///< Random seed
		float	mRotationSpeed = 1.0f;								///< Influences rotation speed

	protected:
		/**
		* Draws a randomly selected mesh at the position of every vertex in the target mesh.
		* @param viewMatrix the camera world space location
		* @param projectionMatrix the camera projection matrix
		*/
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		TransformComponentInstance* mTransform = nullptr;								///< Transform used to position instanced meshes
		std::vector<RenderableMesh> mMeshes;											///< All the valid mesh / material combinations
		MaterialInstance mMaterialInstance;												///< The MaterialInstance as created from the resource. 
		VertexAttribute<glm::vec3>* mPositionAttr = nullptr;							///< Handle to the vertices we want to stamp
		IMesh* mTargetMesh;																///< Mesh to copy onto
		std::vector<RenderableMesh> mCopyMeshes;										///< All renderable variants of the mesh to copy
		std::unordered_map<RenderableMesh, std::vector<glm::vec3>> mPositions;			///< All renderable meshes and their positions
		std::unordered_map<RenderableMesh, RenderService::Pipeline> mPipelines;		///< All pipelines associated with a specific renable mesh combination
		Vec3VertexAttribute* mTargetVertices = nullptr;									///< Point positions to copy meshes onto
		Vec3VertexAttribute* mTargetNormals = nullptr;									///< Point orientation of target mesh
		nap::UniformVec3Instance* mColorUniform = nullptr;								///< Color uniform slot
		nap::UniformMat4Instance* mProjectionUniform = nullptr;							///< Projection matrix uniform slot
		nap::UniformMat4Instance* mViewUniform = nullptr;								///< View matrix uniform slot
		nap::UniformMat4Instance* mModelUniform = nullptr;								///< Model matrix uniform slot
		std::vector<RGBColorFloat> mColors;												///< All selectable colors 
		double mTime = 0.0;																///< Total running time
		float mRandomRotation = 0.0f;													///< Amount of randomization of rotation speed
		RenderService* mRenderService = nullptr;										///< Renderer
	};
}
