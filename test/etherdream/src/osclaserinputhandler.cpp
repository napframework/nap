#include "osclaserinputhandler.h"
#include <nap/entity.h>
#include <utility/stringutils.h>

RTTI_BEGIN_CLASS(nap::OSCLaserInputHandler)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCLaserInputHandlerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	OSCLaserInputHandlerInstance::~OSCLaserInputHandlerInstance()
	{
		mInputComponent->messageReceived.disconnect(mMessageReceivedSlot);
	}


	bool OSCLaserInputHandlerInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		mRotateComponent = getEntityInstance()->findComponent<nap::RotateComponentInstance>();
		if (!errorState.check(mRotateComponent != nullptr, "missing rotate component"))
			return false;

		mMeshComponent = getEntityInstance()->findComponent<nap::RenderableMeshComponentInstance>();
		if (!errorState.check(mMeshComponent != nullptr, "missing mesh component"))
			return false;

		mInputComponent = getEntityInstance()->findComponent<nap::OSCInputComponentInstance>();
		if (!errorState.check(mInputComponent != nullptr, "missing osc input component"))
			return false;

		mInputComponent->messageReceived.connect(mMessageReceivedSlot);

		return true;
	}


	void OSCLaserInputHandlerInstance::handleMessageReceived(const nap::OSCEvent& oscEvent)
	{
		if (utility::gStartsWith(oscEvent.mAddress, "/color"))
		{
			updateColor(oscEvent);
			return;
		}

		if (utility::gStartsWith(oscEvent.mAddress, "/rotate"))
		{
			updateRotate(oscEvent);
			return;
		}
	}

	void OSCLaserInputHandlerInstance::updateColor(const OSCEvent& oscEvent)
	{
		// New value
		float v = oscEvent.getValue<float>(0);

		// Get the vertex colors
		nap::MeshInstance& mesh = mMeshComponent->getMeshInstance();
		Vec4VertexAttribute& color_attr = mesh.GetAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));

		// Get current color based on first vertex (dirty but ok for now)
		assert(color_attr.getSize() > 0);
		glm::vec4 ccolor = color_attr.getData()[0];

		// Get index
		std::vector<std::string> parts;
		utility::gSplitString(oscEvent.mAddress, '/', parts);

		// Get last
		int idx = std::stoi(parts.back().c_str());

		assert(idx > 0 && idx < 5);
		ccolor[idx-1] = v;

		color_attr.setData(std::vector<glm::vec4>(color_attr.getSize(), ccolor));
		nap::utility::ErrorState error;
		if (!mesh.update(error))
		{
			assert(false);
		}
	}


	void OSCLaserInputHandlerInstance::updateRotate(const OSCEvent& oscEvent)
	{
		// New value
		float v = oscEvent.getValue<float>(0);
		
		// Get index
		std::vector<std::string> parts;
		utility::gSplitString(oscEvent.mAddress, '/', parts);
		
		// Get last
		int idx = std::stoi(parts.back().c_str());

		switch (idx)
		{
		case 1:
			mRotateComponent->mProperties.mAxis.x = v;
			break;
		case 2:
			mRotateComponent->mProperties.mAxis.y = v;
			break;
		case 3:
			mRotateComponent->mProperties.mAxis.z = v;
			break;
		case 4:
			mRotateComponent->mProperties.mSpeed = v;
			break;
		default:
			assert(false);
		}
	}


	void OSCLaserInputHandler::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::RotateComponent));
		components.emplace_back(RTTI_OF(nap::RenderableMeshComponent));
		components.emplace_back(RTTI_OF(nap::OSCInputComponent));
	}

}
