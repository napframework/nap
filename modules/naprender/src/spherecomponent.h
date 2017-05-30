#pragma once

#include "renderablemeshcomponent.h"

namespace nap
{
	class Material;
	class RenderService;

	/**
	 * Highly efficient sphere that holds a link to a generic
	 * sphere mesh. Use this sphere for quick drawing but note
	 * that this component shares it's vertex data with other plane
	 * components.
	 */
	class SphereComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
	public:
		// Default constructor
		SphereComponent() = default;
		SphereComponent(MaterialInstance& materialInstance, RenderService& renderService);
	};
}
