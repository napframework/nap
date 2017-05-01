#pragma once

// Nap
#include "coreoperators.h"
#include "coretypeconverters.h"
#include "link.h"
#include "module.h"
#include "patchcomponent.h"

// External
#include <rtti/rtti.h>

/**
 * Defines the core module
 *
 * Handles registration of important components, data types etc.
 */
class ModuleNapCore : public nap::Module
{
	RTTI_ENABLE(nap::Module)
public:
	ModuleNapCore();
};
