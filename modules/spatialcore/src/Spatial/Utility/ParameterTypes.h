#pragma once

#include <parametersimple.h>
#include <parameternumeric.h>
#include <parameterenum.h>
#include <parametervec.h>
#include <string>

namespace nap
{

    using ParameterString = ParameterSimple<std::string>;


    /**
     * Works very much like an enum parameter except that the available values and names can be determined at runtime.
     */
    class NAPAPI ParameterOptionList : public Parameter
    {
        RTTI_ENABLE(Parameter)
        
    public:
        ParameterOptionList() = default;

        int mValue = 0; ///< Property: 'Value'

        /**
         * Sets the list of available options. Resets the current index to 0.
         */
        void setOptions(const std::vector<std::string>& options);

        // Inherited from Parameter
        void setValue(const Parameter& value) override;

        /**
         * Sets the current option choice as index in the list
         */
        void setValue(int index);

        /**
         * Sets the current option using its name. Returns false if the option is not found.
         */
        bool setOption(const std::string& optionName);

        /**
         * @return the index of the current choice.
         */
        int getValue() const { return mValue; }

        /**
         * @return the name of the current choice
         */
        const std::string& getOptionName() const { return mOptions[mValue]; }

        /**
         * @return list of all available options.
         */
        const std::vector<std::string>& getOptions() const { return mOptions; }

        /**
         * Connects an action to the parameter's valueChanged signal and calls it straightaway, to make sure the connected system is in sync with the parameter's current value.
         * ActionType can be of any type that can be connected to the valueChanged signal.
         */
        template <typename ActionType>
        void connect(ActionType action)
        {
            valueChanged.connect(std::forward<ActionType>(action));
            action(mValue);
        }

        /**
         * Signal triggered whenever the choice has changed.
         */
        Signal<int> valueChanged;

    private:
        std::vector<std::string> mOptions;
    };

}
