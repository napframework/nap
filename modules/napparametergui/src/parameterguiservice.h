/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <parameter.h>
#include <unordered_map>

namespace nap
{
	/**
	 * Manages the editors that draw parameters as UI elements.
	 * Default editors are provided for built-in types such as int, float, vec2 etc.
	 * You can register your own editor for a specific Parameter by calling 'registerParameterEditor'
	 */
	class NAPAPI ParameterGUIService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		ParameterGUIService(ServiceConfiguration* configuration);

		/**
		 * A registered CreateParameterEditor function is called whenever the UI for a particular Parameter type needs to be shown. The parameter in question is passed
		 * as argument to the provided function.
		 *
		 * The provided function should use ImGUI internally to draw the UI for that parameter. It is important to note that the value should always be set
		 * on the parameter through the setValue function and not by editing the mValue member directly. The reason for this is that, when editing mValue directly,
		 * no signals will be raised, so clients of the Parameter that are watching for value changes will not receive any.
		 * To ensure this works as expected, the general pattern of a parameter editor looks something like the following:
		 *
		 * 	float value = parameter.mValue;
		 *	if (ImGui::SliderFloat(parameter.getDisplayName().c_str(), &value, parameter.mMinimum, parameter.mMaximum))
		 *		parameter.setValue(value);
		 *
		 * Note that we first retrieve a copy of the old value and display an ImGUI widget that works on that local value. After the user makes an edit,
		 * we set the value on the parameter based on the local copy, which now contains the new value.
		 */
		using CreateParameterEditor = std::function<void(Parameter&)>;

		/**
		 * Register an editor creation function for the given type. The editor creation function is invoked whenever a parameter of the given type
		 * is encountered and should use ImGUI internally to draw the UI for that parameter. The parameter in question is passed to the creation func.
		 *
		 * Registering a creation function for a type that was previously registered will overwrite the previous function, which allows you to customize
		 * the behavior for the built-in types that are registered by default
		 *
		 * @param type The type to register an editor creation function for
		 * @param createParameterEditorFunc The editor creation function
		 */
		void registerParameterEditor(const rtti::TypeInfo& type, const CreateParameterEditor& createParameterEditorFunc);

		/**
		 * find a GUI editor for a specific type of parameter, nullptr if not found
		 * @param parameter parameter to get editor for
		 * @return an editor for a specific type of parameter, nullptr if not found
		 */
		const CreateParameterEditor* findEditor(const nap::Parameter& parameter) const;

		/**
		 * find a GUI editor for a specific type of parameter, nullptr if not found
		 * @param parameterType type of parameter to get editor for
		 * @return an editor for a specific type of parameter, nullptr if not found
		 */
		const CreateParameterEditor* findEditor(rtti::TypeInfo parameterType) const;

	protected:
		/**
		 * Registers all default UI elements for for built-in types such as int, float etc.
		 * @param errorState contains the error message if the service could not be initialized
		 * @return if the service initialized successfully
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

	private:
		using ParameterEditorMap = std::unordered_map<rtti::TypeInfo, CreateParameterEditor>;
		ParameterEditorMap mParameterEditors;
		
		/**
		 * Called on init(), registers all the default parameter editors (float, int etc.)
		 */
		void registerDefaultParameterEditors();
	};
}
