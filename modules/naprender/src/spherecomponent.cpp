#include "spherecomponent.h"
#include "math.h"
#include "modelresource.h"
#include "meshresource.h"


/**
 * Creates an opengl sphere with it's associated CPU data
 * This call will initialize the sphere as well so an active gl context 
 * needs to be available
 */
static opengl::Mesh* createSphere(float radius, unsigned int rings, unsigned int sectors)
{
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texcoords;
	std::vector<float> colors;
	std::vector<unsigned int> indices;

	float const R = 1. / (float)(rings - 1);
	float const S = 1. / (float)(sectors - 1);
	int r, s;

	// Get total amount of vertices
	unsigned int vertex_count = rings * sectors;

	vertices.resize(vertex_count * 3);
	normals.resize(vertex_count * 3);
	texcoords.resize(vertex_count * 3);
	colors.resize(vertex_count * 4);

	std::vector<float>::iterator v = vertices.begin();
	std::vector<float>::iterator n = normals.begin();
	std::vector<float>::iterator t = texcoords.begin();
	std::vector<float>::iterator c = colors.begin();

	for (r = 0; r < rings; r++)
	{
		for (s = 0; s < sectors; s++)
		{
			float const y = sin(-(M_PI / 2.0) + M_PI * r * R);
			float const x = cos(2 * M_PI * s * S) * sin(M_PI * r * R);
			float const z = sin(2 * M_PI * s * S) * sin(M_PI * r * R);

			// Set texture coordinates
			*t++ = 1.0f-(s*S);
			*t++ = r*R;
			*t++ = 0.5f;

			// Set vertex coordinates
			*v++ = x * radius;
			*v++ = y * radius;
			*v++ = z * radius;

			// Set normal coordinates
			*n++ = x;
			*n++ = y;
			*n++ = z;

			// Set color coordinates
			*c++ = 1.0f;
			*c++ = 1.0f;
			*c++ = 1.0f;
			*c++ = 1.0f;
		}
	}

	// Calculate sphere indices
	int irings = rings - 1;
	int isectors = sectors - 1;
	unsigned int index_count = irings * isectors * 6;
	indices.resize(index_count);
	std::vector<unsigned int>::iterator i = indices.begin();
	for (r = 0; r < irings; r++)
	{
		for (s = 0; s < isectors; s++)
		{
			// Triangle A
			*i++ = (r * sectors) + s;
			*i++ = (r * sectors) + (s + 1);
			*i++ = ((r + 1) * sectors) + (s + 1);

			// Triangle B
			*i++ = (r * sectors) + s;
			*i++ = ((r + 1) * sectors) + (s + 1);
			*i++ = ((r + 1) * sectors) + s;
		}
	}

	opengl::Mesh* sphere_mesh = new opengl::Mesh(vertex_count, opengl::EDrawMode::TRIANGLES);
	sphere_mesh->addVertexAttribute(opengl::VertexAttributeIDs::PositionVertexAttr, 3, &vertices.front());
	sphere_mesh->addVertexAttribute(opengl::VertexAttributeIDs::NormalVertexAttr, 3, &normals.front());
	sphere_mesh->addVertexAttribute(nap::stringFormat("%s%d", opengl::VertexAttributeIDs::UVVertexAttr.c_str(), 0), 3, &texcoords.front());
	sphere_mesh->addVertexAttribute(nap::stringFormat("%s%d", opengl::VertexAttributeIDs::ColorVertexAttr.c_str(), 0), 4, &colors.front());
	sphere_mesh->setIndices(index_count, &indices.front());
	
	return sphere_mesh;
}

namespace nap
{
	SphereComponent::SphereComponent(Material& material)
	{
		ErrorState error_state;
		CustomMeshResource* mesh_resource = new CustomMeshResource();
		mesh_resource->mCustomMesh.reset(createSphere(1.0f, 50, 50));
		
		bool success = mesh_resource->init(error_state);
		assert(success);

		mModelResource = new ModelResource();
		mModelResource->mMaterialResource = &material;
		mModelResource->mMeshResource = mesh_resource;
		
		success = mModelResource->init(error_state);
		assert(success);
	}
}

RTTI_DEFINE(nap::SphereComponent)