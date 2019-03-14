//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include <mensia/advanced-visualization.h>
#include <mTGtkGLWidget.hpp>
#include <m_GtkGL.hpp>

#include "StreamRendererSignal.h"

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

#include "Misc.h"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;
using namespace OpenViBETracker;

bool StreamRendererSignal::initialize(void)
{
	// TRendererStimulation < false, CRendererLine >:new CRendererLine
	AdvancedVisualization::IRenderer* pRenderer = AdvancedVisualization::IRenderer::create(AdvancedVisualization::IRenderer::ERendererType::RendererType_Line, true);
	if(pRenderer == NULL) return false;

	const TypeSignal::Buffer* firstChunk = m_Stream->getChunk(0);
	if (!firstChunk)
	{
		return false;
	}

	m_ChannelCount = firstChunk->m_buffer.getDimensionSize(0);
	m_SamplesPerChunk = firstChunk->m_buffer.getDimensionSize(1);
	m_ChunkDuration = firstChunk->m_endTime - firstChunk->m_startTime;

	// Creates renderer context
	m_pRendererContext = AdvancedVisualization::IRendererContext::create();
	m_pRendererContext->clear();
	m_pRendererContext->setScale(1.0);
	m_pRendererContext->setTimeScale(1);
	m_pRendererContext->setCheckBoardVisibility(true);
	m_pRendererContext->setScaleVisibility(m_bIsScaleVisible);
	m_pRendererContext->setDataType(AdvancedVisualization::IRendererContext::DataType_Signal);
	const ovtime_t sampleDuration = m_ChunkDuration / ((ovtime_t)m_SamplesPerChunk);
	m_pRendererContext->setSampleDuration(sampleDuration);

	const TypeSignal::Header& hdr = m_Stream->getHeader();
	for (uint32_t i = 0; i<m_ChannelCount; i++)
	{
		const char* label = hdr.m_header.getDimensionLabel(0, i);
		m_pRendererContext->addChannel(std::string(label));
	}
	
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

	return true;
}

bool StreamRendererSignal::reset(ovtime_t startTime, ovtime_t endTime)
{
	m_StartTime = startTime;
	m_EndTime = endTime;

// 	std::cout << "Overridden signal renderer reset\n";

	m_vRenderer[0]->clear(0);

//	std::cout << "Start time is " << OpenViBE::ITimeArithmetics::timeToSeconds(m_StartTime)
//		<< " end is " << OpenViBE::ITimeArithmetics::timeToSeconds(m_EndTime) << "\n";

	const uint64_t chunkCount = ceilFixedPoint( m_EndTime - m_StartTime ) / m_ChunkDuration;
	const uint32_t numSamples = static_cast<uint32_t>(m_SamplesPerChunk * chunkCount);

	m_vRenderer[0]->setSampleCount(numSamples);
	m_vRenderer[0]->setChannelCount(m_ChannelCount);

	// @FIXME The offset is needed to have correct numbers on the ruler; remove ifdef once the feature is in
#ifdef RENDERER_SUPPORTS_OFFSET
	m_vRenderer[0]->setTimeOffset(m_StartTime);
#endif

	// m_vRenderer[0]->setHistoryDrawIndex(samplesBeforeStart);
	m_vRenderer[0]->rebuild(*m_pRendererContext);

	return true;
}

bool StreamRendererSignal::push(const TypeSignal::Buffer& chunk, bool zeroInput /* = false */)
{
	std::vector<float> tmp;
	if (!zeroInput) {
		tmp.resize(chunk.m_buffer.getBufferElementCount());
		for (size_t i = 0; i < chunk.m_buffer.getBufferElementCount(); i++)
		{
			tmp[i] = static_cast<float>(chunk.m_buffer.getBuffer()[i]);
		}
	}
	else
	{
		tmp.resize(chunk.m_buffer.getBufferElementCount(), 0);
	}
	
	m_vRenderer[0]->feed(&tmp[0], chunk.m_buffer.getDimensionSize(1));
	
	return true;
}

OpenViBE::CString StreamRendererSignal::renderAsText(uint32_t indent) const
{
	auto& hdr = m_Stream->getHeader();

	std::stringstream ss;
	ss << std::string(indent, ' ') << "Sampling rate: " << hdr.m_SamplingRate << "hz" << std::endl;
	ss << std::string(indent, ' ') << "Channels: " << hdr.m_header.getDimensionSize(0) << std::endl;
	ss << std::string(indent, ' ') << "Samples per chunk: " << hdr.m_header.getDimensionSize(1) << std::endl;

	return OpenViBE::CString(ss.str().c_str());
}

bool StreamRendererSignal::mouseButton(int32_t x, int32_t y, int32_t button, int status)
{
	/*
	if (button == 3 && status == 1)
	{
		showChunkList();
	}
	*/

	return StreamRendererBase::mouseButton(x, y, button, status);
}

bool StreamRendererSignal::showChunkList(void)
{
	return showMatrixList<TypeSignal>(m_Stream, &m_StreamListWindow, "List of chunks for Signal stream");
}

