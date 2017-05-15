// Local Includes
#include "fbxconverter.h"
#include "rtti/typeinfo.h"
#include "rtti/jsonwriter.h"

// STL includes
#include <memory>
#include <vector>
#include <fstream>

// External Includes
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "nap/stringutils.h"
#include "nap/object.h"
#include "rtti/binarywriter.h"
#include "nap/fileutils.h"
#include "nap/errorstate.h"
#include "rtti/binaryreader.h"
#include "nmesh.h"

namespace nap
{
	static int gMaxNumUVAttributes = 16;
	static int gMaxNumColorAttributes = 16;

	class MeshData : public Object
	{
		RTTI_ENABLE(Object)

	public:
		struct Attribute
		{
			std::string			mID;
			int					mNumComponents;
			std::vector<float>	mData;
		};

		std::vector<Attribute> mAttributes;
		int mNumVertices = 0;
		std::vector<unsigned int> mIndices;

		const Attribute* FindAttribute(const std::string& id) const
		{
			for (const Attribute& attribute : mAttributes)
				if (attribute.mID == id)
					return &attribute;

			return nullptr;
		}

		Attribute& GetOrCreateAttribute(const std::string& id)
		{
			for (Attribute& attribute : mAttributes)
				if (attribute.mID == id)
					return attribute;

			Attribute new_attribute;
			new_attribute.mID = id;
			mAttributes.push_back(new_attribute);

			return mAttributes[mAttributes.size() - 1];
		}
	};

	RTTI_BEGIN_CLASS(MeshData::Attribute)
		RTTI_PROPERTY("ID",				&MeshData::Attribute::mID)
		RTTI_PROPERTY("NumComponents",	&MeshData::Attribute::mNumComponents)
		RTTI_PROPERTY("Data",			&MeshData::Attribute::mData)
	RTTI_END_CLASS

	RTTI_BEGIN_CLASS(MeshData)
		RTTI_PROPERTY("Attributes", &MeshData::mAttributes)
		RTTI_PROPERTY("NumVertices", &MeshData::mNumVertices)
		RTTI_PROPERTY("Indices", &MeshData::mIndices)
	RTTI_END_CLASS


	bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, EFBXConversionOptions convertOptions, std::vector<std::string>& convertedFiles, ErrorState& errorState)
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

		if (!errorState.check(scene->mNumMeshes != 0, "No meshes found in FBX"))
			return false;

		uint64_t fbx_mod_time;
		if (!errorState.check(getFileModificationTime(fbxPath, fbx_mod_time), "Failed to retrieve modification time from %s", fbxPath.c_str()))
			return false;

		// Create meshes for every contained mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			// Get assimp mesh
			aiMesh* mesh = scene->mMeshes[i];

			// If there is only a single mesh in the file, use the file name as ID. Otherwise, append the index of the mesh to the file name.
			std::string converted_name;
			if (scene->mNumMeshes == 1)
			{
				converted_name = getFileNameWithoutExtension(fbxPath);
			}				
			else
			{
				if (mesh->mName.length != 0)
					converted_name = stringFormat("%s_%s", fbxPath.c_str(), mesh->mName.C_Str());
				else
					converted_name = stringFormat("%s_%d", getFileNameWithoutExtension(fbxPath).c_str(), i);			
			}				

			std::string output_file = getAbsolutePath(stringFormat("%s/%s.mesh", outputDirectory.c_str(), converted_name.c_str()));

			// Determine whether the output file exists and if it does, its modification time
			uint64_t output_mod_time;
			bool output_exists = getFileModificationTime(output_file, output_mod_time);

			// We want to convert the file if it does not exist, or if the source file is newer than the output file
			bool should_convert = !output_exists || convertOptions == EFBXConversionOptions::CONVERT_ALWAYS || fbx_mod_time > output_mod_time;
			if (!should_convert)
				continue;

			MeshData mesh_data;
			mesh_data.mID = converted_name;

			if (!errorState.check(mesh->mNumVertices != 0, "Encountered mesh with no vertices"))
				return false;

			mesh_data.mNumVertices = mesh->mNumVertices;

			// Copy vertex data			
			MeshData::Attribute& position_attribute = mesh_data.GetOrCreateAttribute(opengl::Mesh::VertexAttributeIDs::PositionVertexAttr);
			position_attribute.mNumComponents = 3;
			position_attribute.mData.reserve(mesh->mNumVertices * 3);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
			{
				aiVector3D* current_id = &(mesh->mVertices[vertex]);
				position_attribute.mData.emplace_back(static_cast<float>(current_id->x));
				position_attribute.mData.emplace_back(static_cast<float>(current_id->y));
				position_attribute.mData.emplace_back(static_cast<float>(current_id->z));
			}

			// Copy normals
			if (mesh->HasNormals())
			{
				MeshData::Attribute& normal_attribute = mesh_data.GetOrCreateAttribute(opengl::Mesh::VertexAttributeIDs::NormalVertexAttr);
				normal_attribute.mNumComponents = 3;
				normal_attribute.mData.reserve(mesh->mNumVertices * 3);
				for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
				{
					aiVector3D* current_id = &(mesh->mNormals[vertex]);
					normal_attribute.mData.emplace_back(static_cast<float>(current_id->x));
					normal_attribute.mData.emplace_back(static_cast<float>(current_id->y));
					normal_attribute.mData.emplace_back(static_cast<float>(current_id->z));
				}
			}

			// Copy all uv data channels
			for (unsigned int uv_channel = 0; uv_channel < mesh->GetNumUVChannels(); uv_channel++)
			{
				aiVector3D* uv_channel_data = mesh->mTextureCoords[uv_channel];

				MeshData::Attribute& uv_attribute = mesh_data.GetOrCreateAttribute(opengl::Mesh::VertexAttributeIDs::GetUVVertexAttr(uv_channel));
				uv_attribute.mNumComponents = 3;
				uv_attribute.mData.reserve(mesh->mNumVertices * 3);

				// Copy uv data channel
				for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
				{
					aiVector3D* current_id = &(uv_channel_data[vertex]);
					uv_attribute.mData.emplace_back(static_cast<float>(current_id->x));
					uv_attribute.mData.emplace_back(static_cast<float>(current_id->y));
					uv_attribute.mData.emplace_back(static_cast<float>(current_id->z));
				}
			}


			// Copy all color data channels
			for (unsigned int color_channel = 0; color_channel < mesh->GetNumColorChannels(); color_channel++)
			{
				aiColor4D* color_channel_data = mesh->mColors[color_channel];

				MeshData::Attribute& color_attribute = mesh_data.GetOrCreateAttribute(opengl::Mesh::VertexAttributeIDs::GetColorVertexAttr(color_channel));
				color_attribute.mNumComponents = 4;
				color_attribute.mData.reserve(mesh->mNumVertices * 4);

				// Copy color data channel
				for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
				{
					aiColor4D* current_id = &(color_channel_data[vertex]);
					color_attribute.mData.emplace_back(static_cast<float>(current_id->r));
					color_attribute.mData.emplace_back(static_cast<float>(current_id->g));
					color_attribute.mData.emplace_back(static_cast<float>(current_id->b));
					color_attribute.mData.emplace_back(static_cast<float>(current_id->a));
				}
			}

			// Retrieve index data
			if (!errorState.check(mesh->HasFaces(), "Mesh has no indices"))
				return false;

			mesh_data.mIndices.reserve(mesh->mNumFaces * 3);
			for (int face_index = 0; face_index != mesh->mNumFaces; ++face_index)
			{
				aiFace& face = mesh->mFaces[face_index];
				assert(face.mNumIndices == 3);

				for (int point_index = 0; point_index != face.mNumIndices; ++point_index)
					mesh_data.mIndices.push_back(face.mIndices[point_index]);
			}

			BinaryWriter binaryWriter;
			if (!serializeObjects({ &mesh_data }, binaryWriter, errorState))
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

	std::unique_ptr<opengl::Mesh> loadMesh(const std::string& meshPath, ErrorState& errorState)
	{
		RTTIDeserializeResult deserialize_result;
		if (!errorState.check(readBinary(meshPath, deserialize_result, errorState), "Failed to load mesh from %s", meshPath.c_str()))
			return nullptr;

		if (!errorState.check(deserialize_result.mReadObjects.size() == 1, "Trying to load an invalid mesh file. File contains %d objects", deserialize_result.mReadObjects.size()))
			return nullptr;

		if (!errorState.check(deserialize_result.mReadObjects[0]->get_type() == RTTI_OF(MeshData), "Trying to load an invalid mesh file. File does not contain MeshData"))
			return nullptr;

		assert(deserialize_result.mUnresolvedPointers.empty());

		const MeshData* mesh_data = static_cast<MeshData*>(deserialize_result.mReadObjects[0].get());

		std::unique_ptr<opengl::Mesh> mesh = std::make_unique<opengl::Mesh>(mesh_data->mNumVertices, opengl::EDrawMode::TRIANGLES);

		// Copy vertex attribute data to mesh
		for (const MeshData::Attribute& attribute : mesh_data->mAttributes)
			mesh->addVertexAttribute(attribute.mID, attribute.mNumComponents, attribute.mData.data());

		// Make sure there's position data
		if (!errorState.check(mesh->findVertexAttributeBuffer(opengl::Mesh::VertexAttributeIDs::PositionVertexAttr) != nullptr, "Required attribute 'position' not found in mesh data"))
			return nullptr;
			
		// Copy indices
		if (!errorState.check(!mesh_data->mIndices.empty(), "No index data found in mesh"))
			return nullptr;

		mesh->setIndices(mesh_data->mIndices.size(), mesh_data->mIndices.data());

		return mesh;
	}

} // opengl