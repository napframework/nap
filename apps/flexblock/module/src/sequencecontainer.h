#pragma once

// Local Includes
#include "sequence.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
	namespace timeline
	{
		/**
		* Bundles a set of sequences
		*/
		class SequenceContainer : public Resource
		{
			RTTI_ENABLE(Resource)
		public:
			virtual ~SequenceContainer();

			/**
			* Initialize this object after de-serialization
			* @param errorState contains the error message when initialization fails
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			* @return the number of available compositions in this container
			*/
			const int count() const { return mSequences.size(); }

			/**
			 * 
			 */
			void reconstruct();

			/**
			*
			*/
			void reinit();

			/**
			 * 
			 */
			void removeSequence(const Sequence * sequence);

			/**
			*
			*/
			void insertSequence(std::unique_ptr<Sequence> sequence);

			std::vector<Sequence*>					mSequences;								
			std::vector<ResourcePtr<Sequence>>		mSequenceLinks;
			std::vector<std::unique_ptr<Sequence>>	mOwnedSequences;
		};
	}
}
