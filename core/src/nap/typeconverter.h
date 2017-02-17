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
		TypeConverterBase(RTTI::TypeInfo inType, RTTI::TypeInfo outType);
        virtual ~TypeConverterBase() = default;

		// The type to convert from
		const RTTI::TypeInfo inType() const { return mInType; }

		// The type to convert into
		const RTTI::TypeInfo outType() const { return mOutType; }

		// Here we can only convert values in an opaque container,
		// derived types will expose specialized behavior
		virtual bool convert(const AttributeBase* inAttrib, AttributeBase* outAttrib) const = 0;

	protected:
		const RTTI::TypeInfo mInType;
		const RTTI::TypeInfo mOutType;
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
			: TypeConverterBase(RTTI::TypeInfo::get<I>(), RTTI::TypeInfo::get<O>()), convertFunction(func)
		{
		}

		ConvertFunction convertFunction;

		bool convert(const AttributeBase* inAttrib, AttributeBase* outAttrib) const override
		{
            if (!inAttrib->getTypeInfo().isKindOf<Attribute<I>>()) {
                Logger::fatal("Input attribute of type '%s' should be of type '%s'",
                              inAttrib->getValueType().getName().c_str(),
                              RTTI_OF(I).getName().c_str());
                return false;
            }
            auto inAt = static_cast<const Attribute<I>*>(inAttrib);

            if (!outAttrib->getTypeInfo().isKindOf<Attribute<O>>()) {
                Logger::fatal("Output attribute of type '%s' should be of type '%s'",
                              outAttrib->getValueType().getName().c_str(),
                              RTTI_OF(O).getName().c_str());
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
		TypeConverterPassThrough() : TypeConverterBase(RTTI::TypeInfo::empty(), RTTI::TypeInfo::empty()) {}

		bool convert(const AttributeBase *inAttrib, AttributeBase *outAttrib) const override {
			outAttrib->setValue(*inAttrib);
			return true;
		}
	};
    
}