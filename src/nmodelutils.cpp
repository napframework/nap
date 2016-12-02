// Local Includes
#include "nmodelutils.h"

// External Includes
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace opengl
{
	// Creates a model from file, the model owns the mesh data
	Model* loadModel(const std::string& modelPath)
	{
		Model* new_model = new Model();
		if (!loadModel(*new_model, modelPath))
		{
			delete new_model;
			return nullptr;
		}
		return new_model;
	}


	// Loads a model from file, the model owns all the mesh data
	bool loadModel(Model& model, const std::string& modelPath)
	{
		if (!model.isEmpty())
		{
			printMessage(MessageType::WARNING, "model already contains mesh data, clearing...");
			model.clear();
		}

		// Create importer
		Assimp::Importer importer;

		// Load file
		printMessage(MessageType::INFO, "loading file: %s", modelPath.c_str());
		const aiScene* scene = importer.ReadFile(modelPath,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_SortByPType);

		// Ensure scene is loaded
		if (scene == nullptr)
		{
			opengl::printMessage(opengl::MessageType::ERROR, "unable to load scene from file: %s", modelPath.c_str());
			return false;
		}

		// Ensure there are meshes
		if (scene->mNumMeshes == 0)
		{
			opengl::printMessage(opengl::MessageType::ERROR, "file does not contain any mesh geometry: %s", modelPath.c_str());
			return false;
		}

		// Create meshes for every contained mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			opengl::printMessage(opengl::MessageType::INFO, "mesh: %d, vert count: %d", i, scene->mMeshes[i]->mNumVertices);
			opengl::printMessage(opengl::MessageType::INFO, "mesh: %d, face count: %d", i, scene->mMeshes[i]->mNumFaces);
			opengl::printMessage(opengl::MessageType::INFO, "mesh: %d has %d uv channel(s)", i, scene->mMeshes[i]->GetNumUVChannels());
			opengl::printMessage(opengl::MessageType::INFO, "mesh: %d has %d uv components", i, scene->mMeshes[i]->mNumUVComponents[0]);
			opengl::printMessage(opengl::MessageType::INFO, "mesh: %d has normals: %d", i, scene->mMeshes[i]->HasNormals());
			opengl::printMessage(opengl::MessageType::INFO, "mesh: %d colors channels: %d", i, scene->mMeshes[i]->GetNumColorChannels());
			std::cout << std::endl;

			// Get assimp mesh
			aiMesh* mesh = scene->mMeshes[i];

			// Create new mesh
			Mesh* new_mesh = new Mesh();
			new_mesh->init();


			// Copy vertex data
			std::vector<float> vertex_data;
			vertex_data.reserve(mesh->mNumVertices * 3);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
			{
				aiVector3D* current_id = &(mesh->mVertices[vertex]);
				vertex_data.emplace_back(static_cast<float>(current_id->x));
				vertex_data.emplace_back(static_cast<float>(current_id->y));
				vertex_data.emplace_back(static_cast<float>(current_id->z));
			}
			new_mesh->copyVertexData(mesh->mNumVertices, &vertex_data.front());


			// Copy normals
			if (mesh->HasNormals())
			{
				std::vector<float> normal_data;
				normal_data.reserve(mesh->mNumVertices * 3);
				for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
				{
					aiVector3D* current_id = &(mesh->mNormals[vertex]);
					normal_data.emplace_back(static_cast<float>(current_id->x));
					normal_data.emplace_back(static_cast<float>(current_id->y));
					normal_data.emplace_back(static_cast<float>(current_id->z));
				}
				new_mesh->copyNormalData(mesh->mNumVertices, &normal_data.front());
			}


			// Copy all color data channels
			for (unsigned int color_channel = 0; color_channel < mesh->GetNumColorChannels(); color_channel++)
			{
				aiColor4D* color_channel_data = mesh->mColors[color_channel];
				std::vector<float> color_data;
				color_data.reserve(mesh->mNumVertices * 4);

				// Copy color data channel
				for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
				{
					aiColor4D* current_id = &(color_channel_data[vertex]);
					color_data.emplace_back(static_cast<float>(current_id->r));
					color_data.emplace_back(static_cast<float>(current_id->g));
					color_data.emplace_back(static_cast<float>(current_id->b));
					color_data.emplace_back(static_cast<float>(current_id->a));
				}

				// Copy color data
				new_mesh->copyColorData(4, mesh->mNumVertices, &color_data.front());
			}


			// Copy all uv data channels
			for (unsigned int uv_channel = 0; uv_channel < mesh->GetNumUVChannels(); uv_channel++)
			{
				aiVector3D* uv_channel_data = mesh->mTextureCoords[uv_channel];
				std::vector<float> uv_data;
				uv_data.reserve(mesh->mNumVertices * 3);

				// Copy uv data channel
				for (unsigned int vertex = 0; vertex < mesh->mNumVertices; vertex++)
				{
					aiVector3D* current_id = &(uv_channel_data[vertex]);
					uv_data.emplace_back(static_cast<float>(current_id->x));
					uv_data.emplace_back(static_cast<float>(current_id->y));
					uv_data.emplace_back(static_cast<float>(current_id->z));
				}
				new_mesh->copyUVData(3, mesh->mNumVertices, &uv_data.front());
			}

			// Add mesh
			model.addMesh(new_mesh);
		}

		// Explicetly free scene
		importer.FreeScene();

		return true;
	}
} // opengl