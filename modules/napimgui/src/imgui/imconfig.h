//-----------------------------------------------------------------------------
// USER IMPLEMENTATION
// This file contains compile-time options for ImGui.
// Other options (memory allocation overrides, callbacks, etc.) can be set at runtime via the ImGuiIO structure - ImGui::GetIO().
//-----------------------------------------------------------------------------

#pragma once
#include <color.h>
#include <nap/numeric.h>
#include <mathutils.h>
#include <texture2d.h>
#include <utility/dllexport.h>

//---- Define assertion handler. Defaults to calling assert().
//#define IM_ASSERT(_EXPR)  MyAssert(_EXPR)

//---- Define attributes of all API symbols declarations, e.g. for DLL under Windows.
#ifdef _WIN32
#ifdef NAP_SHARED_LIBRARY_IMGUI
	#define IMGUI_API __declspec(dllexport)    // Export the symbols
#else
	#define IMGUI_API __declspec(dllimport)    // Import the symbols
#endif // NAP_SHARED_LIBRARY
#else
    #define IMGUI_API __attribute__ ((visibility ("default")))	// Export the symbols
#endif // _WIN32

//---- Don't define obsolete functions names. Consider enabling from time to time or when updating to reduce like hood of using already obsolete function/names
//#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

//---- Include imgui_user.h at the end of imgui.h
//#define IMGUI_INCLUDE_IMGUI_USER_H

//---- Don't implement default handlers for Windows (so as not to link with OpenClipboard() and others Win32 functions)
//#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
//#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS

//---- Don't implement test window functionality (ShowTestWindow()/ShowStyleEditor()/ShowUserGuide() methods will be empty)
//---- It is very strongly recommended to NOT disable the test windows. Please read the comment at the top of imgui_demo.cpp to learn why.
//#define IMGUI_DISABLE_TEST_WINDOWS

//---- Don't implement ImFormatString(), ImFormatStringV() so you can reimplement them yourself.
//#define IMGUI_DISABLE_FORMAT_STRING_FUNCTIONS

//---- Pack colors to BGRA instead of RGBA (remove need to post process vertex buffer in back ends)
//#define IMGUI_USE_BGRA_PACKED_COLOR

//---- Implement STB libraries in a namespace to avoid linkage conflicts
//#define IMGUI_STB_NAMESPACE     ImGuiStb

//---- Define constructor and implicit cast operators to convert back<>forth from your math types and ImVec2/ImVec4.
/*
#define IM_VEC2_CLASS_EXTRA                                                 \
        ImVec2(const MyVec2& f) { x = f.x; y = f.y; }                       \
        operator MyVec2() const { return MyVec2(x,y); }
*/

#define IM_VEC4_CLASS_EXTRA                                                 \
        ImVec4(const nap::RGBAColorFloat& f)			{ x = f[0]; y = f[1]; z = f[2]; w = f[3]; }					\
        operator nap::RGBAColorFloat() const			{ return nap::RGBAColorFloat(x,y,z,w); }					\
		ImVec4(const nap::RGBColorFloat& f)				{ x = f[0]; y = f[1]; z = f[2]; w = 1.0f; }					\
		operator nap::RGBColorFloat() const				{ return nap::RGBColorFloat(x, y, z); }						\
		ImVec4(const nap::RGBAColor8& f)				{	x = (float)f[0]/(float)nap::math::max<nap::uint8>();	\
															y = (float)f[1]/(float)nap::math::max<nap::uint8>();	\
															z = (float)f[2]/(float)nap::math::max<nap::uint8>();	\
															w = (float)f[3]/(float)nap::math::max<nap::uint8>();}	\
		operator nap::RGBAColor8() const				{ return nap::RGBAColor8(									\
															static_cast<nap::uint8>((x * (float)nap::math::max<nap::uint8>())),		\
															static_cast<nap::uint8>((y * (float)nap::math::max<nap::uint8>())),		\
															static_cast<nap::uint8>((z * (float)nap::math::max<nap::uint8>())),		\
															static_cast<nap::uint8>((w * (float)nap::math::max<nap::uint8>()))); }  \
		ImVec4(const nap::RGBColor8& f)					{	x = (float)f[0]/(float)nap::math::max<nap::uint8>();	\
															y = (float)f[1]/(float)nap::math::max<nap::uint8>();	\
															z = (float)f[2]/(float)nap::math::max<nap::uint8>();	\
															w = 1.0f;}												\
		operator nap::RGBColor8() const					{ return nap::RGBColor8(									\
															static_cast<nap::uint8>((x * (float)nap::math::max<nap::uint8>())),		\
															static_cast<nap::uint8>((y * (float)nap::math::max<nap::uint8>())),		\
															static_cast<nap::uint8>((z * (float)nap::math::max<nap::uint8>()))); }	\
		ImVec4(const nap::RGBColorFloat& f, float a)	{ x = f[0]; y = f[1]; z = f[2]; w = a; }					\
		ImVec4(const nap::RGBColor8& f, float a)		{	x = (float)f[0]/(float)nap::math::max<nap::uint8>();	\
															y = (float)f[1]/(float)nap::math::max<nap::uint8>();	\
															z = (float)f[2]/(float)nap::math::max<nap::uint8>();	\
															w = a;}	


//---- Use 32-bit vertex indices (instead of default: 16-bit) to allow meshes with more than 64K vertices
#define ImDrawIdx unsigned int

//---- Tip: You can add extra functions within the ImGui:: namespace, here or in your own headers files.
//---- e.g. create variants of the ImGui::Value() helper for your low-level math types, or your own widgets/helpers.
namespace ImGui
{

}

