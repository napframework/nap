#pragma once

#include <nap/serializer.h>

namespace nap {
    class JSONSerializer : public Serializer {
    public:
        void writeObject(std::ostream& ostream, Object& object) const override;
    };

    class JSONDeserializer : public Deserializer {
    public:
        virtual Object*
        readObject(std::istream& istream, Core& core, Object* parent = nullptr) const override;

    };

}