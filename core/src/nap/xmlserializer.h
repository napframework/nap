#pragma once

// Local Includes
#include "serializer.h"
#include "tinyxml2.h"

namespace nap
{
	class XMLSerializer : public Serializer
	{
	public:
		void writeObject(std::ostream& ostream, Object& object) const override;

	private:
		tinyxml2::XMLElement* toXML(tinyxml2::XMLDocument& doc, Object& object) const;
	};

    
	class XMLDeserializer : public Deserializer
	{
	public:
        Object* readObject(std::istream& istream, Core& core, Object* parent) const override final;

    private:
		Object* fromXML(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
        Object* readCompoundAttribute(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
		Object* readAttribute(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
		Object* readObject(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
	};
    
}