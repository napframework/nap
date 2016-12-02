#pragma once

// Local Includes
#include "nmodel.h"

namespace opengl
{
	/**
	 * loadModel
	 *
	 * Creates a model from file, the model owns the mesh data
	 * Note that the model is preferably a triangular mesh
	 * This call will try to triangulate but not guaranteed to work 
	 * @param modelPath the path to the model on disk to load
	 * @return: a new Model, nullptr if unsuccessful
	 */
	Model* loadModel(const std::string& modelPath);


	/**
	 * loadModel
	 *
	 * Loads a model from file, the model owns all the mesh data
	 * If the model already contains data that data will be cleared!
	 * Note that the model is preferably a triangular mesh
	 * This call will try to triangulate but not guaranteed to work
	 * @param modelPath the path to the model on disk to load
	 * @param model the model to populate with mesh data
	 * @return if load was successful or not
	 */
	bool loadModel(Model& model, const std::string& modelPath);
}
