#pragma once
#include "serializer.h"
#include "tinyxml2.h"

namespace nap
{
	class XMLSerializer : public Serializer
	{
	public:
		XMLSerializer(std::ostream& os, Core& core);

		void writeObject(Object& object) override;

	private:
		tinyxml2::XMLElement* toXML(tinyxml2::XMLDocument& doc, Object& object);
	};

    
	class XMLDeserializer : public Deserializer
	{
	public:
		XMLDeserializer(std::istream& is, Core& core) : Deserializer(is, core)
		{
		}

		Object* readObject(Object* parent = nullptr) override final;

    private:
		Object* fromXML(tinyxml2::XMLElement*, Object* parent);
		Object* readAttribute(tinyxml2::XMLElement*, Object* parent);
		Object* readObject(tinyxml2::XMLElement*, Object* parent);
	};
    
}