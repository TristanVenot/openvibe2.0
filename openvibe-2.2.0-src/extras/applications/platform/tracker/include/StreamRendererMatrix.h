//
// OpenViBE Tracker
//

#pragma once

#include "StreamRendererBase.h"

#include "TypeMatrix.h"

namespace OpenViBETracker
{

/**
 * \class StreamRendererMatrix 
 * \brief Renderer for Matrix streams
 * \author J. T. Lindgren
 *
 */
class StreamRendererMatrix : public StreamRendererBase {
public:
	StreamRendererMatrix(const OpenViBE::Kernel::IKernelContext& ctx, std::shared_ptr< const Stream<TypeMatrix> > stream) 
		: StreamRendererBase(ctx), m_Stream(stream) { };

	virtual bool initialize(void) override;
	virtual bool spool(ovtime_t startTime, ovtime_t endTime) override { 
		return spoolImpl<TypeMatrix,StreamRendererMatrix>(m_Stream, *this, startTime, endTime);
	}

	virtual OpenViBE::CString renderAsText(uint32_t indent) const override;
	virtual bool showChunkList(void) override;

protected:

	friend bool spoolImpl<TypeMatrix, StreamRendererMatrix>(std::shared_ptr< const Stream<TypeMatrix> > stream, StreamRendererMatrix& renderer, ovtime_t startTime, ovtime_t endTime);

	virtual bool finalize(void) override;
	virtual bool reset(ovtime_t startTime, ovtime_t endTime) override;
	virtual bool push(const TypeMatrix::Buffer& chunk, bool zeroInput = false);
	virtual bool mouseButton(int32_t x, int32_t y, int32_t button, int status) override;

	virtual bool preDraw(void) override;
	virtual bool draw(void) override;

	size_t m_NumRows = 0;
	size_t m_NumCols = 0;

	std::shared_ptr< const Stream<TypeMatrix> > m_Stream;
	GtkWidget *m_StreamListWindow = nullptr;

	std::vector<float> m_Swap;

	StreamRendererMatrix() = delete;

};

};
