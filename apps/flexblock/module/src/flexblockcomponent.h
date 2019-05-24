#pragma once

#include "controlpointsmesh.h"
#include "framemesh.h"
#include "flexblockmesh.h"
#include "FlexblockData.h"

#include <component.h>
#include <boxmesh.h>
#include <renderablemeshcomponent.h>
#include <componentptr.h>

namespace nap
{
	class FlexBlockComponentInstance;

	/**
	 *	FlexBlockComponent
	 */
	class NAPAPI FlexBlockComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FlexBlockComponent, FlexBlockComponentInstance)
	public:
		//
		ResourcePtr<ControlPointsMesh> mControlPointsMesh;
		ResourcePtr<FrameMesh> mFrameMesh;
		ResourcePtr<FlexBlockMesh> mFlexBlockMesh;

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * FlexBlockComponentInstance	
	 */
	class NAPAPI FlexBlockComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FlexBlockComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
		
		/**
		 * Initialize FlexBlockComponentInstance based on the FlexBlockComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the FlexBlockComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update FlexBlockComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		glm::vec3 get_object_element_force_of_element(int elidx, int direction);

		glm::vec3 get_projected_suspension_forces_on_opposite_point_of_element(int object_element_id, int opposite_column);

		glm::vec3 get_projected_suspension_force_on_opposite_point_of_element(int object_element_id, int suspension_element_id, int opposite_point);

		glm::vec3 get_suspension_force_on_point_of_element(int elidx, int point);

		std::vector<int> get_ids_of_suspension_elements_on_point(int id);

		void calcInput();

		void calcElements();

		void concatElements();
		
		void concatPoints();

		void SetMotorInput(int index, float value);
		glm::vec3 GetControlPoint(int index);

		void setInput(std::vector<float> inputs);
	protected:
		void readShapes();
		void readSizes();
	protected:
		ResourcePtr<ControlPointsMesh> mControlPointsMesh;
		ResourcePtr<FrameMesh> mFrameMesh;
		ResourcePtr<FlexBlockMesh> mFlexBlockMesh;

		//
		float force_object = 10.0f;
		float force_object_spring = 0.02f;
		float force_object2frame = 2.0f;
		float change_speed = 1.0f;

		float maxacc;
		float maxspeed;

		float lengtherror = 0;

		float frequency;

		FlexblockShape objshape;
		FlexblockSize objsize;
		int inputs;
		int countInputs;

		std::vector<glm::vec3> points_object;
		std::vector<glm::vec3> points_frame;

		std::vector<std::vector<int>> elements_object;
		std::vector<std::vector<int>> elements_object2frame;
		std::vector<std::vector<int>> elements_frame;

		std::vector<FlexblockShape> shapes;
		std::vector<FlexblockSize> sizes;

		float endtime = 0.0f;
		std::vector<glm::vec3> points;
		std::vector<std::vector<int>> elements;
		std::vector<std::vector<int>> elements_all;
		std::vector<glm::vec3> elements_vector;
		std::vector<float> elements_length;
		std::vector<float> elements_length_ref;
		std::vector<float> elements_object_length;
		std::vector<float> elements_input;
		std::vector<glm::vec3> point_change;
		std::vector<glm::vec3> point_change_corr;
		glm::vec3 point_force;
		glm::vec3 point_force_corr;
		std::vector<int> element_indices;

		std::vector<float> override;
		float slack = 0.0f;

		float motorspd = 0.0f;
		float motoracc = 0.0f;

		std::vector<float> elements_length_delta;

		double starttime = 0.0;
		std::vector<float> motor_input;
	};
}
