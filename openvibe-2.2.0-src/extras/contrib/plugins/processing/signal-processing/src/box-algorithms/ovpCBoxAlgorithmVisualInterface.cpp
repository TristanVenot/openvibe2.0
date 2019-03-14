#include "ovpCBoxAlgorithmVisualInterface.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;


#include <iostream>
#include <fstream>
#include <cstdlib>

#include <sys/timeb.h>

#include <tcptagging/IStimulusSender.h>

namespace OpenViBEPlugins {

	namespace SignalProcessing
	{


		//Size allocation of the window
		gboolean VisualInterface_SizeAllocateCallback(GtkWidget *widget, GtkAllocation *allocation, gpointer data)
		{
			reinterpret_cast<CBoxAlgorithmVisualInterface*>(data)->resize((uint32)allocation->width, (uint32)allocation->height);
			return FALSE;
		}
		//Call for a drawing
		gboolean VisualInterface_RedrawCallback(GtkWidget *widget, GdkEventExpose *event, gpointer data)
		{
			reinterpret_cast<CBoxAlgorithmVisualInterface*>(data)->redraw();
			return TRUE;
		}

		//Construcor

		CBoxAlgorithmVisualInterface::CBoxAlgorithmVisualInterface(void) :
			m_pBuilderInterface(NULL),
			m_pMainWindow(NULL),
			m_pDrawingArea(NULL),
			m_bFullScreen(false),
			m_pStimulusSender(nullptr),
			m_visualizationContext(nullptr)
		{
			m_oBackgroundColor.pixel = 0;
			m_oBackgroundColor.red = 0;
			m_oBackgroundColor.green = 0;
			m_oBackgroundColor.blue = 0;

			m_oForegroundColor.pixel = 0;
			m_oForegroundColor.red = 0xFFFF;
			m_oForegroundColor.green = 0xFFFF;
			m_oForegroundColor.blue = 0xFFFF;
		}

		bool CBoxAlgorithmVisualInterface::initialize(void)
		{	
			m_uiIdleFuncTag = 0;
			m_pStimulusSender = nullptr;
			m_bFullScreen = FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);


			m_oInput0Decoder.initialize(*this, 0);
			m_oInput1Decoder.initialize(*this, 0);


			//load the gtk builder interface
			m_pBuilderInterface = gtk_builder_new();

			if (!m_pBuilderInterface)
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Couldn't load the interface !";
				return false;
			}

			const CString l_sUIFile = OpenViBE::Directories::getDataDir() + "/plugins/signal-processing/openvibe-simple-visualization-VisualInterface.ui";
			if (!gtk_builder_add_from_file(m_pBuilderInterface, l_sUIFile, NULL))
			{
				getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Error << "Could not load the .ui file " << l_sUIFile << "\n";
				return false;
			}

			gtk_builder_connect_signals(m_pBuilderInterface, NULL);

			m_pDrawingArea = GTK_WIDGET(gtk_builder_get_object(m_pBuilderInterface, "VisualInterfaceDrawingArea"));
			g_signal_connect(G_OBJECT(m_pDrawingArea), "expose_event", G_CALLBACK(VisualInterface_RedrawCallback), this);
			g_signal_connect(G_OBJECT(m_pDrawingArea), "size-allocate", G_CALLBACK(VisualInterface_SizeAllocateCallback), this);

			//set widget bg color
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oBackgroundColor);
			gtk_widget_modify_bg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oBackgroundColor);

			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_NORMAL, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_PRELIGHT, &m_oForegroundColor);
			gtk_widget_modify_fg(m_pDrawingArea, GTK_STATE_ACTIVE, &m_oForegroundColor);



			m_pStimulusSender = TCPTagging::createStimulusSender();

			if (!m_pStimulusSender->connect("localhost", "15361"))
			{
				this->getLogManager() << LogLevel_Warning << "Unable to connect to AS's TCP Tagging plugin, stimuli wont be forwarded.\n";
			}



			if (m_bFullScreen)
			{
				GtkWidget *l_pWindow = gtk_widget_get_toplevel(m_pDrawingArea);
				gtk_window_fullscreen(GTK_WINDOW(l_pWindow));
				gtk_widget_show(l_pWindow);

				// @fixme small mem leak?
				GdkCursor* l_pCursor = gdk_cursor_new(GDK_BLANK_CURSOR);
				gdk_window_set_cursor(gtk_widget_get_window(l_pWindow), l_pCursor);
			}
			else
			{
				m_visualizationContext = dynamic_cast<OpenViBEVisualizationToolkit::IVisualizationContext*>(this->createPluginObject(OVP_ClassId_Plugin_VisualizationContext));
				m_visualizationContext->setWidget(*this, m_pDrawingArea);
			}

			// Invalidate the drawing area in order to get the image resize already called at this point. The actual run will be smoother.
			if (GTK_WIDGET(m_pDrawingArea)->window)
			{
				gdk_window_invalidate_rect(GTK_WIDGET(m_pDrawingArea)->window, NULL, true);
			}

			return true;
		}
		/*******************************************************************************/

		bool CBoxAlgorithmVisualInterface::uninitialize(void)
		{	
			// Remove the possibly dangling idle loop. 
			if (m_uiIdleFuncTag)
			{
				g_source_remove(m_uiIdleFuncTag);
				m_uiIdleFuncTag = 0;
			}
			m_oInput0Decoder.uninitialize();
			m_oInput1Decoder.uninitialize();

			if (m_pStimulusSender)
			{
				delete m_pStimulusSender;
				m_pStimulusSender = nullptr;
			}
			

			// Close the full screen
			if (m_bFullScreen)
			{
				GtkWidget *l_pWindow = gtk_widget_get_toplevel(m_pDrawingArea);
				gtk_window_unfullscreen(GTK_WINDOW(l_pWindow));
				gtk_widget_destroy(l_pWindow);
			}

			//destroy drawing area
			if (m_pDrawingArea)
			{
				gtk_widget_destroy(m_pDrawingArea);
				m_pDrawingArea = nullptr;
			}

			// unref the xml file as it's not needed anymore
			if (m_pBuilderInterface)
			{
				g_object_unref(G_OBJECT(m_pBuilderInterface));
				m_pBuilderInterface = nullptr;
			}

			return true;
		}
		/*******************************************************************************/




		bool CBoxAlgorithmVisualInterface::process(void)
		{

			// the static box context describes the box inputs, outputs, settings structures
			const IBox& l_rStaticBoxContext = this->getStaticBoxContext();
			// the dynamic box context describes the current state of the box inputs and outputs (i.e. the chunks)
			IBoxIO& l_rDynamicBoxContext = this->getDynamicBoxContext();

			IBoxIO* l_pBoxIO = getBoxAlgorithmContext()->getDynamicBoxContext();

			// We decode and save the received stimulations.
			for (uint32 input = 0; input < getBoxAlgorithmContext()->getStaticBoxContext()->getInputCount(); input++)
			{
				for (uint32 chunk = 0; chunk < l_pBoxIO->getInputChunkCount(input); chunk++)
				{
					m_oInput0Decoder.decode(chunk, true);
					if (m_oInput0Decoder.isHeaderReceived())
					{
						// nop
					}
					if (m_oInput0Decoder.isBufferReceived())
					{
						for (uint32 stim = 0; stim < m_oInput0Decoder.getOutputStimulationSet()->getStimulationCount(); stim++)
						{
							// We always add the stimulations to the set to allow passing them to TCP Tagging in order in processClock()
							const uint64 l_ui64StimID = m_oInput0Decoder.getOutputStimulationSet()->getStimulationIdentifier(stim);
							const uint64 l_ui64StimDate = m_oInput0Decoder.getOutputStimulationSet()->getStimulationDate(stim);
							const uint64 l_ui64StimDuration = m_oInput0Decoder.getOutputStimulationSet()->getStimulationDuration(stim);

							const uint64 l_ui64Time = this->getPlayerContext().getCurrentTime();
							if (l_ui64StimDate < l_ui64Time)
							{
								const float64 l_f64Delay = ITimeArithmetics::timeToSeconds(l_ui64Time - l_ui64StimDate) * 1000; //delay in ms
								if (l_f64Delay>50)
									getBoxAlgorithmContext()->getPlayerContext()->getLogManager() << LogLevel_Warning << "Stimulation " << l_ui64StimID << " was received late: " << l_f64Delay << " ms \n";
							}

							if (l_ui64StimDate < l_pBoxIO->getInputChunkStartTime(input, chunk))
							{
								this->getLogManager() << LogLevel_ImportantWarning << "Input Stimulation Date before beginning of the buffer\n";
							}

							m_oPendingStimulationSet.appendStimulation(
								l_ui64StimID,
								l_ui64StimDate,
								l_ui64StimDuration);
						}
					}
				}
			}


			// Tutorials:
			// http://openvibe.inria.fr/documentation/#Developer+Documentation
			// Codec Toolkit page :
			// http://openvibe.inria.fr/codec-toolkit-references/

			// Feel free to ask experienced developers on the forum (http://openvibe.inria.fr/forum) and IRC (#openvibe on irc.freenode.net).

			return true;
		}
		void CBoxAlgorithmVisualInterface::resize(uint32 ui32Width, uint32 ui32Height)
		{
		
			ui32Width = (ui32Width<8 ? 8 : ui32Width);
			ui32Height = (ui32Height<8 ? 8 : ui32Height);
		
		
		
		}

		void CBoxAlgorithmVisualInterface::redraw()
		{
			gint l_iWindowWidth = m_pDrawingArea->allocation.width;
			gint l_iWindowHeight = m_pDrawingArea->allocation.height;

			//increase line's width
			gdk_gc_set_line_attributes(m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)], 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_BEVEL);

			//horizontal line
			gdk_draw_line(m_pDrawingArea->window,
				m_pDrawingArea->style->fg_gc[GTK_WIDGET_STATE(m_pDrawingArea)],
				(l_iWindowWidth / 4), (l_iWindowHeight / 2),
				((3 * l_iWindowWidth) / 4), (l_iWindowHeight / 2)
				);
			
		}

		void CBoxAlgorithmVisualInterface::activate()
		{
			

		}
	};
};
