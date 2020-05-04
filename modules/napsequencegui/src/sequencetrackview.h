#pragma once

// nap includes
#include <nap/core.h>

namespace nap
{
	//
	class SequenceEditorGUIView;
	class SequenceTrack;
	class SequenceTrackView;

	using SequenceTrackViewFactoryFunc = std::unique_ptr<SequenceTrackView>(*)(SequenceEditorGUIView&);

	class SequenceTrackView
	{
	public:
		SequenceTrackView(SequenceEditorGUIView& view);

		virtual void drawTrack(SequenceTrack& track) = 0;

		virtual void handleActions() = 0;

		static std::unordered_map<rttr::type, SequenceTrackViewFactoryFunc>& getFactoryMap();

		static bool registerFactory(rttr::type, SequenceTrackViewFactoryFunc);
	protected:
		SequenceEditorGUIView& mView;
	};
}