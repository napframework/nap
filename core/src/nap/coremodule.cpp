// Local Includes
#include "coremodule.h"
#include "eventdispatcher.h"

/**
 * Nap Core module constructor
 */
ModuleNapCore::ModuleNapCore() : nap::Module("NapCore")
{
    // Types
    NAP_REGISTER_DATATYPE(float)
    NAP_REGISTER_DATATYPE(int)
    NAP_REGISTER_DATATYPE(std::string)
	NAP_REGISTER_DATATYPE(nap::IntArray)
	NAP_REGISTER_DATATYPE(nap::FloatArray)
	NAP_REGISTER_DATATYPE(nap::StringArray)
	NAP_REGISTER_DATATYPE(nap::FloatMap)
	NAP_REGISTER_DATATYPE(nap::IntMap)
	NAP_REGISTER_DATATYPE(nap::StringMap)
	NAP_REGISTER_DATATYPE(nap::DispatchMethod)

    // Components
    NAP_REGISTER_COMPONENT(nap::PatchComponent)

    // Operators
    NAP_REGISTER_OPERATOR(nap::FloatOperator)
    NAP_REGISTER_OPERATOR(nap::AddFloatOperator)
    NAP_REGISTER_OPERATOR(nap::MultFloatOperator)
    NAP_REGISTER_OPERATOR(nap::SimpleTriggerOperator)
    NAP_REGISTER_OPERATOR(nap::IntOperator)

    // TypeConverters
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToFloat)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToInt)
    NAP_REGISTER_TYPECONVERTER(nap::convertIntToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToIntArray)
    NAP_REGISTER_TYPECONVERTER(nap::convertIntArrayToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToFloatArray)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatArrayToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertBinaryToFloatArray)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatArrayToBinary)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToBool)
    NAP_REGISTER_TYPECONVERTER(nap::convertBoolToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToIntMap)
    NAP_REGISTER_TYPECONVERTER(nap::convertIntMapToString)
    NAP_REGISTER_TYPECONVERTER(nap::convertStringToFloatMap)
    NAP_REGISTER_TYPECONVERTER(nap::convertFloatMapToString)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_dispatchmethod)
	NAP_REGISTER_TYPECONVERTER(nap::convert_dispatchmethod_to_string)
}
