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
#include <nap/fileutils.h>
#include <utility/errorstate.h>
#include <rtti/binaryreader.h>

// Local Includes
#include "mesh.h"

namespace nap
{
	bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, EFBXConversionOptions convertOptions, std::vector<std::string>& convertedFiles, utility::ErrorState& errorState)
	{
		// Create importer
		std::unique_ptr<Assimp::Importer> importer = std::make_unique<Assimp::Importer>();

		// Load file
		const aiScene* scene = importer->ReadFile(fbxPath,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_JoinIdenticalVertices);

		if (!errorState.check(scene != nullptr, "Unable to read %s", fbxPath.c_str()))
			return false;

		if (!errorState.check(scene->mNumMeshes != 0, "No meshes found in FBX %s", fbxPath.c_str()))
			return false;

		uint64_t fbx_mod_time;
		if (!errorState.check(getFileModificationTime(fbxPath, fbx_mod_time), "Failed to retrieve modification time from %s", fbxPath.c_str()))
			return false;

		// Create meshes for every contained mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			// Get assimp mesh
			aiMesh* fbx_mesh = scene->mMeshes[i];

			// If there is only a single mesh in the file, use the file name as ID. Otherwise, append the index of the mesh to the file name.
			std::string converted_name;
			if (scene->mNumMeshes == 1)
			{
				converted_name = getFileNameWithoutExtension(fbxPath);
			}				
			else
			{
				if (fbx_mesh->mName.length != 0)
					converted_name = utility::stringFormat("%s_%s", getFileNameWithoutExtension(fbxPath).c_str(), fbx_mesh->mName.C_Str());
				else
					converted_name = utility::stringFormat("%s_%d", getFileNameWithoutExtension(fbxPath).c_str(), i);			
			}				

			std::string output_file = getAbsolutePath(utility::stringFormat("%s/%s.mesh", outputDirectory.c_str(), converted_name.c_str()));

			// Determine whether the output file exists and if it does, its modification time
			uint64_t output_mod_time;
			bool output_exists = getFileModificationTime(output_file, output_mod_time);

			// We want to convert the file if it does not exist, or if the source file is newer than the output file
			bool should_convert = !output_exists || convertOptions == EFBXConversionOptions::CONVERT_ALWAYS || fbx_mod_time > output_mod_time;
			if (!should_convert)
				continue;

			Mesh mesh_data;
			mesh_data.mID = converted_name;

			if (!errorState.check(fbx_mesh->mNumVertices != 0, "Encountered mesh with no vertices"))
				return false;

			mesh_data.mNumVertices = fbx_mesh->mNumVertices;
			mesh_data.mDrawMode = opengl::EDrawMode::TRIANGLES;

			// Copy vertex data			
			TypedMeshAttribute<glm::vec3>& position_attribute = mesh_data.GetOrCreateAttribute<glm::vec3>(opengl::Mesh::VertexAttributeIDs::PositionVertexAttr);
			position_attribute.Reserve(fbx_mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < fbx_mesh->mNumVertices; vertex++)
			{
				aiVector3D* current_id = &(fbx_mesh->mVertices[vertex]);
				position_attribute.Add(glm::vec3(static_cast<float>(current_id->x), static_cast<float>(current_id->y), static_cast<float>(current_id->z)));
			}

			// Copy normals
			if (fbx_mesh->HasNormals())
			{
				TypedMeshAttribute<glm::vec3>& normal_attribute = mesh_data.GetOrCreateAttribute<glm::vec3>(opengl::Mesh::VertexAttributeIDs::NormalVertexAttr);
				normal_attribute.Reserve(fbx_mesh->mNumVertices);
				for (unsigned int vertex = 0; vertex < fbx_mesh->mNumVertices; vertex++)
				{
					aiVector3D* current_id = &(fbx_mesh->mNormals[vertex]);
					normal_attribute.Add(glm::vec3(static_cast<float>(current_id->x), static_cast<float>(current_id->y), static_cast<float>(current_id->z)));
				}
			}

			// Copy all uv data channels
			for (unsigned int uv_channel = 0; uv_channel < fbx_mesh->GetNumUVChannels(); uv_channel++)
			{
				aiVector3D* uv_channel_data = fbx_mesh->mTextureCoords[uv_channel];

				TypedMeshAttribute<glm::vec3>& uv_attribute = mesh_data.GetOrCreateAttribute<glm::vec3>(opengl::Mesh::VertexAttributeIDs::GetUVVertexAttr(uv_channel));
				uv_attribute.Reserve(fbx_mesh->mNumVertices);

				// Copy uv data channel
				for (unsigned int vertex = 0; vertex < fbx_mesh->mNumVertices; vertex++)
				{
					aiVector3D* current_id = &(uv_channel_data[vertex]);
					uv_attribute.Add(glm::vec3(static_cast<float>(current_id->x), static_cast<float>(current_id->y), static_cast<float>(current_id->z)));
				}
			}


			// Copy all color data channels
			for (unsigned int color_channel = 0; color_channel < fbx_mesh->GetNumColorChannels(); color_channel++)
			{
				aiColor4D* color_channel_data = fbx_mesh->mColors[color_channel];

				TypedMeshAttribute<glm::vec4>& color_attribute = mesh_data.GetOrCreateAttribute<glm::vec4>(opengl::Mesh::VertexAttributeIDs::GetColorVertexAttr(color_channel));
				color_attribute.Reserve(fbx_mesh->mNumVertices);

				// Copy color data channel
				for (unsigned int vertex = 0; vertex < fbx_mesh->mNumVertices; vertex++)
				{
					aiColor4D* current_id = &(color_channel_data[vertex]);
					color_attribute.Add(glm::vec4(static_cast<float>(current_id->r), static_cast<float>(current_id->g), static_cast<float>(current_id->b), static_cast<float>(current_id->a)));
				}
			}

			// Retrieve index data
			if (!errorState.check(fbx_mesh->HasFaces(), "Mesh has no indices"))
				return false;

			mesh_data.ReserveIndices(fbx_mesh->mNumFaces * 3);
			for (int face_index = 0; face_index != fbx_mesh->mNumFaces; ++face_index)
			{
				aiFace& face = fbx_mesh->mFaces[face_index];
				assert(face.mNumIndices == 3);

				for (int point_index = 0; point_index != face.mNumIndices; ++point_index)
					mesh_data.AddIndex(face.mIndices[point_index]);
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

	std::unique_ptr<Mesh> loadMesh(const std::string& meshPath, utility::ErrorState& errorState)
	{
		rtti::Factory factory;

		rtti::RTTIDeserializeResult deserialize_result;
		if (!errorState.check(readBinary(meshPath, factory, deserialize_result, errorState), "Failed to load mesh from %s", meshPath.c_str()))
			return nullptr;

		if (!errorState.check(deserialize_result.mReadObjects.size() == 1, "Trying to load an invalid mesh file. File %s contains %d objects, expected 1", meshPath.c_str(), deserialize_result.mReadObjects.size()))
			return nullptr;

		if (!errorState.check(deserialize_result.mReadObjects[0]->get_type() == RTTI_OF(Mesh), "Trying to load an invalid mesh file %s; file does not contain MeshData", meshPath.c_str()))
			return nullptr;

		assert(deserialize_result.mUnresolvedPointers.empty());

		std::unique_ptr<Mesh> mesh = std::move(rtti_cast<Mesh>(deserialize_result.mReadObjects[0]));

		return mesh;
	}

} // opengl
