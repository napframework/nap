#pragma once

namespace RTTI
{
namespace impl
{
template<bool> struct CompileTimeError;
template<> struct CompileTimeError<true> {}; // only true is defined

} // end namespace impl
} // end namespace RTTI

#define RTTI_STATIC_ASSERT(x, msg) { RTTI::impl::CompileTimeError<(x)> ERROR_##msg; (void)ERROR_##msg; } 
