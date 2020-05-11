#pragma once

// internal includes
#include "sequence.h"
#include "sequencecontroller.h"
#include "sequencecurveenums.h"
#include "sequenceplayer.h"
#include "sequenceutils.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>
#include <nap/logger.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceController;

	/**
	 * SequenceEditor
	 * The SequenceEditor is responsible for editing the sequence (model) and makes sure the model stays valid during editing.
	 * It also holds a resource ptr to a player, to make sure that editing the sequence stays thread safe.
	 */
	class NAPAPI SequenceEditor : 
		public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * initializes editor
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		*/
		virtual bool init(utility::ErrorState& errorState);

		/**
		 *
		 * @param file
		 */
		void save(const std::string& file);

		/**
		 *
		 * @param file
		 */
		void load(const std::string& file);

		/**
		 * Returns pointer to base class of controller type, returns nullptr when controller type is not found
		 * @param type rttr::type information of controller type to be returned
		 * @return nullptr to controller base class, null ptr when not found
		 */
		SequenceController* getControllerWithTrackType(rttr::type type);

		/**
		 * Gets reference the controller for a type, performs static cast
		 * @tparam T type of controller
		 * @return reference to controller type
		 */
		template<typename T>
		T& getController() 
		{
			assert(mControllers.find(RTTI_OF(T)) != mControllers.end()); // type not found
			return static_cast<T&>(*mControllers[RTTI_OF(T)].get());
		}

		/**
		 * Static method that registers a certain controller type for a certain view type, this can be used by views to map controller types to view types
		 * @param viewType the viewtype
		 * @param controllerType the controller type
		 * @return true on succesfull registration
		 */
		static bool registerControllerForTrackType(rttr::type viewType, rttr::type controllerType);

		/**
		 * @return duration of sequence
		 */
		double getDuration();
	public:
		// properties
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' ResourcePtr to the sequence player
	private:
		// map of all controllers
		std::unordered_map<rttr::type, std::unique_ptr<SequenceController>> mControllers;
	};
}
