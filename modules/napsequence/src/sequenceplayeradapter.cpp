/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayeradapter.h"
#include "sequenceplayer.h"

namespace nap
{
	std::unordered_map <rttr::type, SequencePlayerAdapterFactoryFunc>& SequencePlayerAdapter::getFactoryMap()
	{
		static std::unordered_map <rttr::type, SequencePlayerAdapterFactoryFunc> map;
		return map;
	}


	bool SequencePlayerAdapter::registerFactory(const rttr::type& type, SequencePlayerAdapterFactoryFunc factory)
	{
		auto& map = getFactoryMap();
		if (!map.empty())
		{
			auto it = map.find(type);
			assert(it == map.end()); // duplicate entry
			if (it == map.end())
			{
				map.emplace(type, factory);
				return true;
			}
		}
		else
		{
			map.emplace(type, factory);
			return true;
		}

		return false;
	}


	std::unique_ptr<SequencePlayerAdapter> SequencePlayerAdapter::invokeFactory(const rttr::type& type, const SequenceTrack& track, SequencePlayerOutput& output, const SequencePlayer& player)
	{
		auto& map = getFactoryMap();

		auto it = map.find(type);
		assert(it != map.end()); // factory method not present
		if (it != map.end())
		{
			return it->second(track, output, player);
		}

		return nullptr;
	}
}
