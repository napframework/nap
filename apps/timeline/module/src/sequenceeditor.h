#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayer.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorGUI;

	/**
	 */
	class NAPAPI SequenceEditor : public Resource
	{
		friend class SequenceEditorGUI;

		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState);

		void registerGUI(SequenceEditorGUI* sequenceEditorGUI);

		void unregisterGUI(SequenceEditorGUI* sequenceEditorGUI);
	public:
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr;
	protected:
		// SequenceEditorGUI interface
		const Sequence& getSequence();

	protected:
		std::list<SequenceEditorGUI*> mGUIs;
	};
}
