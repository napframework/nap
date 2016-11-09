#include "napofmaterial.h"
#include <nap/logger.h>
#include <cassert>

namespace nap
{

	void OFMaterial::registerBindFunction(RTTI::TypeInfo typeInfo, BindFunction bindFunction)
	{
		getBindFunctions()[typeInfo] = bindFunction;
	}


	/**
	@brief Constructor
	**/
	OFMaterial::OFMaterial()
	{
		mShader.connectToValue(mShaderChanged);
		childAdded.connect(this, &OFMaterial::onChildAdded);
		childRemoved.connect(this, &OFMaterial::onChildRemoved);
	}




	AttributeBase* OFMaterial::getBinding(const std::string& name)
	{
		for (auto& binding : mBindings)
			if (binding->getName() == name)
				return binding;
		return nullptr;
	}


	/**
	@brief Loads in the new shader
	**/
	void OFMaterial::shaderChanged(const std::string& inString)
	{
		if (mOFShader.isLoaded())
		{
			mOFShader.unload();
		}

		if (!mOFShader.load(inString))
			Logger::warn("Unable to load shader" + mShader.getValue());
	}


	/**
	@brief Binds the shader and all of its values
	**/
	void OFMaterial::bind()
	{
		if (!mOFShader.isLoaded())
		{
			//Logger::warn("Bind: shader is not loaded: " + mShader.getValueRef());
			return;
		}

		// Bind shader to current GPU stream
		mOFShader.begin();

		int texLocation = 1;
		
		for (auto& binding : mBindings)
		{
			getBindFunctions()[binding->getValueType()](*binding, mOFShader, texLocation);
		}
	}



	/**
	@brief Unbinds the shader from the current GL render state
	**/
	void OFMaterial::unbind()
	{
		if (!mOFShader.isLoaded())
		{
			//Logger::warn("Unbind: shader is not loaded: " + mShader.getValueRef());
			return;
		}

		mOFShader.end();
	}
}

RTTI_DEFINE(nap::OFMaterial)

// binding types registration
