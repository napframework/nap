#pragma once

#include "sequenceplayer.h"

#include <nap/core.h>

namespace nap
{
	// forward declares
	class SequenceController;

	using SequenceControllerFactoryFunc = std::unique_ptr<SequenceController>(*)(SequencePlayer&);

	class NAPAPI SequenceController
	{
	public:
		SequenceController(SequencePlayer& player) : mPlayer(player) {};

		static std::unordered_map<rttr::type, SequenceControllerFactoryFunc>& SequenceController::getControllerFactory();

		static bool registerControllerFactory(rttr::type, SequenceControllerFactoryFunc);
	protected:
		SequencePlayer& mPlayer;
	};
}
