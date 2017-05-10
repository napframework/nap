#pragma once

#include "meshcomponent.h"

namespace nap
{
	class Material;

	/**
	 * Highly efficient sphere that holds a link to a generic
	 * sphere mesh. Use this sphere for quick drawing but note
	 * that this component shares it's vertex data with other plane
	 * components.
	 */
	class SphereComponent : public MeshComponent
	{
		RTTI_ENABLE(MeshComponent)
	public:
		// Default constructor
		SphereComponent() = default;
		SphereComponent(Material& material);
	};
}
