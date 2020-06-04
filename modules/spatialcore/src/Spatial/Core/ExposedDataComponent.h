#pragma once

// Nap includes
#include <component.h>
#include <entity.h>
#include <oscsender.h>
#include <oscevent.h>
#include <signal.h>

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>

// Rtti includes
#include <utility/dllexport.h>
#include <rtti/typeinfo.h>
#include <rtti/rtticast.h>

// Std includes
#include <functional>
#include <map>


namespace nap
{

    namespace spatial
    {

        /**
         * Base class for all exposed data.
         * Subclasses of this class do not contain any actual data but rather labda objects that retrieve data from the system that is to be exposed.
         */
        class NAPAPI ExposedValueBase
        {
            RTTI_ENABLE()

        public:
            ExposedValueBase(const std::string& name) : mName(name) { }
			
            virtual ~ExposedValueBase() = default;

			ExposedValueBase(const ExposedValueBase&) = delete;
			ExposedValueBase(ExposedValueBase&&) = delete;

            /**
             * @return OSCEvent with the @name as tag and the value as arguments, that can be sent through an OSCSender.
             */
            virtual std::unique_ptr<OSCEvent> createOSCEvent(const std::string& addressPrefix) { return nullptr; }

            virtual std::string toString() { return ""; }

            const std::string& getName() const { return mName; }

		protected:
            /**
             * Converts its value to std::string (for GUI etc).
             * For more complex types, this function is overwritten by template specializations (at the end of this header file).
             */
			template <typename T>
			std::string valueToString(const T& value) const { return std::to_string(value); }

            /**
             * Adds an OSC value of type T.
             * For more complex types, this function is overwritten by template specializations (at the end of this header file).
             */
			template <typename T>
			void addOSCValue(OSCEvent& event, const T& value)
			{
				event.template addArgument<OSCValue<T>>(value);
			}

		private:
            std::string mName  = "";
        };


        /**
         * Templated exposed value of primitive type T.
         * @tparam T The data type of the exposed value.
         */
        template <typename T>
        class NAPAPI ExposedValue : public ExposedValueBase
        {
            RTTI_ENABLE(ExposedValueBase)

        public:
            using Getter = std::function<T()>;

            /**
             * Constructor
             * @param getter is a lambda object that returns the value being exposed.
             */
            ExposedValue(const std::string& name, Getter getter) : ExposedValueBase(name), mGetter(getter)
            {
            }

            // Inherited from ExposedValueBase
            std::unique_ptr<OSCEvent> createOSCEvent(const std::string& addressPrefix) override
            {
                auto event = std::make_unique<OSCEvent>(addressPrefix + getName());
                addOSCValue(*event, getValue());
                return event;
            }

            std::string toString() override { return valueToString<T>(getValue()); }

            /**
             * Retrieves the exposed value by calling the getter.
             */
            T getValue() { return mGetter(); }

        private:
            Getter mGetter = nullptr;
        };


        /**
         * Templated exposed array of values of primitive type T.
         * The exposed array does not need to be a physical array in memory.
         * Rather it is defined as a getter function that gets an index as argument.
         * @tparam T The data type of the elements within the exposed array.
         */
        template <typename T>
        class NAPAPI ExposedArray : public ExposedValueBase {
            RTTI_ENABLE(ExposedValueBase)

        public:
            using Getter = std::function<T(int)>;
            using SizeFunction = std::function<unsigned int()>;

            /**
             * Constructor
             * @param getter a lambda object that gets an index as argument and returns an element in the array.
             */
            ExposedArray(const std::string& name, Getter getter, SizeFunction sizeFunction) : ExposedValueBase(name), mGetter(getter), mSizeFunction(sizeFunction) { }

            /**
             * Calls the getter to retrieve the value of an element in the array.
             */
            T getValue(int index) { return mGetter(index); }
            T operator[](int index) { return mGetter(index); }

            // Inherited from ExposedValueBase
            std::unique_ptr<OSCEvent> createOSCEvent(const std::string& addressPrefix) override
            {
                auto event = std::make_unique<OSCEvent>(addressPrefix + getName());
                auto size = mSizeFunction();
                for (auto i = 0; i < size; ++i)
                    addOSCValue(*event, getValue(i));
                return event;
            }


            std::string toString() override
            {
                std::string result = "";
                auto size = mSizeFunction();
                for (auto i = 0; i < size; ++i)
                {
                    if (i > 0)
                        result.append(", ");
                    result.append(valueToString<T>(getValue(i)));
                }
                return result;
            }

        private:
            Getter mGetter = nullptr;
            SizeFunction mSizeFunction = nullptr;
        };


        /**
         * Container for multiple exposed data entries that are stored by their (unique) names.
         */
        class NAPAPI ExposedDataContainer : public ExposedValueBase {
            RTTI_ENABLE(ExposedValueBase)

        public:
            ExposedDataContainer(const std::string& name) : ExposedValueBase(name) { }

            /**
             * Retrieve an exposed value from the container by its name.
             * If no value is found with the given name a nullptr is returned.
             */
            template <typename T>
            ExposedValue<T>* get(const std::string& name)
            {
                auto it = mValues.find(name);
                if (it != mValues.end())
                    return rtti_cast<ExposedValue<T>>(it->second.get());
                return nullptr;
            }

            /**
             * Retrieve an exposed array of values from the container by its name.
             * If no array is found with the given name a nullptr is returned.
             */
            template <typename T>
            ExposedArray<T>* getArray(const std::string& name)
            {
                auto it = mValues.find(name);
                if (it != mValues.end())
                    return rtti_cast<ExposedArray<T>>(it->second.get());
                return nullptr;
            }

            /**
             * Retrieve a subcontainer from the container by its name.
             * If no container is found with the given name a nullptr is returned.
             */
            ExposedDataContainer* getContainer(const std::string& name)
            {
                auto it = mValues.find(name);
                if (it != mValues.end())
                    return rtti_cast<ExposedDataContainer>(it->second.get());
                return nullptr;
            }

            /**
             * Expose a single value of type T with a given name.
             * @tparam T data type of the exposed value.
             * @param name name that the exposed value will be mapped to and that can be used to retrieve it.
             * @param getter lambda object returning the exposed value.
             * @return pointer to the newly created exposed value on sucess, nullptr if the @name is already used.
             */
            template <typename T>
            ExposedValue<T>* expose(const std::string& name, typename ExposedValue<T>::Getter getter)
            {
                auto it = mValues.find(name);
                if (it != mValues.end())
                    return nullptr;

                auto result = std::make_unique<ExposedValue<T>>(name, getter);
                auto resultPtr = result.get();
                mValues[name] = std::move(result);
                return resultPtr;
            }


            template <typename T>
            ExposedValue<T>* expose(const std::string& name, T& value)
            {
                auto valuePtr = &value;
                return expose<T>(name, [valuePtr](){ return *valuePtr; });
            }

            /**
             * Expose an array of values of type T to the container.
             * @tparam T data type of the elements in the array.
             * @param name name that the array will be mapped to within the container and that can be used to retrieve it.
             * @param getter lambda object taking index in the array and returning the value of the element at the index.
             * @return newly created exposed array on success, nullptr if the @name was already used.
             */
            template <typename T>
            ExposedArray<T>* expose(const std::string& name, typename ExposedArray<T>::Getter getter, typename ExposedArray<T>::SizeFunction sizeFunction)
            {
                auto it = mValues.find(name);
                if (it != mValues.end())
                    return nullptr;

                auto result = std::make_unique<ExposedArray<T>>(name, getter, sizeFunction);
                auto resultPtr = result.get();
                mValues[name] = std::move(result);
                return resultPtr;
            }

            /**
             * Expose a subcontainer to the container
             * @param name name that the new subcontainer will be mapped to within this container.
             * @return the new container on success, nullptr if the @name was already being used.
             */
            ExposedDataContainer* expose(const std::string& name)
            {
                auto it = mValues.find(name);
                if (it != mValues.end())
                    return nullptr;

                auto result = std::make_unique<ExposedDataContainer>(name);
                auto resultPtr = result.get();
                mValues[name] = std::move(result);
                return resultPtr;
            }

            const std::map<std::string, std::unique_ptr<ExposedValueBase>>& getValues() const { return mValues; }

        private:
            std::map<std::string, std::unique_ptr<ExposedValueBase>> mValues;
        };


        /**
         * Instance of a @ExposedDataComponent.
         * Owns an @ExposedDataContainer.
         */
        class NAPAPI ExposedDataComponentInstance : public ComponentInstance
        {
        RTTI_ENABLE(ComponentInstance)

        public:
            ExposedDataComponentInstance(EntityInstance &entity, Component &resource) : ComponentInstance(entity,
                                                                                                          resource), mExposedData("")
            {}

            /**
             * A registered CreateExposedDataFunction is called whenever a new parameter is used.
             * The provided function should expose the Parameter at the ExposedDataComponentInstance.
             */
            using ExposeParameterFunction = std::function<void(nap::Parameter&)>;
            

            /**
             * Registers a parameter type to be exposed. 
             * When registering a new type T, it could be necessary to also specialize
             * ExposedValueBase::addOSCValue<T>() and ExposedValueBase::toString<T>()
             */
            template <typename T>
            void registerParameterType();
            
            // Initialize the component
            bool init(utility::ErrorState &errorState) override;
            void update(double deltaTime) override;

            /**
             * @return the @ExposedDataContainer that manages all the exposed data.
             */
            ExposedDataContainer& getRoot() { return mExposedData; }

            void addOSCOutput(ExposedValueBase& value) { mOSCOutputs.emplace(&value); }
            void addOSCOutputByName(const std::string& name);
            void removeOSCOutput(ExposedValueBase& value) { mOSCOutputs.erase(&value); }
            bool isOSCOutput(ExposedValueBase& value) const { return (mOSCOutputs.find(&value) != mOSCOutputs.end()); }

            const std::string& getOSCAddressPrefix() const { return mOSCAddressPrefix; }

        private:
            void registerDefaultExposeParameterFunctions();
            
            /**
             * Registers an ExposeParameterFunction for a Parameter type.
             */
            void registerExposeParameterFunction(const rtti::TypeInfo& type, const ExposeParameterFunction& exposeParameterFunction);
            
            
            Slot<nap::Parameter&> mParameterCreatedSlot = { this, &ExposedDataComponentInstance::parameterCreated };
            void parameterCreated(nap::Parameter& parameter); // called after parameter is created. Exposes its value.
            
            using ExposeParameterFunctionMap = std::unordered_map<rtti::TypeInfo, ExposeParameterFunction>;
            
            ExposeParameterFunctionMap mExposeParameterFunctions; ///< The function to expose the value per parameter type
            
            ExposedDataContainer mExposedData;
            ResourcePtr<OSCSender> mOSCSender = nullptr;
            std::set<ExposedValueBase*> mOSCOutputs;
            std::string mOSCAddressPrefix = "";
        };


        /**
         * Component that manages data that is exposed to the outside world.
         * The data is presented in the form of an @ExposedDataContainer so it can be inspected and queried without any precise knowledge of the actual data that is being exposed or its format.
         * The purpose is to create a clean interface layer between the sound object and the monitor and GUI.
         * ExposedDataComponent automatically exposes parameters created by ParameterComponent.
         */
        class NAPAPI ExposedDataComponent : public Component
        {
            RTTI_ENABLE(Component)
            DECLARE_COMPONENT(ExposedDataComponent, ExposedDataComponentInstance)

        public:
            ExposedDataComponent() : Component() { }

            void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
            
            ResourcePtr<OSCSender> mOscSender = nullptr; ///< Property: 'OscSender' If the component can send output through OSC this property has to point to an OSC sender resource. If OSC output is not supported for this component this property can be left blank.

        private:
        };


        template <>
        inline void ExposedValueBase::addOSCValue<glm::vec2>(OSCEvent& event, const glm::vec2& value)
        {
            event.addValue<float>(value.x);
            event.addValue<float>(value.y);
        }


        template <>
        inline std::string ExposedValueBase::valueToString<glm::vec2>(const glm::vec2& value) const
        {
            return "(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ")";
        }


        template <>
        inline void ExposedValueBase::addOSCValue<glm::vec3>(OSCEvent& event, const glm::vec3& value)
        {
            event.addValue<float>(value.x);
            event.addValue<float>(value.y);
            event.addValue<float>(value.z);
        }


        template <>
        inline std::string ExposedValueBase::valueToString<glm::vec3>(const glm::vec3& value) const
        {
            return "(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z) + ")";
        }


        template <>
        inline void ExposedValueBase::addOSCValue<glm::vec4>(OSCEvent& event, const glm::vec4& value)
        {
            event.addValue<float>(value.x);
            event.addValue<float>(value.y);
            event.addValue<float>(value.z);
            event.addValue<float>(value.w);
        }


        template <>
        inline std::string ExposedValueBase::valueToString<glm::vec4>(const glm::vec4& value) const
        {
            return "(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " + std::to_string(value.z) + ", " + std::to_string(value.w) + ")";
        }

        
        template <>
        inline void ExposedValueBase::addOSCValue<std::string>(OSCEvent& event, const std::string& value)
        {
            event.addValue<const char*>(value.c_str());
        }
        
        template <>
        inline std::string ExposedValueBase::valueToString<std::string>(const std::string& value) const
        {
            return value;
        }
        
        
        template <typename T>
        void ExposedDataComponentInstance::registerParameterType()
        {
            registerExposeParameterFunction(RTTI_OF(T), [&](Parameter& parameter)
                                            {
                                                T* p = rtti_cast<T>(&parameter);
                                                getRoot().expose<decltype(T::mValue)>(p->mName, p->mValue);
                                            });
        }


    }

}
