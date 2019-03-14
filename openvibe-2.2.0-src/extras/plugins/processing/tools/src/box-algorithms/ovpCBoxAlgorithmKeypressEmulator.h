
#pragma once

#include "../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>

#define OVP_ClassId_BoxAlgorithm_KeypressEmulator OpenViBE::CIdentifier(0x38503532, 0x19494145)
#define OVP_ClassId_BoxAlgorithm_KeypressEmulatorDesc OpenViBE::CIdentifier(0x59286224, 0x99423852)

namespace OpenViBEPlugins
{
	namespace Tools
	{
		/**
		 * \class CBoxAlgorithmKeypressEmulator
		 * \author Jussi T. Lindgren / Inria
		 * \date 29.Oct.2019
		 * \brief Emulates keypresses on a keyboard based on input stimulations. Based on a request from Fabien Lotte / POTIOC.
		 *
		 */
		class CBoxAlgorithmKeypressEmulator : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
	
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_KeypressEmulator);

			// Register enums to the kernel used by this box
			static void registerEnums(const OpenViBE::Kernel::IPluginModuleContext& rContext);

		protected:
			OpenViBEToolkit::TStimulationDecoder < CBoxAlgorithmKeypressEmulator > m_Decoder;

			// @todo for multiple triggers, use std::map<> 
			uint64_t m_TriggerStimulation = 0;
			uint64_t m_KeyToPress = 0;
			uint64_t m_Modifier = 0; 
		};

		class CBoxAlgorithmKeypressEmulatorDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Keypress Emulator"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Jussi T. Lindgren"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Emulates pressing keyboard keys when receiving stimulations"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Tools"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("0.1"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-index"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_KeypressEmulator; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Tools::CBoxAlgorithmKeypressEmulator; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				
				rBoxAlgorithmPrototype.addInput("Stimulations",OV_TypeId_Stimulations);
				
				// @todo add support for multiple keys, e.g. look at VRPN boxes for howto
				rBoxAlgorithmPrototype.addSetting("Trigger", OV_TypeId_Stimulation, "OVTK_StimulationId_Label_00");
				rBoxAlgorithmPrototype.addSetting("Key to press", OVP_TypeId_Keypress_Key, "A");
				rBoxAlgorithmPrototype.addSetting("Key modifier", OVP_TypeId_Keypress_Modifier, OVP_TypeId_Keypress_Modifier_None.toString());

				rBoxAlgorithmPrototype.addFlag(OV_AttributeId_Box_FlagIsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_KeypressEmulatorDesc);
		};
	};
};

