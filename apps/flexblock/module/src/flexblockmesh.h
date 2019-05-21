#pragma once


//
#include <mesh.h>
#include <rect.h>
#include <boxmesh.h>
#include <component.h>

namespace nap
{
	class NAPAPI FlexBlockMesh : public Component
	{
		RTTI_ENABLE(Component)

		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(BoxMesh));
		}
	public:
		virtual ~FlexBlockMesh();
	public:
	};
}
