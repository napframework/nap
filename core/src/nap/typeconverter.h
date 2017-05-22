#pragma once

// Local Includes
#include "attribute.h"
#include "logger.h"

// External Includes
#include <rtti/rtti.h>

namespace nap
{
	/**
	Generic type converter for dynamic conversion of values residing inside an opaque container.
	**/
	class TypeConverterBase
	{
	public:
		TypeConverterBase(rtti::TypeInfo inType, rtti::TypeInfo outType);
        virtual ~TypeConverterBase() = default;

		// The type to convert from
		const rtti::TypeInfo inType() const { return mInType; }

		// The type to convert into
		const rtti::TypeInfo outType() const { return mOutType; }

		// Here we can only convert values in an opaque container,
		// derived types will expose specialized behavior
		virtual bool convert(const AttributeBase* inAttrib, AttributeBase* outAttrib) const = 0;

	protected:
		const rtti::TypeInfo mInType;
		const rtti::TypeInfo mOutType;
	};

    
	/**
	This class should not be derived from.
	Only instantiate it (statically) and provide the conversion function.
	For example:

		TypeConverter<float, int> intToString([](const float& value, int& outValue) {
			outValue = round(value);
		});

	**/
	template <typename I, typename O>
	class TypeConverter : public TypeConverterBase
	{
	public:
        typedef bool(*ConvertFunction)(const I&, O&);

		TypeConverter(ConvertFunction func)
			: TypeConverterBase(rtti::TypeInfo::get<I>(), rtti::TypeInfo::get<O>()), convertFunction(func)
		{
		}

		ConvertFunction convertFunction;

		bool convert(const AttributeBase* inAttrib, AttributeBase* outAttrib) const override
		{
            if (!inAttrib->get_type().is_derived_from<Attribute<I>>()) {
                Logger::fatal("Input attribute of type '%s' should be of type '%s'",
                              inAttrib->getValueType().get_name().data(),
                              RTTI_OF(I).get_name().data());
                return false;
            }
            auto inAt = static_cast<const Attribute<I>*>(inAttrib);

            if (!outAttrib->get_type().is_derived_from<Attribute<O>>()) {
                Logger::fatal("Output attribute of type '%s' should be of type '%s'",
                              outAttrib->getValueType().get_name().data(),
                              RTTI_OF(O).get_name().data());
                return false;
            }
			auto outAt = static_cast<Attribute<O>*>(outAttrib);

			bool success = convertFunction(inAt->getValue(), outAt->getValueRef());

			outAt->valueChanged(*outAt);

			return success;
		}
	};

    
	class TypeConverterPassThrough : public TypeConverterBase {
	public:
		TypeConverterPassThrough() : TypeConverterBase(rtti::TypeInfo::empty(), rtti::TypeInfo::empty()) {}

		bool convert(const AttributeBase *inAttrib, AttributeBase *outAttrib) const override {
			outAttrib->setValue(*inAttrib);
			return true;
		}
	};
    
}