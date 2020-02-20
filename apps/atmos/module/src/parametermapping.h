#pragma once

// External Includes
#include <nap/resource.h>
#include <glm/glm.hpp>
#include <nap/resourceptr.h>
#include <parameter.h>
#include <parameternumeric.h>

namespace nap
{
	/**
	 * Maps a received sensor value to a parameter int or float.
	 */
	class NAPAPI ParameterMapping : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~ParameterMapping();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * maps and sets the value based on the given input and output range
		 */
		void setValue(float value);

		bool mMap = true;								///< Property: 'Map' if the sensor input is mapped to output
		ResourcePtr<Parameter> mParameter = nullptr;	///< Property: 'Parameter' Parameter to apply input to
		glm::vec2 mInputRange	= { 0.0f, 1.0f };		///< Property: 'InputRange' the sensor input range
		glm::vec2 mOutputRange	= { 0.0f, 1.0f };		///< Property: 'OutputRange' the parameter output range

	private:
		template<typename T>
		void push(float value, nap::Parameter& param);
	};


	template<typename T>
	void nap::ParameterMapping::push(float value, nap::Parameter& param)
	{
		assert(param.get_type().is_derived_from(RTTI_OF(ParameterNumeric<T>)));
		ParameterNumeric<T>& cparam = static_cast<ParameterNumeric<T>&>(param);
		cparam.setValue((T)value);
	}

}
