//
// OpenViBE Tracker
//


#pragma once

#include "StreamRendererLabel.h"
#include "TypeChannelLocalization.h"

namespace OpenViBETracker
{

/**
 * \class StreamRendererChannelLocalization 
 * \brief Renderer for Channel Localization streams
 * \author J. T. Lindgren
 *
 * @ fixme this class inherits a lot of junk from the root class which is not initialized properly and
 * hence cannot be used. but if just the interface below is used, its alright. Can't make base class
 * private as we store the renderer as a pointer to the base.
 *
 */
class StreamRendererChannelLocalization : public StreamRendererLabel {
public:
	StreamRendererChannelLocalization(const OpenViBE::Kernel::IKernelContext& ctx, std::shared_ptr<const Stream<TypeChannelLocalization>> stream) 
		: StreamRendererLabel(ctx), m_Stream(stream) { };

	virtual OpenViBE::CString renderAsText(uint32_t indent) const override;

	virtual bool showChunkList(void) override;

protected:

	std::shared_ptr< const Stream<TypeChannelLocalization> > m_Stream;

};

};
