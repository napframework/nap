#pragma once

// Core Includes
#include <nap/component.h>
#include <nap/attribute.h>
#include <nap/coremodule.h>

// Std Includes
#include <iostream>

namespace nap
{
	/**
	@brief For simple testing
	**/
	class TransformComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM_2(AttributeObject, Component)
	public:
        TransformComponent(float inX, float inY, float inZ);
		TransformComponent() = default;

		///@name Some test attributes
		Attribute<float>	mX = { this, "PosX", 0.0f };
		Attribute<float>	mY = { this, "PosY", 0.0f };
		Attribute<float>	mZ = { this, "PosZ", 0.0f };
	};



	/**
	@brief Simple receiving object (SLOT)
	**/
	class Receiver
	{
	public:
        Receiver() = default;

        // cretae a slot for PrintReceivingValue function
        nap::Slot<nap::AttributeBase&> mAttrSlot = { this, &Receiver::PrintReceivingValue };
		
	private:
		//@name Functionality
		void PrintReceivingValue(AttributeBase& inAttribute)
		{
			std::string v;
			inAttribute.toString(v);
			std::cout << "Value changed to: " << v.c_str() << " of Attribute: " <<inAttribute.getName().c_str() << "\n";
		}

		//@name Value changed slot
		void PrintValueChanged(float& inValue)
		{
			std::cout << "Value changed to: " << inValue << "\n";
		}

	};
}

RTTI_DECLARE(nap::TransformComponent)
RTTI_DECLARE(nap::Receiver)