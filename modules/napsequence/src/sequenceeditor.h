#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayer.h"
#include "sequenceutils.h"
#include "sequenceeditortypes.h"
#include "sequencecontroller.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <nap/logger.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorController;
	class SequenceController;

	/**
	 * SequenceEditor
	 * The SequenceEditor is responsible for editing the sequence (model) and makes sure the model stays valid during editing.
	 * It also holds a resource ptr to a player, to make sure that editing the sequence stays thread safe.
	 */
	class NAPAPI SequenceEditor : 
		public Resource
	{
		friend class SequenceEditorGUI;

		RTTI_ENABLE(Resource)
	public:
		/**
		 * init
		 * initializes editor
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		*/
		virtual bool init(utility::ErrorState& errorState);

		template<typename T>
		T& getController() 
		{
			assert(mControllers.find(RTTI_OF(T)) != mControllers.end()); // type not found
			return static_cast<T&>(*mControllers[RTTI_OF(T)].get());
		}
	public:
		// properties
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' ResourcePtr to the sequence player
	protected:
		std::unordered_map<rttr::type, std::unique_ptr<SequenceController>> mControllers;
	private:

		/**
		 * getController
		 * @return reference to controller
		 */
		SequenceEditorController& getController();
	};
}
