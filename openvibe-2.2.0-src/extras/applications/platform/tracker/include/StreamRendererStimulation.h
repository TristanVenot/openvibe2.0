//
// OpenViBE Tracker
//

#pragma once

#include "StreamRendererBase.h"
#include "TypeStimulation.h"

namespace OpenViBETracker
{

/**
 * \class StreamRendererStimulation 
 * \brief Renderer for Stimulation streams
 * \author J. T. Lindgren
 *
 */
class StreamRendererStimulation : public StreamRendererBase {
public:
	StreamRendererStimulation(const OpenViBE::Kernel::IKernelContext& ctx, std::shared_ptr< const Stream<TypeStimulation> > stream) 
		: StreamRendererBase(ctx), m_Stream(stream) { };

	virtual bool initialize(void) override;
	virtual bool spool(ovtime_t startTime, ovtime_t endTime) override {
		return spoolImpl< TypeStimulation, StreamRendererStimulation >(m_Stream, *this, startTime, endTime);
	}
	virtual OpenViBE::CString renderAsText(uint32_t indent) const override;
	virtual bool showChunkList(void) override;

protected:

	friend bool spoolImpl<TypeStimulation, StreamRendererStimulation>(std::shared_ptr< const Stream<TypeStimulation> > stream, StreamRendererStimulation& renderer, ovtime_t startTime, ovtime_t endTime);

	virtual bool push(const TypeStimulation::Buffer& chunk, bool zeroInput = false);

	virtual bool reset(ovtime_t startTime, ovtime_t endTime) override;

	virtual bool mouseButton(int32_t x, int32_t y, int32_t button, int status) override;

	size_t m_ChannelCount = 0;
	size_t m_SamplesPerChunk = 0;
	uint32_t m_SamplingRate = 0;

	GtkWidget* m_StimulationListWindow = nullptr;

	std::shared_ptr< const Stream<TypeStimulation> > m_Stream;

	StreamRendererStimulation() = delete;
};

};
