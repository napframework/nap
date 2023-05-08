/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "depthsorter.h"
#include "renderablemeshcomponent.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static void sortSubsetByDepth(std::vector<RenderableComponentInstance*>& outComps, const std::vector<RenderableComponentInstance*>& subsetComps, const glm::mat4& viewMatrix)
	{
		// Split into front to back and back to front meshes
		std::vector<RenderableComponentInstance*> front_to_back;
		front_to_back.reserve(subsetComps.size());
		std::vector<RenderableComponentInstance*> back_to_front;
		back_to_front.reserve(subsetComps.size());

		for (RenderableComponentInstance* component : subsetComps)
		{
			RenderableMeshComponentInstance* renderable_mesh = rtti_cast<RenderableMeshComponentInstance>(component);
			if (renderable_mesh != nullptr)
			{
				RenderableMeshComponentInstance* renderable_mesh = static_cast<RenderableMeshComponentInstance*>(component);
				EBlendMode blend_mode = renderable_mesh->getMaterialInstance().getBlendMode();
				if (blend_mode == EBlendMode::AlphaBlend)
					back_to_front.emplace_back(component);
				else
					front_to_back.emplace_back(component);
			}
			else
			{
				front_to_back.emplace_back(component);
			}
		}

		// Sort front to back and render those first
		DepthComparer front_to_back_sorter(DepthComparer::EMode::FrontToBack, viewMatrix);
		std::sort(front_to_back.begin(), front_to_back.end(), front_to_back_sorter);

		// Then sort back to front and render these
		DepthComparer back_to_front_sorter(DepthComparer::EMode::BackToFront, viewMatrix);
		std::sort(back_to_front.begin(), back_to_front.end(), back_to_front_sorter);

		// concatinate both in to the output
		outComps.insert(outComps.end(), std::make_move_iterator(front_to_back.begin()), std::make_move_iterator(front_to_back.end()));
		outComps.insert(outComps.end(), std::make_move_iterator(back_to_front.begin()), std::make_move_iterator(back_to_front.end()));
	}


	//////////////////////////////////////////////////////////////////////////
	// DepthComparer
	//////////////////////////////////////////////////////////////////////////

	DepthComparer::DepthComparer(EMode mode, const glm::mat4x4& viewMatrix) :
		mViewMatrix(viewMatrix),
		mMode(mode) { }


	bool DepthComparer::operator()(const ComponentInstance* objectA, const ComponentInstance* objectB)
	{
		// Get the transform of objectA in view space
		const EntityInstance& entityA = *objectA->getEntityInstance();
		const TransformComponentInstance& transformA = entityA.getComponent<TransformComponentInstance>();
		const glm::mat4 view_space_a = mViewMatrix * transformA.getGlobalTransform();

		// Get the transform of objectB in view space
		const EntityInstance& entityB = *objectB->getEntityInstance();
		const TransformComponentInstance& transformB = entityB.getComponent<TransformComponentInstance>();
		const glm::mat4 view_space_b = mViewMatrix * transformB.getGlobalTransform();

		// Get the z-component (i.e. depth) of both entities
		float a_z = view_space_a[3].z;
		float b_z = view_space_b[3].z;

		// Compare
		return mMode == EMode::BackToFront ? a_z < b_z : a_z > b_z;
	}


	namespace sorter
	{
		void sortObjectsByDepth(std::vector<RenderableComponentInstance*>& comps, const glm::mat4& viewMatrix)
		{
			// Count layer occurrences
			std::map<LayerIndex, int> hist;
			for (const auto& comp : comps)
				++hist[comp->getRenderLayer()];

			// If all objects are in the same layer, sort the full set by depth and return
			if (hist.size() == 1)
			{
				const auto full_set = comps;
				comps.clear();
				sortSubsetByDepth(comps, full_set, viewMatrix);
				return;
			}

			std::map<LayerIndex, std::vector<RenderableComponentInstance*>> layer_map;
			for (const auto& item : hist)
			{
				auto& v = layer_map[item.first] = {};
				v.reserve(item.second);
			}

			for (const auto& comp : comps)
				layer_map[comp->getRenderLayer()].emplace_back(comp);

			comps.clear();
			for (const auto& entry : layer_map)
			{
				auto& layer_comps = entry.second;
				sortSubsetByDepth(comps, layer_comps, viewMatrix);
			}
		}
	}
}
