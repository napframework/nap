#pragma once

// Local Includes
#include "renderablemeshcomponent.h"

namespace nap
{
	class Material;
	class RenderService;

	/**
	 * Highly efficient plane that holds a link to generic
	 * plane mesh. Use this plane for quick drawing but
	 * note that this component shares it's vertex data with 
	 * other plane components. TODO: It's better to introduce
	 * a mesh resource and let the mesh component link to it
	 */
	class PlaneComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
	public:
		// Default constructor
		PlaneComponent() = default;
		PlaneComponent(MaterialInstance& materialInstance);
	};
}
