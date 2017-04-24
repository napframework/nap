#pragma once

// External Includes
#include <nap.h>
#include "renderwindowcomponent.h"

namespace nap
{
	/**
	 * THIS IS A TEST OPERATOR
	 */
	class ExecuteDrawOperator : public Operator
	{
		RTTI_ENABLE_DERIVED_FROM(Operator)
	public:
		ExecuteDrawOperator();

		OutputTriggerPlug drawOutputPlug  = { this, "draw" };

	private:
		void init();
	};
}

RTTI_DECLARE(nap::ExecuteDrawOperator)
