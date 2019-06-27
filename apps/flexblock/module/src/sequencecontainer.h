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
			 * Rearranges the vector holding the sequences
			 */
			void reconstruct();

			/**
			 * removes all sequences, also removes owning sequences from memory
			 */
			void clearSequences();

			/**
			 * Removes sequences from vector holding the sequences
			 * @param sequence raw pointer to sequence that needs to be removed
			 */
			void removeSequence(const Sequence * sequence);

			/**
			 * Removes element from given sequence
			 * @param sequence, sequence holding the element
			 * @param element, element that needs to be removed from given sequence
			 */
			void removeSequenceElement(const Sequence * sequence, const SequenceElement * element);

			/**
			 * Inserts sequences to vector containing sequences
			 * @param sequence unique pointer of sequence, sequence container will own the unique_ptr
			 */
			void insertSequence(std::unique_ptr<Sequence> sequence);

			/**
			 * Returns const reference to vector holding sequences
			 * @return reference to vector holding sequences
			 */
			const std::vector<Sequence*>& getSequences() const { return mSequences; }

			/**
			* @return the number of available sequences in this container
			*/
			const int count() const { return mSequences.size(); }
		public:
			std::vector<ResourcePtr<Sequence>>		mSequenceResourcePtrs; ///< Property: 'Sequences' sequences hold by the container
		protected:
			std::vector<Sequence*>					mSequences;
			std::vector<std::unique_ptr<Sequence>>	mOwnedSequences;
		};
	}
}
