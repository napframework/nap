#pragma once

// External Includes
#include <nap/resource.h>

#include "sequenceelement.h"

namespace nap
{
	namespace timeline
	{
		//////////////////////////////////////////////////////////////////////////

		/**
		* Sequence
		 * A sequence contains Sequence Elements
		 * When playing a sequence it looks up the element that needs to do something in the given time
		*/
		class NAPAPI Sequence : public Resource
		{
			RTTI_ENABLE(Resource)
		public:
			virtual ~Sequence();

			/**
			 * Initialize this object after de-serialization
			 * @param errorState contains the error message when initialization fails
			 */
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * Sets the parameter according to the values they are assigned to in this sequence time slot
			 * @param time the elapsed time
			 * @param endValues a reference to the parameters that need to be set
			 * @return returns true if this element has to do something ( sequence falls in this time slot )
			 */
			virtual int process(double time, std::vector<Parameter*>& outParameters);

			/**
			 * Insert sequence element, sequence will own this element
			 * @param element sequence element unique_ptr 
			 */
			void insertElement(std::unique_ptr<SequenceElement> element);

			/**
			 * Remove element, will remove stored unique pointer of element as well
			 * @param element , raw pointer to element
			 */
			void removeElement(const SequenceElement* element);

			/**
			 * Gets element at given time, return nullptr if no element available at given time
			 * @param time the time of element
			 */
			SequenceElement* getElementAtTime(const double time)  const;

			/**
			 * Sets the start time of this sequence
			 * @param startTime, the new start time
			 */
			void setStartTime(double startTime);

			/**
			 * Erases elements from the sequence, also removes owning unique pointers
			 * @param start start index
			 * @param end end index
			 */
			void eraseElements(const int start, const int end);

			/**
			* @return returns the total duration of this sequence
			*/
			const double getDuration() const { return mDuration; }

			/**
			* @return returns the start time of this sequence
			*/
			const double getStartTime() const { return mStartTime; } 

			/**
			* @return returns the id of this resource
			*/
			const std::string getID() const { return mID; }
		
			/*
			 * @return returns const reference to sequence element vector
			 */
			const std::vector<SequenceElement*>& getElements() const { return mElements; }

		public:
			// properties
			std::vector<ResourcePtr<SequenceElement>> mSequenceElementsResourcePtrs;
			std::vector<ResourcePtr<Parameter>> mStartParametersResourcePtrs;

			std::string mName;

			//
			std::vector<Parameter*> mStartParameters;
			std::vector<Parameter*> mStartParametersReference;

			int mIndexInSequenceContainer = 0;

			bool mUseReference = false;
		protected:
			double mDuration = 0.0;
			double mStartTime = 0.0;
			int mCurrentElementIndex = 0;

			std::vector<SequenceElement*> mElements;
			std::vector<std::unique_ptr<SequenceElement>> mOwnedElements;
		};
	}
}
