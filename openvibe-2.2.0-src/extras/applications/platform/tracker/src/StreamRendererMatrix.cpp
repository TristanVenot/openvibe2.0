//
// OpenViBE Tracker
//
// @author J.T. Lindgren
// 
// @todo add horizontal scaling support
// @todo add event handlers
// @todo add ruler, stimulations, channel names, a million of other things

#include <openvibe/ov_all.h>
#include <toolkit/ovtk_all.h>
#include <iostream>

#include <mensia/advanced-visualization.h>
#include <m_GtkGL.hpp>

#include <system/ovCTime.h>
#include <openvibe/ovITimeArithmetics.h>

#include "StreamRendererMatrix.h"


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
#include "ruler/mCRulerRightFrequency.hpp"

#include "ruler/mCRulerRightTexture.hpp"

#include "Misc.h"

#include <openvibe/ovITimeArithmetics.h>

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;

using namespace OpenViBE::Kernel;
using namespace OpenViBETracker;

bool StreamRendererMatrix::initialize(void)
{
	const TypeMatrix::Buffer* firstChunk = m_Stream->getChunk(0);
	if (firstChunk)
	{
		m_ChunkDuration = firstChunk->m_endTime - firstChunk->m_startTime;
		
		if (firstChunk->m_buffer.getDimensionCount() == 1)
		{
			// Degenerate case like a feature vector
			m_NumRows = firstChunk->m_buffer.getDimensionSize(0);
			m_NumCols = 0;
		}
		else if (firstChunk->m_buffer.getDimensionCount() == 2)
		{
			m_NumRows = firstChunk->m_buffer.getDimensionSize(0);
			m_NumCols = firstChunk->m_buffer.getDimensionSize(1);
		}
		else if (firstChunk->m_buffer.getDimensionCount() > 2)
		{
			std::cout << "Warning: The matrix renderer does not work correctly if dims>2 (for tensors)\n";
			m_NumRows = firstChunk->m_buffer.getDimensionSize(0);
			m_NumCols = firstChunk->m_buffer.getDimensionSize(1);
		}
		else
		{
			log() << LogLevel_Error << "Error: Dimension count " << firstChunk->m_buffer.getDimensionCount() << " not supported\n";
		}
	}
	else
	{
		std::cout << "Stream is empty\n";
		return false;
	}


	m_pRendererContext = IRendererContext::create();
	m_pRendererContext->clear();
	m_pRendererContext->setTimeScale(1);
	m_pRendererContext->setScaleVisibility(m_bIsScaleVisible);
	m_pRendererContext->setCheckBoardVisibility(true);
	m_pRendererContext->setTimeLocked(true);
	m_pRendererContext->setPositiveOnly(false);
	m_pRendererContext->setDataType(AdvancedVisualization::IRendererContext::DataType_Matrix);
	m_pRendererContext->setSampleDuration(m_ChunkDuration);
	m_pRendererContext->setParentRendererContext(&getContext());

	m_pRendererContext->setAxisDisplay(true);

	auto& hdr = m_Stream->getHeader();

	for (uint32_t j = 0; j<m_NumRows; j++)
	{
		m_pRendererContext->addChannel(std::string(hdr.m_header.getDimensionLabel(0, j)));
	}

	m_Swap.resize(m_NumRows);

	m_vRenderer.push_back(AdvancedVisualization::IRenderer::create(AdvancedVisualization::IRenderer::RendererType_Bitmap, false));

	m_pRuler = new TRulerPair < CRulerProgressV, TRulerPair < TRulerAutoType < IRuler, TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, IRuler >, TRulerPair < CRulerLeftChannelNames, CRulerRightTexture > > >;

//	m_pRuler = new TRulerPair < TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, TRulerPair < TRulerAutoType < IRuler, IRuler, CRulerRightFrequency >, TRulerPair < CRulerLeftChannelNames, CRulerProgressV > > >;
	m_pRuler->setRendererContext(m_pRendererContext);
	m_pRuler->setRenderer(m_vRenderer[0]);

	if(!StreamRendererBase::initialize())
	{
		return false;
	}

	m_oGtkGLWidget.initialize(*this, m_pViewport, m_pLeft, m_pRight, m_pBottom);
	m_oGtkGLWidget.setPointSmoothingActive(false);

	return true;
}



bool StreamRendererMatrix::reset(ovtime_t startTime, ovtime_t endTime)
{
	m_StartTime = startTime;
	m_EndTime = endTime;

	const uint32_t numBuffers = static_cast<uint32_t>(ceilFixedPoint(m_EndTime - m_StartTime) / m_ChunkDuration);

	m_pRendererContext->setElementCount(numBuffers);

	m_vRenderer[0]->clear(0);
	m_vRenderer[0]->setSampleCount(numBuffers); // $$$
	m_vRenderer[0]->setChannelCount(m_NumRows);

	//  @FIXME  The offset is needed to have correct numbers on the ruler; remove ifdef once the feature is in
#ifdef RENDERER_SUPPORTS_OFFSET
	m_vRenderer[0]->setTimeOffset(m_StartTime);
#endif

	return true;
}
	
#include <openvibe/ovITimeArithmetics.h>

bool StreamRendererMatrix::push(const TypeMatrix::Buffer& chunk, bool zeroInput /* = false */)
{
#if 0
	static uint32_t pushed = 0;
	std::cout << "Push spec chk " << pushed << " " << chunk.m_buffer.getDimensionSize(0)
		<< " " << chunk.m_buffer.getDimensionSize(1) << " "
		<< OpenViBE::ITimeArithmetics::timeToSeconds(chunk.m_startTime) << "," 
		<< OpenViBE::ITimeArithmetics::timeToSeconds(chunk.m_endTime)
		<< "\n";
	std::cout << pushed << " first bytes "
		<< chunk.m_buffer.getBuffer()[0]
		<< chunk.m_buffer.getBuffer()[1]
		<< chunk.m_buffer.getBuffer()[2]
		<< chunk.m_buffer.getBuffer()[3]
		<< "\n";
	pushed++;
#endif


	m_pRendererContext->setSpectrumFrequencyRange(uint32_t((ovtime_t(m_NumRows) << 32) / m_ChunkDuration));

	// Handle the degenerate case NumCols=0
	const size_t actualCols = std::max<size_t>(m_NumCols, 1);

	// Feed renderer with actual samples
	for (uint32_t j = 0; j<actualCols; j++)
	{
		for (uint32_t k = 0; k<m_NumRows; k++)
		{
			m_Swap[k] = float(chunk.m_buffer.getBuffer()[k*actualCols + j]);
		}
		m_vRenderer[0]->feed(&m_Swap[0]);
	}

	return true;
}


bool StreamRendererMatrix::draw(void)
{
	StreamRendererMatrix::preDraw();

	::glPushAttrib(GL_ALL_ATTRIB_BITS);
	::glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
	m_vRenderer[0]->render(*m_pRendererContext);
	::glPopAttrib();

	StreamRendererMatrix::postDraw();

	return true;
}

bool StreamRendererMatrix::preDraw(void)
{
	this->updateRulerVisibility();

	//	auto m_sColorGradient=OpenViBE::CString("0:0,0,0; 100:100,100,100");

	auto sColorGradient = OpenViBE::CString("0:0, 0, 50; 25:0, 100, 100; 50:0, 50, 0; 75:100, 100, 0; 100:75, 0, 0");


//	auto m_sColorGradient = OpenViBE::CString("0:100, 100, 100; 12:50, 100, 100; 25:0, 50, 100; 38:0, 0, 50; 50:0, 0, 0; 62:50, 0, 0; 75:100, 50, 0; 88:100, 100, 50; 100:100, 100, 100");

	if (!m_TextureId)
	{
		m_TextureId = m_oGtkGLWidget.createTexture(sColorGradient.toASCIIString());
	}
	::glBindTexture(GL_TEXTURE_1D, m_TextureId);

	m_pRendererContext->setAspect(m_pViewport->allocation.width * 1.f / m_pViewport->allocation.height);

	return true;
}

bool StreamRendererMatrix::finalize(void)
{

	m_vRenderer[0]->rebuild(*m_pRendererContext);
	m_vRenderer[0]->refresh(*m_pRendererContext);

	redraw(true);

	return true;
}

OpenViBE::CString StreamRendererMatrix::renderAsText(uint32_t indent) const
{
	std::stringstream ss;
	ss << std::string(indent, ' ') << "Rows: " << m_NumRows << std::endl;
	ss << std::string(indent, ' ') << "Cols: " << m_NumCols << std::endl;

//	ss << std::string(indent, ' ') << "Channels: " << m_Header.m_header.getDimensionSize(0) << std::endl;
//	ss << std::string(indent, ' ') << "Samples per chunk: " << m_Header.m_header.getDimensionSize(1) << std::endl;
	return OpenViBE::CString(ss.str().c_str());
}

bool StreamRendererMatrix::mouseButton(int32_t x, int32_t y, int32_t button, int status)
{
	/*
	if (button == 3 && status == 1)
	{
		showChunkList();
	}
	*/

	return StreamRendererBase::mouseButton(x, y, button, status);
}

bool StreamRendererMatrix::showChunkList(void)
{
	return showMatrixList<TypeMatrix>(m_Stream, &m_StreamListWindow, "List of chunks for Matrix stream");
}

