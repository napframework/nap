#pragma once

#include <nap/resourcemanager.h>
#include <nap/serializer.h>

namespace nap
{
	/**
	 * Serializer that will read from and write to JSON format
	 */
	class JSONSerializer : public Serializer
	{
		RTTI_ENABLE(Serializer)
	public:
		void writeObject(std::ostream& ostream, Object& object, bool writePointers = false) const override;
		void writeModuleInfo(std::ostream& ostream, ModuleManager& moduleManager) const override;
		Object* readObject(std::istream& istream, Core& core, Object* parent = nullptr) const override;
	};

	class JSONResource : public rtti::RTTIObject
	{
        RTTI_ENABLE(rtti::RTTIObject)
	public:
		JSONResource(const std::string& path, const std::string& contents) : mContents(contents), mPath(path) {}

        Object* createInstance(Core& core, Object& parent)
		{
            return JSONSerializer().fromString(mContents, core, &parent);
        }

    private:
		const std::string mContents;
		std::string mPath;
	};

}
