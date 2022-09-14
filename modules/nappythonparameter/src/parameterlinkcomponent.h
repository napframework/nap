
#pragma once

#include <component.h>
#include <nap/resourceptr.h>
#include <parameter.h>
#include <parametergroup.h>

namespace nap
{

	// Forward declarations
	class ParameterLinkComponentInstance;


	/**
	 * Wraps a parameter into an object that is exposed to python.
	 * @tparam T The type of the parameter. (ie ParameterFloat)
	 */
	template <typename T>
	class PythonParameterWrapper : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)

	public:
		using ValueType = decltype(T::mValue);
		static rtti::TypeInfo getParameterType() { return RTTI_OF(T); }

		/**
		 * Contructor
		 * @param parameter the parameter that will be wrapped
		 */
		PythonParameterWrapper(Parameter* parameter) : rtti::Object()
		{
			assert(rtti::isTypeMatch(parameter->get_type(), RTTI_OF(T), rtti::ETypeCheck::EXACT_MATCH));
			mParameter = rtti_cast<T>(parameter);
		}

		void setValue(const ValueType& value) { mParameter->setValue(value); }
		ValueType getValue() const  { return mParameter->mValue; }
		void connectToValueChanged(pybind11::function func) { mParameter->valueChanged.connect(func); }

	private:
		T* mParameter = nullptr;
	};


	/**
	 * Component that refers to a parameter group and makes the parameters within the group accessible for sibling (python scripted) components
	 */
	class ParameterLinkComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ParameterLinkComponent, ParameterLinkComponentInstance)

	public:
		ParameterLinkComponent() = default;

		ResourcePtr<ParameterGroup> mParameterGroup =
			nullptr; ///< property: 'ParameterGroup' Pointer to group with parameters that will be accessed from within this entity.


		template <typename WrapperType>
		struct ParameterWrapperRegistrationClass
		{
			ParameterWrapperRegistrationClass()
			{
				sParameterWrapperFactory[WrapperType::getParameterType()] = [](Parameter* parameter){
					return std::move(std::make_unique<WrapperType>(parameter));
				};
			}
		};

		using ParameterWrapperCreationFunction = std::function<std::unique_ptr<rtti::Object>(Parameter*)>;
		static std::map<rtti::TypeInfo, ParameterWrapperCreationFunction> sParameterWrapperFactory;
	};


	/**
	 * Instance of ParameterLinkComponent
	 */
	class ParameterLinkComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		ParameterLinkComponentInstance(EntityInstance& entityInstance, Component& component) : ComponentInstance(entityInstance, component) { }

		// Inhertited from ComponentInstance
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Tries to find a parameter by ID within the linked ParameterGroup.
		 * @tparam T the type of the parameter that we want to find
		 * @param mID the mID of the parameter we want to find
		 * @return if found a pointer to the parameter, otherwise nullptr
		 */
		template <typename T>
		T* findParameter(const std::string& mID)
		{
			rtti_cast<T>(mParameterGroup->findParameter(mID));
		}

		/**
		 * Tries to find a parameter with the given mID within the linked ParameterGroup, and returns a PythonParameterWrapper for this parameter.
		 * @param mID mID of the parameter to find
		 * @return if found, a python wrapper for the parameter, otherwise nullptr.
		 */
		rtti::Object* getParameterWrapper(const std::string& mID);

	private:
		ResourcePtr<ParameterGroup> mParameterGroup = nullptr;
		std::map<Parameter*, std::unique_ptr<rtti::Object>> mPythonParameterWrappers;
	};


// Use this macro to define a wrapper for parameters with type Type
#define DEFINE_PYTHON_PARAMETER_WRAPPER(Type) \
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type)\
		RTTI_FUNCTION("setValue", &Type::setValue) \
		RTTI_FUNCTION("getValue", &Type::getValue) \
		RTTI_FUNCTION("connectToValueChanged", &Type::connectToValueChanged) \
		nap::ParameterLinkComponent::ParameterWrapperRegistrationClass<Type> registerObject; \
	RTTI_END_CLASS

}