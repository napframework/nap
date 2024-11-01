/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "fbxconverter.h"
#include "rtti/typeinfo.h"
#include "rtti/jsonwriter.h"
#include "rtti/factory.h"

// STL includes
#include <memory>
#include <vector>
#include <fstream>

// External Includes
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <utility/stringutils.h>
#include <rtti/binarywriter.h>
#include <utility/fileutils.h>
#include <utility/errorstate.h>
#include <rtti/binaryreader.h>
#include <rtti/defaultlinkresolver.h>
#include <renderglobals.h>

// Local Includes
#include "mesh.h"

namespace nap
{
	/**
	 * Creates a vertex attribute in the Mesh. Because the Mesh is used ObjectPtrs to refer to attributes, a storage object is used that
	 * has ownership over the objects. The storage object should live at least as long than the Mesh.
	 */
	template<class T>
	static VertexAttribute<T>& CreateAttribute(Mesh& mesh, const std::string& id, std::vector<std::unique_ptr<BaseVertexAttribute>>& storage)
	{
		auto new_attribute = std::make_unique<VertexAttribute<T>>();
		new_attribute->mAttributeID = id;

		assert(!mesh.mID.empty());
		new_attribute->mID = mesh.mID + "_" + id;
		mesh.mProperties.mAttributes.push_back(new_attribute.get());
		storage.emplace_back(std::move(new_attribute));

		return static_cast<VertexAttribute<T>&>(*mesh.mProperties.mAttributes[mesh.mProperties.mAttributes.size() - 1]);
	}

	template<class T>
	static void CreateAttributeData(Mesh& mesh, const aiVector3D* fbxAttribute, uint numVertices, const std::string& id, std::vector<std::unique_ptr<BaseVertexAttribute>>& storage);

	template<>
	void CreateAttributeData<glm::vec3>(Mesh& mesh, const aiVector3D* fbxAttribute, uint numVertices, const std::string& id, std::vector<std::unique_ptr<BaseVertexAttribute>>& storage)
	{
		auto& attribute = CreateAttribute<glm::vec3>(mesh, id, storage);
		attribute.reserve(numVertices);
		for (uint vertex = 0; vertex < numVertices; vertex++)
		{
			auto* current_id = &fbxAttribute[vertex];
			attribute.addData(glm::vec3(static_cast<float>(current_id->x), static_cast<float>(current_id->y), static_cast<float>(current_id->z)));
		}
	}

	template<>
	void CreateAttributeData<glm::vec4>(Mesh& mesh, const aiVector3D* fbxAttribute, uint numVertices, const std::string& id, std::vector<std::unique_ptr<BaseVertexAttribute>>& storage)
	{
		auto& attribute = CreateAttribute<glm::vec4>(mesh, id, storage);
		attribute.reserve(numVertices);
		for (uint vertex = 0; vertex < numVertices; vertex++)
		{
			auto* current_id = &fbxAttribute[vertex];
			attribute.addData(glm::vec4(static_cast<float>(current_id->x), static_cast<float>(current_id->y), static_cast<float>(current_id->z), 0.0f));
		}
	}


	bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, EFBXConversionOptions convertOptions, bool convertCompute, bool noTangents, std::vector<std::string>& convertedFiles, utility::ErrorState& errorState)
	{
		// Create importer
		std::unique_ptr<Assimp::Importer> importer = std::make_unique<Assimp::Importer>();

		// Load file
		const aiScene* scene = importer->ReadFile(fbxPath,
		  	(!noTangents ? aiProcess_CalcTangentSpace : 0x0) |
			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_JoinIdenticalVertices
		);

		if (!errorState.check(scene != nullptr, "Unable to read %s", fbxPath.c_str()))
			return false;

		if (!errorState.check(scene->mNumMeshes != 0, "No meshes found in FBX %s", fbxPath.c_str()))
			return false;

		uint64_t fbx_mod_time;
		if (!errorState.check(utility::getFileModificationTime(fbxPath, fbx_mod_time), "Failed to retrieve modification time from %s", fbxPath.c_str()))
			return false;

		// Create meshes for every contained mesh
		for (uint i = 0; i < scene->mNumMeshes; i++)
		{
			// Get assimp mesh
			aiMesh* fbx_mesh = scene->mMeshes[i];

			// If there is only a single mesh in the file, use the file name as ID. Otherwise, append the index of the mesh to the file name.
			std::string converted_name;
			if (scene->mNumMeshes == 1)
			{
				converted_name = utility::getFileNameWithoutExtension(fbxPath);
			}				
			else
			{
				if (fbx_mesh->mName.length != 0)
					converted_name = utility::stringFormat("%s_%s", utility::getFileNameWithoutExtension(fbxPath).c_str(), fbx_mesh->mName.C_Str());
				else
					converted_name = utility::stringFormat("%s_%d", utility::getFileNameWithoutExtension(fbxPath).c_str(), i);			
			}				

			std::string output_file = utility::getAbsolutePath(utility::stringFormat("%s/%s.mesh", outputDirectory.c_str(), converted_name.c_str()));

			// Determine whether the output file exists and if it does, its modification time
			uint64_t output_mod_time;
			bool output_exists = utility::getFileModificationTime(output_file, output_mod_time);

			// We want to convert the file if it does not exist, or if the source file is newer than the output file
			bool should_convert = !output_exists || convertOptions == EFBXConversionOptions::CONVERT_ALWAYS || fbx_mod_time > output_mod_time || !rtti::checkBinaryVersion(output_file);
			if (!should_convert)
				continue;

			Mesh mesh_data;
			mesh_data.mID = converted_name;

			if (!errorState.check(fbx_mesh->mNumVertices != 0, "Encountered mesh with no vertices"))
				return false;

			const uint num_verts = fbx_mesh->mNumVertices;
			mesh_data.mProperties.mNumVertices = num_verts;
			mesh_data.mProperties.mDrawMode = EDrawMode::Triangles;

			std::vector<std::unique_ptr<BaseVertexAttribute>> vertex_attribute_storage;

			if (!convertCompute)
			{
				// Copy vertex data
				CreateAttributeData<glm::vec3>(mesh_data, fbx_mesh->mVertices, num_verts, vertexid::position, vertex_attribute_storage);

				// Copy normals
				if (fbx_mesh->HasNormals())
					CreateAttributeData<glm::vec3>(mesh_data, fbx_mesh->mNormals, num_verts, vertexid::normal, vertex_attribute_storage);

				// Copy tangents
				if (fbx_mesh->HasTangentsAndBitangents())
				{
					CreateAttributeData<glm::vec3>(mesh_data, fbx_mesh->mTangents, num_verts, vertexid::tangent, vertex_attribute_storage);
					CreateAttributeData<glm::vec3>(mesh_data, fbx_mesh->mBitangents, num_verts, vertexid::bitangent, vertex_attribute_storage);
				}

				// Copy all uv data channels
				for (int uv_channel = 0; uv_channel < fbx_mesh->GetNumUVChannels(); uv_channel++)
					CreateAttributeData<glm::vec3>(mesh_data, fbx_mesh->mTextureCoords[uv_channel], num_verts, vertexid::getUVName(uv_channel), vertex_attribute_storage);
			}
			else
			{
				// Copy vertex data
				CreateAttributeData<glm::vec4>(mesh_data, fbx_mesh->mVertices, num_verts, vertexid::position, vertex_attribute_storage);

				// Copy normals
				if (fbx_mesh->HasNormals())
					CreateAttributeData<glm::vec4>(mesh_data, fbx_mesh->mNormals, num_verts, vertexid::normal, vertex_attribute_storage);

				// Copy tangents
				if (fbx_mesh->HasTangentsAndBitangents())
				{
					CreateAttributeData<glm::vec4>(mesh_data, fbx_mesh->mTangents, num_verts, vertexid::tangent, vertex_attribute_storage);
					CreateAttributeData<glm::vec4>(mesh_data, fbx_mesh->mBitangents, num_verts, vertexid::bitangent, vertex_attribute_storage);
				}

				// Copy all uv data channels
				for (int uv_channel = 0; uv_channel < fbx_mesh->GetNumUVChannels(); uv_channel++)
					CreateAttributeData<glm::vec4>(mesh_data, fbx_mesh->mTextureCoords[uv_channel], num_verts, vertexid::getUVName(uv_channel), vertex_attribute_storage);
			}

			// Copy all color data channels
			for (uint color_channel = 0; color_channel < fbx_mesh->GetNumColorChannels(); color_channel++)
			{
				aiColor4D* color_channel_data = fbx_mesh->mColors[color_channel];

				VertexAttribute<glm::vec4>& color_attribute = CreateAttribute<glm::vec4>(mesh_data, vertexid::getColorName(color_channel), vertex_attribute_storage);
				color_attribute.reserve(fbx_mesh->mNumVertices);

				// Copy color data channel
				for (uint vertex = 0; vertex < fbx_mesh->mNumVertices; vertex++)
				{
					aiColor4D* current_id = &(color_channel_data[vertex]);
					color_attribute.addData(glm::vec4(static_cast<float>(current_id->r), static_cast<float>(current_id->g), static_cast<float>(current_id->b), static_cast<float>(current_id->a)));
				}
			}

			// Retrieve index data
			if (!errorState.check(fbx_mesh->HasFaces(), "Mesh has no indices"))
				return false;

			mesh_data.mProperties.mShapes.push_back(MeshShape());
			MeshShape& shape = mesh_data.mProperties.mShapes.back();

			std::vector<uint32>& indices = shape.getIndices();
			indices.reserve(fbx_mesh->mNumFaces * 3);
			for (int face_index = 0; face_index != fbx_mesh->mNumFaces; ++face_index)
			{
				aiFace& face = fbx_mesh->mFaces[face_index];
				assert(face.mNumIndices == 3);

				for (int point_index = 0; point_index != face.mNumIndices; ++point_index)
					indices.emplace_back(face.mIndices[point_index]);
			}

			rtti::BinaryWriter binaryWriter;
			if (!rtti::serializeObjects({ &mesh_data }, binaryWriter, errorState))
				return false;

			std::ofstream bin_output(output_file, std::ofstream::out | std::ofstream::binary);
			if (!errorState.check(bin_output.is_open(), "Failed to open %s for writing", output_file.c_str()))
				return false;

			bin_output.write((const char*)binaryWriter.getBuffer().data(), binaryWriter.getBuffer().size());
			bin_output.close();

			convertedFiles.push_back(output_file);
		}

		return true;
	}

	std::unique_ptr<MeshInstance> loadMesh(RenderService& renderService, const std::string& meshPath, utility::ErrorState& errorState)
	{
		rtti::Factory factory;

		rtti::DeserializeResult deserialize_result;
		if (!errorState.check(readBinary(meshPath, factory, deserialize_result, errorState), "Failed to load mesh from %s", meshPath.c_str()))
			return nullptr;

		if (!errorState.check(rtti::DefaultLinkResolver::sResolveLinks(deserialize_result.mReadObjects, deserialize_result.mUnresolvedPointers, errorState), "Failed to resolve pointers"))
			return nullptr;
		
		// Find mesh(es) in the file
		int numMeshes = 0;
		std::unique_ptr<Mesh> mesh;
		for (auto& object : deserialize_result.mReadObjects)
		{
			if (object->get_type() == RTTI_OF(nap::Mesh))
			{
				mesh = rtti_cast<Mesh>(object);
				++numMeshes;
			}
		}

		if (!errorState.check(numMeshes == 1, "Trying to load an invalid mesh file. File %s contains %d meshes, expected 1", meshPath.c_str(), numMeshes))
			return nullptr;

		// We create the MeshInstance here instead of returning the Mesh object. The reason is that the objects that are read (in this case, Mesh and multiple 
		// VertexAttribute objects) are owned by the deserialize_result object. Instead of returning the mesh, we create the MeshInstance here. The init()
		// will clone contents and take ownership of the cloned content. 
		// The RTTI data in Mesh is lost, which is intentional as we don't need an extra copy of CPU data in memory. If we need to have an option to keep the source CPU data for binary
		// meshes, this can be supported later by adding it. This could be the case if we are doing dynamic geometry based on a binary mesh where we keep the source mesh for reference.
		std::unique_ptr<MeshInstance> mesh_instance = std::make_unique<MeshInstance>(renderService);
		mesh_instance->copyMeshProperties(mesh->mProperties);

		return mesh_instance;
	}
}
