#pragma once

// local includes
#include "sequenceplayeroutput.h"

// nap includes
#include <nap/resourceptr.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequencePlayerCurveAdapterBase;
	class SequenceService;

	/**
	 * SequencePlayerCurveOutput is used to link a parameter to a curve track
	 */
	class NAPAPI SequencePlayerCurveOutput : public SequencePlayerOutput
	{
		RTTI_ENABLE(SequencePlayerOutput)
	public:
		SequencePlayerCurveOutput(SequenceService& service);

		// properties
		ResourcePtr<Parameter>	mParameter; 	///< Property: 'Parameter' parameter resource
		bool					mUseMainThread; ///< Property: 'Use Main Thread' update in main thread or player thread

		/**
		 * registers a parameter setter to the output. Parameter setters are called from main thread
		 * @param parameterSetter ptr to parameter setter
		 */
		void registerAdapter(SequencePlayerCurveAdapterBase* curveAdapter);

		/**
		 * removes parameter setter
		 * @param parameterSetter ptr to parameter setter
		 */
		void removeAdapter(SequencePlayerCurveAdapterBase* curveAdapter);
	protected:
		/**
		 * called from update loop sequence service main thread
		 * @param deltaTime time since last update
		 */
		virtual void update(double deltaTime) override ;

		// vector holding registered parameter setters
		std::vector<SequencePlayerCurveAdapterBase*> mAdapters;
	private:

	};

	using SequencePlayerCurveInputObjectCreator = rtti::ObjectCreator<SequencePlayerCurveOutput, SequenceService>;
}
