#include "napofshaderbinding.h"
#include <napofmaterial.h>
#include <ofMain.h>
#include <ofShader.h>

namespace nap {

	void bindTextureShader(AttributeBase& inAttr, ofShader& shader, int& texLocation) {
		auto attr = static_cast<Attribute<ofTexture*>*>(&inAttr);
		if (!attr->getValue() || !attr->getValue()->isAllocated()) {
			Logger::fatal("Failed to bind texture on %s", ObjectPath(inAttr).toString().c_str());
			return;
		}
		shader.setUniformTexture(attr->getName(), *attr->getValue(), texLocation++);
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
		OFMaterial::registerBindFunction(RTTI_OF(ofTexture*), bindTextureShader);
		OFMaterial::registerBindFunction(RTTI_OF(float), bindFloatShader);
		OFMaterial::registerBindFunction(RTTI_OF(int), bindIntShader);
		OFMaterial::registerBindFunction(RTTI_OF(ofVec2f), bindOfVec2fShader);
		OFMaterial::registerBindFunction(RTTI_OF(ofVec3f), bindOfVec3fShader);
	}


}
