#pragma once

// Local Includes
#include "sequence.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
	namespace timeline
	{
		//////////////////////////////////////////////////////////////////////////
		
		/**
		 * Sequence Container
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
			* @return the number of available sequences in this container
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

			std::vector<Sequence*>&	getSequences() { return mSequences; }

		public:
			std::vector<ResourcePtr<Sequence>>		mSequenceLinks;

			std::vector<Sequence*>					mSequences;
		protected:
			std::vector<std::unique_ptr<Sequence>>	mOwnedSequences;
		};
	}
}
