#pragma once

#include <nap/attribute.h>
#include <nap/attributeobject.h>
#include <nap/plug.h>
#include <rtti/rtti.h>
#include <set>

namespace nap
{

	// Forward declarations
	class Core;
	class Patch;


	// The operator class is a unit within a patch that can be connected to other operators using plugs. The operator
	// has any number of input and output plugs to communicate with other operators. Because the operator is a AttributeObject it
	// can also have attributes to store data that is coming in or going out through the plugs.
	class Operator : public AttributeObject
	{

		RTTI_ENABLE_DERIVED_FROM(AttributeObject)

		friend class InputPlugBase;
		friend class OutputPlugBase;
		friend class Patch;

	public:
		// Default constructor
		Operator() = default;
        
        // Virtual destructor because of virtual methods!
        virtual ~Operator() = default;

		// Return this Operator's input plugs
        std::vector<InputPlugBase*> getInputPlugs() { return getChildrenOfType<InputPlugBase>(); }
		// Return this operator's output plugs
        std::vector<OutputPlugBase*> getOutputPlugs() { return getChildrenOfType<OutputPlugBase>(); }

		// Check wether an input plug is registered with name
        bool hasInputPlug(const std::string& name) { return hasChildOfType<InputPlugBase>(name); }
		// Check wether an output plug is registered with name
        bool hasOutputPlug(const std::string& name) { return hasChildOfType<OutputPlugBase>(name); }

		// Retrieve input plug by name, returns nullptr if the plug does not exist
        InputPlugBase* getInputPlug(const std::string& name) { return getChild<InputPlugBase>(name); }

		// Retrieve output plug by name, returns nullptr if the plug does not exist
        OutputPlugBase* getOutputPlug(const std::string& name) { return getChild<OutputPlugBase>(name); }

		// Return the patch this Operator resides in
        Patch* getPatch();

	};
}

RTTI_DECLARE_BASE(nap::Operator)
