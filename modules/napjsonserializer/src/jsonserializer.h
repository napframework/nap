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
		RTTI_ENABLE_DERIVED_FROM(Serializer)
	public:
		void writeObject(std::ostream& ostream, Object& object, bool writePointers = false) const override;
		void writeModuleInfo(std::ostream& ostream, ModuleManager& moduleManager) const override;
		Object* readObject(std::istream& istream, Core& core, Object* parent = nullptr) const override;
	};

	class JSONResource : public Resource
	{
        RTTI_ENABLE_DERIVED_FROM(Resource)
	public:
		JSONResource(const std::string& path, const std::string& contents) : Resource(path), mContents(contents) {}

        Object* createInstance(Core& core, Object& parent) override {
            return JSONSerializer().fromString(mContents, core, &parent);
        }
    private:
		const std::string mContents;
	};



	class JSONFileFactory : public ResourceLoader
	{
        RTTI_ENABLE_DERIVED_FROM(ResourceLoader)
	public:
		JSONFileFactory() : ResourceLoader() { addFileExtension("napj"); }

		bool loadResource(const std::string& resourcePath, std::unique_ptr<Resource>& outResource) const override;

	private:
		JSONSerializer mSerializer;
	};
}

RTTI_DECLARE(nap::JSONSerializer)
RTTI_DECLARE_BASE(nap::JSONResource)
RTTI_DECLARE(nap::JSONFileFactory)
