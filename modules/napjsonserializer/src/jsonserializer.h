#pragma once

#include <nap/serializer.h>

namespace nap {
    /**
     * Serializer that will write to JSON format
     */
    class JSONSerializer : public Serializer
    {
        RTTI_ENABLE_DERIVED_FROM(Serializer)
    public:
        void writeObject(std::ostream& ostream, Object& object, bool writePointers = false) const override;
        void writeModuleInfo(std::ostream& ostream, ModuleManager& moduleManager) const override;
    };

    /**
     * Deserializer that will read from JSON format
     */
    class JSONDeserializer : public Deserializer
    {
        RTTI_ENABLE_DERIVED_FROM(Deserializer)
    public:
        Object* readObject(std::istream& istream, Core& core, Object* parent = nullptr) const override;
    };

}

RTTI_DECLARE(nap::JSONSerializer)
RTTI_DECLARE(nap::JSONDeserializer)