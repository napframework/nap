#pragma once

#include <nap/serializer.h>

namespace nap {
    /**
     * Serializer that will write to JSON format
     */
    class JSONSerializer : public Serializer {
    public:
        void writeObject(std::ostream& ostream, Object& object, bool writePointers = false) const override;
        virtual void writeModuleInfo(std::ostream& ostream, ModuleManager& moduleManager) const override;
    };

    /**
     * Deserializer that will read from JSON format
     */
    class JSONDeserializer : public Deserializer {
    public:
        virtual Object*
        readObject(std::istream& istream, Core& core, Object* parent = nullptr) const override;

    };

}