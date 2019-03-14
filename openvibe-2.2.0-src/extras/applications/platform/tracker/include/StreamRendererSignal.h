//
// OpenViBE Tracker
//

#pragma once

#include "StreamRendererBase.h"
#include "TypeSignal.h"

namespace OpenViBETracker
{

/**
 * \class StreamRendererSignal 
 * \brief Renderer for Signal streams
 * \author J. T. Lindgren
 *
 */
class StreamRendererSignal : public StreamRendererBase {
public:
	StreamRendererSignal(const OpenViBE::Kernel::IKernelContext& ctx, std::shared_ptr< const Stream<TypeSignal> > stream) 
		: StreamRendererBase(ctx), m_Stream(stream) { };

	virtual bool initialize(void) override;
	virtual bool spool(ovtime_t startTime, ovtime_t endTime) override {
		return spoolImpl< TypeSignal, StreamRendererSignal >(m_Stream, *this, startTime, endTime);
	}

	virtual OpenViBE::CString renderAsText(uint32_t indent) const override;
	virtual bool showChunkList(void) override;

protected:

	friend bool spoolImpl<TypeSignal, StreamRendererSignal>(std::shared_ptr< const Stream<TypeSignal> > stream, StreamRendererSignal& renderer, ovtime_t startTime, ovtime_t endTime);

	virtual bool push(const TypeSignal::Buffer& chunk, bool zeroInput = false);
	virtual bool reset(ovtime_t startTime, ovtime_t endTime) override;
	virtual bool mouseButton(int32_t x, int32_t y, int32_t button, int status) override;

	size_t m_ChannelCount = 0;
	size_t m_SamplesPerChunk = 0;

	std::shared_ptr< const Stream<TypeSignal> > m_Stream;
	GtkWidget* m_StreamListWindow = nullptr;

	StreamRendererSignal() = delete;
};

};
