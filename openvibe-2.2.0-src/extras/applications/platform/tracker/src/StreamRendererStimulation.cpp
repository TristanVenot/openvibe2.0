//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 
// @note This renderer is intended to be used when there is only 
// a stimulation track. More often, the Tracker users might want to
// see the stimulations overlayed on the signal track.
//
#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include <mensia/advanced-visualization.h>
#include <mTGtkGLWidget.hpp>
#include <m_GtkGL.hpp>

#include "StreamRendererStimulation.h"
#include "TypeStimulation.h"

using namespace Mensia;

#include "ruler/mTRulerAutoType.hpp"
#include "ruler/mTRulerPair.hpp"
#include "ruler/mTRulerConditionalPair.hpp"
#include "ruler/mCRulerConditionIsTimeLocked.hpp"

#include "ruler/mCRulerProgressV.hpp"

#include "ruler/mCRulerBottomCount.hpp"
#include "ruler/mCRulerBottomTime.hpp"

#include "ruler/mCRulerLeftChannelNames.hpp"

#include "ruler/mCRulerRightCount.hpp"
#include "ruler/mCRulerRightScale.hpp"
#include "ruler/mCRulerRightLabels.hpp"

using namespace Mensia::AdvancedVisualization;
using namespace OpenViBETracker;

bool StreamRendererStimulation::initialize(void)
{
	m_ChannelCount = 1;
	m_SamplingRate = 512;
	m_SamplesPerChunk = 32;
	m_ChunkDuration = OpenViBE::ITimeArithmetics::sampleCountToTime(m_SamplingRate, m_SamplesPerChunk);

	// TRendererStimulation < false, CRendererLine >:new CRendererLine
	AdvancedVisualization::IRenderer* pRenderer = AdvancedVisualization::IRenderer::create(AdvancedVisualization::IRenderer::ERendererType::RendererType_Line, true);
	if(pRenderer == NULL) return false;

	// Creates renderer context
	m_pRendererContext = AdvancedVisualization::IRendererContext::create();
	m_pRendererContext->clear();
	m_pRendererContext->setTimeScale(1);
	m_pRendererContext->setCheckBoardVisibility(true);
	m_pRendererContext->setScaleVisibility(m_bIsScaleVisible);
	m_pRendererContext->setDataType(AdvancedVisualization::IRendererContext::DataType_Signal);
	m_pRendererContext->addChannel("Stims");
	m_pRendererContext->setSampleDuration(OpenViBE::ITimeArithmetics::sampleCountToTime(m_SamplingRate, 1));

	m_pRuler = new TRulerPair < CRulerProgressV, TRulerPair < TRulerAutoType < IRuler, TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, IRuler >, TRulerPair < CRulerLeftChannelNames, CRulerRightScale > > >;

	m_pRuler->setRendererContext(m_pRendererContext);
	m_pRuler->setRenderer(pRenderer);

	if(!StreamRendererBase::initialize())
	{
		return false;
	}

	m_oGtkGLWidget.initialize(*this, m_pViewport, m_pLeft, m_pRight, m_pBottom);
	m_oGtkGLWidget.setPointSmoothingActive(false);
	
	m_vRenderer.push_back(pRenderer);

	gtk_widget_set_size_request(m_pMain, 640, 100);

	return true;
}

bool StreamRendererStimulation::reset(ovtime_t startTime, ovtime_t endTime)
{
	m_StartTime = startTime;
	m_EndTime = endTime;

//	std::cout << "Overridden stimulation renderer reset\n";

	m_vRenderer[0]->clear(0);

	const uint64_t chunkCount = (m_EndTime - m_StartTime) / m_ChunkDuration;
	const uint32_t numSamples = static_cast<uint32_t>(m_SamplesPerChunk * chunkCount);

	m_vRenderer[0]->setChannelCount(m_ChannelCount);
	m_vRenderer[0]->setSampleCount(numSamples);

	//  @FIXME The offset is needed to have correct numbers on the ruler; remove ifdef once the feature is in
#ifdef RENDERER_SUPPORTS_OFFSET
	m_vRenderer[0]->setTimeOffset(m_StartTime);
#endif

	// Stick in empty chunks to get a background
	std::vector<float> empty;
	empty.resize(m_SamplesPerChunk * 1, 0);  // 1 channel
	for (uint64_t i = 0; i < chunkCount; i++)
	{
		m_vRenderer[0]->feed(&empty[0], m_SamplesPerChunk);
	}

	m_vRenderer[0]->rebuild(*m_pRendererContext);

	return true;
}

bool StreamRendererStimulation::push(const TypeStimulation::Buffer& chunk, bool zeroInput /* = false */)
{

	for (size_t i = 0; i < chunk.m_buffer.getStimulationCount(); i++)
	{
		m_vRenderer[0]->feed(chunk.m_buffer.getStimulationDate(i) - m_StartTime,
			chunk.m_buffer.getStimulationIdentifier(i));
	}
	
	return true;
}

OpenViBE::CString StreamRendererStimulation::renderAsText(uint32_t indent) const
{
	// No specific details for stimulation streams
	std::stringstream ss;
	return OpenViBE::CString(ss.str().c_str());
}

bool StreamRendererStimulation::showChunkList(void)
{
	if (m_StimulationListWindow)
	{
		gtk_window_present(GTK_WINDOW(m_StimulationListWindow));
		return true;
	}

	GtkBuilder* pBuilder = gtk_builder_new();
	const OpenViBE::CString l_sFilename = OpenViBE::Directories::getDataDir() + "/applications/tracker/tracker.ui";
	if (!gtk_builder_add_from_file(pBuilder, l_sFilename, NULL)) {
		std::cout << "Problem loading [" << l_sFilename << "]\n";
		return false;
	}

	m_StimulationListWindow = GTK_WIDGET(::gtk_builder_get_object(pBuilder, "tracker-stimulation_list"));
	GtkTreeView* m_pChannelTreeView = GTK_TREE_VIEW(::gtk_builder_get_object(pBuilder, "tracker-stimulation_list-treeview"));
	//	GtkListStore* m_pChannelListStore = GTK_LIST_STORE(::gtk_builder_get_object(pBuilder, "liststore_select"));
	GtkTreeStore* m_pChannelListStore = gtk_tree_store_new(7,
		G_TYPE_UINT, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_DOUBLE, G_TYPE_UINT64, G_TYPE_STRING, G_TYPE_DOUBLE);

	gtk_window_set_title(GTK_WINDOW(m_StimulationListWindow), "List of Stimulations in the stream");

	add_column(m_pChannelTreeView, "Chunk#", 0, 20);
	add_column(m_pChannelTreeView, "ChunkStart (s)", 1, 10);
	add_column(m_pChannelTreeView, "ChunkEnd (s)", 2, 10);
	add_column(m_pChannelTreeView, "Stim Time (s)", 3, 20);
	add_column(m_pChannelTreeView, "Stim Id", 4, 5);
	add_column(m_pChannelTreeView, "Stim Name", 5, 40);
	add_column(m_pChannelTreeView, "Stim Duration", 6, 10);

	::gtk_tree_view_set_model(m_pChannelTreeView, GTK_TREE_MODEL(m_pChannelListStore));

	::GtkTreeIter l_oGtkTreeIterator;
	::gtk_tree_store_clear(m_pChannelListStore);

	//	::gtk_tree_view_set_model(m_pChannelTreeView, NULL);
	for (size_t i = 0; i < m_Stream->getChunkCount(); i++)
	{
		const TypeStimulation::Buffer* ptr = m_Stream->getChunk(i);
		if (!ptr)
		{
			break;
		}

		for (uint32_t s = 0; s < ptr->m_buffer.getStimulationCount(); s++)
		{
			const OpenViBE::CString stimName = m_KernelContext.getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, ptr->m_buffer.getStimulationIdentifier(s));

			::gtk_tree_store_append(m_pChannelListStore, &l_oGtkTreeIterator, nullptr);
			::gtk_tree_store_set(m_pChannelListStore, &l_oGtkTreeIterator,
				0, i,
				1, OpenViBE::ITimeArithmetics::timeToSeconds(ptr->m_startTime),
				2, OpenViBE::ITimeArithmetics::timeToSeconds(ptr->m_endTime),
				3, OpenViBE::ITimeArithmetics::timeToSeconds(ptr->m_buffer.getStimulationDate(s)),
				4, ptr->m_buffer.getStimulationIdentifier(s),
				5, stimName.toASCIIString(),
				6, OpenViBE::ITimeArithmetics::timeToSeconds(ptr->m_buffer.getStimulationDuration(s)),
				-1);
		}
	}

	//	GList* cols = gtk_tree_view_get_columns(m_pChannelTreeView);

	// Hide instead of destroy on closing the window
	g_signal_connect(m_StimulationListWindow, "delete_event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);

	gtk_widget_show_all(GTK_WIDGET(m_StimulationListWindow));

	g_object_unref(pBuilder);

	return true;
}

bool StreamRendererStimulation::mouseButton(int32_t x, int32_t y, int32_t button, int status)
{
	if (button == 3 && status == 1)
	{
		//		showStimulationList();
	}

	return StreamRendererBase::mouseButton(x, y, button, status);
}
