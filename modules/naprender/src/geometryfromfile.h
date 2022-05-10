#pragma once

// External Includes
#include <mesh.h>

namespace nap
{
	/**
	 * Imports 3D Geometry from file and converts the individual meshes into a single nap::MeshInstance.
	 * Every extracted mesh becomes a single shape, vertex attributes are shared.
	 * Many 3D geometry file formats are supported, including '.obj', '.fbx' etc.
	 * For a full list of supported file formats visit http://assimp.sourceforge.net/
	 *
	 * The resource checks, on initialization, which attributes it needs to create, based on the extracted mesh data.
	 * If the file contains multiple meshes with varying types of data, and a particular mesh doesn't have a specific set of data, 
	 * the attribute data of that mesh is initialized to zero and a warning is issued.
	 *
	 * For example: Say the file contains 2 meshes, mesh 1 has color data but mesh 2 does not. This means that the color 
	 * data of mesh 2 will be initialized to 0. If both meshes carry no color data at all no color attribute is created.
	 * 
	 * The operator attempts to extract Vertex, Normal, Color, UV and Tangent information. Up to 8 Color and 
	 * UV channels are supported. You can choose to generate normals, tangents and bi-tangents. These are only
	 * generated when not already present on the mesh and 'GenerateNormals' / 'CalculateTangents' is set to true.
	 * 
	 * Only models that contain closed faces (quads / triangles etc.) are currently supported
	 * The mesh is triangulated on load.
	 *
	 * Note that it is faster and often better to load geometry from a '.mesh' file using the nap::MeshFromFile resource.
	 * '.mesh' files are created by the 'fbxconverter' tool and are stored in a more optimal (binary) format, optimized for faster reads.
	 * Another advantage of using the 'fbxconverter' tool is that every mesh contained in that file is converted into an individual '.mesh' file, 
	 * subsequently giving you more control over material assignment.
	 */
	class NAPAPI GeometryFromFile : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		GeometryFromFile(Core& core);

		/**
		 * Loads the 3D geometry from file and creates the mesh instance.
		 * Every mesh becomes a single shape, vertex attributes are shared.
		 * @param errorState contains the error if initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh instance, created during init after a successful load.
		 */
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		/**
		 * @return the mesh instance, created during init after a successful load.
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

		std::string		mPath;									///< Property: 'Path' path to the geometry (.fbx, .obj, etc.) file on disk
		EMemoryUsage	mUsage = EMemoryUsage::Static;		///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		EPolygonMode	mPolygonMode = EPolygonMode::Fill;		///< Property: 'PolygonMode' Mesh polygon mode (fill, wires, points)
		ECullMode		mCullMode = ECullMode::Back;			///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.
		bool			mGenerateNormals = false;				///< Property: 'GenerateNormals' If normals are generated when not present.
		float			mSmoothingAngle = 60.0f;				///< Property: 'NormalSmoothingAngle' The maximum angle between two normals at the same vertex position to be considered the same. Only used when normals are generated.
		bool			mCalculateTangents = true;				///< Property: 'CalculateTangents' If tangents and bi-tangents are calculated, does nothing if no normals are present or calculated

	private:
		RenderService* mRenderService = nullptr;
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;
	};
}
