#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
//                                                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_BoxAlgorithm_LatencyEvaluation          OpenViBE::CIdentifier(0x0AD11EC1, 0x7EF3690B)
#define OVP_ClassId_BoxAlgorithm_LatencyEvaluationDesc      OpenViBE::CIdentifier(0x5DB56A54, 0x5380262B)

#define OVP_ClassId_BoxAlgorithm_MouseTracking              OpenViBE::CIdentifier(0x1E386EE5, 0x203E13C6)
#define OVP_ClassId_BoxAlgorithm_MouseTrackingDesc          OpenViBE::CIdentifier(0x7A31C11B, 0xF522262E)
#define OV_AttributeId_Box_FlagIsUnstable                   OpenViBE::CIdentifier(0x666FFFFF, 0x666FFFFF)

#define OVP_TypeId_Keypress_Key                             OpenViBE::CIdentifier(0x23E525F9, 0x005F187D)
#define OVP_TypeId_Keypress_Modifier                        OpenViBE::CIdentifier(0x0C4F4585, 0x78B246AC)
#define OVP_TypeId_Keypress_Modifier_None                   OpenViBE::CIdentifier(0x5A70497E, 0x52F57199)
#define OVP_TypeId_Keypress_Modifier_Shift                  OpenViBE::CIdentifier(0x47534AA6, 0x138A1130)
#define OVP_TypeId_Keypress_Modifier_Control                OpenViBE::CIdentifier(0x67C6455F, 0x08E6134D)
#define OVP_TypeId_Keypress_Modifier_Alt                    OpenViBE::CIdentifier(0x77577AEA, 0x74CD7E3E)


//___________________________________________________________________//
//                                                                   //
// Global defines                                                   //
//___________________________________________________________________//
//                                                                   //

#ifdef TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines
 #include "ovp_global_defines.h"
#endif // TARGET_HAS_ThirdPartyOpenViBEPluginsGlobalDefines

//___________________________________________________________________//


#endif // __OpenViBEPlugins_Defines_H__
