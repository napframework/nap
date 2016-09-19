#include "gridlayout.h"
#include "typeconverters.h"

NAP_MODULE_BEGIN(NapGUI)
{
	NAP_REGISTER_TYPECONVERTER(conv_rect_to_string)
	NAP_REGISTER_TYPECONVERTER(conv_string_to_rect)

	NAP_REGISTER_TYPECONVERTER(conv_margins_to_string)
	NAP_REGISTER_TYPECONVERTER(conv_string_to_margins)

	NAP_REGISTER_DATATYPE(nap::Rect)
	NAP_REGISTER_DATATYPE(nap::Margins)

	NAP_REGISTER_COMPONENT(nap::LayoutComponent)
	NAP_REGISTER_COMPONENT(nap::GridLayout)
}
NAP_MODULE_END(NapGUI)