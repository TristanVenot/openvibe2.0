
#include "ovp_defines.h"

#include "box-algorithms/ovpCKeyboardStimulator.h"
#include "box-algorithms/ovpCSignChangeDetector.h"

#include "box-algorithms/ovpCBoxAlgorithmRunCommand.h"
#if defined TARGET_HAS_ThirdPartyLua
 #include "box-algorithms/ovpCBoxAlgorithmLuaStimulator.h"
#endif // TARGET_HAS_ThirdPartyLua
#if defined TARGET_HAS_ThirdPartyOpenAL
 #include "box-algorithms/ovpCBoxAlgorithmOpenALSoundPlayer.h"
#endif // TARGET_HAS_ThirdPartyOpenAL

#include "box-algorithms/adaptation/ovpCBoxAlgorithmStimulationFilter.h"

#include "box-algorithms/ovpCBoxAlgorithmP300SpellerStimulator.h"
#include "box-algorithms/ovpCBoxAlgorithmP300IdentifierStimulator.h"

OVP_Declare_Begin();
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OV_TypeId_BoxAlgorithmFlag, OV_AttributeId_Box_FlagIsUnstable.toString(), OV_AttributeId_Box_FlagIsUnstable.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationType(OVP_TypeId_StimulationFilterAction, "Stimulation Filter Action");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_StimulationFilterAction, "Select", OVP_TypeId_StimulationFilterAction_Select.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_StimulationFilterAction, "Reject", OVP_TypeId_StimulationFilterAction_Reject.toUInteger());

#if defined(TARGET_HAS_ThirdPartyGTK)
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CKeyboardStimulatorDesc);
#endif

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CSignChangeDetectorDesc);

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmRunCommandDesc);
#if defined TARGET_HAS_ThirdPartyLua
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmLuaStimulatorDesc);
#endif // TARGET_HAS_ThirdPartyLua

#if defined TARGET_HAS_ThirdPartyOpenAL
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmOpenALSoundPlayerDesc);
#endif // TARGET_HAS_ThirdPartyOpenAL

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmStimulationFilterDesc);

	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300SpellerStimulatorDesc);
	OVP_Declare_New(OpenViBEPlugins::Stimulation::CBoxAlgorithmP300IdentifierStimulatorDesc);

	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_Voting_ClearVotes, "Clear votes");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Voting_ClearVotes, "When expires",  OVP_TypeId_Voting_ClearVotes_WhenExpires.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Voting_ClearVotes, "After output", OVP_TypeId_Voting_ClearVotes_AfterOutput.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_Voting_OutputTime, "Output time");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Voting_OutputTime, "Time of voting",  OVP_TypeId_Voting_OutputTime_Vote.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Voting_OutputTime, "Time of last winning stimulus", OVP_TypeId_Voting_OutputTime_Winner.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Voting_OutputTime, "Time of last voting stimulus", OVP_TypeId_Voting_OutputTime_Last.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationType (OVP_TypeId_Voting_RejectClass_CanWin, "Reject can win");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Voting_RejectClass_CanWin, "Yes",  OVP_TypeId_Voting_RejectClass_CanWin_Yes.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Voting_RejectClass_CanWin, "No", OVP_TypeId_Voting_RejectClass_CanWin_No.toUInteger());

OVP_Declare_End();