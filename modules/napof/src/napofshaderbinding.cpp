#include "napofshaderbinding.h"
#include <napofmaterial.h>
#include <ofMain.h>
#include <ofShader.h>

namespace nap {

	void bindTextureShader(AttributeBase& inAttr, ofShader& shader, int& texLocation) 
	{
		nap::ObjectLinkAttribute* attr = static_cast<nap::ObjectLinkAttribute*>(&inAttr);
		if (!attr->isLinked())
			return;

		nap::Attribute<ofTexture*>* target_texture = attr->getTarget<nap::Attribute<ofTexture*>>();
		if (attr == nullptr)
		{
			nap::Logger::warn("unable to resolve texture link: %s", attr->getPath().toString().c_str());
			return;
		}

		// Get target texture
		ofTexture* tex = target_texture->getValue();
		if (tex == nullptr) 
		{
			Logger::warn("Texture not set: %s", attr->getPath().toString());
			return;
		}

		// Make sure it's allocated
		if (!tex->isAllocated()) 
		{
			Logger::fatal("Texture not allocated: %s", attr->getPath().toString().c_str());
			return;
		}
		shader.setUniformTexture(attr->getName(), *tex, texLocation++);
	}

	void bindFloatShader(AttributeBase& inAttr, ofShader& shader, int& texLocation) {
		auto attr = static_cast<Attribute<float>*>(&inAttr);
		shader.setUniform1f(attr->getName(), attr->getValue());
	}

	void bindIntShader(AttributeBase& inAttr, ofShader& shader, int& texLocation)  {
		auto attr = static_cast<Attribute<int>*>(&inAttr);
		shader.setUniform1i(attr->getName(), attr->getValue());
	}

	void bindOfVec2fShader(AttributeBase& inAttr, ofShader& shader, int& texLocation) {
		auto attr = static_cast<Attribute<ofVec2f>*>(&inAttr);
		shader.setUniform2f(attr->getName(), attr->getValue());
	}

	void bindOfVec3fShader(AttributeBase& inAttr, ofShader& shader, int& texLocation) {
		auto attr = static_cast<Attribute<ofVec3f>*>(&inAttr);
		shader.setUniform3f(attr->getName(), attr->getValue());
	}

	void registerOfShaderBindings()
	{
		OFMaterial::registerBindFunction(RTTI_OF(float), bindFloatShader);
		OFMaterial::registerBindFunction(RTTI_OF(int), bindIntShader);
		OFMaterial::registerBindFunction(RTTI_OF(ofVec2f), bindOfVec2fShader);
		OFMaterial::registerBindFunction(RTTI_OF(ofVec3f), bindOfVec3fShader);
		OFMaterial::registerBindFunction(RTTI_OF(nap::Link), bindTextureShader);
	}


}
