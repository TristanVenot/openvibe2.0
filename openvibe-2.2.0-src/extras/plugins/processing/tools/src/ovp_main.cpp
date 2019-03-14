#include "ovp_defines.h"

//// #include "box-algorithms/ovpCBoxAlgorithmLatencyEvaluation.h"
#include "box-algorithms/ovpCBoxAlgorithmMouseTracking.h"
#include "box-algorithms/ovpCBoxAlgorithmKeypressEmulator.h"

#include "openvibe/ovCIdentifier.h"

OVP_Declare_Begin();

	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_BoxAlgorithmFlag, OV_AttributeId_Box_FlagIsUnstable.toString(), OV_AttributeId_Box_FlagIsUnstable.toUInteger());
// 	OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmLatencyEvaluationDesc);
#if defined(TARGET_HAS_ThirdPartyGTK)
	OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmMouseTrackingDesc);
#endif

	OVP_Declare_New(OpenViBEPlugins::Tools::CBoxAlgorithmKeypressEmulatorDesc);
	// @note the following code is a bit long so we've implemented it inside the class
	OpenViBEPlugins::Tools::CBoxAlgorithmKeypressEmulator::registerEnums(rPluginModuleContext);

OVP_Declare_End();
