#ifndef __OpenViBEPlugins_BoxAlgorithm_P300MagicCardVisualization_H__
#define __OpenViBEPlugins_BoxAlgorithm_P300MagicCardVisualization_H__

#include "../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <visualization-toolkit/ovviz_all.h>

#include <gtk/gtk.h>
#include <map>

// TODO:
// - please move the identifier definitions in ovp_defines.h
// - please include your desciptor in ovp_main.cpp

#define OVP_ClassId_BoxAlgorithm_P300MagicCardVisualization     OpenViBE::CIdentifier(0x841F46EF, 0x471AA2A4)
#define OVP_ClassId_BoxAlgorithm_P300MagicCardVisualizationDesc OpenViBE::CIdentifier(0x37FAFF20, 0xA74685DB)

namespace TCPTagging
{
	class IStimulusSender; // fwd declare
};

namespace OpenViBEPlugins
{
	namespace SimpleVisualization
	{
		class CBoxAlgorithmP300MagicCardVisualization : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual bool initialize(void);
			virtual bool uninitialize(void);
			virtual bool processInput(OpenViBE::uint32 ui32Index);
			virtual bool process(void);

			void flushQueue(void);					// Sends all accumulated stimuli to the TCP Tagging
			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_P300MagicCardVisualization);

		private:

			typedef struct
			{
				int iIndex;
				::GdkColor oBackgroundColor;
				::GtkWidget* pParent;
				::GtkWidget* pWidget;
				::GtkWidget* pImage;
			} SWidgetStyle;

			typedef void (CBoxAlgorithmP300MagicCardVisualization::*_cache_callback_)(CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& rWidgetStyle, void* pUserData);

			void _cache_build_from_table_(::GtkTable* pTable);
			void _cache_for_each_(_cache_callback_ fpCallback, void* pUserData);
			void _cache_for_each_if_(int iCard, _cache_callback_ fpIfCallback, _cache_callback_ fpElseCallback, void* pIfUserData, void* pElseUserData);
			void _cache_change_null_cb_(CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _cache_change_image_cb_(CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& rWidgetStyle, void* pUserData);
			void _cache_change_background_cb_(CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& rWidgetStyle, void* pUserData);

		protected:

			OpenViBE::CString m_sInterfaceFilename;
			OpenViBE::uint64 m_ui64CardStimulationBase;

		private:

			OpenViBE::Kernel::IAlgorithmProxy* m_pSequenceStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pTargetStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pTargetFlaggingStimulationEncoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pCardSelectionStimulationDecoder;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pSequenceMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IMemoryBuffer*> ip_pTargetMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler<const OpenViBE::IStimulationSet*> ip_pTargetFlaggingStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pSequenceStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IStimulationSet*> op_pTargetStimulationSet;
			OpenViBE::Kernel::TParameterHandler<OpenViBE::IMemoryBuffer*> op_pTargetFlaggingMemoryBuffer;
			OpenViBE::uint64 m_ui64LastTime;

			::GtkBuilder* m_pMainWidgetInterface;
			::GtkBuilder* m_pToolbarWidgetInterface;
			::GtkWidget* m_pMainWindow;
			::GtkWidget* m_pToolbarWidget;
			::GtkTable* m_pTable;
			::GtkLabel* m_pResult;
			::GtkLabel* m_pTarget;
			::GdkColor m_oBackgroundColor;
			::GdkColor m_oTargetBackgroundColor;
			::GdkColor m_oSelectedBackgroundColor;
			OpenViBE::uint64 m_ui64TableRowCount;
			OpenViBE::uint64 m_ui64TableColumnCount;
			OpenViBE::uint64 m_ui64CardCount;

			int m_iTargetCard;

			std::vector < ::GtkWidget* > m_vForegroundImage;
			std::vector < ::GtkWidget* > m_vBackgroundImage;

			bool m_bTableInitialized;

			std::map < unsigned long, CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle > m_vCache;

			OpenViBEVisualizationToolkit::IVisualizationContext* m_visualizationContext = nullptr;

			// TCP Tagging
			std::vector< OpenViBE::uint64 > m_vStimuliQueue;
			guint m_uiIdleFuncTag;
			TCPTagging::IStimulusSender* m_pStimulusSender;
		};

		class CBoxAlgorithmP300MagicCardVisualizationDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("P300 Magic Card Visualization"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Yann Renard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INRIA"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Presents a matrix of images to the user in various ways"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Visualization/Presentation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-select-font"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_P300MagicCardVisualization; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SimpleVisualization::CBoxAlgorithmP300MagicCardVisualization; }

			virtual bool hasFunctionality(OpenViBE::CIdentifier functionalityIdentifier) const
			{
				return functionalityIdentifier == OVD_Functionality_Visualization;
			}

			virtual bool getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput ("Sequence stimulations",            OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Target stimulations",              OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addInput ("Card selection stimulations",      OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addOutput("Target / Non target flagging (deprecated)",     OV_TypeId_Stimulations);

				rBoxAlgorithmPrototype.addSetting("Interface filename",              OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card.ui");
				rBoxAlgorithmPrototype.addSetting("Background color",                OV_TypeId_Color,       "90,90,90");
				rBoxAlgorithmPrototype.addSetting("Target background color",         OV_TypeId_Color,       "10,40,10");
				rBoxAlgorithmPrototype.addSetting("Selected background color",       OV_TypeId_Color,       "70,20,20");
				rBoxAlgorithmPrototype.addSetting("Card stimulation base",           OV_TypeId_Stimulation, "OVTK_StimulationId_Label_01");
				rBoxAlgorithmPrototype.addSetting("TCP Tagging Host address",        OV_TypeId_String,      "localhost");
				rBoxAlgorithmPrototype.addSetting("TCP Tagging Host port",           OV_TypeId_Integer,     "15361");
				rBoxAlgorithmPrototype.addSetting("Default background filename",     OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/openvibe-logo.png-offscreen");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/01.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/02.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/03.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/04.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/05.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/06.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/07.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/08.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/09.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/10.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/11.png");
				rBoxAlgorithmPrototype.addSetting("Card filename",                   OV_TypeId_Filename,    "${Path_Data}/plugins/simple-visualization/p300-magic-card/12.png");

				return true;
			}

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_P300MagicCardVisualizationDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_P300MagicCardVisualization_H__
