#pragma once

// Local Includes
#include "layoutcomponent.h"

// External Includes
#include <nap/coreattributes.h>

namespace nap
{

	class GridLayout : public LayoutComponent
	{
		RTTI_ENABLE(LayoutComponent)
	public:
		GridLayout() : LayoutComponent() {}

		bool layout() override;

		Attribute<int> rowCount = {this, "RowCount", 2};
		Attribute<int> colCount = {this, "ColCount", 2};
	};
}
