//
// OpenViBE Tracker
//

#pragma once

#include "StreamRendererBase.h"

namespace OpenViBETracker
{

/**
 * \class StreamRendererNothing
 * \brief Renders nothing, to avoid null ptrs when we don't want even a label rendered
 * \author J. T. Lindgren
 *
 * @ fixme this class inherits a lot of junk from the base class which is not initialized properly and
 * hence cannot be used. but if just the interface below is used, its alright. Can't make base class
 * private as we store the renderer as a pointer to the base.
 *
 */
class StreamRendererNothing : public StreamRendererBase {
public:
	StreamRendererNothing(const OpenViBE::Kernel::IKernelContext& ctx) : StreamRendererBase(ctx) { };

	virtual bool initialize(void) override { return true; };
	virtual bool uninitialize(void) override { return true; };
	virtual bool setTitle(const char *title) override { return true; };

	virtual bool spool(ovtime_t startTime, ovtime_t endTime) override { return true; };

	virtual bool setRulerVisibility(bool isVisible) override { return true; };

	virtual bool showChunkList(const char* title) { return true; };

	OpenViBE::CString renderAsText(uint32_t indent /* = 0 */) const override
	{
		std::stringstream ss; ss << std::string(indent, ' ') << "Stream not selected and hence has no renderer in current mode\n";
		return OpenViBE::CString(ss.str().c_str());
	};

protected:

	virtual bool reset(ovtime_t startTime, ovtime_t endTime) override { return true; };
	virtual bool realize(void) override { return true; };

};

};
