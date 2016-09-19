#include <nap/component.h>
#include <nap/module.h>
#include <napofattributes.h>
#include <napofshaderbinding.h>

#include <napoftransform.h>
#include <napofsplinecomponent.h>
#include <napofrendercomponent.h>
#include <napofupdatecomponent.h>
#include <napofsimplecamcomponent.h>
#include <napofsimpleshapecomponent.h>
#include <napoftracecomponent.h>
#include <napofsplinemodulationcomponent.h>
#include <napoflagcomponent.h>
#include <napofmaterial.h>
#include <napofframebuffercomponent.h>
//#include <napofspoutsendcomponent.h>
#include <napofvideocomponent.h>
#include <napofimagecomponent.h>
#include <napofsoundcomponent.h>
#include <utils/ofVec2i.h>
#include <napofblendtype.h>


NAP_MODULE_BEGIN(NAP_OF)
{
	NAP_REGISTER_DATATYPE(ofVec3f)
	NAP_REGISTER_DATATYPE(ofVec2i)
	NAP_REGISTER_DATATYPE(nap::OFBlendType)
	NAP_REGISTER_DATATYPE(ofTexture)

	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_ofVec3f)
	NAP_REGISTER_TYPECONVERTER(nap::convert_ofVec3f_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_ofVec2f)
	NAP_REGISTER_TYPECONVERTER(nap::convert_ofVec2f_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_SplineType)
	NAP_REGISTER_TYPECONVERTER(nap::convert_SplineType_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_ofFloatColor)
	NAP_REGISTER_TYPECONVERTER(nap::convert_ofFloatColor_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_ofEasyCam)
	NAP_REGISTER_TYPECONVERTER(nap::convert_ofEasyCam_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_OFVectorMap_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_OFVectorMap)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_ofVec2i)
	NAP_REGISTER_TYPECONVERTER(nap::convert_ofVec2i_to_string)
	NAP_REGISTER_TYPECONVERTER(nap::convert_string_to_ofblendtype)
	NAP_REGISTER_TYPECONVERTER(nap::convert_ofblendtype_to_string)

	NAP_REGISTER_COMPONENT(nap::OFTransform)
	NAP_REGISTER_COMPONENT(nap::OFSplineSelectionComponent)
	NAP_REGISTER_COMPONENT(nap::OFSplineComponent)
	NAP_REGISTER_COMPONENT(nap::OFSplineColorComponent)
	NAP_REGISTER_COMPONENT(nap::OFSimpleCamComponent)
	NAP_REGISTER_COMPONENT(nap::OFShapeComponent)
	NAP_REGISTER_COMPONENT(nap::OFPlaneComponent)
	NAP_REGISTER_COMPONENT(nap::OFTraceComponent)
	NAP_REGISTER_COMPONENT(nap::OFTextComponent)
	NAP_REGISTER_COMPONENT(nap::OFRotateComponent)
	NAP_REGISTER_COMPONENT(nap::OFSplineUpdateGPUComponent)
	NAP_REGISTER_COMPONENT(nap::OFSplineLFOModulationComponent)
	NAP_REGISTER_COMPONENT(nap::OFLagComponentBase)
	NAP_REGISTER_COMPONENT(nap::OFFloatLagComponent)
	NAP_REGISTER_COMPONENT(nap::OFVec3LagComponent)
	NAP_REGISTER_COMPONENT(nap::OFMaterial)
	NAP_REGISTER_COMPONENT(nap::OFFrameBufferComponent)
//	NAP_REGISTER_COMPONENT(nap::OFVideoComponent)
	NAP_REGISTER_COMPONENT(nap::OFImageComponent)
	NAP_REGISTER_COMPONENT(nap::OFPlaySoundComponent)

	nap::registerOfShaderBindings();
}
NAP_MODULE_END(NAP_OF)
