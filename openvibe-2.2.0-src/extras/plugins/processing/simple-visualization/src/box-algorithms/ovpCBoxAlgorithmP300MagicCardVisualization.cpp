#include "ovpCBoxAlgorithmP300MagicCardVisualization.h"

#include <system/ovCMemory.h>

#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <tcptagging/IStimulusSender.h>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualization;

namespace
{
	class _AutoCast_
	{
	public:
		_AutoCast_(const IBox& rBox, IConfigurationManager& rConfigurationManager, const uint32 ui32Index)
			: m_rConfigurationManager(rConfigurationManager)
		{
			rBox.getSettingValue(ui32Index, m_sSettingValue);
			m_sSettingValue = m_rConfigurationManager.expand(m_sSettingValue);
		}
		operator ::GdkColor(void)
		{
			::GdkColor l_oColor;
			int r = 0, g = 0, b = 0;
			sscanf(m_sSettingValue.toASCIIString(), "%i,%i,%i", &r, &g, &b);
			l_oColor.pixel = 0;
			l_oColor.red = (r * 65535) / 100;
			l_oColor.green = (g * 65535) / 100;
			l_oColor.blue = (b * 65535) / 100;
			// std::cout << r << " " << g << " " << b << "\n";
			return l_oColor;
		}
	protected:
		IConfigurationManager& m_rConfigurationManager;
		CString m_sSettingValue;
	};
};

// This callback flushes all accumulated stimulations to the TCP Tagging 
// after the rendering has completed.
gboolean flush_callbackb(gpointer pUserData)
{
	((CBoxAlgorithmP300MagicCardVisualization*)pUserData)->flushQueue();

	return false;	// Only run once
}

boolean CBoxAlgorithmP300MagicCardVisualization::initialize(void)
{
	const IBox& l_rStaticBoxContext = this->getStaticBoxContext();

	m_pMainWidgetInterface= nullptr;
	m_pToolbarWidgetInterface= nullptr;

	m_sInterfaceFilename               = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
	m_oBackgroundColor                 = _AutoCast_(getStaticBoxContext(), getConfigurationManager(), 1);
	m_oTargetBackgroundColor           = _AutoCast_(getStaticBoxContext(), getConfigurationManager(), 2);
	m_oSelectedBackgroundColor         = _AutoCast_(getStaticBoxContext(), getConfigurationManager(), 3);
	m_ui64CardStimulationBase          = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 4);
	OpenViBE::CString l_sTCPTaggingHostAddress = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 5);
	OpenViBE::CString l_sTCPTaggingHostPort = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 6);	
	CString l_sBackgroundImageFilename = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 7);

	for(uint32 i=6; i<l_rStaticBoxContext.getSettingCount(); i++)
	{
		GError *l_pError = nullptr;

		CString l_sForegroundImageFilename=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), i);
		::GdkPixbuf* l_pTmp = gdk_pixbuf_new_from_file(l_sForegroundImageFilename.toASCIIString(), &l_pError);

 		::GtkWidget *l_pForegroundImage = gtk_image_new_from_pixbuf(gdk_pixbuf_scale_simple(l_pTmp,192,192,GDK_INTERP_BILINEAR));
		g_object_unref(l_pTmp);

		gtk_widget_show(l_pForegroundImage);
		g_object_ref(l_pForegroundImage);
		m_vForegroundImage.push_back(l_pForegroundImage);
		::GtkWidget* l_pBackgroundImage;
		if(l_sBackgroundImageFilename == CString(""))
		{
			l_pBackgroundImage=gtk_image_new_from_file((l_sForegroundImageFilename+CString("-offscreen")).toASCIIString());
		}
		else
		{
			l_pBackgroundImage=gtk_image_new_from_file(l_sBackgroundImageFilename.toASCIIString());
		}
		gtk_widget_show(l_pBackgroundImage);
		g_object_ref(l_pBackgroundImage);
		m_vBackgroundImage.push_back(l_pBackgroundImage);
	}

	// ----------------------------------------------------------------------------------------------------------------------------------------------------------

	m_pSequenceStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pSequenceStimulationDecoder->initialize();

	m_pTargetStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pTargetStimulationDecoder->initialize();

	m_pTargetFlaggingStimulationEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pTargetFlaggingStimulationEncoder->initialize();

	m_pCardSelectionStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pCardSelectionStimulationDecoder->initialize();

	ip_pSequenceMemoryBuffer.initialize(m_pSequenceStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pSequenceStimulationSet.initialize(m_pSequenceStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	ip_pTargetMemoryBuffer.initialize(m_pTargetStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pTargetStimulationSet.initialize(m_pTargetStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	ip_pTargetFlaggingStimulationSet.initialize(m_pTargetFlaggingStimulationEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
	op_pTargetFlaggingMemoryBuffer.initialize(m_pTargetFlaggingStimulationEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	m_ui64LastTime=0;

	m_pMainWidgetInterface=gtk_builder_new(); // glade_xml_new(m_sInterfaceFilename.toASCIIString(), "p300-magic-card-main", NULL);
	if(!gtk_builder_add_from_file(m_pMainWidgetInterface, m_sInterfaceFilename.toASCIIString(), NULL))
	{
		this->getLogManager() << LogLevel_ImportantWarning << "Could not load interface file [" << m_sInterfaceFilename << "]\n";
		this->getLogManager() << LogLevel_ImportantWarning << "The file may be missing. However, the interface files now use gtk-builder instead of glade. Did you update your files ?\n";
		return false;
	}

	m_pToolbarWidgetInterface=gtk_builder_new(); // glade_xml_new(m_sInterfaceFilename.toASCIIString(), "p300-magic-card-toolbar", NULL);
	gtk_builder_add_from_file(m_pToolbarWidgetInterface, m_sInterfaceFilename.toASCIIString(), NULL);

	m_pMainWindow=GTK_WIDGET(gtk_builder_get_object(m_pMainWidgetInterface, "p300-magic-card-main"));
	m_pToolbarWidget=GTK_WIDGET(gtk_builder_get_object(m_pToolbarWidgetInterface, "p300-magic-card-toolbar"));
	m_pTable=GTK_TABLE(gtk_builder_get_object(m_pMainWidgetInterface, "p300-magic-card-table"));
	gtk_widget_modify_bg(m_pMainWindow, GTK_STATE_NORMAL, &m_oBackgroundColor);

	gtk_builder_connect_signals(m_pMainWidgetInterface, NULL);
	gtk_builder_connect_signals(m_pToolbarWidgetInterface, NULL);

	m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
	m_visualizationContext->setWidget(*this, m_pMainWindow);
	m_visualizationContext->setToolbar(*this, m_pToolbarWidget);

	guint l_uiRowCount=0;
	guint l_uiColumnCount=0;
	g_object_get(m_pTable, "n-rows", &l_uiRowCount, NULL);
	g_object_get(m_pTable, "n-columns", &l_uiColumnCount, NULL);

	m_ui64TableRowCount=l_uiRowCount;
	m_ui64TableColumnCount=l_uiColumnCount;
	m_ui64CardCount=m_ui64TableRowCount*m_ui64TableColumnCount;
	m_iTargetCard=-1;

	m_bTableInitialized=false;
	this->_cache_build_from_table_(m_pTable);
	this->_cache_for_each_(&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_, &m_vForegroundImage);
	this->_cache_for_each_(&CBoxAlgorithmP300MagicCardVisualization::_cache_change_background_cb_, &m_oBackgroundColor);

	//TCP TAGGING
	m_uiIdleFuncTag = 0;
	m_vStimuliQueue.clear();
	m_pStimulusSender = TCPTagging::createStimulusSender();
	if (!m_pStimulusSender->connect("localhost", "15361"))
	{
		this->getLogManager() << LogLevel_Warning << "Unable to connect to AS's TCP Tagging plugin, stimuli wont be forwarded.\n";
	}


	return true;
}

boolean CBoxAlgorithmP300MagicCardVisualization::uninitialize(void)
{
	if(m_pToolbarWidgetInterface)
	{
		g_object_unref(m_pToolbarWidgetInterface);
		m_pToolbarWidgetInterface= nullptr;
	}

	if(m_pMainWidgetInterface)
	{
		g_object_unref(m_pMainWidgetInterface);
		m_pMainWidgetInterface= nullptr;
	}

	ip_pTargetFlaggingStimulationSet.uninitialize();
	op_pTargetFlaggingMemoryBuffer.uninitialize();

	op_pTargetStimulationSet.uninitialize();
	ip_pTargetMemoryBuffer.uninitialize();

	op_pSequenceStimulationSet.uninitialize();
	ip_pSequenceMemoryBuffer.uninitialize();

	if(m_pCardSelectionStimulationDecoder)
	{
		m_pCardSelectionStimulationDecoder->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pCardSelectionStimulationDecoder);
		m_pCardSelectionStimulationDecoder= nullptr;
	}

	if(m_pTargetFlaggingStimulationEncoder)
	{
		m_pTargetFlaggingStimulationEncoder->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pTargetFlaggingStimulationEncoder);
		m_pTargetFlaggingStimulationEncoder= nullptr;
	}

	if(m_pTargetStimulationDecoder)
	{
		m_pTargetStimulationDecoder->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pTargetStimulationDecoder);
		m_pTargetStimulationDecoder= nullptr;
	}

	if(m_pSequenceStimulationDecoder)
	{
		m_pSequenceStimulationDecoder->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pSequenceStimulationDecoder);
		m_pSequenceStimulationDecoder= nullptr;
	}

	if(m_visualizationContext)
	{
		this->releasePluginObject(m_visualizationContext);
		m_visualizationContext = nullptr;
	}


	//TCP TAGGING
	m_vStimuliQueue.clear();
	if (m_pStimulusSender)
	{
		delete m_pStimulusSender;
		m_pStimulusSender = NULL;
	}
	return true;
}

boolean CBoxAlgorithmP300MagicCardVisualization::processInput(uint32 ui32Index)
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	if(!m_bTableInitialized)
	{
		this->_cache_for_each_(&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_, &m_vForegroundImage);
		m_bTableInitialized=true;
	}

	return true;
}

boolean CBoxAlgorithmP300MagicCardVisualization::process(void)
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	uint32 i, j;

	// --- Sequence stimulations

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		CStimulationSet l_oFlaggingStimulationSet;

		ip_pSequenceMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		ip_pTargetFlaggingStimulationSet=&l_oFlaggingStimulationSet;
		op_pTargetFlaggingMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);

		m_pSequenceStimulationDecoder->process();

		m_ui64LastTime=l_rDynamicBoxContext.getInputChunkEndTime(0, i);

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			m_pTargetFlaggingStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		}

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			IStimulationSet* l_pStimulationSet=op_pSequenceStimulationSet;
			for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
			{
				uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
				if(l_ui64StimulationIdentifier >= m_ui64CardStimulationBase && l_ui64StimulationIdentifier < m_ui64CardStimulationBase+m_ui64CardCount)
				{
					int l_iCard=(int)(l_ui64StimulationIdentifier-m_ui64CardStimulationBase);
					if(l_iCard==m_iTargetCard)
					{
						m_vStimuliQueue.push_back(OVTK_StimulationId_Target);
						l_oFlaggingStimulationSet.appendStimulation(OVTK_StimulationId_Target, l_pStimulationSet->getStimulationDate(j), 0);
					}
					else
					{
						m_vStimuliQueue.push_back(OVTK_StimulationId_NonTarget);
						l_oFlaggingStimulationSet.appendStimulation(OVTK_StimulationId_NonTarget, l_pStimulationSet->getStimulationDate(j), 0);
					}

					this->_cache_for_each_if_(
						l_iCard,
						&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_,
						&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_,
						&m_vForegroundImage,
						&m_vBackgroundImage);
					this->_cache_for_each_(
						&CBoxAlgorithmP300MagicCardVisualization::_cache_change_background_cb_,
						&m_oBackgroundColor);
				}
				if(l_ui64StimulationIdentifier == OVTK_StimulationId_ExperimentStart)
				{
					this->getLogManager() << LogLevel_Debug << "Received OVTK_StimulationId_ExperimentStart - resets grid\n";
					this->_cache_for_each_(&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_, &m_vBackgroundImage);
				}
				if(l_ui64StimulationIdentifier == OVTK_StimulationId_VisualStimulationStop)
				{
					this->getLogManager() << LogLevel_Debug << "Received OVTK_StimulationId_VisualStimulationStop - resets grid\n";
					this->_cache_for_each_(&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_, &m_vBackgroundImage);
				}
				// Pass the stimulation to the server also as-is. If its a flash, it can be differentiated from a 'target' spec because
				// its NOT between OVTK_StimulationId_RestStart and OVTK_StimulationId_RestStop stimuli in the generated P300 timeline.
				m_vStimuliQueue.push_back(l_ui64StimulationIdentifier);
			}

			m_pTargetFlaggingStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
		}

		if(m_pSequenceStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			m_pTargetFlaggingStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeEnd);
		}

		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
	}

	// --- Target stimulations

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(1); i++)
	{
		if(m_ui64LastTime>=l_rDynamicBoxContext.getInputChunkStartTime(1, i))
		{
			ip_pTargetMemoryBuffer=l_rDynamicBoxContext.getInputChunk(1, i);
			m_pTargetStimulationDecoder->process();

			if(m_pTargetStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
			{
			}

			if(m_pTargetStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				IStimulationSet* l_pStimulationSet=op_pTargetStimulationSet;
				for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
				{
					uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
					if(l_ui64StimulationIdentifier >= m_ui64CardStimulationBase && l_ui64StimulationIdentifier < m_ui64CardStimulationBase+m_ui64CardCount)
					{
						this->getLogManager() << LogLevel_Debug << "Received Target Card " << l_ui64StimulationIdentifier << "\n";
						m_iTargetCard=(int)(l_ui64StimulationIdentifier-m_ui64CardStimulationBase);

						this->getLogManager() << LogLevel_Debug << "Displays Target Cell\n";
						this->_cache_for_each_if_(
							m_iTargetCard,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_null_cb_,
							&m_vForegroundImage,
							NULL);
						this->_cache_for_each_if_(
							m_iTargetCard,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_background_cb_,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_null_cb_,
							&m_oTargetBackgroundColor,
							NULL);

						// Merge the current target into the stimulation stream. It can be differentiated
						// from a 'flash' spec because it IS between OVTK_StimulationId_RestStart and
						// OVTK_StimulationId_RestStop stimulations in the P300 timeline.
						{
							//or just l_ui64StimulationIdentifier
							m_vStimuliQueue.push_back(m_iTargetCard + m_ui64CardStimulationBase);
						}

					}
				}
			}

			if(m_pTargetStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
			}

			l_rDynamicBoxContext.markInputAsDeprecated(1, i);
		}
	}

	// --- Selection stimulations

	TParameterHandler < const IMemoryBuffer* > ip_pSelectionMemoryBuffer(m_pCardSelectionStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	TParameterHandler < IStimulationSet* > op_pSelectionStimulationSet(m_pCardSelectionStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(2); i++)
	{
		if(m_ui64LastTime>=l_rDynamicBoxContext.getInputChunkStartTime(2, i))
		{
			ip_pSelectionMemoryBuffer=l_rDynamicBoxContext.getInputChunk(2, i);
			m_pCardSelectionStimulationDecoder->process();

			if(m_pCardSelectionStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
			{
			}

			if(m_pCardSelectionStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
			{
				IStimulationSet* l_pStimulationSet=op_pSelectionStimulationSet;
				for(j=0; j<l_pStimulationSet->getStimulationCount(); j++)
				{
					uint64 l_ui64StimulationIdentifier=l_pStimulationSet->getStimulationIdentifier(j);
					if(l_ui64StimulationIdentifier >= m_ui64CardStimulationBase && l_ui64StimulationIdentifier < m_ui64CardStimulationBase+m_ui64CardCount)
					{
						this->getLogManager() << LogLevel_Debug << "Received Selected Card " << l_ui64StimulationIdentifier << "\n";
						int l_iSelectedCard=(int)(l_ui64StimulationIdentifier-m_ui64CardStimulationBase);

						this->getLogManager() << LogLevel_Debug << "Displays Selected Cell\n";

						this->_cache_for_each_if_(
							l_iSelectedCard,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_null_cb_,
							&m_vForegroundImage,
							NULL);
						this->_cache_for_each_if_(
							l_iSelectedCard,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_background_cb_,
							&CBoxAlgorithmP300MagicCardVisualization::_cache_change_null_cb_,
							&m_oSelectedBackgroundColor,
							NULL);
					}
					if(l_ui64StimulationIdentifier == OVTK_StimulationId_Label_00)
					{
						this->getLogManager() << LogLevel_Trace << "Selection Rejected !\n";
						std::string l_sString;
						l_sString=gtk_label_get_text(m_pResult);
						l_sString+="*";
						gtk_label_set_text(m_pResult, l_sString.c_str());
					}
				}
			}

			if(m_pCardSelectionStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
			{
			}

			l_rDynamicBoxContext.markInputAsDeprecated(2, i);
		}
	}

	// After any possible rendering, we flush the accumulated stimuli. The default idle func is low priority, so it should be run after rendering by gtk.
	if (m_uiIdleFuncTag == 0)
	{
		m_uiIdleFuncTag = g_idle_add(flush_callbackb, this);
	}

	return true;
}

// _________________________________________________________________________________________________________________________________________________________
//

void CBoxAlgorithmP300MagicCardVisualization::_cache_build_from_table_(::GtkTable* pTable)
{
	if(pTable)
	{
		::GtkTableChild* l_pTableChild= nullptr;
		::GList* l_pList= nullptr;
		for(l_pList=pTable->children; l_pList; l_pList=l_pList->next)
		{
			l_pTableChild=(::GtkTableChild*)l_pList->data;

			for(unsigned long i=l_pTableChild->top_attach; i<l_pTableChild->bottom_attach; i++)
			{
				for(unsigned long j=l_pTableChild->left_attach; j<l_pTableChild->right_attach; j++)
				{
					int l_iIndex=(int)(i*m_ui64TableColumnCount+j);
					CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& l_rWidgetStyle=m_vCache[l_iIndex];
					l_rWidgetStyle.iIndex=l_iIndex;
					l_rWidgetStyle.pParent=l_pTableChild->widget;
					l_rWidgetStyle.pWidget=gtk_bin_get_child(GTK_BIN(l_rWidgetStyle.pParent));
					l_rWidgetStyle.pImage=gtk_bin_get_child(GTK_BIN(l_rWidgetStyle.pWidget));
				}
			}
		}
	}
}

void CBoxAlgorithmP300MagicCardVisualization::_cache_for_each_(_cache_callback_ fpCallback, void* pUserData)
{
	std::map < unsigned long, CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle >::iterator i;

	for(i=m_vCache.begin(); i!=m_vCache.end(); i++)
	{
		(this->*fpCallback)(i->second, pUserData);
	}
}

void CBoxAlgorithmP300MagicCardVisualization::_cache_for_each_if_(int iCard, _cache_callback_ fpIfCallback, _cache_callback_ fpElseCallback, void* pIfUserData, void* pElseUserData)
{
	std::map < unsigned long, CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle >::iterator i;

	for(i=m_vCache.begin(); i!=m_vCache.end(); i++)
	{
		if(iCard==i->second.iIndex)
		{
			(this->*fpIfCallback)(i->second, pIfUserData);
		}
		else
		{
			(this->*fpElseCallback)(i->second, pElseUserData);
		}
	}
}

void CBoxAlgorithmP300MagicCardVisualization::_cache_change_null_cb_(CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& rWidgetStyle, void* pUserData)
{
}

void CBoxAlgorithmP300MagicCardVisualization::_cache_change_image_cb_(CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GtkContainer* m_pContainer=GTK_CONTAINER(rWidgetStyle.pWidget);
	std::vector < ::GtkWidget* > * l_pvImage=(std::vector < ::GtkWidget* > *) pUserData;

	::GtkWidget* l_pImage=(*l_pvImage)[rWidgetStyle.iIndex];

	if(rWidgetStyle.pImage!=l_pImage)
	{
		if(rWidgetStyle.pImage)
		{
			gtk_container_remove(m_pContainer, rWidgetStyle.pImage);
		}
		gtk_container_add(m_pContainer, l_pImage);
		rWidgetStyle.pImage=l_pImage;
	}
}

void CBoxAlgorithmP300MagicCardVisualization::_cache_change_background_cb_(CBoxAlgorithmP300MagicCardVisualization::SWidgetStyle& rWidgetStyle, void* pUserData)
{
	::GdkColor oColor=*(::GdkColor*)pUserData;
	if(!System::Memory::compare(&rWidgetStyle.oBackgroundColor, &oColor, sizeof(::GdkColor)))
	{
		gtk_widget_modify_bg(rWidgetStyle.pParent, GTK_STATE_NORMAL, &oColor);
		gtk_widget_modify_bg(rWidgetStyle.pWidget, GTK_STATE_NORMAL, &oColor);
		gtk_widget_modify_bg(rWidgetStyle.pImage, GTK_STATE_NORMAL, &oColor);
		rWidgetStyle.oBackgroundColor=oColor;
	}
}

// Note that we don't need concurrency control here as gtk callbacks run in the main thread
void CBoxAlgorithmP300MagicCardVisualization::flushQueue(void)
{
	for (size_t i = 0; i<m_vStimuliQueue.size(); i++)
	{
		m_pStimulusSender->sendStimulation(m_vStimuliQueue[i]);
	}
	m_vStimuliQueue.clear();

	// This function will be automatically removed after completion, so set to 0
	m_uiIdleFuncTag = 0;
}