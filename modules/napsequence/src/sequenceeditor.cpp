// local includes
#include "sequenceeditor.h"
#include "sequencetrack.h"
#include "sequencetracksegment.h"
#include "sequenceutils.h"

// external includes
#include <nap/logger.h>
#include <fcurve.h>
#include <functional>

RTTI_BEGIN_CLASS(nap::SequenceEditor)
RTTI_PROPERTY("Sequence Player", &nap::SequenceEditor::mSequencePlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

using namespace nap::SequenceEditorTypes;

namespace nap
{
	bool SequenceEditor::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		// create controllers for all types of tracks
		auto& factory = SequenceController::getControllerFactory();
		for (auto it : factory)
		{
			mControllers.emplace(it.first, it.second(*mSequencePlayer.get()));
		}

		return true;
	}
}
