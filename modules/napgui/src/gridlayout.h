#pragma once

#include <nap/coremodule.h>
#include "layoutcomponent.h"


namespace nap
{

	class GridLayout : public LayoutComponent
	{
		RTTI_ENABLE_DERIVED_FROM(LayoutComponent)
	public:
		GridLayout() : LayoutComponent() {}

		bool layout() override;

		Attribute<int> rowCount = {this, "RowCount", 2};
		Attribute<int> colCount = {this, "ColCount", 2};
	};
}

RTTI_DECLARE(nap::GridLayout)
