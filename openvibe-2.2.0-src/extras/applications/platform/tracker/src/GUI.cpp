//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 
// @todo connect all the vertical and horizontal sliders and scales
// @todo make horizontal scale related to seconds rather than chunks
// @todo add rulers
// @todo need a much more clear design to handle multiple tracks with multiple streams of different sizes

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>
#include <iomanip>
#include <clocale> // std::setlocale
#include <string.h> // strchr on Ubuntu
#include <algorithm>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <fs/Files.h>
#include <system/ovCMath.h>
#include <system/ovCTime.h>

#include "GUI.h"

#include "Stream.h"
#include "StreamBundle.h"
#include "StreamRendererFactory.h"

#include "TypeError.h"

using namespace std;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

// These take in a string identidifer as MENUCHOICE to be given to builder. Return FALSE from callbacks so they don't block further signal handlers.
#define GTK_CALLBACK_MAPPER(MENUCHOICE, ACTION, MEMBERFUN) \
	auto MEMBERFUN = [](::GtkWidget* pMenuItem, gpointer pUserData) -> gboolean { static_cast<GUI*>(pUserData)->MEMBERFUN(); return FALSE; }; \
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pInterface, MENUCHOICE)), \
		ACTION, G_CALLBACK((gboolean(*)(::GtkWidget* pMenuItem, gpointer pUserData)) ( MEMBERFUN ) ), this);
#define GTK_CALLBACK_MAPPER_PARAM(MENUCHOICE, ACTION, MEMBERFUN) \
	auto MEMBERFUN = [](::GtkWidget* pMenuItem, gpointer pUserData) -> gboolean { static_cast<GUI*>(pUserData)->MEMBERFUN(pMenuItem); return FALSE; }; \
	g_signal_connect(G_OBJECT(gtk_builder_get_object(m_pInterface, MENUCHOICE)), \
		ACTION, G_CALLBACK((gboolean(*)(::GtkWidget* pMenuItem, gpointer pUserData)) ( MEMBERFUN ) ), this);
// These work on gtk objects directly, used when menus are constructed in code, not in .ui file
#define GTK_CALLBACK_MAPPER_OBJECT(MENUOBJECT, ACTION, MEMBERFUN) \
	auto MEMBERFUN = [](::GtkWidget* pMenuItem, gpointer pUserData) -> gboolean { static_cast<GUI*>(pUserData)->MEMBERFUN(); return FALSE; }; \
	g_signal_connect(GTK_OBJECT(MENUOBJECT), \
	ACTION, G_CALLBACK((gboolean(*)(::GtkWidget* pMenuItem, gpointer pUserData)) MEMBERFUN), this);
#define GTK_CALLBACK_MAPPER_OBJECT_PARAM(MENUOBJECT, ACTION, MEMBERFUN) \
	auto MEMBERFUN = [](::GtkWidget* pMenuItem, gpointer pUserData) -> gboolean { static_cast<GUI*>(pUserData)->MEMBERFUN(pMenuItem); return FALSE; }; \
	g_signal_connect(GTK_OBJECT(MENUOBJECT), \
		ACTION, G_CALLBACK((gboolean(*)(::GtkWidget* pMenuItem, gpointer pUserData)) MEMBERFUN), this);

GUI::~GUI() 
{ 	
	clearRenderers();

	if (m_pLogListener)
	{
		getLogManager().removeListener(m_pLogListener);
		delete m_pLogListener;
	}
}

GUI::GUI(int argc, char* argv[], Tracker& app) : Contexted(app.getKernelContext()), m_rTracker(app)
{
	gtk_init(&argc, &argv);

	// We rely on this with 64bit/gtk 2.24, to roll back gtk_init() sometimes switching
	// the locale to one where ',' is needed instead of '.' for separators of floats, 
	// causing issues, for example getConfigurationManager.expandAsFloat("0.05") -> 0; 
	// due to implementation by std::stod().
	std::setlocale(LC_ALL, "C");

	m_pInterface=gtk_builder_new(); 
	const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/tracker/tracker.ui";
	if(!gtk_builder_add_from_file(m_pInterface, l_sFilename, NULL)) {
		std::cout << "Problem loading [" << l_sFilename << "]\n";
		return;
	}

	m_hMainWindow=GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker"));

	// Tricky getting the quit handler properly in this case, note the different CB params.
	auto quitFun = [](::GtkWidget* pMenuItem, GdkEventKey *e, gpointer pUserData) { return (gboolean)static_cast<GUI*>(pUserData)->quitCB(); };
	typedef gboolean quitHandler_t(::GtkWidget* pMenuItem, GdkEventKey *e, gpointer pUserData);
	g_signal_connect(G_OBJECT(m_hMainWindow),
		"delete_event", G_CALLBACK( (quitHandler_t*)quitFun), this);

	m_hScrollbar = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "hscrollbar1"));
	m_hScale = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "hscale1"));
	m_hTimeDisplay = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-label_current_time"));
	m_hTrackCounter = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-label_trackno"));
	m_hWorkspaceLabel = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-workspace_label"));

	gtk_range_set_range(GTK_RANGE(m_hScale), 1, 60*60); // from 1 sec to 1 hour

	gtk_builder_connect_signals(m_pInterface, NULL);

	auto idleFun = [](gpointer pUserData) -> gboolean
	{ 
		static_cast<GUI*>(pUserData)->step(); 
		return TRUE; 
	};
	g_idle_add(idleFun, this);

	// Register callbacks only after we've done all the initializations

	// Menu callbacks
	GTK_CALLBACK_MAPPER("tracker-menu_open_file", "activate", addTrackCB);
	GTK_CALLBACK_MAPPER("tracker-menu_open_workspace", "activate", openWorkspaceCB);
	GTK_CALLBACK_MAPPER("tracker-menu_open_processor", "activate", openProcessorCBFinal);
	GTK_CALLBACK_MAPPER("tracker-menu_clear", "activate", clearCB);
	GTK_CALLBACK_MAPPER("tracker-menu_save", "activate", saveCB);
	GTK_CALLBACK_MAPPER("tracker-menu_saveas", "activate", saveAsCB);
	GTK_CALLBACK_MAPPER("tracker-menu_incrementrevsave", "activate", incrementRevSaveCB);
	GTK_CALLBACK_MAPPER("tracker-menu_quit", "activate", quitCB);

	GTK_CALLBACK_MAPPER("tracker-menu_select", "activate", editSelectionCB);
	GTK_CALLBACK_MAPPER("tracker-menu_select_all", "activate", selectAllCB);
	GTK_CALLBACK_MAPPER("tracker-menu_select_none", "activate", selectNoneCB);

	GTK_CALLBACK_MAPPER("tracker-menu_processor_preferences", "activate", processorPreferencesCB);
	GTK_CALLBACK_MAPPER("tracker-menu_workspace_notes", "activate", editNotesCB);
	GTK_CALLBACK_MAPPER("tracker-menu_delete_all", "activate", deleteAllTracksCB);

	GTK_CALLBACK_MAPPER("tracker-menu_workspace_about", "activate", workspaceInfoCB);
	GTK_CALLBACK_MAPPER("tracker-menu_about", "activate", aboutCB);

	GTK_CALLBACK_MAPPER_PARAM("hscrollbar1", "value-changed", hScrollCB);
	GTK_CALLBACK_MAPPER_PARAM("hscale1", "value-changed", hScaleCB);

	// Buttons
	GTK_CALLBACK_MAPPER("tracker-button_notes", "clicked", editNotesCB2);
	GTK_CALLBACK_MAPPER("tracker-button_play_pause", "clicked", playCB);
	GTK_CALLBACK_MAPPER("tracker-button_forward", "clicked", playFastCB);
	GTK_CALLBACK_MAPPER("tracker-button_stop", "clicked", stopCB);

	GTK_CALLBACK_MAPPER("tracker-select_tracks", "clicked", editSelectionCB2);
	GTK_CALLBACK_MAPPER("tracker-processor_properties", "clicked", processorEditCB);

	GTK_CALLBACK_MAPPER("tracker-processor_choose", "clicked", openProcessorCB);
	GTK_CALLBACK_MAPPER("tracker-processor_edit2", "clicked", processorEditCB2);
	GTK_CALLBACK_MAPPER("processorpreferences-button_ok", "clicked", processorPreferencesButtonOkCB);
	GTK_CALLBACK_MAPPER("processorpreferences-button_cancel", "clicked", processorPreferencesButtonCancelCB);
	GTK_CALLBACK_MAPPER("trackerpreferences-workspace_path-choose", "clicked", selectWorkspacePathCB);


	// Attach a log listener
	m_pLogListener = new OpenViBETracker::CLogListenerTracker(getKernelContext(), m_pInterface);
//	m_pLogListener->m_CenterOnBoxFun = [this](CIdentifier& id) { this->getCurrentInterfacedScenario()->centerOnBox(id); };
	getLogManager().addListener(m_pLogListener);
	GTK_CALLBACK_MAPPER("openvibe-messages_tb_clear", "clicked", clearMessagesCB);
	// Enable the default buttons
	GtkWidget* toggleButtonActive_Info = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "openvibe-messages_tb_info"));
	GtkWidget* toggleButtonActive_Warning = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "openvibe-messages_tb_warning"));
	GtkWidget* toggleButtonActive_ImportantWarning = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "openvibe-messages_tb_impwarning"));
	GtkWidget* toggleButtonActive_Error = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "openvibe-messages_tb_error"));
	GtkWidget* toggleButtonActive_Fatal = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "openvibe-messages_tb_fatal"));
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toggleButtonActive_Info), true);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toggleButtonActive_Warning), true);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toggleButtonActive_ImportantWarning), true);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toggleButtonActive_Error), true);
	gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toggleButtonActive_Fatal), true);

	std::string title(gtk_window_get_title(GTK_WINDOW(m_hMainWindow)));
	title += " v" + std::string(OV_VERSION_MAJOR) + "." + std::string(OV_VERSION_MINOR) + "." + std::string(OV_VERSION_PATCH);
#if defined(TARGET_ARCHITECTURE_x64)
	title += " (64bit)";
;
#else
	title += " (32bit)";
#endif
	gtk_window_set_title(GTK_WINDOW(m_hMainWindow), title.c_str());

	gtk_widget_show(m_hMainWindow);

	return;
}

bool GUI::clearRenderers(void)
{
	::GtkWidget* l_pTrackTable = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "table_tracks"));

	for (auto trk : m_Tracks)
	{
		// To avoid issues, we delete everything first before detaching the widget
		GtkWidget* frame = trk->m_Frame;

		delete trk;

		if(frame)
		{
			gtk_container_remove(GTK_CONTAINER(l_pTrackTable), frame);
		}
	}

	m_Tracks.clear();

	return true;
}


bool GUI::initializeRenderers(void)
{
	clearRenderers();
 
	const size_t numTracks = m_rTracker.getWorkspace().getNumTracks();

	if (numTracks == 0)
	{
		addTracksToMenu();
		resetWidgetProperties();
		m_RequestRedraw = true;
		return true;
	}

	const bool showSelectedOnly = m_KernelContext.getConfigurationManager().expandAsBoolean("${Tracker_Workspace_GUI_ShowSelectedOnly}", false);
	const uint32_t maxRendererCount = (uint32_t) m_KernelContext.getConfigurationManager().expandAsUInteger("${Tracker_Workspace_GUI_MaxRendererCount}", 32);
	std::stringstream ss; ss << maxRendererCount;
	m_KernelContext.getConfigurationManager().addOrReplaceConfigurationToken("Tracker_Workspace_GUI_MaxRendererCount", ss.str().c_str()); // If it didn't exist


	::GtkWidget* l_pTrackTable = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "table_tracks"));

	gtk_table_resize(GTK_TABLE(l_pTrackTable), numTracks, 1);

	uint32_t renderersAllocated = 0;

	for (size_t i = 0; i < numTracks; i++)
	{
		const StreamBundle* bundle = m_rTracker.getWorkspace().getTrack(i);

		const size_t numStreams = (bundle ? bundle->getNumStreams() : 1);

		GUITrack* track = new GUITrack();

		GtkWidget* streamTable = gtk_table_new(1, 2, false);

		m_Tracks.push_back(track);

		if (numStreams == 0)
		{
			std::cout << "Track " << i+1 << " has no streams\n";
			continue;
		}

		std::stringstream title;
		title << "Track " << (i + 1) << " : " << (bundle ? bundle->getSource() : "Unknown")
			<< (bundle->getDirtyBit() ? " (unsaved)" : " (on disk)");

		if(!showSelectedOnly || m_rTracker.getWorkspace().getSelection().countSelectedStreams(i) > 0)
		{
			track->m_Frame = gtk_frame_new(title.str().c_str());

			gtk_container_add(GTK_CONTAINER(track->m_Frame), streamTable);

			gtk_table_attach_defaults(GTK_TABLE(l_pTrackTable), track->m_Frame, 0, 1, i, i + 1);

			gtk_table_set_row_spacing(GTK_TABLE(l_pTrackTable), i, 0);

			gtk_table_resize(GTK_TABLE(streamTable), numStreams, 1);

		}

		for (size_t j = 0; j < numStreams; j++)
		{
			StreamPtrConst stream = (bundle ? bundle->getStream(j) : nullptr);
			CIdentifier typeIdentifier = OV_UndefinedIdentifier;

			if (stream)
			{
				typeIdentifier = stream->getTypeIdentifier();
			}
			else
			{
				std::cout << "Stream " << j << " is null, using label...\n";
			}

			log() << LogLevel_Debug << "Building renderer for track " << i+1 << " stream " << j+1 << "\n";

			StreamRendererBase* renderer;
			if(showSelectedOnly && !stream->getSelected())
			{
				renderer = StreamRendererFactory::getDummyRenderer(getKernelContext());
			}
			else if (m_rTracker.getWorkspace().getMemorySaveMode() || renderersAllocated>=maxRendererCount)
			{
				StreamPtr empty = std::make_shared< Stream<TypeError> > (m_KernelContext);
				renderer = StreamRendererFactory::getRenderer(getKernelContext(), empty);
			}
			else
			{
				renderer = StreamRendererFactory::getRenderer(getKernelContext(), stream);
				renderersAllocated++;
				if(renderersAllocated==maxRendererCount)
				{
					log() << LogLevel_Info << "Reached maximum " << maxRendererCount << " renderers, later streams will be shown as placeholders.\n";
					log() << LogLevel_Info << "This is a limitation in the current code and the fact that there is limited number of OpenGL contexts.\n";
				}
			}

			if (renderer && renderer->initialize())
			{
				if (stream)
				{
					std::stringstream ss;
					ss << "Stream " << j + 1 << " : "
						<< getTypeManager().getTypeName(typeIdentifier).toASCIIString()
						<< ((stream->getOverlapping() || stream->getNoncontinuous()) ? " -- Warning: " : "")
						<< (stream->getOverlapping() ? "Overlapping " : "")
						<< (stream->getNoncontinuous() ? "Noncontinuous " : "")
						<< (renderersAllocated>=maxRendererCount ? " - all renderers in use" : "");

					renderer->setTitle(ss.str().c_str());
				}
				else
				{
					renderer->setTitle("Unknown");
				}

				std::stringstream prefix; prefix << "Tracker_Workspace_GUI_Renderer"
					<< "_Track_" << std::setw(3) << std::setfill('0') << (i + 1) 
					<< "_Stream_" << std::setw(2) << std::setfill('0') << (j + 1);
				renderer->restoreSettings(prefix.str());

				if(renderer->getWidget()) // n.b. 'nothing' renderer does not have a widget
				{
					gtk_table_attach_defaults(GTK_TABLE(streamTable), renderer->getWidget(), 0, 1, j, j + 1);
					gtk_widget_unref(renderer->getWidget()); // we kept +1 ref so far to make sure gtk doesnt garbage collect it
				}
				renderer->realize();
			}
			else
			{
				log() << LogLevel_Error << "Error: Failed to initialize renderer for stream of type " << typeIdentifier << "\n";
				delete renderer;
				renderer = nullptr;
			}

			track->m_Renderers.push_back(renderer);
		}

		gtk_widget_show(streamTable);
		if(track->m_Frame)
		{
			gtk_widget_show(track->m_Frame);
		}
	}

	if (m_rTracker.getWorkspace().getMaxDuration() > 0)
	{
		gtk_range_set_range(GTK_RANGE(m_hScrollbar), 0, ITimeArithmetics::timeToSeconds(m_rTracker.getWorkspace().getMaxDuration()));
	}


	updateRulerState();

	addTracksToMenu();

	resetWidgetProperties();

#if defined(TARGET_OS_Windows)
	// @note this block is a way avoid black rectangles appearing on Windows as a side effect of drawing. they do not appear on Linux,
	// so it may be a gtk or at least a platform specific issue. 
	// Basically what this block does is to move the scrollbar to the top, close the expander, and then request them to be
	// reseted to their original positions in a few frames after the content has been drawn. For some reason these actions 'repair' 
	// the drawing on Win (whereas just gtk_widget_queue_draw, gdk_window_invalidate_rect, etc do not seem to cut it).
	// to reproduce the glitch on Windows: Disable this code. Have enough tracks so that some tracks are outside 
	// the view on the upper end of the viewport. Delete a visible track. A black rectangle will be drawn across the buttons.

	GtkWidget* scrollWin = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "scrolledwindow-tracks"));
	GtkAdjustment* adjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrollWin));
	m_HScrollBarValue = gtk_adjustment_get_value(adjust);
	gtk_adjustment_set_value(adjust, 0);

	GtkWidget* expander = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "openvibe-expander_messages"));
	m_ExpanderOpen = (gtk_expander_get_expanded(GTK_EXPANDER(expander)) == TRUE ? true : false);
	gtk_expander_set_expanded(GTK_EXPANDER(expander), false);

	m_RequestResetInNFrames = 2;

	auto resetFun = [](gpointer pUserData) -> gboolean
	{ 
		GUI* ptr = static_cast<GUI*>(pUserData); 
		if( ptr->m_RequestResetInNFrames > 0)
		{
			ptr->m_RequestResetInNFrames--;
		}
		if( ptr->m_RequestResetInNFrames == 0)
		{
			GtkWidget* expander = GTK_WIDGET(gtk_builder_get_object(ptr->m_pInterface, "openvibe-expander_messages"));
			gtk_expander_set_expanded(GTK_EXPANDER(expander), ptr->m_ExpanderOpen);

			GtkWidget* scrollWin = GTK_WIDGET(gtk_builder_get_object(ptr->m_pInterface, "scrolledwindow-tracks"));
			GtkAdjustment* adjust = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrollWin));
			gtk_adjustment_set_value(adjust, ptr->m_HScrollBarValue);

			ptr->m_RequestResetInNFrames = -1;
			return FALSE; // Don't call again until re-added
		}

		return TRUE;
	};
	g_idle_add(resetFun, this);

#endif

	m_RequestRedraw = true;

	return true;
}

bool GUI::redrawAllTracks(void)
{
	for (size_t i = 0; i < m_Tracks.size(); i++)
	{
		redrawTrack(i);
	}
	m_LastRedraw = System::Time::zgetTime();

	return true;
}

bool GUI::step(void)
{
	Tracker::TrackerState state = m_rTracker.step();

	if (state == Tracker::State_Playing)
	{
		const ovtime_t currentTime = System::Time::zgetTime();
		if (currentTime - m_PreviousTime > ITimeArithmetics::secondsToTime(1.0))
		{
			const ovtime_t processedTime = m_rTracker.getWorkspace().getProcessedTime();
			const ovtime_t totalTime = m_rTracker.getWorkspace().getPlaylistDuration();

			float64 processedTimeSecs = ITimeArithmetics::timeToSeconds(processedTime);
			float64 totalTimeSecs = ITimeArithmetics::timeToSeconds(totalTime);

			char tmp[128];
			sprintf(tmp, "Processed : %.1fm / %.1fm (%.0f%%)", processedTimeSecs/60.0, totalTimeSecs/60.0, (totalTime>0 ? 100.0 * processedTimeSecs/totalTimeSecs : 0) );
			gtk_label_set(GTK_LABEL(m_hTimeDisplay), tmp);

			m_PreviousTime = currentTime;

			const uint32_t tracksOnPlaylist = m_rTracker.getWorkspace().getNumTracksOnPlaylist();
			const uint32_t tracksDone = m_rTracker.getWorkspace().getNumTracksDone();

			sprintf(tmp, "Playing tracks (%d until finish)", tracksOnPlaylist - tracksDone);
			gtk_label_set(GTK_LABEL(m_hTrackCounter), tmp);
		}
	}
	else if(state == Tracker::State_Stopped && 	m_WaitingForStop)
	{
		setPlaytimeWidgetState(true);

		gtk_label_set(GTK_LABEL(m_hTimeDisplay), "Processed : -");
		gtk_label_set(GTK_LABEL(m_hTrackCounter), "Stopped");

		initializeRenderers();

		m_WaitingForStop = false;
	}
	else
	{
		// nop
	}

	if (m_RequestRedraw)
	{
		redrawAllTracks();
		m_RequestRedraw = false;
	}

	return true;
}

bool GUI::run(void)
{
	initializeRenderers();

	gtk_main();

	return true;
}

bool GUI::addTrackCB(void)
{

// 	m_KernelContext.getLogManager() << LogLevel_Debug << "openScenarioCB\n";

	::GtkFileFilter* l_pFileFilterSpecific=gtk_file_filter_new();

	gtk_file_filter_add_pattern(l_pFileFilterSpecific, "*.ov");
	gtk_file_filter_set_name(l_pFileFilterSpecific, "OV files");

	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
			"Select file to open...",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(l_pWidgetDialogOpen),true);

	const CString defaultPath = OpenViBE::Directories::getDataDir() + CString("/scenarios/signals/");

	gtk_file_chooser_set_current_folder(
			GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
			defaultPath.toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(l_pWidgetDialogOpen),true);

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		GSList * l_pFile, *l_pList;
		l_pFile = l_pList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		while(l_pFile)
		{
			char* l_sFileName = (char*)l_pFile->data;
			char* l_pBackslash = NULL;
			while((l_pBackslash = ::strchr(l_sFileName, '\\'))!=NULL)
			{
				*l_pBackslash = '/';
			}

			m_rTracker.getWorkspace().addTrack(l_sFileName);

			g_free(l_pFile->data);
			l_pFile=l_pFile->next;
		}
		g_slist_free(l_pList);

		initializeRenderers();
	}

	gtk_widget_destroy(l_pWidgetDialogOpen);

	return true;
}

bool GUI::resetWidgetProperties(void)
{
	static const double defaultScaleSecs = 30.0; // how many seconds of data to view at once per default

	gtk_range_set_value(GTK_RANGE(m_hScrollbar), 0);

	if (m_Tracks.size() > 0 && !m_rTracker.getWorkspace().getMemorySaveMode())
	{
		const ovtime_t maxDuration = m_rTracker.getWorkspace().getMaxDuration();
		const double maxSecs = OpenViBE::ITimeArithmetics::timeToSeconds(maxDuration);

		if (maxSecs > 0)
		{
			gtk_range_set_range(GTK_RANGE(m_hScale), 1, maxSecs);
			gtk_range_set_value(GTK_RANGE(m_hScale), (maxSecs < defaultScaleSecs ? maxSecs : defaultScaleSecs));
		}
		gtk_widget_set_sensitive(m_hScrollbar, true);
		gtk_widget_set_sensitive(m_hScale, true);
	}
	else
	{
		gtk_widget_set_sensitive(m_hScrollbar, false);
		gtk_widget_set_sensitive(m_hScale, false);
	}

	const CString workspaceFile = m_rTracker.getWorkspace().getFilename();
	const uint64_t revision = m_rTracker.getWorkspace().getRevision();

	GtkWidget* saveAsH = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-menu_save"));
	if (workspaceFile.length() > 0)
	{
		gtk_widget_set_sensitive(saveAsH, true);
		char basename[512];
		FS::Files::getFilename(workspaceFile, basename, 512);
		std::stringstream ss;
		ss << "Workspace: " << basename << " (rev " << revision << ")";
		gtk_label_set_text(GTK_LABEL(m_hWorkspaceLabel), ss.str().c_str());
	}
	else
	{
		gtk_widget_set_sensitive(saveAsH, false);
		gtk_label_set_text(GTK_LABEL(m_hWorkspaceLabel), "Workspace: Unnamed");
	}

	const CString processorFile = m_rTracker.getWorkspace().getProcessorFile();
	GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_file"));
	char basename[512];
	FS::Files::getFilename(processorFile.toASCIIString(), basename, 512);
	gtk_label_set_text(GTK_LABEL(widget), basename);
	gtk_widget_show(widget);

	updateIdleWidgetState();

#if 0

	m_totalChunks = 0;
	m_numChannels = 0;
	m_chunkSize = 0;

	const StreamBundle* tr = m_rTracker.getWorkspace().getTrack(0);
	if (!tr) {
		return false;
	}
	const Stream<TypeSignal>* ptr = nullptr;

	for (size_t i=0;i<tr->getNumStreams();i++)
	{
		if(tr->getStream(i) && tr->getStream(i)->getTypeIdentifier()==OV_TypeId_Signal)
		{
			ptr = reinterpret_cast< const Stream<TypeSignal>* >(tr->getStream(i));
			break;
		}
	}
	if(ptr)
	{
		// Count samples first
		TypeSignal::Buffer* buf;
		ptr->getChunk(0, &buf);
		if(buf && buf->m_buffer.getDimensionCount()==2) 
		{
			m_totalChunks = ptr->getChunkCount();
			m_numChannels = buf->m_buffer.getDimensionSize(0);
			m_chunkSize = buf->m_buffer.getDimensionSize(1);
		}
	}

	if(m_totalChunks<2) { std::cout << "Warning: File has less than 2 chunks, ranges may behave oddly\n"; }

	const uint32_t chunksPerView = std::min<uint32_t>(20,m_totalChunks-1);

	// @todo change the scrollbars to seconds
	gtk_range_set_range(GTK_RANGE(m_hScale), 1, m_totalChunks-1);
	gtk_range_set_value(GTK_RANGE(m_hScale), chunksPerView);

	gtk_range_set_range(GTK_RANGE(m_hScrollbar), 0, m_rTracker.getWorkspace().getMaxDuration() );
	gtk_range_set_value(GTK_RANGE(m_hScrollbar), 0);

	// gtk_range_set_value(GTK_RANGE(m_hScrollbar), 0);
	// gtk_range_set_round_digits(GTK_RANGE(m_hScrollbar), 0); //not in gtk ver we use on Windows
	gtk_range_set_increments(GTK_RANGE(m_hScrollbar), 10, 100);

#endif

	if (m_hSelectionWindow && gtk_widget_get_visible(m_hSelectionWindow))
	{
		// Will redraw the window
		editSelectionCB();
	}

	return true;

}

bool GUI::redrawStream(size_t trackIndex, size_t streamIndex)
{
	const StreamBundle* tr = m_rTracker.getWorkspace().getTrack(trackIndex);
	if (!tr)
	{
		std::cout << "No track for index " << trackIndex << "\n";
		
		return false;
	}

	StreamRendererBase* renderer = m_Tracks[trackIndex]->m_Renderers[streamIndex];
	if (!renderer)
	{
		std::cout << "No renderer for stream " << streamIndex << " in track " << trackIndex << "\n";
		return false;
	}

	StreamPtrConst stream = tr->getStream(streamIndex);
	if (!stream)
	{
		std::cout << "No stream " << streamIndex << " in track " << trackIndex << "\n";
		return false;
	}

	const float64 hValue = (float64)std::floor(gtk_range_get_value(GTK_RANGE(m_hScrollbar)));
	const float64 hScale = (float64)std::floor(gtk_range_get_value(GTK_RANGE(m_hScale)));

	// std::cout << "HSlider is at " << hValue << " and " << hScale << "\n";

	const ovtime_t startTime = ITimeArithmetics::secondsToTime(hValue);
	const ovtime_t endTime = startTime + ITimeArithmetics::secondsToTime(hScale);

//	std::cout << "Spooling " << trackIndex << " : " << streamIndex << " for " << ITimeArithmetics::timeToSeconds(startTime) << " to "
//		<< ITimeArithmetics::timeToSeconds(endTime) << "\n";

	renderer->spool(startTime, endTime);

	return true;
}

bool GUI::redrawTrack(size_t index)
{
	//	ovtime_t time = System::Time::zgetTime();

	for (size_t streamIndex = 0; streamIndex < m_Tracks[index]->m_Renderers.size(); streamIndex++)
	{
		redrawStream(index, streamIndex);
	}

	return true;
}

bool GUI::openWorkspaceCB(void)
{
	// 	m_KernelContext.getLogManager() << LogLevel_Debug << "openScenarioCB\n";

	::GtkFileFilter* l_pFileFilterSpecific = gtk_file_filter_new();

	gtk_file_filter_add_pattern(l_pFileFilterSpecific, "*.ovw");
	gtk_file_filter_set_name(l_pFileFilterSpecific, "Workspace files");

	::GtkWidget* l_pWidgetDialogOpen = gtk_file_chooser_dialog_new(
		"Select file to open...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), true);

	const CString defaultPath = OpenViBE::Directories::getUserDataDir() + "/scenarios/workspaces/";

	gtk_file_chooser_set_current_folder(
		GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
		defaultPath.toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), false);

	bool fileSelected = false;
	if (gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		GSList * l_pFile, *l_pList;
		l_pFile = l_pList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		while (l_pFile)
		{
			char* l_sFilename = (char*)l_pFile->data;
			char* l_pBackslash = NULL;
			while ((l_pBackslash = ::strchr(l_sFilename, '\\')) != NULL)
			{
				*l_pBackslash = '/';
			}

			m_rTracker.getWorkspace().load(CString(l_sFilename));

			initializeRenderers();

			fileSelected = true;

			g_free(l_pFile->data);
			l_pFile = l_pFile->next;
		}
		g_slist_free(l_pList);
	}
	else
	{
//		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "processorpreferences-filename"));
//		gtk_entry_set_text(GTK_ENTRY(widget), "");
	}


	gtk_widget_destroy(l_pWidgetDialogOpen);

	return fileSelected;
}


bool GUI::openProcessorCB(void)
{
// 	m_KernelContext.getLogManager() << LogLevel_Debug << "openScenarioCB\n";

	::GtkFileFilter* l_pFileFilterSpecific=gtk_file_filter_new();

	gtk_file_filter_add_pattern(l_pFileFilterSpecific, "*.xml");
	gtk_file_filter_set_name(l_pFileFilterSpecific, "Scenario files");

	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
			"Select file to open...",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(l_pWidgetDialogOpen),true);

	const CString defaultPath = OpenViBE::Directories::getDataDir() + CString("/applications/tracker/");

	gtk_file_chooser_set_current_folder(
			GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
			defaultPath.toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(l_pWidgetDialogOpen),false);

	bool fileSelected = false;
	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		GSList * l_pFile, *l_pList;
		l_pFile = l_pList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		while(l_pFile)
		{
			char* l_sFileName = (char*)l_pFile->data;
			char* l_pBackslash = NULL;
			while((l_pBackslash = ::strchr(l_sFileName, '\\'))!=NULL)
			{
				*l_pBackslash = '/';
			}

			GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "processorpreferences-filename"));
			gtk_entry_set_text(GTK_ENTRY(widget), l_sFileName);

			fileSelected = true;

			g_free(l_pFile->data);
			l_pFile=l_pFile->next;
		}
		g_slist_free(l_pList);
	}
	else
	{
		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "processorpreferences-filename"));
		gtk_entry_set_text(GTK_ENTRY(widget), "");
	}

	gtk_widget_destroy(l_pWidgetDialogOpen);

	return fileSelected;
}

bool GUI::openProcessorCBFinal(void)
{
	if (openProcessorCB())
	{
		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "processorpreferences-filename"));
		const char* filename = gtk_entry_get_text(GTK_ENTRY(widget));

		m_rTracker.getWorkspace().setProcessor(filename);

		resetWidgetProperties();
	}
	return true;
}

bool GUI::selectWorkspacePathCBFinal(void)
{
	if (selectWorkspacePathCB())
	{
		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "trackerpreferences-workspace_path"));
		const char* l_sFilename = gtk_entry_get_text(GTK_ENTRY(widget));

		return m_rTracker.getWorkspace().setWorkingPath(CString(l_sFilename));
	}
	else
	{
		return false;
	}
}
// Note: this doesn't set the path to the workspace as it may be called from Preferences place with "Cancel"
bool GUI::selectWorkspacePathCB(void)
{
	// 	m_KernelContext.getLogManager() << LogLevel_Debug << "openScenarioCB\n";

	::GtkFileFilter* l_pFileFilterSpecific = gtk_file_filter_new();

	//gtk_file_filter_add_pattern(l_pFileFilterSpecific, "*.xml");
	gtk_file_filter_set_name(l_pFileFilterSpecific, "Folders");

	::GtkWidget* l_pWidgetDialogOpen = gtk_file_chooser_dialog_new(
		"Please select work path folder ...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), true);

	const CString defaultPath = OpenViBE::Directories::getUserDataDir();

	gtk_file_chooser_set_current_folder(
		GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
		defaultPath.toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), false);

	bool fileSelected = false;
	if (gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		//char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		GSList * l_pFile, *l_pList;
		l_pFile = l_pList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		while (l_pFile)
		{
			char* l_sFilename = (char*)l_pFile->data;
			char* l_pBackslash = NULL;
			while ((l_pBackslash = ::strchr(l_sFilename, '\\')) != NULL)
			{
				*l_pBackslash = '/';
			}

			GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "trackerpreferences-workspace_path"));
			gtk_entry_set_text(GTK_ENTRY(widget), l_sFilename);

			fileSelected = true;

			g_free(l_pFile->data);
			l_pFile = l_pFile->next;
		}
		g_slist_free(l_pList);
	}
	else
	{
//		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "trackerpreferences-workspace_path"));
//		gtk_entry_set_text(GTK_ENTRY(widget), defaultPath);
	}

	gtk_widget_destroy(l_pWidgetDialogOpen);

	return fileSelected;
}

bool GUI::saveAsCB(void)
{
	if (m_rTracker.getWorkspace().getWorkingPath().length() == 0)
	{
		if (!selectWorkspacePathCBFinal())
		{
			return false;
		}
	}

	// 	m_KernelContext.getLogManager() << LogLevel_Debug << "openScenarioCB\n";

	::GtkFileFilter* l_pFileFilterSpecific = gtk_file_filter_new();

	gtk_file_filter_add_pattern(l_pFileFilterSpecific, "*.ovw");
	gtk_file_filter_set_name(l_pFileFilterSpecific, "Workspace files");

	::GtkWidget* l_pWidgetDialogOpen = gtk_file_chooser_dialog_new(
		"Select file to save as...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_pFileFilterSpecific);

	// GTK 2 known bug: this won't work if  setCurrentFolder is also used on the dialog.
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), true);

	const CString defaultPath = OpenViBE::Directories::getUserDataDir();

	gtk_file_chooser_set_current_folder(
		GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
		defaultPath.toASCIIString());

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), false);

	bool fileSelected = false;
	if (gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen)) == GTK_RESPONSE_ACCEPT)
	{
		//char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		GSList * l_pFile, *l_pList;
		l_pFile = l_pList = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		while (l_pFile)
		{
			char* l_sFilename = (char*)l_pFile->data;
			char* l_pBackslash = NULL;
			while ((l_pBackslash = ::strchr(l_sFilename, '\\')) != NULL)
			{
				*l_pBackslash = '/';
			}

			std::string fn = std::string(l_sFilename);
			if (fn.substr(fn.length() - 4, 4) != std::string(".ovw"))
			{
				fn += std::string(".ovw");
			}

			storeRendererSettings();

			if (m_rTracker.getWorkspace().save(fn.c_str()))
			{
				resetWidgetProperties(); // Filenames may have changed
				initializeRenderers();
				m_RequestRedraw = true;

				fileSelected = true;
			}

			g_free(l_pFile->data);
			l_pFile = l_pFile->next;
		}
		g_slist_free(l_pList);
	}
	else
	{

	}

	gtk_widget_destroy(l_pWidgetDialogOpen);

	return fileSelected;

}

bool GUI::saveCB(void)
{
	if (m_rTracker.getWorkspace().getWorkingPath().length() == 0)
	{
		if (!selectWorkspacePathCBFinal())
		{
			return false;
		}		
	}

	storeRendererSettings();

	m_rTracker.getWorkspace().save(m_rTracker.getWorkspace().getFilename());

	// filenames may have changed, reset
	resetWidgetProperties(); 

	initializeRenderers();

	m_RequestRedraw = true;

	return true;
}

bool GUI::incrementRevSaveCB(void)
{
	if (m_rTracker.getWorkspace().getWorkingPath().length() == 0)
	{
		if (!selectWorkspacePathCBFinal())
		{
			return false;
		}		
	}

	storeRendererSettings();

	m_rTracker.getWorkspace().incrementRevisionAndSave(m_rTracker.getWorkspace().getFilename());

	// filenames may have changed, reset
	resetWidgetProperties(); 

	initializeRenderers();

	m_RequestRedraw = true;

	return true;
}

#include <BoxAdapter.h>

bool GUI::quitCB(void)
{
	this->clearRenderers();
	
	gtk_main_quit();

	return false;
}

bool GUI::clearCB(void)
{
	if (m_hSelectionWindow)
	{
		gtk_widget_destroy(m_hSelectionWindow);
		m_hSelectionWindow = nullptr;
	}

	m_rTracker.getWorkspace().clear();

	initializeRenderers();

	return true;
}

bool GUI::deleteAllTracksCB(void)
{
	m_rTracker.getWorkspace().clearTracks();

	initializeRenderers();

	return true;
}

bool GUI::playCB(void)
{
	m_WaitingForStop = true;

	setPlaytimeWidgetState(false);

	if (!m_rTracker.play(false))
	{
		setPlaytimeWidgetState(true);
		return false;
	}

	return true;
}

bool GUI::playFastCB(void)
{
	m_WaitingForStop = true;

	setPlaytimeWidgetState(false);

	if (!m_rTracker.play(true))
	{
		setPlaytimeWidgetState(true);
		return false;
	}

	return true;
}


bool GUI::stopCB(void)
{
	// Send stop request. The threads may stop later.
	bool retVal = m_rTracker.stop();

	return retVal;
}

bool GUI::editSelectionCB(void)
{
	if (m_hSelectionWindow)
	{
		gtk_widget_destroy(m_hSelectionWindow);
	}

	Workspace& wp = m_rTracker.getWorkspace();
	Selection& selection = m_rTracker.getWorkspace().getSelection();

	size_t numRows = wp.getNumTracks();
	size_t numCols = 0;
	for (size_t i = 0; i < numRows; i++)
	{
		numCols = std::max<size_t>(numCols, wp.getTrack(i)->getNumStreams());
	}

	m_hSelectionWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(m_hSelectionWindow), "Select tracks/streams");

	if (!selection.isEmpty())
	{
		GtkWidget* table = gtk_table_new(numRows + 1, numCols + 1, true);
		gtk_table_set_col_spacings(GTK_TABLE(table), 5);

		gtk_container_add(GTK_CONTAINER(m_hSelectionWindow), table);

		for (size_t i = 0; i < numRows; i++)
		{
			for (size_t j = 0; j < wp.getTrack(i)->getNumStreams(); j++)
			{
				GtkWidget* button = gtk_check_button_new();
				gtk_table_attach_defaults(GTK_TABLE(table), button, j + 1, j + 2, i + 1, i + 2);

				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), wp.getTrack(i)->getStream(j)->getSelected());

				g_object_set_data(G_OBJECT(button), "setSelection-track", reinterpret_cast<void*>(i));
				g_object_set_data(G_OBJECT(button), "setSelection-stream", reinterpret_cast<void*>(j));
				GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(button), "clicked", setSelectionCB);

				gtk_widget_show(button);
			}

			std::stringstream ss; ss << "Track" << i + 1;
			GtkWidget *label = gtk_label_new(ss.str().c_str());
			gtk_table_attach_defaults(GTK_TABLE(table), label, 0, 1, i + 1, i + 2);
			gtk_widget_show(label);
		}
		for (size_t i = 0; i < numCols; i++)
		{
			std::stringstream ss; ss << "Stream" << i + 1;
			GtkWidget *label = gtk_label_new(ss.str().c_str());
			gtk_table_attach_defaults(GTK_TABLE(table), label, i + 1, i + 2, 0, 1);
			gtk_widget_show(label);
		}

		gtk_widget_show(table);
	}
	else
	{
		GtkWidget* label = GTK_WIDGET(gtk_label_new("No tracks with streams"));
		gtk_container_add(GTK_CONTAINER(m_hSelectionWindow), label);
		gtk_widget_show(label);
	}

	// Hide instead of destroy on closing the window
	g_signal_connect(m_hSelectionWindow, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	gtk_widget_show(m_hSelectionWindow);

	return true;
}

bool GUI::selectAllCB(void)
{
	m_rTracker.getWorkspace().getSelection().reset(true);
	if (m_hSelectionWindow && gtk_widget_get_visible(m_hSelectionWindow))
	{
		editSelectionCB();
	}
	updateIdleWidgetState();

	const bool selectedOnlyState = m_KernelContext.getConfigurationManager().expandAsBoolean("${Tracker_Workspace_GUI_ShowSelectedOnly}", false);
	if(selectedOnlyState)
	{
		initializeRenderers();
	}

	return true;
}

bool GUI::selectNoneCB(void)
{
	m_rTracker.getWorkspace().getSelection().reset(false);
	if (m_hSelectionWindow && gtk_widget_get_visible(m_hSelectionWindow))
	{
		editSelectionCB();
	}
	updateIdleWidgetState();

	const bool selectedOnlyState = m_KernelContext.getConfigurationManager().expandAsBoolean("${Tracker_Workspace_GUI_ShowSelectedOnly}", false);
	if(selectedOnlyState)
	{
		initializeRenderers();
	}

	return true;
}


bool GUI::processorPreferencesCB(void)
{
	uint32_t sendPort, receivePort;
	m_rTracker.getWorkspace().getProcessorPorts(sendPort, receivePort);

	GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "processorpreferences-filename"));
	gtk_entry_set_text(GTK_ENTRY(widget), m_rTracker.getWorkspace().getProcessorFile());

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_port"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), sendPort);

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-numthreads"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), m_rTracker.getNumThreads());

	bool noGUI, doSend, doReceive;
	m_rTracker.getWorkspace().getProcessorFlags(noGUI, doSend, doReceive);
	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_dosend"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), doSend);
	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_doreceive"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), doReceive);
	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_nogui"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), noGUI);

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_catenate"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), m_rTracker.getWorkspace().getCatenateMode());


	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "trackerpreferences-workspace_path"));
	CString workspacePath = m_rTracker.getWorkspace().getWorkingPath();
	gtk_entry_set_text(GTK_ENTRY(widget), workspacePath.toASCIIString());

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-memorysave"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), m_rTracker.getWorkspace().getMemorySaveMode());

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-inplacemode"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), m_rTracker.getWorkspace().getInplaceMode());

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "trackerpreferences-processor_arguments"));
	const CString args = m_rTracker.getWorkspace().getProcessorArguments();
	gtk_entry_set_text(GTK_ENTRY(widget), args.toASCIIString());

	GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(m_pInterface, "dialog-processor-preferences"));
	gtk_dialog_run(GTK_DIALOG(dialog));

	return true;
}


bool GUI::processorPreferencesButtonOkCB(void)
{
	GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "processorpreferences-filename"));
	const char* filename = gtk_entry_get_text(GTK_ENTRY(widget));
	m_rTracker.getWorkspace().setProcessor(filename);

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_port"));
	gtk_spin_button_update(GTK_SPIN_BUTTON(widget));
	const uint32_t firstPort = (uint32_t)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	m_rTracker.getWorkspace().setProcessorPorts(firstPort, firstPort+1);

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-numthreads"));
	gtk_spin_button_update(GTK_SPIN_BUTTON(widget));
	const uint32_t numThreads = (uint32_t)gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	m_rTracker.setNumThreads(numThreads);

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_catenate"));
	m_rTracker.getWorkspace().setCatenateMode((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) != 0));

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_nogui"));
	bool noGui = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) != 0);
	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_dosend"));
	bool doSend = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) != 0);
	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-processor_doreceive"));
	bool doReceive = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) != 0);

	m_rTracker.getWorkspace().setProcessorFlags(noGui, doSend, doReceive);

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "trackerpreferences-workspace_path"));
	const char* path = gtk_entry_get_text(GTK_ENTRY(widget));
	m_rTracker.getWorkspace().setWorkingPath(CString(path));

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-memorysave"));
	const bool oldMemorySaveMode = m_rTracker.getWorkspace().getMemorySaveMode();
	const bool newMemorySaveMode = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) != 0);
	if (newMemorySaveMode != oldMemorySaveMode)
	{
		m_rTracker.getWorkspace().setMemorySaveMode(newMemorySaveMode);
		initializeRenderers();
	}

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-inplacemode"));
	m_rTracker.getWorkspace().setInplaceMode((gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) != 0));

	widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "trackerpreferences-processor_arguments"));
	const char* args = gtk_entry_get_text(GTK_ENTRY(widget));
	m_rTracker.getWorkspace().setProcessorArguments(CString(args));

	GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(m_pInterface, "dialog-processor-preferences"));
	gtk_widget_hide(GTK_WIDGET(dialog));

	resetWidgetProperties();

	return true;
}


bool GUI::processorPreferencesButtonCancelCB(void)
{
	GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(m_pInterface, "dialog-processor-preferences"));
	gtk_widget_hide(GTK_WIDGET(dialog));

	return true;
}


bool GUI::processorEditCB(void)
{
	// Here we configure the processor which already has an .xml set
	return m_rTracker.getWorkspace().configureProcessor(nullptr);
}

bool GUI::processorEditCB2(void)
{
	// This tries to configure whatever the user has entered into the box, even if
	// Ok button hasn't been pressed yet to register that choice.
	GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "processorpreferences-filename"));
	const char* filename = gtk_entry_get_text(GTK_ENTRY(widget));

	return m_rTracker.getWorkspace().configureProcessor(filename);
}

#include "ovdCCommentEditorDialog.h"

bool GUI::editNotesCB(void)
{
	const OpenViBE::CString guiFile = OpenViBE::Directories::getDataDir() + "/applications/tracker/designer-interface.ui";

	OpenViBEDesigner::CCommentEditorDialog l_oCommentEditorDialog(getKernelContext(), 
		m_rTracker.getWorkspace().getNotes(), guiFile.toASCIIString());
	
	return l_oCommentEditorDialog.run();
}


bool GUI::hScrollCB(GtkWidget* widget)
{
//	std::cout << "Hscroll" << gtk_range_get_value(GTK_RANGE(widget)) << "\n";

	m_RequestRedraw = true;

	return true;
}

bool GUI::hScaleCB(GtkWidget* widget)
{
	// @since we store the pointers in the class, no need to pass in widget...
	//std::cout << "Hscale " << gtk_range_get_value(GTK_RANGE(widget)) << "\n";

//	double stepSizeSecs = gtk_range_get_value(GTK_RANGE(widget));

//	ovtime_t maxDuration = m_rTracker.getWorkspace().getMaxDuration();
//	uint64_t verticalStepsAvailable = ITimeArithmetics::secondsToTime(stepSizeSecs);
//	const uint32_t chunksPerView = static_cast<uint32_t>( gtk_range_get_value(GTK_RANGE(widget)));

	m_RequestRedraw = true;

	return true;
}

bool GUI::workspaceInfoCB(void)
{
	GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-workspace_information"));
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(gtk_builder_get_object(m_pInterface, "tracker-textbuffer-workspace_information"));

	// Hide instead of destroy on closing the window
	g_signal_connect(window, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	CString workspaceInfo = getWorkspaceInfo();

	gtk_text_buffer_set_text(buffer, workspaceInfo.toASCIIString(), -1);

	gtk_widget_show_all(window);

	gtk_window_present(GTK_WINDOW(window));

	return true;
}

bool GUI::aboutCB(void)
{
	GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "aboutdialog-newversion"));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_CLOSE, true);
	g_signal_connect(G_OBJECT(window), "response", G_CALLBACK(gtk_widget_hide), NULL);

	gtk_dialog_run(GTK_DIALOG(window));
	return true;
}

bool GUI::deleteTrackCB(GtkWidget* widget)
{
	const size_t index = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "delete-track-cb"));
	if (index >= m_Tracks.size())
	{
		return false;
	}


	::GtkWidget* l_pTrackTable = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "table_tracks"));

	// To avoid issues, we delete everything first before detaching the widget
	GtkWidget* frame = m_Tracks[index]->m_Frame;

	delete m_Tracks[index];
	m_Tracks.erase(m_Tracks.begin() + index);

	if(frame)
	{
		gtk_container_remove(GTK_CONTAINER(l_pTrackTable), frame);
	}

	m_rTracker.getWorkspace().removeTrack(index);


	// This will store the current renderer configurations but with the new indexing
	storeRendererSettings();

 	initializeRenderers();

	return true;
}

bool GUI::moveTrackCB(GtkWidget* widget)
{
	const size_t track = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "move-track-cb"));
	if (track >= m_Tracks.size())
	{
		return false;
	}
	GtkWidget* dialog = gtk_dialog_new_with_buttons("Choose target slot",
		GTK_WINDOW(m_hMainWindow),
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_REJECT,
		NULL
		);

	GtkAdjustment* adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(track+1, 1, m_rTracker.getWorkspace().getNumTracks(), 1, 1, 0));
	GtkWidget* button = gtk_spin_button_new(adjustment, 1, 0);
	// gtk_spin_button_set_range(GTK_SPIN_BUTTON(button), 1, m_rTracker.getWorkspace().getNumTracks());

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), button);
	gtk_widget_show(button);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gtk_spin_button_update(GTK_SPIN_BUTTON(button));
		size_t newTrack = static_cast<size_t>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(button))) - 1;

		// @fixme scalings will be lost
		m_rTracker.getWorkspace().moveTrack(track, newTrack);

		// This will store the current renderer configurations but with the new indexing
		// storeRendererSettings();

		initializeRenderers();
	}

	gtk_widget_destroy(dialog);

	return true;
}

bool GUI::moveStreamCB(GtkWidget* widget)
{
	const size_t track = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "move-stream-cb-track"));
	const size_t stream = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "move-stream-cb-stream"));

	GtkWidget* dialog = gtk_dialog_new_with_buttons("Choose track & stream",
		GTK_WINDOW(m_hMainWindow),
		(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_REJECT,
		NULL
		);

	// Target track
	GtkAdjustment* adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(track+1, 1, m_rTracker.getWorkspace().getNumTracks(), 1, 1, 0));
	GtkWidget* buttonTrack = gtk_spin_button_new(adjustment, 1, 0);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), buttonTrack);
	gtk_widget_show(buttonTrack);

	// Target stream
	// @fixme the limits would depend on the target track
	size_t maxNumStreams = 0;
	for (size_t i = 0; i < m_rTracker.getWorkspace().getNumTracks(); i++)
	{
		maxNumStreams = std::max<size_t>(maxNumStreams, m_rTracker.getWorkspace().getTrack(i)->getNumStreams());
	}
	adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(stream+1, 1, maxNumStreams, 1, 1, 0));
	GtkWidget* buttonStream = gtk_spin_button_new(adjustment, 1, 0);
	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), buttonStream);
	gtk_widget_show(buttonStream);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gtk_spin_button_update(GTK_SPIN_BUTTON(buttonTrack));
		size_t newTrack = static_cast<size_t>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(buttonTrack))) - 1;

		gtk_spin_button_update(GTK_SPIN_BUTTON(buttonStream));
		size_t newStream = static_cast<size_t>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(buttonStream))) - 1;

		newStream = std::min<size_t>(newStream, m_rTracker.getWorkspace().getTrack(newTrack)->getNumStreams());

		// @fixme scalings will be lost
		m_rTracker.getWorkspace().moveStream(track, stream, newTrack, newStream);

		// This will store the current renderer configurations but with the new indexing
		// storeRendererSettings();

		initializeRenderers();
	}

	gtk_widget_destroy(dialog);

	return true;
}


bool GUI::showChunksCB(GtkWidget* widget)
{
	void* ptr = g_object_get_data(G_OBJECT(widget), "show-chunks-cb");
	StreamRendererBase* renderer = reinterpret_cast<StreamRendererBase*>(ptr);
	if (!renderer)
	{
		std::cout << "Error: No renderer for stream\n";
		return false;
	}
	return renderer->showChunkList();
}

bool GUI::deleteStreamCB(GtkWidget* widget)
{
	const size_t track = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "delete-stream-cb-track"));
	const size_t stream = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "delete-stream-cb-stream"));
	
	// We must make sure the renderer is not holding refs anymore to the track
	// @todo it'd be more efficient just to delete the right renderer and not all of them
	m_rTracker.getWorkspace().removeStream(track,stream);

	initializeRenderers();

	return true;
}

bool GUI::toggleRulerCB(GtkWidget* widget)
{
	const bool newState = (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) != 0 ? true : false);

	m_KernelContext.getConfigurationManager().addOrReplaceConfigurationToken("Tracker_Workspace_GUI_ShowRulers", 
		(newState ? "true" : "false"));

	updateRulerState();


	return true;
}

bool GUI::toggleShowSelectedOnlyCB(GtkWidget* widget)
{
	const bool newState = (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)) != 0 ? true : false);

	m_KernelContext.getConfigurationManager().addOrReplaceConfigurationToken("Tracker_Workspace_GUI_ShowSelectedOnly", 
		(newState ? "true" : "false"));

	initializeRenderers();

	return true;
}

#include "ovdCBoxConfigurationDialog.h"

bool GUI::applyBoxPluginCB(GtkWidget* widget)
{
	// Which filter to apply?
	const size_t index = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "applyBoxPlugin-cb"));

	// Get the box so we can present the configuration GUI
	auto& plugins = m_rTracker.getBoxPlugins().getBoxPlugins();
	BoxAdapterStream* box = plugins[index];

	// The .ui files used are taken by CMake from Designer, like the dialog code
	const CString defaultPath = OpenViBE::Directories::getDataDir() + CString("/applications/tracker/");
	const CString guiFilename = defaultPath + "designer-interface.ui";
	const CString settingsFilename = defaultPath + "designer-interface-settings.ui";

	bool applyClicked;
	if (box->getBox().getSettingCount() > 0)
	{
		OpenViBEDesigner::CBoxConfigurationDialog dialog(getKernelContext(),
			box->getBox(), guiFilename, settingsFilename, false);
		applyClicked = dialog.run();
	}
	else
	{
		// if theres no settings, just consider this as auto-applying
		applyClicked = true;
	}
	
	if (applyClicked)
	{
		m_rTracker.applyBoxPlugin(index);

		initializeRenderers();
		m_RequestRedraw = true;
	}

	return true;
}

bool GUI::trackerPluginCB(GtkWidget* widget)
{
	// Which filter to apply?
	const size_t index = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "trackerPlugin-cb"));

	bool success = m_rTracker.applyTrackerPlugin(index);

	initializeRenderers();
	m_RequestRedraw = true;

	return success;
}


bool GUI::setSelectionCB(GtkWidget* widget)
{
	const size_t track = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "setSelection-track"));
	const size_t stream = reinterpret_cast<size_t>(g_object_get_data(G_OBJECT(widget), "setSelection-stream"));

	gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

	m_rTracker.getWorkspace().getTrack(track)->getStream(stream)->setSelected( active != 0);

	updateIdleWidgetState();

	const bool selectedOnlyState = m_KernelContext.getConfigurationManager().expandAsBoolean("${Tracker_Workspace_GUI_ShowSelectedOnly}", false);
	if(selectedOnlyState)
	{
		initializeRenderers();
	}

	return true;
}

bool GUI::clearMessagesCB(void)
{
	m_pLogListener->clearMessages();
	return true;
}

GUI::GUITrack::~GUITrack() 
{
	for (auto renderer : m_Renderers)
	{
		if (renderer)
		{
			renderer->uninitialize();
			delete renderer;
		}
	}
	m_Renderers.clear();
}

// @todo refactor to Workspace. but can't do that neatly while renderers are responsible for the text.
OpenViBE::CString GUI::getWorkspaceInfo(void) const
{
	stringstream ss;
	Workspace& wp = m_rTracker.getWorkspace();

	ss << "Current workspace" << endl;
	ss << "-----------------" << endl;
	ss << "Number of tracks: " << wp.getNumTracks() << endl;
	ss << "Maximum track duration: " << ITimeArithmetics::timeToSeconds(wp.getMaxDuration()) << "s" << endl;
	ss << "Working path: " << wp.getWorkingPath() << endl;
	ss << "Workspace file: " << wp.getFilename() << endl;
	ss << "Workspace revision: " << wp.getRevision() << endl;

	if (wp.getMemorySaveMode())
	{
		ss << endl << "Note: Memory save mode enabled, track details unknown." << endl;
	}

	for (size_t i = 0; i < wp.getNumTracks(); i++)
	{
		const StreamBundle* bundle = wp.getTrack(i);
		if (!bundle)
		{
			ss << endl;
			ss << "Track " << i + 1 << " is empty\n";
			continue;
		}

		ss << endl;
		ss << "Track " << i+1 << "/" << wp.getNumTracks() << " : " << bundle->getNumStreams() << " streams" << endl;
		ss << "  Source: " << bundle->getSource() << endl;
		for (size_t j = 0; j < m_Tracks[i]->m_Renderers.size(); j++)
		{
			StreamPtrConst stream = bundle->getStream(j);
			const CIdentifier typeId = stream->getTypeIdentifier();

			ss << "  Stream " << j + 1 << "/" << bundle->getNumStreams() << " : "
				<< getTypeManager().getTypeName(typeId).toASCIIString()
				<< " " << typeId.toString() << endl;
			ss << "    Time range: [" << ITimeArithmetics::timeToSeconds(stream->getStartTime()) << ", "
				<< ITimeArithmetics::timeToSeconds(stream->getDuration()) << "]s" << endl;
			ss << "    Total chunks: " << stream->getChunkCount() << " ( "
				<< (stream->getOverlapping() ? "Overlapping " : "")
				<< (stream->getNoncontinuous() ? "Noncontinuous" : "") 
				<< (!(stream->getOverlapping() || stream->getNoncontinuous()) ? "Continuous" : "")
				<< " )"
				<< endl;
			ss << "    Selected: " << (stream->getSelected() ? "Yes" : "No") << endl;

			// @fixme not too happy that the renderer is responsible for this, but since Stream is a template
			// class, it'd need a override for each type
			if (m_Tracks[i]->m_Renderers[j])
			{
				ss << m_Tracks[i]->m_Renderers[j]->renderAsText(4);
			}
			else
			{
				ss << "    Empty stream" << endl;
			}
		}
	}
	
	ss << "\nConfiguration tokens (refreshed on save): \n\n";

	auto tokens = m_rTracker.getWorkspace().getConfigurationTokens();
	for (auto token : tokens)
	{
		ss << token.first << " = " << token.second << "\n";
	}

	return OpenViBE::CString(ss.str().c_str());
}

bool GUI::addTracksToMenu(void)
{
	::GtkWidget* menuRoot = GTK_WIDGET(gtk_builder_get_object(m_pInterface, "tracker-menu_tracks"));

	// Instead of carefully making sure that the menu is consistent, just rebuild the whole thing
	GtkWidget* menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(menuRoot));
	if (menu) {
		gtk_widget_destroy(menu);
	}

	const bool memorySaveMode = m_rTracker.getWorkspace().getMemorySaveMode();

	m_TrackMenuWidgets.clear();

	menu = gtk_menu_new();

	bool rulerState = m_KernelContext.getConfigurationManager().expandAsBoolean("${Tracker_Workspace_GUI_ShowRulers}", true);
	GtkWidget* menuItem = gtk_check_menu_item_new_with_label("Show rulers");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuItem), rulerState);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
	GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(menuItem), "activate", toggleRulerCB);
	gtk_widget_set_sensitive(menuItem, !memorySaveMode);
	// m_TrackMenuWidgets.push_back(menuItem);

	bool selectedOnlyState = m_KernelContext.getConfigurationManager().expandAsBoolean("${Tracker_Workspace_GUI_ShowSelectedOnly}", false);
	menuItem = gtk_check_menu_item_new_with_label("Show selected only");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuItem), selectedOnlyState);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
	GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(menuItem), "activate", toggleShowSelectedOnlyCB);

	GtkWidget* separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

	/// Box plugins menu
	menuItem = gtk_menu_item_new_with_label("Apply box plugin");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
	m_TrackMenuWidgets.push_back(menuItem);

	GtkWidget* subMenu = gtk_menu_new();
	m_TrackMenuWidgets.push_back(subMenu);

	auto& plugins = m_rTracker.getBoxPlugins().getBoxPlugins();
	for (size_t i = 0; i < plugins.size(); i++)
	{
		const CString name = plugins[i]->getBox().getName();

		GtkWidget* subMenuItem = gtk_menu_item_new_with_label(name.toASCIIString());
		gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), subMenuItem);
		g_object_set_data(G_OBJECT(subMenuItem), "applyBoxPlugin-cb", reinterpret_cast<void*>(i));
		GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(subMenuItem), "activate", applyBoxPluginCB);
		m_TrackMenuWidgets.push_back(subMenuItem);
	}

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);

	gtk_widget_show(menuItem);
	gtk_widget_show(subMenu);

	//// Tracker plugins menu
	menuItem = gtk_menu_item_new_with_label("Apply Tracker plugin");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
	m_TrackMenuWidgets.push_back(menuItem);

	subMenu = gtk_menu_new();
	m_TrackMenuWidgets.push_back(subMenu);

	auto& trackerPlugins = m_rTracker.getTrackerPlugins().getTrackerPlugins();
	for (size_t i = 0; i < trackerPlugins.size(); i++)
	{
		const std::string name = trackerPlugins[i]->getName();

		GtkWidget* subMenuItem = gtk_menu_item_new_with_label(name.c_str());
		gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), subMenuItem);
		g_object_set_data(G_OBJECT(subMenuItem), "trackerPlugin-cb", reinterpret_cast<void*>(i));
		GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(subMenuItem), "activate", trackerPluginCB);
		m_TrackMenuWidgets.push_back(subMenuItem);
	}

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);

	gtk_widget_show(menuItem);
	gtk_widget_show(subMenu);

	//// Per track

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

	{
		std::unique_lock<std::mutex>( m_rTracker.getWorkspace().getMutex() );
		const size_t numTracks =  m_rTracker.getWorkspace().getNumTracks();

		for (size_t i = 0; i < numTracks; i++)
		{
			stringstream ss; ss << "Track " << i + 1;
			GtkWidget* menuItem = gtk_menu_item_new_with_label(ss.str().c_str());
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);

			GtkWidget* subMenu = gtk_menu_new();

			GtkWidget* subMenuItem = gtk_menu_item_new_with_label("Delete Track");
			gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), subMenuItem);
			g_object_set_data(G_OBJECT(subMenuItem), "delete-track-cb", reinterpret_cast<void*>(i));
			GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(subMenuItem), "activate", deleteTrackCB);

			subMenuItem = gtk_menu_item_new_with_label("Move Track");
			gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), subMenuItem);
			g_object_set_data(G_OBJECT(subMenuItem), "move-track-cb", reinterpret_cast<void*>(i));
			GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(subMenuItem), "activate", moveTrackCB);

			GtkWidget* separator = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), separator);

			const StreamBundle* tr = m_rTracker.getWorkspace().getTrack(i);
			for (size_t j = 0; tr && j < tr->getNumStreams(); j++)
			{
				stringstream ss2; ss2 << "Stream " << j + 1;
				GtkWidget* subMenuItem = gtk_menu_item_new_with_label(ss2.str().c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(subMenu), subMenuItem);

				GtkWidget* streamMenuItems = gtk_menu_new();
			

				GtkWidget* tmp = gtk_menu_item_new_with_label("Show structure");
				gtk_menu_shell_append(GTK_MENU_SHELL(streamMenuItems), tmp);
				g_object_set_data(G_OBJECT(tmp), "show-chunks-cb", reinterpret_cast<void*>(m_Tracks[i]->m_Renderers[j]));
				GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(tmp), "activate", showChunksCB);
				gtk_widget_set_sensitive(tmp, !memorySaveMode);

				tmp = gtk_menu_item_new_with_label("Move stream");
				gtk_menu_shell_append(GTK_MENU_SHELL(streamMenuItems), tmp);
				g_object_set_data(G_OBJECT(tmp), "move-stream-cb-track", reinterpret_cast<void*>(i));
				g_object_set_data(G_OBJECT(tmp), "move-stream-cb-stream", reinterpret_cast<void*>(j));
				GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(tmp), "activate", moveStreamCB);
				gtk_widget_set_sensitive(tmp, !memorySaveMode);

				tmp = gtk_menu_item_new_with_label("Delete stream");
				gtk_menu_shell_append(GTK_MENU_SHELL(streamMenuItems), tmp);
				g_object_set_data(G_OBJECT(tmp), "delete-stream-cb-track", reinterpret_cast<void*>(i));
				g_object_set_data(G_OBJECT(tmp), "delete-stream-cb-stream", reinterpret_cast<void*>(j));
				GTK_CALLBACK_MAPPER_OBJECT_PARAM(GTK_OBJECT(tmp), "activate", deleteStreamCB);
				gtk_widget_set_sensitive(tmp, !memorySaveMode);

				gtk_menu_item_set_submenu(GTK_MENU_ITEM(subMenuItem), streamMenuItems);

				gtk_widget_show(subMenuItem);
			}
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);

			gtk_widget_show(menuItem);
			gtk_widget_show(subMenu);
		}

	}

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuRoot), menu);

	gtk_widget_show_all(menu);

	return true;
}

bool GUI::setPlaytimeWidgetState(bool enabled)
{
	// Before enabling any of these during play, test thoroughly that it works during it
	const std::vector<std::string> widgets{
		"tracker-button_play_pause"
		, "tracker-button_forward"
		, "tracker-menu_open_file"
		, "tracker-menu_open_workspace"	
		, "tracker-menu_open_processor"
		, "tracker-menu_clear" 
		, "tracker-menu_save"
		, "tracker-menu_saveas"
		, "tracker-menu_incrementrevsave"
		, "tracker-menu_select"
		, "tracker-menu_select_all","tracker-menu_select_none"
		, "tracker-menu_processor_preferences"
		, "tracker-menu_delete_all"
		, "tracker-select_tracks"
		, "tracker-processor_properties"
		, "tracker-menu_tracks"
		, "tracker-menu_workspace_about"
	};

	for (auto widgetName : widgets)
	{
		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, widgetName.c_str()));

		gtk_widget_set_sensitive(widget, enabled);
	}

	for (auto widget : m_TrackMenuWidgets)
	{
		gtk_widget_set_sensitive(widget, enabled);
	}

	return true;
}

bool GUI::updateIdleWidgetState(void)
{
	const std::vector<std::string> widgetsNeedingStreams{
		"tracker-menu_select"
		, "tracker-select_tracks"
		, "tracker-menu_select_all"
		, "tracker-menu_select_none"
		, "tracker-menu_delete_all"
		, "tracker-menu_tracks"
	};
	// n.b. actually the 'apply' widgets might be the only ones needing
	// selection, we must allow 'play' on empty for situations where the user only wants to record
	const std::vector<std::string> widgetsNeedingSelection;
	/*
	const std::vector<std::string> widgetsNeedingSelection{
		"tracker-button_play_pause"
		, "tracker-button_forward"
		, "tracker-button_stop"
	};
	*/

	const bool haveStreams = !m_rTracker.getWorkspace().getSelection().isEmpty();

	for (auto widgetName : widgetsNeedingStreams)
	{
		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, widgetName.c_str()));
		if(!widget)
		{
			log() << LogLevel_Error << "Error getting widget " << widgetName.c_str() << "\n";
			continue;
		}
		
		gtk_widget_set_sensitive(widget, haveStreams);
	}
	
	const bool haveSelection = m_rTracker.getWorkspace().getSelection().isSomethingSelected();

	for (auto widgetName : widgetsNeedingSelection)
	{
		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, widgetName.c_str()));
		if(!widget)
		{
			log() << LogLevel_Error << "Error getting widget " << widgetName.c_str() << "\n";
			continue;
		}
		
		gtk_widget_set_sensitive(widget, haveSelection);
	}

	for (auto widget : m_TrackMenuWidgets)
	{
		gtk_widget_set_sensitive(widget, haveSelection);
	}

	// some widgets need a processor
	bool hasProcessor = m_rTracker.getWorkspace().hasProcessor();
	const std::vector<std::string> widgetsNeedingProcessor{
	"tracker-button_play_pause"
	, "tracker-button_stop"
	, "tracker-button_forward"
	, "tracker-processor_properties" };
	for (auto widgetName : widgetsNeedingProcessor)
	{
		GtkWidget* widget = GTK_WIDGET(gtk_builder_get_object(m_pInterface, widgetName.c_str()));

		gtk_widget_set_sensitive(widget, hasProcessor);
	}

	return true;
}

bool GUI::storeRendererSettings(void)
{
	bool retVal = true;

	// get rid of possibly dangling tokens from deleted streams/tracks
	std::string prefix("Tracker_Workspace_GUI_Renderer_");
	m_rTracker.getWorkspace().wipeConfigurationTokens(prefix);

	// Store current ones
	for (size_t i = 0; i < m_Tracks.size(); i++)
	{
		if (!m_Tracks[i]) continue;

		for (size_t j = 0; j < m_Tracks[i]->m_Renderers.size(); j++)
		{
			std::stringstream prefix; prefix << "Tracker_Workspace_GUI_Renderer" 
				<< "_Track_" << std::setw(3) << std::setfill('0') << (i+1) 
				<< "_Stream_" << std::setw(2) << std::setfill('0') << (j+1);
			retVal &= m_Tracks[i]->m_Renderers[j]->storeSettings(prefix.str());
		}
	}

	return retVal;
}

bool GUI::updateRulerState(void)
{
	bool rulerState = m_KernelContext.getConfigurationManager().expandAsBoolean("${Tracker_Workspace_GUI_ShowRulers}", true);

	for (auto trk : m_Tracks)
	{
		for (auto renderer : trk->m_Renderers)
		{
			if(renderer)
			{
				renderer->setRulerVisibility(rulerState);
			}
		}
	}

	return true;
}
