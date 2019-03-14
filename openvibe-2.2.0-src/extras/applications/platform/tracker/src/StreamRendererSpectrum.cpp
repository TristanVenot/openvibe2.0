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

#include "StreamRendererSpectrum.h"


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

#include "Misc.h"

using namespace Mensia;
using namespace Mensia::AdvancedVisualization;
using namespace OpenViBETracker;

bool StreamRendererSpectrum::initialize(void)
{
	const TypeSpectrum::Buffer* firstChunk = m_Stream->getChunk(0);
	if (!firstChunk)
	{
		return false;
	}

	m_ChannelCount = firstChunk->m_buffer.getDimensionSize(0);
	m_SpectrumElements = firstChunk->m_buffer.getDimensionSize(1);
	m_ChunkDuration = firstChunk->m_endTime - firstChunk->m_startTime;

	m_pRendererContext = IRendererContext::create();
	m_pRendererContext->clear();
	m_pRendererContext->setTimeScale(1);
	m_pRendererContext->setScaleVisibility(m_bIsScaleVisible);
	m_pRendererContext->setCheckBoardVisibility(true);
	m_pRendererContext->setTimeLocked(true);
	m_pRendererContext->setDataType(AdvancedVisualization::IRendererContext::DataType_Spectrum);
	m_pRendererContext->setSampleDuration(m_ChunkDuration);

	const TypeSpectrum::Header& hdr = m_Stream->getHeader();
	for (uint32_t j = 0; j<m_ChannelCount; j++)
	{
		const char* name = hdr.m_header.getDimensionLabel(0, j);
		m_pRendererContext->addChannel(std::string(name));
	}

	m_pSubRendererContext = IRendererContext::create();
	m_pSubRendererContext->clear();
	m_pSubRendererContext->setParentRendererContext(m_pRendererContext);
	m_pSubRendererContext->setTimeLocked(true);
	m_pSubRendererContext->setStackCount(m_ChannelCount);
	m_pSubRendererContext->setDataType(IRendererContext::DataType_Spectrum);
	m_pSubRendererContext->setSampleDuration(m_ChunkDuration);

	for (uint32_t j = 0; j<m_SpectrumElements; j++)
	{
		std::stringstream ss; ss << j;
		m_pSubRendererContext->addChannel(ss.str());
	}

	m_vSwap.resize(m_SpectrumElements);

	for (size_t i = 0; i<m_vRenderer.size(); i++)
	{
		AdvancedVisualization::IRenderer::release(m_vRenderer[i]);
	}
	m_vRenderer.clear();
	m_vRenderer.resize(m_ChannelCount);

	for (uint32_t j = 0; j<m_ChannelCount; j++)
	{
		m_vRenderer[j] = AdvancedVisualization::IRenderer::create(AdvancedVisualization::IRenderer::RendererType_Bitmap, false);

		m_vRenderer[j]->setChannelCount(m_SpectrumElements);
	}

	m_pRuler = new TRulerPair < TRulerConditionalPair < CRulerBottomTime, CRulerBottomCount, CRulerConditionIsTimeLocked >, TRulerPair < TRulerAutoType < IRuler, IRuler, CRulerRightFrequency >, TRulerPair < CRulerLeftChannelNames, CRulerProgressV > > >;
	m_pRuler->setRendererContext(m_pRendererContext);
	m_pRuler->setRenderer(m_vRenderer[0]);

	if(!StreamRendererBase::initialize())
	{
		return false;
	}

	m_oGtkGLWidget.initialize(*this, m_pViewport, m_pLeft, m_pRight, m_pBottom);
	m_oGtkGLWidget.setPointSmoothingActive(false);

	m_bRotate = true;

	return true;
}



bool StreamRendererSpectrum::reset(ovtime_t startTime, ovtime_t endTime)
{
	m_StartTime = startTime;
	m_EndTime = endTime;

	// Each spectrum buffer has one spectrum per channel, so numBuffers is just:
	// ( @todo: is this really needed in addition to setSampleCount? )
	const uint32_t numBuffers = static_cast<uint32_t>(ceilFixedPoint(m_EndTime - m_StartTime) / m_ChunkDuration);
	m_pRendererContext->setElementCount(numBuffers);

	for (uint32_t j = 0; j<m_ChannelCount; j++)
	{
		m_vRenderer[j]->clear(0);
		m_vRenderer[j]->setSampleCount(numBuffers); // $$$
	}

	//  @FIXME  The offset is needed to have correct numbers on the ruler; remove ifdef once the feature is in
#ifdef RENDERER_SUPPORTS_OFFSET
	m_vRenderer[0]->setTimeOffset(m_StartTime);
#endif

	return true;
}
	
#include <openvibe/ovITimeArithmetics.h>

// Spectrum chunks are organized as [channel x freqs], so to push
// with freqs on the y axis for each subrenderer, we transpose
bool StreamRendererSpectrum::push(const TypeSpectrum::Buffer& chunk, bool zeroInput /* = false */)
{
#if 0
	std::cout << "Push spec chk " << m_Pushed << " " << chunk.m_buffer.getDimensionSize(0)
		<< " " << chunk.m_buffer.getDimensionSize(1) << " "
		<< OpenViBE::ITimeArithmetics::timeToSeconds(chunk.m_startTime) << "," 
		<< OpenViBE::ITimeArithmetics::timeToSeconds(chunk.m_endTime)
		<< "\n";
	std::cout << m_Pushed << " first bytes "
		<< chunk.m_buffer.getBuffer()[0]
		<< chunk.m_buffer.getBuffer()[1]
		<< chunk.m_buffer.getBuffer()[2]
		<< chunk.m_buffer.getBuffer()[3]
		<< "\n";
	m_Pushed++;
#endif

	const uint32_t numFreq = chunk.m_buffer.getDimensionSize(1);

	m_pRendererContext->setSpectrumFrequencyRange(uint32_t((ovtime_t(numFreq) << 32) / m_ChunkDuration));

	for (uint32_t j = 0; j < m_vRenderer.size(); j++)
	{
		if (!zeroInput)
		{
			// Feed renderer with actual samples
			for (uint32_t k = 0; k < numFreq; k++)
			{
				m_vSwap[numFreq - k - 1] = float(chunk.m_buffer.getBuffer()[j*numFreq + k]);
			}
		}
		else
		{
			std::fill(m_vSwap.begin(), m_vSwap.end(), 0.0f);
		}
		m_vRenderer[j]->feed(&m_vSwap[0]);

	}

	return true;
}


bool StreamRendererSpectrum::draw(void)
{
	StreamRendererSpectrum::preDraw();

	if (m_pRendererContext->getSelectedCount() != 0)
	{
		::glPushMatrix();
		::glScalef(1, 1.f / m_pRendererContext->getSelectedCount(), 1);
		for (size_t i = 0; i<m_pRendererContext->getSelectedCount(); i++)
		{
			::glPushAttrib(GL_ALL_ATTRIB_BITS);
			::glPushMatrix();
			::glColor4f(m_oColor.r, m_oColor.g, m_oColor.b, m_pRendererContext->getTranslucency());
			::glTranslatef(0, m_pRendererContext->getSelectedCount() - i - 1.f, 0);
			if (!m_bRotate)
			{
				::glScalef(1, -1, 1);
				::glRotatef(-90, 0, 0, 1);
			}
			m_pSubRendererContext->setAspect(m_pRendererContext->getAspect());
			m_pSubRendererContext->setStackCount(m_pRendererContext->getSelectedCount());
			m_pSubRendererContext->setStackIndex(i);
			m_vRenderer[m_pRendererContext->getSelected(i)]->render(*m_pSubRendererContext);
			
			if (0)
			//if (bDrawBorders)
			{
				::glDisable(GL_TEXTURE_1D);
				::glDisable(GL_BLEND);
				::glColor3f(0, 0, 0);
				::glBegin(GL_LINE_LOOP);
				::glVertex2f(0, 0);
				::glVertex2f(1, 0);
				::glVertex2f(1, 1);
				::glVertex2f(0, 1);
				::glEnd();
			}
			::glPopMatrix();
			::glPopAttrib();
		}
		::glPopMatrix();
	}

	StreamRendererSpectrum::postDraw();

	return true;
}

bool StreamRendererSpectrum::preDraw(void)
{
	this->updateRulerVisibility();

	//	auto m_sColorGradient=OpenViBE::CString("0:0,0,0; 100:100,100,100");

	auto m_sColorGradient = OpenViBE::CString("0:100, 100, 100; 12:50, 100, 100; 25:0, 50, 100; 38:0, 0, 50; 50:0, 0, 0; 62:50, 0, 0; 75:100, 50, 0; 88:100, 100, 50; 100:100, 100, 100");

	if (!m_TextureId)
	{
		m_TextureId = m_oGtkGLWidget.createTexture(m_sColorGradient.toASCIIString());
	}
	::glBindTexture(GL_TEXTURE_1D, m_TextureId);

	m_pRendererContext->setAspect(m_pViewport->allocation.width * 1.f / m_pViewport->allocation.height);

	return true;
}

bool StreamRendererSpectrum::finalize(void)
{
	for (size_t i = 0; i < m_vRenderer.size(); i++)
	{
		m_vRenderer[i]->rebuild(*m_pSubRendererContext);
		m_vRenderer[i]->refresh(*m_pSubRendererContext);
	}
	redraw(true);

	return true;
}

OpenViBE::CString StreamRendererSpectrum::renderAsText(uint32_t indent) const
{
	auto& hdr = m_Stream->getHeader();

	std::stringstream ss;
	ss << std::string(indent, ' ') << "Sampling rate: " << hdr.m_SamplingRate << "hz" << std::endl;
	ss << std::string(indent, ' ') << "Channels: " << hdr.m_header.getDimensionSize(0) << std::endl;
	ss << std::string(indent, ' ') << "Abscissas per spectrum: " << hdr.m_Abscissas.getBufferElementCount() << std::endl;

//	ss << std::string(indent, ' ') << "Channels: " << m_Header.m_header.getDimensionSize(0) << std::endl;
//	ss << std::string(indent, ' ') << "Samples per chunk: " << m_Header.m_header.getDimensionSize(1) << std::endl;
	return OpenViBE::CString(ss.str().c_str());
}

bool StreamRendererSpectrum::mouseButton(int32_t x, int32_t y, int32_t button, int status)
{
	/*
	if (button == 3 && status == 1)
	{
		showChunkList();
	}
	*/

	return StreamRendererBase::mouseButton(x, y, button, status);
}

bool StreamRendererSpectrum::showChunkList(void)
{
	return showMatrixList<TypeSpectrum>(m_Stream, &m_StreamListWindow, "List of chunks for Spectrum stream");
}

