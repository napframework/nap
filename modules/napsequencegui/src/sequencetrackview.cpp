#include "sequencetrackview.h"
#include "sequenceeditorgui.h"

namespace nap
{
	std::unordered_map<rttr::type, SequenceTrackViewFactoryFunc>& SequenceTrackView::getFactoryMap()
	{
		static std::unordered_map<rttr::type, SequenceTrackViewFactoryFunc> map;
		return map;
	}

	bool SequenceTrackView::registerFactory(rttr::type type, SequenceTrackViewFactoryFunc func)
	{
		auto& map = getFactoryMap();
		auto it = map.find(type);
		assert(it == map.end()); // duplicate entry
		if (it == map.end())
		{
			map.emplace(type, func);

			return false;
		}

		return false;
	}

	SequenceTrackView::SequenceTrackView(SequenceEditorGUIView& view) :
		mView(view)
	{

	}
}