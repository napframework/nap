// Local Includes
#include "particleemittercomponent.h"
#include "entity.h"
#include "transformcomponent.h"
#include "rect.h"

RTTI_BEGIN_CLASS(nap::ParticleEmitterComponent)
	RTTI_PROPERTY("Mesh",		&nap::ParticleEmitterComponent::mMesh,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParticleEmitterComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

	// All the plane uvs
	static glm::vec3 plane_uvs[] =
	{
		{ 0.0f,	0.0f,	0.0f },
		{ 1.0f,	0.0f,	0.0f },
		{ 0.0f,	1.0f,	0.0f },
		{ 1.0f,	1.0f,	0.0f },
	};

	// All the plane colors
	static glm::vec4 plane_colors[] =
	{
		{ 1.0f,	0.0f,	0.0f,	1.0f },
		{ 0.0f,	1.0f,	0.0f,	1.0f },
		{ 0.0f,	0.0f,	1.0f,	1.0f },
		{ 1.0f,	1.0f,	1.0f,	1.0f },
	};

	// Plane connectivity indices
	static unsigned int plane_indices[] =
	{
		0,1,3,
		0,3,2
	};


	ParticleEmitterComponentInstance::ParticleEmitterComponentInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	ParticleEmitterComponentInstance::~ParticleEmitterComponentInstance()
	{
	}


	bool ParticleEmitterComponentInstance::init(utility::ErrorState& errorState)
	{
		ParticleEmitterComponent* component = getComponent<ParticleEmitterComponent>();
		MeshInstance& mesh_instance = component->mMesh->getMeshInstance();
		
		int numVertices = 4;
		mesh_instance.setNumVertices(numVertices);
		mesh_instance.setDrawMode(opengl::EDrawMode::TRIANGLES);

		glm::vec2 mSize(5.0f, 5.0f);
		// Create the position vertices
		float dsizex = 0.0f - (mSize.x / 2.0f);
		float dsizey = 0.0f - (mSize.y / 2.0f);
		math::Rect rect(dsizex, dsizey, mSize.x, mSize.y);

		// All the plane vertices
		glm::vec3 plane_vertices[] =
		{
			{ rect.getMin().x,	rect.getMin().y, 0.0f },
			{ rect.getMax().x,	rect.getMin().y, 0.0f },
			{ rect.getMin().x,	rect.getMax().y, 0.0f },
			{ rect.getMax().x,	rect.getMax().y, 0.0f },
		};

		Vec3VertexAttribute& position_attribute = mesh_instance.getOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
		Vec3VertexAttribute& uv_attribute = mesh_instance.getOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0));
		Vec4VertexAttribute& color_attribute = mesh_instance.getOrCreateAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));

		position_attribute.setData(plane_vertices, numVertices);
		uv_attribute.setData(plane_uvs, numVertices);
		color_attribute.setData(plane_colors, numVertices);
		mesh_instance.setIndices(plane_indices, 6);

		if (!mesh_instance.update(errorState))
			return false;

		return true;
	}
}