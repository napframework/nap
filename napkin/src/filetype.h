#pragma once

// External Includes
#include <memory>
#include <nap/serializer.h>
#include <nap/xmlserializer.h>
#include <string>
#include <vector>

/**
 * FileType
 *
 * Abstract file type for serialization / deserialization
 */
class FileType
{
public:
	virtual std::string name()      const = 0;
	virtual std::string extension() const = 0;

	virtual void serialize(std::ostream& stream, nap::Core& core, nap::Object& obj) const = 0;
	virtual void deserialize(std::istream& stream, nap::Core& core) const = 0;

private:
	static std::vector<std::unique_ptr<FileType>> mFileTypes;
};


/**
 * XMLFileType
 *
 * XML file type used when serializing / deserializing
 */
class XMLFileType : public FileType
{
public:
	std::string name()      const override { return "NAP XML"; }
	std::string extension() const override { return "napx"; }


    virtual void serialize(std::ostream& stream, nap::Core& core,
                           nap::Object& obj) const override
    {
        nap::XMLSerializer ser(stream, core);
        ser.writeObject(core.getRoot());
    }


    virtual void deserialize(std::istream& stream, nap::Core& core) const override
    {
        nap::XMLDeserializer ser(stream, core);
        ser.readObject();
    }
};


/**
 * Returns all registered file types
 * @return vector of unique file type pointers
 */
static std::vector<std::unique_ptr<FileType>>& getFileTypes()
{
	static std::vector<std::unique_ptr<FileType>> types;
	if (types.empty())
    {
		types.push_back(std::make_unique<XMLFileType>());
	}
	return types;
}