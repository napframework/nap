/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "layersorter.h"
#include "renderablemeshcomponent.h"

namespace nap
{
	LayerComparer::LayerComparer(EMode mode, const glm::mat4x4& viewMatrix) :
		mViewMatrix(viewMatrix),
		mMode(mode) { }


	bool LayerComparer::operator()(const ComponentInstance* objectA, const ComponentInstance* objectB)
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
		void sortObjectsByLayer(std::vector<RenderableComponentInstance*>& comps, const glm::mat4& viewMatrix)
		{
			std::map<LayerIndex, std::vector<RenderableComponentInstance*>> layer_map;
			std::map<LayerIndex, int> hist;

			for (const auto& comp : comps)
				++hist[comp->getRenderLayer()];

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

				// Split into front to back and back to front meshes
				std::vector<RenderableComponentInstance*> front_to_back;
				front_to_back.reserve(layer_comps.size());
				std::vector<RenderableComponentInstance*> back_to_front;
				back_to_front.reserve(layer_comps.size());

				for (RenderableComponentInstance* component : layer_comps)
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
				LayerComparer front_to_back_sorter(LayerComparer::EMode::FrontToBack, viewMatrix);
				std::sort(front_to_back.begin(), front_to_back.end(), front_to_back_sorter);

				// Then sort back to front and render these
				LayerComparer back_to_front_sorter(LayerComparer::EMode::BackToFront, viewMatrix);
				std::sort(back_to_front.begin(), back_to_front.end(), back_to_front_sorter);

				// concatinate both in to the output
				comps.insert(comps.end(), std::make_move_iterator(front_to_back.begin()), std::make_move_iterator(front_to_back.end()));
				comps.insert(comps.end(), std::make_move_iterator(back_to_front.begin()), std::make_move_iterator(back_to_front.end()));
			}
		}
	}
}
