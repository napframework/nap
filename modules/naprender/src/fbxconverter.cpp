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
#include "nap/resource.h"
#include "rtti/binaryreader.h"
#include "nmesh.h"

namespace nap
{
	static std::string gPositionVertexAttr("Position");
	static std::string gNormalVertexAttr("Normal");
	static std::string gUVVertexAttr("UV");	
	static std::string gColorVertexAttr("Color");
	static int gMaxNumUVAttributes = 16;
	static int gMaxNumColorAttributes = 16;

	class MeshData : public Object
	{
		RTTI_ENABLE(Object)

	public:
		struct Attribute
		{
			std::string mID;
			std::vector<float> mData;
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
		RTTI_PROPERTY("ID",		&MeshData::Attribute::mID)
		RTTI_PROPERTY("Data",	&MeshData::Attribute::mData)
	RTTI_END_CLASS

	RTTI_BEGIN_CLASS(MeshData)
		RTTI_PROPERTY("Attributes", &MeshData::mAttributes)
		RTTI_PROPERTY("NumVertices", &MeshData::mNumVertices)
		RTTI_PROPERTY("Indices", &MeshData::mIndices)
	RTTI_END_CLASS


	bool convertFBX(const std::string& fbxPath, const std::string& outputDirectory, InitResult& initResult)
	{
		// Create importer
		std::unique_ptr<Assimp::Importer> importer = std::make_unique<Assimp::Importer>();

		// Load file
		const aiScene* scene = importer->ReadFile(fbxPath,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_JoinIdenticalVertices);

		if (scene == nullptr)
			return false;

		if (scene->mNumMeshes == 0)
			return false;

		// Create meshes for every contained mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			// Get assimp mesh
			aiMesh* mesh = scene->mMeshes[i];

			MeshData mesh_data;
			if (mesh->mName.length != 0)
				mesh_data.mID = stringFormat("%s:%s", fbxPath.c_str(), mesh->mName.C_Str());
			else
				mesh_data.mID = stringFormat("%s:mesh%d", fbxPath.c_str(), i);

			if (!initResult.check(mesh->mNumVertices != 0, "Encountered mesh with no vertices"))
				return false;

			mesh_data.mNumVertices = mesh->mNumVertices;

			// Copy vertex data			
			MeshData::Attribute& position_attribute = mesh_data.GetOrCreateAttribute(gPositionVertexAttr);
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
				MeshData::Attribute& normal_attribute = mesh_data.GetOrCreateAttribute(gNormalVertexAttr);
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

				MeshData::Attribute& uv_attribute = mesh_data.GetOrCreateAttribute(nap::stringFormat("%s%d", gUVVertexAttr.c_str(), uv_channel));
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

				MeshData::Attribute& color_attribute = mesh_data.GetOrCreateAttribute(nap::stringFormat("%s%d", gColorVertexAttr.c_str(), color_channel));
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
			if (!initResult.check(mesh->HasFaces(), "Mesh has no indices"))
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
			if (!initResult.check(serializeObjects({ &mesh_data }, binaryWriter), "Failed to serialize mesh data to binary"))
				return false;

			std::string output_file = stringFormat("%s/%s.mesh", outputDirectory.c_str(), getFileNameWithoutExtension(fbxPath).c_str());
			std::ofstream bin_output(output_file, std::ofstream::out | std::ofstream::binary);
			if (!initResult.check(bin_output.is_open(), "Failed to open %s for writing", output_file.c_str()))
				return false;

			bin_output.write((const char*)binaryWriter.getBuffer().data(), binaryWriter.getBuffer().size());
			bin_output.close();
		}

		return true;
	}

	std::unique_ptr<opengl::Mesh> loadMesh(const std::string& meshPath, InitResult& initResult)
	{
		RTTIDeserializeResult deserialize_result;
		if (!initResult.check(readBinary(meshPath, deserialize_result, initResult), "Failed to load mesh from %s", meshPath.c_str()))
			return nullptr;

		if (!initResult.check(deserialize_result.mReadObjects.size() == 1, "Trying to load an invalid mesh file. File contains %d objects", deserialize_result.mReadObjects.size()))
			return nullptr;

		if (!initResult.check(deserialize_result.mReadObjects[0]->get_type() == RTTI_OF(MeshData), "Trying to load an invalid mesh file. File does not contain MeshData"))
			return nullptr;

		assert(deserialize_result.mUnresolvedPointers.empty());

		const MeshData* mesh_data = static_cast<MeshData*>(deserialize_result.mReadObjects[0].get());

		std::unique_ptr<opengl::Mesh> mesh = std::make_unique<opengl::Mesh>();

		// Copy vertex data
		const MeshData::Attribute* position_attribute = mesh_data->FindAttribute(gPositionVertexAttr);
		if (!initResult.check(position_attribute != nullptr, "Required attribute 'position' not found in mesh data"))
			return nullptr;

		mesh->copyVertexData(mesh_data->mNumVertices, position_attribute->mData.data());

		// Copy normal data
		const MeshData::Attribute* normal_attribute = mesh_data->FindAttribute(gNormalVertexAttr);
		if (normal_attribute != nullptr)
			mesh->copyNormalData(mesh_data->mNumVertices, normal_attribute->mData.data());

		// Copy UVs
		for (int uv = 0; uv < gMaxNumUVAttributes; ++uv)
		{
			const MeshData::Attribute* uv_attribute = mesh_data->FindAttribute(stringFormat("%s%d", gUVVertexAttr.c_str(), uv));
			if (uv_attribute == nullptr)
				break;

			mesh->copyUVData(3, mesh_data->mNumVertices, uv_attribute->mData.data());
		}

		// Copy Colors
		for (int color = 0; color< gMaxNumColorAttributes; ++color)
		{
			const MeshData::Attribute* color_attribute = mesh_data->FindAttribute(stringFormat("%s%d", gColorVertexAttr.c_str(), color));
			if (color_attribute == nullptr)
				break;

			mesh->copyColorData(4, mesh_data->mNumVertices, color_attribute->mData.data());
		}

		// Copy indices
		if (!initResult.check(!mesh_data->mIndices.empty(), "No index data found in mesh"))
			return nullptr;

		mesh->copyIndexData(mesh_data->mIndices.size(), mesh_data->mIndices.data());

		return mesh;
	}

} // opengl