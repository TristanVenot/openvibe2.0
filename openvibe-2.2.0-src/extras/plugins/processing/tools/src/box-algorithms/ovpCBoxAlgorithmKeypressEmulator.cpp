
#include "ovpCBoxAlgorithmKeypressEmulator.h"

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Tools;
using namespace OpenViBEToolkit;

#if defined(TARGET_OS_Windows)
#include <winuser.h>
#endif

boolean CBoxAlgorithmKeypressEmulator::initialize(void)
{
	m_Decoder.initialize(*this,0);

	m_TriggerStimulation = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_KeyToPress = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
	const uint64_t modifier = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 2);

#if defined(TARGET_OS_Windows)

	if (modifier == OVP_TypeId_Keypress_Modifier_Shift)
	{
		m_Modifier = VK_SHIFT;
	} 
	else if(modifier == OVP_TypeId_Keypress_Modifier_Control)
	{
		m_Modifier = VK_CONTROL;
	}
	else if(modifier == OVP_TypeId_Keypress_Modifier_Alt)
	{
		m_Modifier = VK_MENU;
	}
#elif defined(TARGET_OS_Linux)
	// @todo implement the whole linux solution
	// Handle modifier on linux here
	getLogManager() << LogLevel_Error << "This box is only implemented on Windows for the moment\n";
	return false;
#endif

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmKeypressEmulator::uninitialize(void)
{
	m_Decoder.uninitialize();

	return true;
}
/*******************************************************************************/

boolean CBoxAlgorithmKeypressEmulator::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}



/*******************************************************************************/

boolean CBoxAlgorithmKeypressEmulator::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	
	for (uint32_t i = 0; i < l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_Decoder.decode(i);

		if (m_Decoder.isHeaderReceived())
		{
			// NOP
		}
		if (m_Decoder.isBufferReceived())
		{
			const IStimulationSet* set = m_Decoder.getOutputStimulationSet();
			for (uint64_t s = 0; s < set->getStimulationCount(); s++)
			{
				if (set->getStimulationIdentifier(s) == m_TriggerStimulation)
				{
#if defined(TARGET_OS_Windows)
					getLogManager() << LogLevel_Debug << "Received " << m_TriggerStimulation 
						<< ", pressing Windows virtual key " << m_KeyToPress
						<< " with modifier " << m_Modifier
						<< ", delay is at least " << ITimeArithmetics::timeToSeconds(this->getPlayerContext().getCurrentTime() - set->getStimulationDate(s)) << "s"
						<< "\n";

					// @todo the stimulation time is not necessarily RIGHT NOW. A very
					// accurate solution would call SendInput exactly when the key needs to be pressed. Unfortunately when 
					// we receive the stim chunk, the keypress is already old, and we can't press keys into the past. In this sense
					// sending immediately might be the best we can do. 
					// Note that the speccing the "time" field of INPUT as nonzero doesn't send keypresses to the past or future.
					if (m_Modifier != 0)
					{
						INPUT keypress[3];
						memset(keypress, 0, sizeof(INPUT)* 3);
						keypress[0].type = INPUT_KEYBOARD;
						keypress[0].ki.wVk = (WORD)m_Modifier;
						keypress[1].type = INPUT_KEYBOARD;
						keypress[1].ki.wVk = (WORD)m_KeyToPress;
						keypress[2].type = INPUT_KEYBOARD;
						keypress[2].ki.wVk = (WORD)m_Modifier;
						keypress[2].ki.dwFlags = KEYEVENTF_KEYUP;
						SendInput(3, keypress, sizeof(INPUT));
					}
					else
					{
						INPUT keypress;
						memset(&keypress, 0, sizeof(INPUT));
						keypress.type = INPUT_KEYBOARD;
						keypress.ki.wVk = (WORD)m_KeyToPress;
						SendInput(1, &keypress, sizeof(INPUT));
					}
#elif defined(TARGET_OS_Linux)
					// @todo
#endif
				}
			}
		}
		if (m_Decoder.isEndReceived())
		{
			// NOP
		}

	}
	return true;
}

void CBoxAlgorithmKeypressEmulator::registerEnums(const OpenViBE::Kernel::IPluginModuleContext& rPluginModuleContext)
{
	rPluginModuleContext.getTypeManager().registerEnumerationType(OVP_TypeId_Keypress_Modifier, "Key modifier");
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Keypress_Modifier, "None", OVP_TypeId_Keypress_Modifier_None.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Keypress_Modifier, "Shift", OVP_TypeId_Keypress_Modifier_Shift.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Keypress_Modifier, "Ctrl", OVP_TypeId_Keypress_Modifier_Control.toUInteger());
	rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Keypress_Modifier, "Alt", OVP_TypeId_Keypress_Modifier_Alt.toUInteger());

	rPluginModuleContext.getTypeManager().registerEnumerationType(OVP_TypeId_Keypress_Key, "Key");

#if defined(TARGET_OS_Windows)
	// @note this solution makes the scenario not portable between win<->linux. The challenge is keys such as F1 etc,
	// otherwise we could just read ascii char and map it to a key.
	// @fixme we're also not able to expose all the possible keys to the user this way, but the problem seems to be that 
	// GetKeyNameTextA does not give meaningful & different names to all keys for some reason. 
	// - The current appoarch attempts to expose the usual 'english' keys that can be visualized by pango.
	// - We do not necessarily get arrow keys listed as these may not have a separate representation from 
	//   the corresponding 'numpad' keys with the functions we call here; esp. GetKeyNameTextA seems to
	//   depend on the keyboard in use. This limits the scenario portability across keyboards.
	for (int i = 0; i < 256; i++)
	{
		UINT mapped = MapVirtualKeyA(UINT(i), MAPVK_VK_TO_VSC);
		LONG scanCode = mapped << 16;

		if (scanCode)
		{
			CHAR buffer[512];
			if (GetKeyNameTextA(scanCode, buffer, 512))
			{
				std::string tmp(buffer);

				if (tmp.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_*&$%@^#!-+/\\ :;,.'`~()[]\"=") != std::string::npos)
				{
					// Some weird characters here, may be difficult to render for pango, ignore this key
					continue;
				}

				// GetKeyNameTextA can return the same name for different keys. Only use the first one.
				if (rPluginModuleContext.getTypeManager().getEnumerationEntryValueFromName(OVP_TypeId_Keypress_Key, buffer) == OV_UndefinedIdentifier)
				{
					rPluginModuleContext.getTypeManager().registerEnumerationEntry(OVP_TypeId_Keypress_Key, buffer, i);
				}
			}
		}
	}

#elif defined(TARGET_OS_Linux)
	// @todo register the linux keys to the enum here
#endif


}

