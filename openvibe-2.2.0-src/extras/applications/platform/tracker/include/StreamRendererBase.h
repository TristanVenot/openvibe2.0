//
// OpenViBE Tracker
//


#pragma once

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>
#include <memory> // shared_ptr

#include <mensia/advanced-visualization.h>
#include <mTGtkGLWidget.hpp>
#include <m_GtkGL.hpp>

#include <mIRuler.hpp>

#include "Stream.h"
#include "Contexted.h"

typedef struct _GtkWidget GtkWidget;

namespace OpenViBETracker
{

/**
 * \class StreamRendererBase 
 * \brief Abstract, non-typed class for visually rendering the Streams in the Tracker GUI
 * \details See the derived, typed versions of the class
 *
 * \author J. T. Lindgren
 *
 */
class StreamRendererBase : protected Contexted {
public:
	StreamRendererBase(const OpenViBE::Kernel::IKernelContext&ctx ) : Contexted(ctx) { };
	virtual ~StreamRendererBase() { };

	// Calls for API users

	// Initialize the renderer
	virtual bool initialize(void);
	virtual bool uninitialize(void);
	
	// Show the windows
	virtual bool realize(void);

	virtual bool setTitle(const char *title);

	// Draw chunks in range [startTime,endTime]
	virtual bool spool(ovtime_t startTime, ovtime_t endTime) = 0;

	virtual OpenViBE::CString renderAsText(uint32_t indent = 0) const;

	virtual bool showChunkList(void) { log() << OpenViBE::Kernel::LogLevel_Warning << "Chunk listing not implemented for stream type (or unavailable in current Tracker mode)\n"; return true; };

	virtual ovtime_t getChunkDuration(void) const { return m_ChunkDuration; };

	virtual bool setRulerVisibility(bool isVisible);

	// Save/load settings as configuration manager tokens
	// The tokens loaded/saved will have the given prefix
	virtual bool restoreSettings(const std::string& prefix);
	virtual bool storeSettings(const std::string& prefix);

	GtkWidget* getWidget(void) { return m_pMain; };
	
	// GtkGlWidget calls the following

	virtual bool redraw(bool bImmediate = false);
	virtual bool reshape(uint32_t width, uint32_t height);
	virtual bool draw(void);
	virtual bool preDraw(void);
	virtual bool postDraw(void);
	virtual void drawLeft(void);
	virtual void drawRight(void);
	virtual void drawBottom(void);
	virtual bool mouseButton(int32_t x, int32_t y, int32_t button, int status);
	virtual bool mouseMotion(int32_t x, int32_t y);

	virtual bool keyboard(uint32_t width, uint32_t height, uint32_t value, bool unused) { std::cout << "keyb requested\n";  return true; };


protected:

	// Resets the renderer aperture (number of samples/chunks)
	virtual bool reset(ovtime_t startTime, ovtime_t endTime) = 0;
	virtual bool updateRulerVisibility(void);
	// After pushing samples, request rebuild & redraw
	virtual bool finalize(void);

	ovtime_t m_StartTime = 0;
	ovtime_t m_EndTime = 0;
	ovtime_t m_ChunkDuration = 0;

	uint32_t m_Width = 640;
	uint32_t m_Height = 480;
	uint32_t m_TextureId = 0;

	bool m_bRotate = false;
	bool m_bIsScaleVisible = true;

	typedef struct
	{
		float r;
		float g;
		float b;
	} TColor;

	TColor m_oColor;

	GtkWidget* m_pViewport = nullptr;
	GtkWidget* m_pMain = nullptr;
	GtkWidget* m_pTop = nullptr;
	GtkWidget* m_pLeft = nullptr;
	GtkWidget* m_pRight = nullptr;
	GtkWidget* m_pBottom = nullptr;
	GtkWidget* m_pCornerLeft = nullptr;
	GtkWidget* m_pCornerRight = nullptr;

	std::string m_sTitle;

	std::vector<Mensia::AdvancedVisualization::IRenderer*> m_vRenderer;

	Mensia::AdvancedVisualization::IRendererContext* m_pRendererContext = nullptr;
	Mensia::AdvancedVisualization::IRendererContext* m_pSubRendererContext = nullptr;

	Mensia::AdvancedVisualization::TGtkGLWidget < StreamRendererBase > m_oGtkGLWidget;

	Mensia::AdvancedVisualization::IRuler* m_pRuler = nullptr;

	// mouse related
	std::map < int32_t, int > m_vButton;
	int32_t m_i32MouseX = 0;
	int32_t m_i32MouseY = 0;
	bool m_bMouseInitialized = false;

};

/**
 * \class spoolImpl
 * \brief Push chunks in a specified interval to a renderer
 * \details
 *
 * A generic template that is used internally by the classes derived from StreamRendererBase
 * to implement spool(t1,t2) : pushing a [t1,t2] interval of chunks to the renderer.
 *
 * \author J. T. Lindgren
 *
 */
template<typename T, typename Renderer> bool spoolImpl(std::shared_ptr< const Stream<T> > stream, Renderer& renderer, ovtime_t startTime, ovtime_t endTime)
{
	renderer.reset(startTime, endTime);

	typename T::Buffer* buf;

	// Push the visible chunks to renderer
	uint64_t chunksSent = 0;
	for (size_t i = 0; i < stream->getChunkCount(); i++)
	{
		if (stream->getChunk(i, &buf) && buf->m_startTime >= startTime && buf->m_endTime <= endTime)
		{
			renderer.push(*buf);
			chunksSent++;
		}
	}

	renderer.finalize();

	return true;
}

// Helper function for showMatrixList(), adds a column to a tree view, @fixme move somewhere else
void add_column(GtkTreeView* treeView, const char* name, uint32_t id, uint32_t minWidth);

/**
 * \class showMatrixList
 * \brief Generic template that can be used to draw lists of any stream that derives from the matrix type.
 * @fixme might refactor somewhere else
 * \author J. T. Lindgren
 *
 */
template<typename T> bool showMatrixList(std::shared_ptr< const Stream<T> > stream, GtkWidget **listWindow, const char *title)
{
	if (*listWindow)
	{
		gtk_window_present(GTK_WINDOW(*listWindow));
		return true;
	}

	GtkBuilder* pBuilder = gtk_builder_new();
	const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/tracker/tracker.ui";
	if (!gtk_builder_add_from_file(pBuilder, l_sFilename, NULL)) {
		std::cout << "Problem loading [" << l_sFilename << "]\n";
		return false;
	}

	*listWindow = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "tracker-stimulation_list"));
	GtkTreeView* m_pChannelTreeView = GTK_TREE_VIEW(::gtk_builder_get_object(pBuilder, "tracker-stimulation_list-treeview"));
	//	GtkListStore* m_pChannelListStore = GTK_LIST_STORE(::gtk_builder_get_object(pBuilder, "liststore_select"));
	GtkTreeStore* m_pChannelListStore = gtk_tree_store_new(4,
		G_TYPE_UINT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_STRING);

	gtk_window_set_title(GTK_WINDOW(*listWindow), title);

	add_column(m_pChannelTreeView, "Chunk#", 0, 10);
	add_column(m_pChannelTreeView, "ChunkStart (s)", 1, 5);
	add_column(m_pChannelTreeView, "ChunkEnd (s)", 2, 5);
	add_column(m_pChannelTreeView, "Chunk dims", 3, 10);

	::gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));

	::GtkTreeIter l_oGtkTreeIterator;
	::gtk_tree_store_clear(m_pChannelListStore);

	//	::gtk_tree_view_set_model(m_pChannelTreeView, NULL);
	for (size_t i = 0; i < stream->getChunkCount(); i++)
	{
		const typename T::Buffer* ptr = stream->getChunk(i);
		if (!ptr)
		{
			break;
		}

		std::stringstream ss;
		ss << ptr->m_buffer.getDimensionCount() << " : ";
		for (uint32_t j = 0; j<ptr->m_buffer.getDimensionCount(); j++)
		{
			ss << (j>0 ? " x " : "") << ptr->m_buffer.getDimensionSize(j);
		}

		::gtk_tree_store_append(m_pChannelListStore, &l_oGtkTreeIterator, nullptr);
		::gtk_tree_store_set(m_pChannelListStore, &l_oGtkTreeIterator,
			0, i,
			1, OpenViBE::ITimeArithmetics::timeToSeconds(ptr->m_startTime),
			2, OpenViBE::ITimeArithmetics::timeToSeconds(ptr->m_endTime),
			3, ss.str().c_str(),
			-1);
	}

	// Hide instead of destroy on closing the window
	g_signal_connect(*listWindow, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	gtk_widget_show_all(GTK_WIDGET(*listWindow));

	g_object_unref(pBuilder);

	return true;
}

};
