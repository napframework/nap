// local includes
#include "sequenceeditor.h"

// external includes

RTTI_BEGIN_CLASS(nap::SequenceEditor)
RTTI_PROPERTY("Sequence Player", &nap::SequenceEditor::mSequencePlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	
	bool SequenceEditor::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		return true;
	}


	void SequenceEditor::registerGUI(SequenceEditorGUI* sequenceEditorGUI)
	{
		bool found = (std::find(mGUIs.begin(), mGUIs.end(), sequenceEditorGUI) != mGUIs.end());

		if (!found)
		{
			mGUIs.emplace_back(sequenceEditorGUI);
		}
	}


	void SequenceEditor::unregisterGUI(SequenceEditorGUI* sequenceEditorGUI)
	{
		bool found = (std::find(mGUIs.begin(), mGUIs.end(), sequenceEditorGUI) != mGUIs.end());

		if (found)
		{
			mGUIs.remove(sequenceEditorGUI);
		}
	}


	const Sequence& SequenceEditor::getSequence()
	{
		return mSequencePlayer->getSequence();
	}
}
