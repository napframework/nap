#pragma once

#include <nap/serviceablecomponent.h>
#include <napofattributes.h>
#include <nap/rttinap.h>

namespace nap
{
	class NledPanelComponent : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		/**
		 * Default constructor
		 */
		NledPanelComponent() = default;

		/**
		 * Corresponding panel identifier managed by nled service
		 */
		Attribute<int> panelID = { this, "panelID", 0 };

		/**
		 * Uvs used when sampling part of the buffer
		 */
		Attribute<ofVec4f> uvs = { this, "uvs", {0.0f,1.0f,0.0f,1.0f} };
	};
}

RTTI_DECLARE(nap::NledPanelComponent)