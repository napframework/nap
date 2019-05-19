#pragma once

#include "mesh.h"
#include <rect.h>

namespace nap
{
	class NAPAPI FlexBlockMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		virtual ~FlexBlockMesh();
		/**
		* Sets up and initializes the box as a mesh based on the provided parameters.
		* @param errorState contains the error message if the mesh could not be created.
		* @return if the mesh was successfully created and initialized.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Creates and prepares the mesh but doesn't initialize it.
		* Call this when you want to prepare a box without creating the GPU representatino
		* You have to manually call init() on the mesh instance afterwards.
		*/
		void setup();

		float lerp(float a, float b, float t);

		glm::vec3 lineLineIntersection(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 D);

		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }
	private:
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
