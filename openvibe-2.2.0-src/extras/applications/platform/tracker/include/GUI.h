
#pragma once

#include "Tracker.h"
#include "Workspace.h"

#include "Stream.h"
#include "TypeSignal.h"
#include "TypeSpectrum.h"
#include "TypeStimulation.h"

#include "Contexted.h"

#include "Misc.h"

#include "CLogListenerTracker.h"

// Forward declare
struct _GtkBuilder;
typedef struct _GtkWidget   GtkWidget;
// typedef struct _GtkTable    GtkTable;

namespace OpenViBETracker {

class StreamRendererBase;

/**
 * \class GUI 
 * \brief The main GUI code for the OpenViBE Tracker. 
 * \author J. T. Lindgren
 *
 * \details
 * This file handles most of the GTK related GUI of the Tracker, with the actual functions modifying the data trying to be as much as possible 
 * implemented in the non-GUI classes. Although the GUI is currently quite monolithic, the rendering of the different Stream types has been
 * delegated to StreamRenderer* classes, which in turn use the Mensia Advanced Visualizations toolkit. Code from Designer is also used by 
 * making CMake include it.
 *
 */
class GUI : protected Contexted {

public:

	GUI(int argc, char* argv[], Tracker& app);
	~GUI();

	bool run();
	
	/**
	 * \class GUITrack
	 * \brief Holds the renderers for each stream of a Track, and the correspondig GTK table for the widgets.
	 * \author J. T. Lindgren
	 */
	class GUITrack
	{
	public:
		GUITrack() { };
		~GUITrack();

		GtkWidget* m_Frame = nullptr;

		// Renderer per stream
		std::vector<StreamRendererBase*> m_Renderers;
	};

protected: 

	// The main loop to do work, e.g. feed the tracks to processor
	bool step();

	OpenViBE::CString getWorkspaceInfo(void) const;

	bool addTracksToMenu(void);

	// Enable/disable certain widgets during play (processing)
	bool setPlaytimeWidgetState(bool enabled);
	// Enable/disable certain widgets depending on the current state (streams, processors, etc)
	bool updateIdleWidgetState(void);

	bool storeRendererSettings(void);

	// Callbacks
	bool addTrackCB(void);
	bool openWorkspaceCB(void);
	bool openProcessorCB(void);
	bool openProcessorCBFinal(void);
	bool saveAsCB(void);
	bool saveCB(void);
	bool incrementRevSaveCB(void);
	bool clearCB(void);
	bool quitCB(void);
	bool stopCB(void);
	bool playCB(void);
	bool playFastCB(void);
	bool workspaceInfoCB(void);
	bool aboutCB(void);

	bool selectWorkspacePathCB(void);
	bool selectWorkspacePathCBFinal(void);

	bool editSelectionCB(void);
	bool editSelectionCB2(void) { return editSelectionCB(); };
	bool selectAllCB(void);
	bool selectNoneCB(void);

	bool processorPreferencesCB(void);
	bool processorPreferencesButtonOkCB(void);
	bool processorPreferencesButtonCancelCB(void);
	bool processorEditCB(void);
	bool processorEditCB2(void);

	bool editNotesCB(void);
	bool editNotesCB2(void) { return editNotesCB(); }

	bool hScrollCB(GtkWidget* widget);
	bool hScaleCB(GtkWidget* widget);

	bool deleteAllTracksCB(void);
	bool deleteTrackCB(GtkWidget* widget);
	bool moveTrackCB(GtkWidget* widget);
	bool toggleRulerCB(GtkWidget* widget);
	bool toggleShowSelectedOnlyCB(GtkWidget* widget);

	bool showChunksCB(GtkWidget* widget);
	bool moveStreamCB(GtkWidget* widget);
	bool deleteStreamCB(GtkWidget* widget);

	bool applyBoxPluginCB(GtkWidget* widget);
	bool trackerPluginCB(GtkWidget* widget);
	bool setSelectionCB(GtkWidget* widget);

	bool clearMessagesCB(void);

	// Render a stream by pushing the chunks in view to the renderer
	template<typename T, typename Renderer> bool draw(const Stream<T>* stream, Renderer& renderer,
		ovtime_t startTime, ovtime_t endTime)
	{
		renderer.reset();

		typename T::Buffer* buf;

		// Push the visible chunks to renderer
		uint64_t chunksSent = 0;
		for (size_t i = 0; i<stream->getChunkCount(); i++)
		{
			if (stream->getChunk(i, &buf) && buf->m_startTime >= startTime && buf->m_endTime <= endTime)
			{
				renderer.push(*buf);
				chunksSent++;
			}
		}
		
		/*
		ovtime_t_t viewDuration = ceilFixedPoint(endTime - startTime);
		uint64_t chunkCount = viewDuration /renderer.getChunkDuration();

		std::cout << "View duration " << OpenViBE::ITimeArithmetics::timeToSeconds(viewDuration) << ", chks " << chunkCount << "\n";

		std::cout << "Pushed " << chunksSent << " chks, padding " << chunkCount-chunksSent << " -> " << chunkCount << "\n";
		if (buf && chunksSent < chunkCount)
		{
			while (chunksSent < chunkCount)
			{
				// @fixme push empty
				renderer.push(*buf, true);
				chunksSent++;
			}
		}
		*/
		renderer.finalize();

		return true;
	}

	bool redrawTrack(size_t index);
	bool redrawAllTracks(void);
	bool redrawStream(size_t trackIndex, size_t streamIndex);
	bool resetWidgetProperties(void);

	bool initializeRenderers(void);
	bool clearRenderers(void);
	bool updateRulerState(void);

	Tracker& m_rTracker;

	std::vector<GUITrack*> m_Tracks; // One per each track in workspace

	struct _GtkBuilder* m_pInterface = nullptr;
	
	GtkWidget* m_hMainWindow = nullptr;
	GtkWidget* m_hScrollbar = nullptr;
	GtkWidget* m_hScale = nullptr;
	GtkWidget* m_hWorkspaceLabel = nullptr;
	GtkWidget* m_hTimeDisplay = nullptr;
	GtkWidget* m_hTrackCounter = nullptr;

	GtkWidget* m_hSelectionWindow = nullptr;

	uint64_t m_numChannels = 0;
	uint64_t m_chunkSize = 0;
	uint64_t m_totalChunks = 0;

	ovtime_t m_PreviousTime = 0;

	ovtime_t m_LastRedraw = 0;

#if defined(TARGET_OS_Windows)
	int32_t m_RequestResetInNFrames = -1;
	bool m_ExpanderOpen = false;
	double m_HScrollBarValue = 0;
#endif

	bool m_RequestRedraw = false;
	bool m_WaitingForStop = false;

	OpenViBETracker::CLogListenerTracker* m_pLogListener = nullptr;

	// This hack is needed since on Ubuntu 16.04, for some reason
	// gtk_widget_set_sensitive doesn't work on main level menu items,
	// and descending the menu as a container doesn't seem to allow
	// graying out the menu items either
	std::vector<GtkWidget*> m_TrackMenuWidgets;

};

};

