// Local Includes
#include "geometryfromfile.h"

// External Includes
#include <renderservice.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <renderglobals.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GeometryFromFile)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY_FILELINK("Path",			&nap::GeometryFromFile::mPath,				nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::Mesh)
	RTTI_PROPERTY("GenerateNormals",		&nap::GeometryFromFile::mGenerateNormals,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CalculateTangents",		&nap::GeometryFromFile::mCalculateTangents, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NormalSmoothingAngle",	&nap::GeometryFromFile::mSmoothingAngle,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",					&nap::GeometryFromFile::mUsage,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",				&nap::GeometryFromFile::mCullMode,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",			&nap::GeometryFromFile::mPolygonMode,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	static void copyVec3Data(uint vertexCount, aiVector3D* sourceData, VertexAttribute<glm::vec3>* targetAttribute, const std::string& attributeName, const std::string& meshName)
	{
		// No target to copy to
		if (targetAttribute == nullptr)
			return;

		// No data top copy, append empty set
		if (sourceData == nullptr)
		{
			nap::Logger::warn("%s has no %s, initializing to 0", meshName.c_str(), attributeName.c_str());
			std::vector<glm::vec3>& attr_data = targetAttribute->getData();
			std::vector<glm::vec3> empty(vertexCount, {0.0f, 0.0f, 0.0f});
			attr_data.insert(attr_data.end(), empty.begin(), empty.end());
			return;
		}

		// Otherwise convert and add
		for (uint i = 0; i < vertexCount; i++)
		{
			aiVector3D* cid = &(sourceData[i]);
			targetAttribute->addData(glm::vec3(static_cast<float>(cid->x), static_cast<float>(cid->y), static_cast<float>(cid->z)));
		}
	}


	static void copyVec4Data(uint vertexCount, aiColor4D* sourceData, VertexAttribute<glm::vec4>* targetAttribute, const std::string& attributeName, const std::string& meshName)
	{
		// No target to copy to
		if (targetAttribute == nullptr)
			return;

		// No data top copy, append empty set
		if (sourceData == nullptr)
		{
			nap::Logger::warn("Mesh %s has no %s, initializing to 0", meshName.c_str(), attributeName.c_str());
			std::vector<glm::vec4>& attr_data = targetAttribute->getData();
			std::vector<glm::vec4> empty(vertexCount, { 0.0f, 0.0f, 0.0f, 0.0f });
			attr_data.insert(attr_data.end(), empty.begin(), empty.end());
			return;
		}

		// Otherwise convert and add
		for (uint i = 0; i < vertexCount; i++)
		{
			aiColor4D* cid = &(sourceData[i]);
			targetAttribute->addData(glm::vec4(static_cast<float>(cid->r), static_cast<float>(cid->g), static_cast<float>(cid->b), static_cast<float>(cid->a)));
		}
	}


	GeometryFromFile::GeometryFromFile(Core& core) : mRenderService(core.getService<RenderService>())
	{ }


	bool GeometryFromFile::init(utility::ErrorState& errorState)
	{
		// Load our mesh
		nap::Logger::info("Loading geometry: %s", mPath.c_str());

		// Create mesh instance
		std::unique_ptr<MeshInstance> mesh_instance = std::make_unique<MeshInstance>(*mRenderService);

		// Set the usage and cull mode for the mesh
		mesh_instance->setUsage(mUsage);
		mesh_instance->setCullMode(mCullMode);
		mesh_instance->setDrawMode(EDrawMode::Triangles);
		mesh_instance->setPolygonMode(mPolygonMode);

		// Create importer
		Assimp::Importer importer;

		// Setup flags
		nap::uint flags = aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_JoinIdenticalVertices;
		flags = mGenerateNormals ? flags | aiProcess_GenSmoothNormals : flags;
		flags = mCalculateTangents ? flags | aiProcess_CalcTangentSpace : flags;

		// Load file using flags
		importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, (int)mSmoothingAngle);
		const aiScene* ai_scene = importer.ReadFile(mPath, flags);

		// Ensure the file was read
		if (!errorState.check(ai_scene != nullptr, "Unable to load %s", mPath.c_str()))
			return false;

		// Used to figure out what data to extract
		uint uv_channel_count = 0;
		uint co_channel_count = 0;
		bool has_normals  = false;
		bool has_tangents = false;

		// Figure out which and how many attributes to create
		// If an attribute is present on one mesh and not the other, we still create it,
		// but initialize it empty later on
		std::vector<aiMesh*> valid_meshes;
		std::vector<std::string> mesh_names;
		valid_meshes.reserve(ai_scene->mNumMeshes);
		mesh_names.reserve(ai_scene->mNumMeshes);

		uint total_vertex_count = 0;
		for (uint i = 0; i < ai_scene->mNumMeshes; i++)
		{
			// Get unique name for mesh
			aiMesh* ai_mesh = ai_scene->mMeshes[i];
			std::string mesh_name = ai_mesh->mName.length == 0 ?
				utility::stringFormat("%s:%d", utility::getFileNameWithoutExtension(mPath).c_str(), i) :
				ai_mesh->mName.C_Str();

			// Ensure the mesh has vertex data
			if (ai_mesh->mNumVertices == 0)
			{
				nap::Logger::warn("%s has no vertex data", mesh_name.c_str());
				continue;
			}

			// Ensure the mesh has indices
			if (!(ai_mesh->HasFaces()))
			{
				nap::Logger::warn("%s has no indices", mesh_name.c_str());
				continue;
			}

			// Ensure it's a triangle mesh
			// TODO: Add support for other polygon types (line, lines-trip etc.)
			if (ai_mesh->mFaces[0].mNumIndices != 3)
			{
				nap::Logger::warn("%s is not a triangle mesh", mesh_name.c_str());
				continue;
			}

			// Update attribute requirements
			has_normals  = !has_normals  ? ai_mesh->HasNormals() : true;
			has_tangents = !has_tangents ? ai_mesh->HasTangentsAndBitangents() : true;
			uv_channel_count = ai_mesh->GetNumUVChannels() > uv_channel_count ? ai_mesh->GetNumUVChannels() : uv_channel_count;
			co_channel_count = ai_mesh->GetNumColorChannels() > co_channel_count ? ai_mesh->GetNumColorChannels() : co_channel_count;
			
			// Add as valid mesh
			valid_meshes.emplace_back(ai_mesh);
			mesh_names.emplace_back(mesh_name);
			total_vertex_count += ai_mesh->mNumVertices;
		}

		// Ensure there is geometry
		if (!errorState.check(!(valid_meshes.empty()), "No valid meshes found in file %s", mPath.c_str()))
			return false;

		// Create color attributes based on found number of channels
		std::vector<VertexAttribute<glm::vec4>*> col_attribs(co_channel_count);
		for (uint i = 0; i < co_channel_count; i++)
		{
			col_attribs[i] = &mesh_instance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(i));
			col_attribs[i]->reserve(total_vertex_count);
		}

		// Create uv attributes based on found number of channels
		std::vector<VertexAttribute<glm::vec3>*> uvs_attribs(uv_channel_count);
		for (uint i = 0; i < uv_channel_count; i++)
		{
			uvs_attribs[i] = &mesh_instance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(i));
			uvs_attribs[i]->reserve(total_vertex_count);
		}

		// Create attributes, position should always be there, others are optional
		VertexAttribute<glm::vec3>* pos_attribute = &mesh_instance->getOrCreateAttribute<glm::vec3>(vertexid::position);
		pos_attribute->reserve(total_vertex_count);

		// Create normal attribute if found on any mesh
		VertexAttribute<glm::vec3>* nor_attribute = nullptr;
		if (has_normals)
		{
			nor_attribute = &mesh_instance->getOrCreateAttribute<glm::vec3>(vertexid::normal);
			nor_attribute->reserve(total_vertex_count);
		}

		// Create tangent related attributes if found on any mesh
		VertexAttribute<glm::vec3>* tan_attribute = nullptr;
		VertexAttribute<glm::vec3>* ban_attribute = nullptr;
		if (has_tangents)
		{
			tan_attribute = &mesh_instance->getOrCreateAttribute<glm::vec3>(vertexid::tangent);
			tan_attribute->reserve(total_vertex_count);
			ban_attribute = &mesh_instance->getOrCreateAttribute<glm::vec3>(vertexid::bitangent);
			ban_attribute->reserve(total_vertex_count);
		}

		// Create a shape for every mesh
		uint vertex_offset = 0;
		for (uint i = 0; i < valid_meshes.size(); i++)
		{
			// Should not occur, but if so, error
			aiMesh* mesh = valid_meshes[i];
			const std::string& mesh_name = mesh_names[i];

			// Reserve space for attributes to add
			nap::Logger::info("Loading mesh: %s", mesh_name.c_str());
			uint ai_vert_count = mesh->mNumVertices;

			// Copy vertex data
			copyVec3Data(ai_vert_count, mesh->mVertices, pos_attribute, vertexid::position, mesh_name);

			// Copy normal data if present
			copyVec3Data(ai_vert_count, mesh->mNormals, nor_attribute, vertexid::normal, mesh_name);

			// Copy tangent data if present
			copyVec3Data(ai_vert_count, mesh->mTangents, tan_attribute, vertexid::tangent, mesh_name);
			
			// Copy bitangent data if present
			copyVec3Data(ai_vert_count, mesh->mBitangents, ban_attribute, vertexid::bitangent, mesh_name);

			// Copy uv data
			for (uint u = 0; u < uv_channel_count; u++)
				copyVec3Data(ai_vert_count, mesh->mTextureCoords[u], uvs_attribs[u], vertexid::getUVName(u), mesh_name);

			// Copy color data
			for (uint c = 0; c < co_channel_count; c++)
				copyVec4Data(ai_vert_count, mesh->mColors[c], col_attribs[c], vertexid::getColorName(c), mesh_name);

			// Create new shape and add indices
			uint face_count = mesh->mNumFaces;
			MeshShape& shape = mesh_instance->createShape();
			std::vector<uint32>& indices = shape.getIndices();
			indices.reserve(face_count * 3);
			for (int f = 0; f != face_count; f++)
			{
				aiFace& face = mesh->mFaces[f];
				assert(face.mNumIndices == 3);
				for (int fi = 0; fi != face.mNumIndices; ++fi)
				{
					indices.emplace_back(face.mIndices[fi] + vertex_offset);
				}
			}

			// Compute vertex offset, required for next shape
			vertex_offset += ai_vert_count;
		}

		// Set number of vertices and initialize
		mesh_instance->setNumVertices(pos_attribute->getCount());
		if (!mesh_instance->init(errorState))
			return false;

		// Move
		mMeshInstance = std::move(mesh_instance);

		return true;
	}
}
