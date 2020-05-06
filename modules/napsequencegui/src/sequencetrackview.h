#pragma once

// Local Includes
#include "sequenceeditorgui.h"
#include "sequenceeditorguiactions.h"
#include "sequenceeditorguistate.h"

// nap includes
#include <nap/core.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceTrack;
	class SequenceTrackView;
	class SequenceEditorGUIView;

	// shortcut to factory function
	using SequenceTrackViewFactoryFunc = std::unique_ptr<SequenceTrackView>(*)(SequenceEditorGUIView&);

	/**
	 * Base class of track views
	 * Responsible for drawing a track of a specific type
	 * Needs to be extended for each track type. F.E. SequenceCurveTrackView is responsible for drawing curve tracks
	 */
	class SequenceTrackView
	{
	public:
		/**
		 * Constructor
		 * @param view reference to view
		 */
		SequenceTrackView(SequenceEditorGUIView& view);

		/**
		 * draws track 
		 * given track derived class is expected to be of track type that is used by this view
		 * @param track reference to sequence track
		 */
		virtual void drawTrack(const SequenceTrack& track, SequenceEditorGUIState& state) = 0;

		/**
		 * handles popups
		 * popups must be handled after all tracks are drawn
		 */
		virtual void handlePopups(SequenceEditorGUIState& state) = 0;

		/////////////////////////////////////////////////////////////////////////////
		// static factory methods
		////////////////////////////////////////////////////////////////////////////

		/**
		 * @return reference to static map holding all factory functions for registered track types
		 */
		static std::unordered_map<rttr::type, SequenceTrackViewFactoryFunc>& getFactoryMap();

		/**
		 * register a factory function
		 * @param type the type
		 * @param func the factory function
		 */
		static bool registerFactory(rttr::type type, SequenceTrackViewFactoryFunc func);
	public:
		/////////////////////////////////////////////////////////////////////////////
		// static utility methods
		////////////////////////////////////////////////////////////////////////////

		/**
		 * Combo
		 * Combobox that takes std::vector as input
		 * @param label label of box
		 * @param currIndex current index of combo box
		 * @param values vector of string values
		 * @return true if something is selected
		 */
		static bool Combo(const char* label, int* currIndex, std::vector<std::string>& values);

		/**
		 * ListBox
		 * ListBox that takes std::vector as input
		 * @param label label of box
		 * @param currIndex current index of combo box
		 * @param values vector of string values
		 * @return true if something is selected
		 */
		static bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values);


		/**
		 * formatTimeString
		 * formats time ( seconds ) to human readable time
		 * @param time time
		 * @return string with readable time
		 */
		static std::string formatTimeString(double time);
	protected:
		// reference to gui view
		SequenceEditorGUIView& mView;

		const SequencePlayer& getPlayer();

		SequenceEditor& getEditor();

		SequenceEditorGUIState* mState = nullptr;
	};
}