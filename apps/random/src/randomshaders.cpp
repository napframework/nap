#include "randomshaders.h"
#include "randomapp.h"

#include <mathutils.h>
#include <orbitcomponent.h>

namespace nap
{
	RandomShaders::RandomShaders(RandomApp& app) : mApp(app)
	{

	}

	void RandomShaders::update(double deltaTime)
	{
		// Update lighting mode transition if active
		if (mLightingModeTransitionActive)
			updateLightingModeTransition(deltaTime);
	}

	void RandomShaders::startLightingModeTransition(nap::Texture2D& oldTexture, nap::Texture2D& newTexture)
	{
		// Set lighting mode transition properties
		mLightingModeTransitionActive = true;
		mLightingModeTransitionVelocity = 0.0f;

		// Set combination plane uniform values at transition start
		nap::RenderableMeshComponentInstance& combination_plane = mApp.mCombination->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformFloat& uBlendValue = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uBlendValue");
		nap::UniformTexture2D& uTextureOne = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("uTextureOne");
		nap::UniformTexture2D& uTextureTwo = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformTexture2D>("uTextureTwo");
		uBlendValue.setValue(0.0f);
		uTextureOne.setTexture(oldTexture);
		uTextureTwo.setTexture(newTexture);
	}

	void RandomShaders::updateLightingModeTransition(double deltaTime)
	{
		// Smoothly increment the blend uniform value in the combination plane
		nap::RenderableMeshComponentInstance& combination_plane = mApp.mCombination->getComponent<nap::RenderableMeshComponentInstance>();
		nap::UniformFloat& uBlendValue = combination_plane.getMaterialInstance().getOrCreateUniform<nap::UniformFloat>("uBlendValue");
		nap::math::smooth(uBlendValue.mValue, 1.0f, mLightingModeTransitionVelocity, static_cast<float>(deltaTime), 0.5f, 1000.0f);

		// Stop the updating when the transition is complete
		if (uBlendValue.mValue >= 1.0f)
		{
			mLightingModeTransitionActive = false;
			mApp.resetOldLightingMode();
		}
	}
}
