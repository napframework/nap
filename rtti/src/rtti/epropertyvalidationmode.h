#pragma once

namespace nap
{
	namespace rtti
	{
		enum class EPropertyValidationMode : uint8_t
		{
			DisallowMissingProperties,				///< When a required property is missing from the file, an error will be returned
			AllowMissingProperties					///< When a required property is missing from the file, no errors will be returned
		};

	} //< End Namespace rtti
}
