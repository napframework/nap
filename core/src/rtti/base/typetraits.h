#pragma once

#include <rtti/base/rawtype.h>

#ifdef RTTI_USE_BOOST
    #include <boost/type_traits.hpp>
#else
    #include <type_traits>
#endif

namespace RTTI
{

#ifdef RTTI_USE_BOOST
    namespace Traits = boost;
#else
    #if RTTI_COMPILER == RTTI_COMPILER_MSVC
        #if RTTI_COMP_VER > 1500
            namespace Traits = std;
        #elif RTTI_COMP_VER == 1500
            namespace Traits = std::tr1;
        #else
            #error "No type_traits for this Visual Studio version available! Please upgrade Visual Studio or use Boost."
        #endif
    #else
        namespace Traits = std;
    #endif
#endif

// This is an own enable_if implementation, so we can reuse it with boost::integral_constant and std::integral_constant.
namespace impl
{
  template<bool B, class T = void>
  struct enable_if {};

  template<class T>
  struct enable_if<true, T> { typedef T type; };
} // end namespace impl

} // end namespace RTTI
