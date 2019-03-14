//
// OpenViBE Tracker
//

#pragma once

#include "StreamRendererBase.h"

#include "TypeSpectrum.h"

namespace OpenViBETracker
{

/**
 * \class StreamRendererSpectrum 
 * \brief Renderer for Spectrum streams
 * \author J. T. Lindgren
 *
 */
class StreamRendererSpectrum : public StreamRendererBase {
public:
	StreamRendererSpectrum(const OpenViBE::Kernel::IKernelContext& ctx, std::shared_ptr< const Stream<TypeSpectrum> > stream) 
		: StreamRendererBase(ctx), m_Stream(stream) { };

	virtual bool initialize(void) override;
	virtual bool spool(ovtime_t startTime, ovtime_t endTime) override { 
		return spoolImpl<TypeSpectrum,StreamRendererSpectrum>(m_Stream, *this, startTime, endTime);
	}

	virtual OpenViBE::CString renderAsText(uint32_t indent) const override;
	virtual bool showChunkList(void) override;

protected:

	friend bool spoolImpl<TypeSpectrum, StreamRendererSpectrum>(std::shared_ptr< const Stream<TypeSpectrum> > stream, StreamRendererSpectrum& renderer, ovtime_t startTime, ovtime_t endTime);

	virtual bool finalize(void) override;
	virtual bool reset(ovtime_t startTime, ovtime_t endTime) override;
	virtual bool push(const TypeSpectrum::Buffer& chunk, bool zeroInput = false);
	virtual bool mouseButton(int32_t x, int32_t y, int32_t button, int status) override;

	virtual bool preDraw(void) override;
	virtual bool draw(void) override;

	size_t m_ChannelCount = 0;
	size_t m_SpectrumElements = 0;

	std::vector<float> m_vSwap;
	
	std::shared_ptr< const Stream<TypeSpectrum> > m_Stream;
	GtkWidget *m_StreamListWindow = nullptr;

	StreamRendererSpectrum() = delete;

};

};
