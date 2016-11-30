#pragma once

// Local Includes
#include "serializer.h"
#include "tinyxml2.h"

namespace nap
{
	class XMLSerializer : public Serializer
	{
        RTTI_ENABLE_DERIVED_FROM(Serializer)
	public:
		void writeObject(std::ostream& ostream, Object& object, bool writePointers) const override;
        virtual void writeModuleInfo(std::ostream& ostream, ModuleManager& moduleManager) const override;
        Object* readObject(std::istream& istream, Core& core, Object* parent) const override final;
    private:
		tinyxml2::XMLElement* toXML(tinyxml2::XMLDocument& doc, Object& object) const;
        Object* fromXML(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
        Object* readCompoundAttribute(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
        Object* readAttribute(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
        Object* readObject(tinyxml2::XMLElement* xml, Core& core, Object* parent) const;
	};

}

RTTI_DECLARE(nap::XMLSerializer)
