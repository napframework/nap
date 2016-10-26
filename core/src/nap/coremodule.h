#pragma once

#include <nap/coreoperators.h>
#include <nap/coretypeconverters.h>
#include <nap/link.h>
#include <nap/module.h>
#include <nap/patchcomponent.h>
#include <rtti/rtti.h>

/**
 * Defines the core module
 *
 * Handles registration of important components, data types etc.
 */
class ModuleNapCore : public nap::Module
{
	RTTI_ENABLE_DERIVED_FROM(nap::Module)
public:
	ModuleNapCore();
};
RTTI_DECLARE(ModuleNapCore)
