#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resource.h>
#include "nap/resourceptr.h"
#include "apivalue.h"
#include "nap/service.h"
#include "nap/signalslot.h"
#include "mathutils.h"

namespace nap
{
	/**
	* 
	*/
	class NAPAPI Parameter : public Resource
	{
		RTTI_ENABLE(Resource)
	
	public:
		virtual void setValue(const Parameter& value) = 0;

	public:
	};

	class NAPAPI ParameterContainer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		ResourcePtr<Parameter> findParameter(const std::string& name) const;
		ResourcePtr<ParameterContainer> findChild(const std::string& name) const;

	public:
		std::vector<ResourcePtr<Parameter>>				mParameters;
		std::vector<ResourcePtr<ParameterContainer>>	mChildren;
	};

	/**
	* 
	*/
	template<typename T>
	class NumericParameter : public Parameter
	{
		RTTI_ENABLE(Parameter)
	public:
		virtual void setValue(const Parameter& value) override
		{
			const NumericParameter<T>* derived_type = rtti_cast<const NumericParameter<T>>(&value);
			assert(derived_type != nullptr);

			mMinimum = derived_type->mMinimum;
			mMaximum = derived_type->mMaximum;

			setValue(derived_type->mValue);
		}

		void setValue(T value)
		{
			T oldValue = mValue;
			mValue = math::clamp(value, mMinimum, mMaximum);
			if (oldValue != mValue)
			{
				valueChanged(mValue);
			}
		}

	public:
		T	mValue;											///< managed value
		T	mMinimum = std::numeric_limits<T>::min();
		T	mMaximum = std::numeric_limits<T>::max();

		Signal<T> valueChanged;
	};

	class NAPAPI ParameterService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using PresetFileList = std::vector<std::string>;
		ParameterService(ServiceConfiguration* configuration);

		PresetFileList getPresets() const;
		bool loadPreset(const std::string& presetFile, utility::ErrorState& errorState);
		bool savePreset(const std::string& presetFile, utility::ErrorState& errorState);
	
		bool hasParameters() const { return mRootContainer != nullptr; }
		ParameterContainer& getParameters() { assert(hasParameters()); return *mRootContainer; }

	protected:		
		virtual void resourcesLoaded() override;

	private:
		void setParametersRecursive(const ParameterContainer& sourceParameters, ParameterContainer& destinationParameters);

	private:
		ResourcePtr<ParameterContainer> mRootContainer;
	};


	//////////////////////////////////////////////////////////////////////////
	// API Type Definitions
	//////////////////////////////////////////////////////////////////////////

	using ParameterFloat = NumericParameter<float>;
	using ParameterBool = NumericParameter<bool>;
	using ParameterInt = NumericParameter<int>;
	using ParameterChar = NumericParameter<char>;
	using ParameterByte = NumericParameter<uint8_t>;
	using ParameterDouble = NumericParameter<double>;
	using ParameterLong = NumericParameter<int64_t>;
}
